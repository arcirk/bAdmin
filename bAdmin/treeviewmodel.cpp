#include "treeviewmodel.h"
#include "query_builder.hpp"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QEventLoop>

QString TreeViewModel::cryptPass(const QString &source, const QString &key)
{
    std::string result = arcirk::crypt(source.toStdString(), key.toStdString());

    return QString::fromStdString(result);
}

nlohmann::json TreeViewModel::http_data(const QString &parentUuid) const{

    std::string uuid_form_ = arcirk::uuids::nil_string_uuid();

    nlohmann::json param = {
            {"table", true},
            {"uuid_form", uuid_form_},
            {"empty_column", false},
            {"recursive", false},
            {"parent", parentUuid.toStdString()}
    };

    auto http_param = arcirk::synchronize::http_param();
    http_param.command = arcirk::enum_synonym(arcirk::server::server_commands::ProfileDirFileList);
    http_param.param = QByteArray(param.dump().data()).toBase64().toStdString();

    QEventLoop loop;
    int httpStatus = 200;
    QByteArray httpData;
    QNetworkAccessManager httpService;

    auto finished = [&loop, &httpStatus, &httpData](QNetworkReply* reply) -> void
    {
       QVariant status_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
       if(status_code.isValid()){
           httpStatus = status_code.toInt();
           if(httpStatus != 200){
                qCritical() << QDateTime::currentDateTime().toString("hh:mm:ss") << __FUNCTION__ << "Error: " << httpStatus << " " + reply->errorString() ;
           }else
           {
               httpData = reply->readAll();
           }
       }
       loop.quit();

    };

    loop.connect(&httpService, &QNetworkAccessManager::finished, finished);

    QUrl ws(conf_.server_host.data());
    QString protocol = ws.scheme() == "wss" ? "https://" : "http://";
    QString http_query = "/api/info";
    QUrl url(protocol + ws.host() + ":" + QString::number(ws.port()) + http_query);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QString headerData = "Token " + authString_;
    request.setRawHeader("Authorization", headerData.toLocal8Bit());

    httpService.post(request, QByteArray::fromStdString(pre::json::to_json(http_param).dump()));

    loop.exec();

    if(httpStatus != 200){
        return {};
    }

    if(httpData.isEmpty())
        return {};

    if(httpData == "error"){
        return {};
    }

    auto msg = pre::json::from_json<arcirk::server::server_response>(httpData.toStdString());

    if(msg.result.empty())
        return {};

    auto http_result = nlohmann::json::parse(QByteArray::fromBase64(msg.result.data()).toStdString());

    return http_result;
}

nlohmann::json TreeViewModel::getNodeData(const QString &parentUuid) const
{
    if(table_name_.isEmpty())
        return {};

    using namespace arcirk::database;
    auto query = builder::query_builder();

    auto cols = nlohmann::json::array();
    foreach (auto &itr, columns) {
        cols += itr.toStdString();
    }
    auto where = nlohmann::json::object({
                                           {"parent", parentUuid.toStdString()}
                                        });

    if(group_only_){
        where["is_group"] = 1;
    }
    std::string query_text = query.select(cols).from(table_name_.toStdString()).where(where, true).order_by(nlohmann::json{{"is_group", 1}}).prepare();//,{"first", 0}

    auto param = nlohmann::json::object();
    param["query_text"] = query_text;

    auto http_param = arcirk::synchronize::http_param();
    http_param.command = arcirk::enum_synonym(arcirk::server::server_commands::ExecuteSqlQuery);
    http_param.param = QByteArray(param.dump().data()).toBase64().toStdString();

    QEventLoop loop;
    int httpStatus = 200;
    QByteArray httpData;
    QNetworkAccessManager httpService;

    auto finished = [&loop, &httpStatus, &httpData](QNetworkReply* reply) -> void
    {
       QVariant status_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
       if(status_code.isValid()){
           httpStatus = status_code.toInt();
           if(httpStatus != 200){
                qCritical() << QDateTime::currentDateTime().toString("hh:mm:ss") << __FUNCTION__ << "Error: " << httpStatus << " " + reply->errorString() ;
           }else
           {
               httpData = reply->readAll();
           }
       }
       loop.quit();

    };

    loop.connect(&httpService, &QNetworkAccessManager::finished, finished);

    QUrl ws(conf_.server_host.data());
    QString protocol = ws.scheme() == "wss" ? "https://" : "http://";
    QString http_query = "/api/info";
    QUrl url(protocol + ws.host() + ":" + QString::number(ws.port()) + http_query);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QString headerData = "Token " + authString_;
    request.setRawHeader("Authorization", headerData.toLocal8Bit());

    httpService.post(request, QByteArray::fromStdString(pre::json::to_json(http_param).dump()));
    loop.exec();

    if(httpStatus != 200){
        return {};
    }

    if(httpData.isEmpty())
        return {};

    if(httpData == "error"){
        return {};
    }

    auto msg = pre::json::from_json<arcirk::server::server_response>(httpData.toStdString());

    if(msg.result.empty())
        return {};

    auto http_result = nlohmann::json::parse(QByteArray::fromBase64(msg.result.data()).toStdString());

    return http_result;
}

