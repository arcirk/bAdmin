//
// Created by arcady on 10.11.2021.
//

#include "sortmodel.h"
#include "treeviewmodel.h"

SortModel::SortModel(QObject *parent)
: QSortFilterProxyModel(parent)
{
    m_sortRole = Qt::DisplayRole;
}

int SortModel::sortRole()
{
    return m_sortRole;
}

void SortModel::setSortRole(int role)
{
    m_sortRole = role;
    this->setDynamicSortFilter(true);
}

bool SortModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const {

    auto model = (TreeViewModel*)sourceModel();

    if (m_filter.size() > 0){
        QMapIterator<QString, QVariant> i(m_filter);
        while (i.hasNext()){
            i.next();
            int col = model->get_column_index(i.key());
            if(col == -1)
                continue;
            QModelIndex index = model->index(sourceRow, col, sourceParent);

            //if (i.value().typeId() == QMetaType::QString){
            if (i.value().userType() == QMetaType::QString){
                QString value = model->data(index, m_sortRole).toString();
                QString filter = i.value().toString();
                if(filter.left(1) != "!"){
                    if (filter != value)
                        return false;
                }else{
                    filter = filter.right(filter.length() - 1);
                    if (filter == value)
                        return false;
                }
            }
            else if (i.value().userType() == QMetaType::Int){
                if (i.value().toInt() != model->data(index, m_sortRole).toInt())
                    return false;
            }
            else if (i.value().userType() == QMetaType::Bool){
                if (i.value().toBool() != model->data(index, m_sortRole).toBool())
                    return false;
            }
            else if (i.value().userType() == QMetaType::Double){
                if (i.value().toDouble() != model->data(index, m_sortRole).toDouble())
                    return false;
            }
            else if (i.value().userType() == QMetaType::LongLong){
                if (i.value().toLongLong() != model->data(index, m_sortRole).toLongLong())
                    return false;
            }
            //qDebug() << i.value().typeId() << ":" << i.value().typeName();
        }

    }

    return true;
}

QString SortModel::filter() {
    return m_FilterText;
}

void SortModel::setFilter(const QString &filterText) {

    m_FilterText = filterText;
    set_filter();
}

void SortModel::set_filter() {

    using json = nlohmann::json;

    if(sourceModel()){
        beginResetModel();
    }
    m_filter.clear();
    auto obj = json::parse(m_FilterText.toStdString());
    if (obj.empty())
        return;

    for (auto itr = obj.items().begin(); itr != obj.items().end(); ++itr) {
        QString key = itr.key().c_str();
        auto val = itr.value();
        QVariant value;
        if(val.is_string())
            value = val.get<std::string>().c_str();
        else if(val.is_boolean())
            value = val.get<bool>();
        else if(val.is_number_float())
            value = val.get<double>();
        else if(val.is_number_integer())
            value = val.get<int>();
        else if(val.is_number_unsigned())
            value = val.get<int>();

        m_filter.insert(key, value);

    }
    if(sourceModel()){
        endResetModel();
    }
}