QVariant TreeViewModel::get_value(const nlohmann::json &node, int col) const
{
    auto val = node[columns[0].toStdString()];
    if(val.is_string())
        return QString::fromStdString(val.get<std::string>());
    else if(val.is_number_float())
        return val.get<double>();
    else if(val.is_number_integer())
        return val.get<int>();
    else if(val.is_boolean())
        return val.get<bool>();
    else
        return QVariant();
}

TreeViewModel::TreeViewModel(const arcirk::client::client_conf& conf, QObject *parent)
    : QAbstractItemModel{parent},
      conf_(conf)
{
    QByteArray data = QByteArray(conf.hash.data()).toBase64();
    authString_ = data;
    item_icons.insert(item_icons_enum::ItemGroup, QIcon(":/img/group.png"));
    item_icons.insert(item_icons_enum::Item, QIcon(":/img/item.png"));
    item_icons.insert(item_icons_enum::DeletedItemGroup, QIcon(":/img/groupDeleted.png"));
    item_icons.insert(item_icons_enum::DeletedItem, QIcon(":/img/deletionMarkItem.png"));

    is_loaded_ = false;
    group_only_ = false;

    column_aliases = {};
    server_object_ = arcirk::server::server_objects::OBJ_INVALID;
}

TreeViewModel::TreeViewModel(QObject *parent)
    : QAbstractItemModel{parent}
{
    conf_ = arcirk::client::client_conf();
    item_icons.insert(item_icons_enum::ItemGroup, QIcon(":/img/group.png"));
    item_icons.insert(item_icons_enum::Item, QIcon(":/img/item.png"));
    item_icons.insert(item_icons_enum::DeletedItemGroup, QIcon(":/img/groupDeleted.png"));
    item_icons.insert(item_icons_enum::DeletedItem, QIcon(":/img/deletionMarkItem.png"));

    is_loaded_ = false;
    group_only_ = false;

    column_aliases = {};
    server_object_ = arcirk::server::server_objects::OBJ_INVALID;
}

struct TreeViewModel::NodeInfo
{
    NodeInfo():
        parent(0),
        mapped(false)
    {}

    NodeInfo(const nlohmann::json& rowData, NodeInfo* parent = 0):
        rowData(rowData),
        parent(parent),
        mapped(false)
    {}

    bool operator ==(const NodeInfo& another) const
    {
        bool r = this->rowData == another.rowData;
        Q_ASSERT(!r || this->parent == another.parent);
        Q_ASSERT(!r || this->mapped == another.mapped);
        Q_ASSERT(!r || this->children == another.children);
        return r;
    }

    nlohmann::json rowData;
    QVector<NodeInfo> children;
    NodeInfo* parent;

    bool mapped;
};

TreeViewModel::~TreeViewModel()
{}

QModelIndex TreeViewModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent)) {
        return QModelIndex();
    }

    if (!parent.isValid()) {
        Q_ASSERT(_nodes.size() > row);
        return createIndex(row, column, const_cast<NodeInfo*>(&_nodes[row]));
    }

    NodeInfo* parentInfo = static_cast<NodeInfo*>(parent.internalPointer());
    Q_ASSERT(parentInfo != 0);
    Q_ASSERT(parentInfo->mapped);
    Q_ASSERT(parentInfo->children.size() > row);
    return createIndex(row, column, &parentInfo->children[row]);
}

QModelIndex TreeViewModel::parent(const QModelIndex &child) const
{
    if (!child.isValid()) {
        return QModelIndex();
    }

    NodeInfo* childInfo = static_cast<NodeInfo*>(child.internalPointer());
    Q_ASSERT(childInfo != 0);
    NodeInfo* parentInfo = childInfo->parent;
    if (parentInfo != 0) {
        return createIndex(findRow(parentInfo), 0, parentInfo);
    }
    else {
        return QModelIndex();
    }
}

QHash<int, QByteArray> TreeViewModel::roleNames() const
{
    QHash<int, QByteArray> names;

    if (columns.isEmpty())
        return names;

    for (int i = 0; i < columns.size(); ++i) {
        const QString& key = columns[i];
        int index = Qt::UserRole + i;
        names[index] = key.toUtf8();
    }

    return names;
}

int TreeViewModel::findRow(const NodeInfo *nodeInfo) const
{
    Q_ASSERT(nodeInfo != 0);      
    const NodeInfoList& parentInfoChildren = nodeInfo->parent != 0 ? nodeInfo->parent->children: _nodes;
    NodeInfoList::const_iterator position = std::find(parentInfoChildren.constBegin(), parentInfoChildren.end(),  *nodeInfo);// qFind(parentInfoChildren, *nodeInfo);
    Q_ASSERT(position != parentInfoChildren.end());
    return std::distance(parentInfoChildren.begin(), position);
}

int TreeViewModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        return _nodes.size();
    }
    const NodeInfo* parentInfo = static_cast<const NodeInfo*>(parent.internalPointer());
    Q_ASSERT(parentInfo != 0);

    return parentInfo->children.size();
}

bool TreeViewModel::hasChildren(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        const NodeInfo* parentInfo = static_cast<const NodeInfo*>(parent.internalPointer());
        Q_ASSERT(parentInfo != 0);
        if (!parentInfo->mapped) {
            return true;//QDir(parentInfo->fileInfo.absoluteFilePath()).count() > 0;
        }
    }
    return QAbstractItemModel::hasChildren(parent);
}

int TreeViewModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return columns.size();
}

bool TreeViewModel::field_is_exists(const nlohmann::json &object, const std::string &name) const {
    auto itr = object.find(name);
    return itr != object.end();
}

QVariant TreeViewModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    const NodeInfo* nodeInfo = static_cast<NodeInfo*>(index.internalPointer());
    const nlohmann::json& rowData = nodeInfo->rowData;
    Q_ASSERT(nodeInfo != 0);

    if(index.column() < columns.size()){
        if(index.column() == 0)
            return firstData(rowData, role, index);
        else{
            if (role == Qt::DisplayRole) {
                if(!field_is_exists(rowData, columns[index.column()].toStdString()))
                    return QVariant();
                auto val = rowData[columns[index.column()].toStdString()];
                if(val.is_string())
                    return QString::fromStdString(val.get<std::string>());
                else if(val.is_number_integer())
                    if(columns[index.column()] != "size")
                        return val.get<int>();
                    else{
                        if(table_name_ != "ProfileDirectory")
                            return val.get<int>();
                        else{
                            auto val_i = val.get<int>();
                            if(val_i > 1000)
                                return val.get<int>() / 1000;
                            else{
                                if(val.get<int>() > 0){
                                    double value = (double)val.get<int>() / (double)1000;
                                    double exp = 100.0;
                                    double result = std::trunc(value * exp) / exp;
                                    return result;
                                }else
                                    return QVariant();
                            }
                        }
                    }
                else if(val.is_number_float())
                    return val.get<double>();
            }
        }

    }
//    switch (index.column()) {
////    case 0:
////        return rowData["-id"]; ////nameData(fileInfo, role);
////    case 1:
////        if (role == Qt::DisplayRole) {
////            //return fileInfo.lastModified();
////        }
////        break;
////    case 2:
////        if (role == Qt::DisplayRole) {
////            //return fileInfo.isDir()? QVariant(): fileInfo.size();
////        }
////        break;
////    case 3:
////        if (role == Qt::DisplayRole) {
////            //return _metaProvider->type(fileInfo);
////        }
////        break;
//    default:
//        break;
//    }
    return QVariant();
}

bool TreeViewModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid()) {
        return false;
    }
//    if (role != Qt::EditRole) {
//        return false;
//    }
//	if (index.column() != NameColumn) {
//		return false;
//	}
    auto column_name = get_column_name(role);
    nlohmann::json val;
    if(value.typeId() == QMetaType::QString){
        val = value.toString().toStdString();
    }else if(value.typeId() == QMetaType::Int){
        val = value.toInt();
    }else if(value.typeId() == QMetaType::Double){
        val = value.toDouble();
    }else if(value.typeId() == QMetaType::Float){
        val = value.toFloat();
    }else if(value.typeId() == QMetaType::Bool){
        val = value.toBool();
    }else
        val = "";
    NodeInfo* nodeInfo = static_cast<NodeInfo*>(index.internalPointer());
    Q_ASSERT(nodeInfo != 0);
//    auto object = nodeInfo->rowData;
//    nodeInfo->rowData = object;
    emit dataChanged(index, index.sibling(index.row(), columns.size()));
    try {
        nodeInfo->rowData[column_name.toStdString()] = val;
    } catch (...) {
        return false;
    }

//	QString newName = value.toString();
//	if (newName.contains('/') || newName.contains(QDir::separator())) {
//		return false;
//	}
//	NodeInfo* nodeInfo = static_cast<NodeInfo*>(index.internalPointer());
//	QString fullNewName = nodeInfo->fileInfo.absoluteDir().path() +"/" + newName;
//	QString fullOldName = nodeInfo->fileInfo.absoluteFilePath();
//	qDebug() << fullOldName << fullNewName;
//	bool renamed = QFile::rename(fullOldName, fullNewName);
//	qDebug() << renamed;
//	if (renamed) {
//        nodeInfo->rowData. = QFileInfo(fullNewName);
//		emit dataChanged(index, index.sibling(index.row(), ColumnCount));
//	}
//	return renamed;

    return true;
}

QVariant TreeViewModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole && section < columns.size()) {
        auto name = QString::fromStdString(columns[section].toStdString());
        auto itr = column_aliases.find(name);
        if(itr != column_aliases.end())
            return itr.value();
        else
            return name;
    }
    return QVariant();
}

bool TreeViewModel::canFetchMore(const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        return false;
    }

    const NodeInfo* parentInfo = static_cast<const NodeInfo*>(parent.internalPointer());
    Q_ASSERT(parentInfo != 0);
    return !parentInfo->mapped;
}

void TreeViewModel::fetchMore(const QModelIndex &parent)
{

    Q_ASSERT(parent.isValid());
    NodeInfo* parentInfo = static_cast<NodeInfo*>(parent.internalPointer());
    Q_ASSERT(parentInfo != 0);
    Q_ASSERT(!parentInfo->mapped);

    const nlohmann::json& row = parentInfo->rowData;
    int is_group = row.value("is_group", 0);

    Q_ASSERT(is_group == 1);

    if(table_name_.isEmpty())
        return;

//    auto http_result = getNodeData(row["ref"].get<std::string>().c_str());

    nlohmann::json http_result{};
    if(table_name_ != "ProfileDirectory"){
        http_result = getNodeData(row["ref"].get<std::string>().c_str());
        set_current_parent_path(row["ref"].get<std::string>().c_str());
    }else{
        http_result = http_data(row["path"].get<std::string>().c_str());
        set_current_parent_path(row["path"].get<std::string>().c_str());
    }

    if(http_result.is_object()){

        auto children = http_result["rows"];

        if(children.is_array()){
            int insrtCnt = (int)children.size() - 1;
            if (insrtCnt < 0) {
                insrtCnt = 0;
            }
            beginInsertRows(parent, 0, insrtCnt);
            parentInfo->children.reserve(children.size());
            for(auto entry = children.begin(); entry != children.end(); ++entry) {
                nlohmann::json e = *entry;
                NodeInfo nodeInfo(e, parentInfo);
                nodeInfo.mapped = e["is_group"] != 1;
                parentInfo->children.push_back(std::move(nodeInfo));
            }
            parentInfo->mapped = true;
            endInsertRows();
        }
    }

    emit fetch();
}

void TreeViewModel::set_table(const nlohmann::json& tableModel){

    beginResetModel();
    clear();
    columns.clear();

    //Q_ASSERT(tableModel.is_object());
    if(!tableModel.is_object()){
        return;
        endResetModel();
    }


    auto cols = tableModel.value("columns", nlohmann::json::array());
    for (auto itr = cols.begin(); itr != cols.end(); ++itr) {
        columns.push_back(QString::fromStdString(*itr));
    }
    auto rows = tableModel.value("rows", nlohmann::json::array());
    for(auto entry = rows.begin(); entry != rows.end(); ++entry) {
        nlohmann::json e = *entry;
        NodeInfo nodeInfo(e);
        nodeInfo.mapped = true;// e["is_group"] != 1;
        _nodes.push_back(std::move(nodeInfo));
    }
    endResetModel();
    is_loaded_ = true;
}

QString TreeViewModel::current_parent_path() const
{
    return current_parent_path_;
}

void TreeViewModel::refresh(const QModelIndex& parent)
{
    Q_UNUSED(parent);
//    NodeInfo* parentInfo = static_cast<NodeInfo*>(parent.internalPointer());
//    parentInfo->children.erase(parentInfo->children.cbegin(), parentInfo->children.cend());
//    reset();
////    if (parent.model()->hasChildren())
////        parent.model()->removeRows(0,parent.model()->rowCount());
//    //    fetchMore(parent);
}

void TreeViewModel::remove(const QModelIndex &index)
{
    if(!index.isValid())
        return;

    NodeInfo* itemInfo = static_cast<NodeInfo*>(index.internalPointer());
    NodeInfo* parentInfo = itemInfo->parent;
    auto pos = findRow(itemInfo);

    if (parentInfo != 0) {
        beginRemoveRows(index.parent(), pos, pos);
        parentInfo->children.remove(pos);
        endRemoveRows();
    }else{
        beginRemoveRows(index.parent(), pos, pos);
        _nodes.remove(pos);
        endRemoveRows();
    }
}

void TreeViewModel::add(const nlohmann::json object, const QModelIndex &parent)
{
    NodeInfo* parentInfo = static_cast<NodeInfo*>(parent.internalPointer());
    if(parentInfo == 0){
        if(columns.size() == 0){
            bool is_valid_hierarchy = false;
            for (auto itr = object.items().begin(); itr != object.items().end(); ++itr) {
                std::string name = itr.key();
                if(!use_hierarchy_.empty() && use_hierarchy_ == name){
                    is_valid_hierarchy = true;
                    continue;
                }
                columns.push_back(QString::fromStdString(name));
            }
            if(is_valid_hierarchy){
                columns.insert(0, QString::fromStdString(use_hierarchy_));
            }
        }

        NodeInfo nodeInfo(object);
        int is_group = object.value("is_group", 0);
        nodeInfo.mapped = is_group != 1;// e["is_group"] != 1;
        _nodes.push_back(std::move(nodeInfo));
        reset();
    }else{
        int insrtCnt = parentInfo->children.size() - 1;
        if (insrtCnt < 0) {
            insrtCnt = 0;
        }
        beginInsertRows(parent, 0, insrtCnt);
        NodeInfo nodeInfo(object, parentInfo);
        nodeInfo.mapped = object["is_group"] != 1;
        parentInfo->children.push_back(std::move(nodeInfo));
        endInsertRows();
    }
}

void TreeViewModel::set_object(const QModelIndex &index, const nlohmann::json &object)
{

    if(!index.isValid())
        return;
    NodeInfo* nodeInfo = static_cast<NodeInfo*>(index.internalPointer());
    Q_ASSERT(nodeInfo != 0);
    nodeInfo->rowData = object;
    emit dataChanged(index, index.sibling(index.row(), columns.size()));
}

void TreeViewModel::use_hierarchy(const std::string &column)
{
    use_hierarchy_ = column;
}

QModelIndex TreeViewModel::find_in_table(QAbstractItemModel *model, const QString &value, int column, int role, bool findData)
{
    int rows =  model->rowCount();
    for (int i = 0; i < rows; ++i) {
        auto index = model->index(i, column);
        if(findData){
            if(value == index.data(role).toString())
                return index;
        }else{
            QString data = index.data().toString();
            if(value == data)
                return index;
        }
    }

    return QModelIndex();
}

QModelIndex TreeViewModel::find(const QString& find_value, int col, const QModelIndex& parent){
    QModelIndexList matches = match(
                index(0,col, parent),
                Qt::DisplayRole,
                QVariant(find_value),
                -1,
                Qt::MatchRecursive);
    foreach (const QModelIndex &match, matches){
        return match;
    }
    return QModelIndex();
}

void TreeViewModel::move_up(const QModelIndex &index)
{
    Q_ASSERT(index.isValid());
    if(index.row() < 1)
        return;

    beginMoveRows(index.parent(), index.row(), index.row(), index.parent(), index.row()-1);
    NodeInfo* itemInfo = static_cast<NodeInfo*>(index.internalPointer());
    Q_ASSERT(itemInfo != 0);
    NodeInfo* parentInfo = itemInfo->parent;
    auto pos = findRow(itemInfo);

    if(parentInfo != 0)
        parentInfo->children.move(pos, pos - 1);
    else
        _nodes.move(pos, pos - 1);

    endMoveRows();
}

void TreeViewModel::move_down(const QModelIndex &index)
{
    Q_ASSERT(index.isValid());
    if(index.row() > rowCount(index.parent()) - 2)
        return;


    NodeInfo* itemInfo = static_cast<NodeInfo*>(index.internalPointer());
    Q_ASSERT(itemInfo != 0);
    NodeInfo* parentInfo = itemInfo->parent;
    auto pos = findRow(itemInfo);

    if(parentInfo != 0){
        beginMoveRows(index.parent(), pos, pos, index.parent(), (pos >= parentInfo->children.size()) ? parentInfo->children.size() : (pos + 2));
        parentInfo->children.move(pos, pos + 1);
    }else{
        beginMoveRows(index.parent(), pos, pos, index.parent(), (pos >= _nodes.size()) ? _nodes.size() : (pos + 2));
        _nodes.move(pos, pos + 1);
    }

    endMoveRows();
}

void TreeViewModel::set_group_only(bool value)
{
    group_only_ = value;
}

bool TreeViewModel::group_only() const
{
    return group_only_;
}

void TreeViewModel::set_current_parent_path(const QString &value)
{
    current_parent_path_ = value;
}

void TreeViewModel::fetchRoot(const QString& table_name, const QString& root_dir)
{
    if(table_name == table_name_ && is_loaded_)
        return;

    clear();

    table_name_ = table_name;
    current_parent_path_ = root_dir;

    nlohmann::json http_result{};
    if(table_name_ != "ProfileDirectory"){
        http_result = getNodeData("00000000-0000-0000-0000-000000000000");
    }else{
        http_result = http_data(root_dir);
    }
    if(http_result.is_object()){
        columns.clear();
        auto cols = http_result["columns"];
        if(cols.is_array()){
            bool is_valid_hierarchy = false;
            for (auto itr = cols.begin(); itr != cols.end(); ++itr) {
                std::string name = *itr;
                if(!use_hierarchy_.empty() && use_hierarchy_ == name){
                    is_valid_hierarchy = true;
                    continue;
                }
                columns.push_back(QString::fromStdString(name));
            }
            if(is_valid_hierarchy){
                columns.insert(0, QString::fromStdString(use_hierarchy_));
            }
        }
        beginResetModel();
        auto rows = http_result["rows"];
        if(rows.is_array()){
            //std::copy(rows.begin(), rows.end(), std::back_inserter(_nodes));
            for(auto entry = rows.begin(); entry != rows.end(); ++entry) {
                nlohmann::json e = *entry;
                NodeInfo nodeInfo(e);
                int is_group = e.value("is_group", 0);
                nodeInfo.mapped = is_group != 1;// e["is_group"] != 1;
                _nodes.push_back(std::move(nodeInfo));
            }
        }
        endResetModel();
        is_loaded_ = true;
    }
}

QVariant TreeViewModel::firstData(const nlohmann::json &node, int role, const QModelIndex& index) const
{
    switch (role) {
    case Qt::EditRole:{
////        auto  first = node.value("first", "");
////        return QString::fromStdString(first);
//        //return QString::fromStdString(node["first"].get<std::string>());
//        auto roles = roleNames();
//        if(!roles[role].isEmpty())
//            return QString::fromStdString(node[roles[role].toStdString()].get<std::string>());
//        else
//             return QVariant();

        return get_value(node);
    }case Qt::DisplayRole:{
        return get_value(node);
//        //auto  first = node.value(0, "");
////        if(node.find("first") != node.end())
////            return QString::fromStdString(node["first"].get<std::string>());
////        else{
//            auto val = node[columns[0].toStdString()];
//            if(val.is_string())
//                return QString::fromStdString(val.get<std::string>());
//            else if(val.is_number_float())
//                return val.get<double>();
//            else if(val.is_number_integer())
//                return val.get<int>();
//            else if(val.is_boolean())
//                return val.get<bool>();
//            else
//                return QVariant();
////        }
////            return QVariant();
///*        auto roles = roleNames();
//        if(!roles[role].isEmpty())
//            return QString::fromStdString(node[roles[role].toStdString()].get<std::string>());
//        else
//             return QVariant();*/
    }case Qt::DecorationRole:{
        auto is_group = node.value("is_group", 0);
        auto deletion_mark = node.value("deletion_mark", 0);
        auto p = qMakePair(index.row(), 0);
        auto itr = row_icons.find(p);
        if(itr != row_icons.end())
            return itr.value();
        else{
            if(rows_icon.isNull()){
                if(is_group == 0){
                    if(deletion_mark == 0)
                        return item_icons[item_icons_enum::Item];
                    else{
                        return item_icons[item_icons_enum::DeletedItem];
                    }
                }else{
                    if(deletion_mark == 0)
                        return item_icons[item_icons_enum::ItemGroup];
                    else
                        return item_icons[item_icons_enum::DeletedItemGroup];
                }
            }else
                return rows_icon;
        }
    }default:
        return QVariant();
    }
    Q_UNREACHABLE();
}

bool TreeViewModel::is_loaded()
{
    return is_loaded_;
}

void TreeViewModel::set_column_aliases(const QMap<QString, QString> values)
{
    column_aliases = values;
}

int TreeViewModel::get_column_index(const QString &name)
{
    auto names = roleNames();
    if (names.size() == 0)
        return -1;

    foreach (const auto& key , names.keys()) {

        if (names[key] == name.toUtf8()){
            return key - Qt::UserRole;
        }
    }

    return -1;
}

QString TreeViewModel::get_column_name(int column) const
{

    if(column < columns.size())
        return columns[column];

    return {};
}

void TreeViewModel::columns_establish_order(const QVector<QString> &names)
{
    QVector<QString> result;
    std::copy(names.begin(), names.end(), std::back_inserter(result));

    for (auto itr = columns.constBegin(); itr != columns.constEnd(); ++itr) {
        if(result.indexOf(*itr) == -1){
            result.push_back(*itr);
        }
    }

    columns.clear();
    std::copy(result.begin(), result.end(), std::back_inserter(columns));
}

void TreeViewModel::set_columns(const QVector<QString> cols)
{
    columns = cols;
}

void TreeViewModel::set_server_object(arcirk::server::server_objects obj)
{
    server_object_ = obj;
}

arcirk::server::server_objects TreeViewModel::server_object() const
{
    return server_object_;
}

nlohmann::json TreeViewModel::get_object(const QModelIndex &index) const
{
    const NodeInfo* nodeInfo = static_cast<NodeInfo*>(index.internalPointer());
    const nlohmann::json& rowData = nodeInfo->rowData;
    Q_ASSERT(nodeInfo != 0);
    return rowData;

}

nlohmann::json TreeViewModel::get_objects(const QModelIndex &parent) const
{
    NodeInfo* parentInfo = static_cast<NodeInfo*>(parent.internalPointer());

    using json = nlohmann::json;

    json result = json::array();

    if(parentInfo == 0){
        for (auto itr = _nodes.begin(); itr != _nodes.end(); ++itr) {
            result += itr->rowData;
        }
    }else{
        for (auto itr = parentInfo->children.begin(); itr != parentInfo->children.end(); ++itr) {
            result += itr->rowData;
        }
    }

    return result;

}

nlohmann::json TreeViewModel::get_table_model(const QModelIndex &parent) const
{
    using json = nlohmann::json;
    auto roles = roleNames();
    auto columns_j = json::array();
    foreach (const auto& key , roles.keys()) {
        columns_j += roles[key].toStdString();
    }

    auto rows = get_objects(parent);

    return json::object({
        {"columns" , columns_j},
        {"rows", rows}
    });
}

void TreeViewModel::clear()
{
    //rows_icon = QIcon();
    row_icons.clear();
    //columns.clear();
    _nodes.clear();
    is_loaded_ = false;
    table_name_ = "";
}

void TreeViewModel::reset()
{
    //qDebug() << "start reset" << QTime::currentTime();
    beginResetModel();
    //qDebug() << "end reset" << QTime::currentTime();
    endResetModel();
}

Qt::ItemFlags TreeViewModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags = QAbstractItemModel::flags(index);
//	if (index.isValid() && index.column() == NameColumn) {
//		const NodeInfo* nodeInfo = static_cast<const NodeInfo*>(index.internalPointer());
//		if (!nodeInfo->fileInfo.isRoot()) {
//			flags |= Qt::ItemIsEditable;
//		}
//	}
    return flags;
}

void TreeViewModel::set_rows_icon(const QIcon &ico)
{
    rows_icon = ico;
}

void TreeViewModel::set_icon(const QModelIndex &index, const QIcon &ico)
{
    row_icons.insert(qMakePair(index.row(), index.column()), ico);
}
