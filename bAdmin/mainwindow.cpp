#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QStandardPaths>
//#include "tabledelegate.h"
#include <QMessageBox>
#include "query_builder.hpp"
#include "dialoguser.h"
#include "connectiondialog.h"
#include <QVector>
#include <dialogprofilefolder.h>
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_client = new WebSocketClient(this);
    connect(m_client, &WebSocketClient::connectionChanged, this, &MainWindow::connectionChanged);
    connect(m_client, &WebSocketClient::connectionSuccess, this, &MainWindow::connectionSuccess);
    connect(m_client, &WebSocketClient::displayError, this, &MainWindow::displayError);
    connect(m_client, &WebSocketClient::serverResponse, this, &MainWindow::serverResponse);

    if(!m_client->conf().is_auto_connect)
        openConnectionDialog();
    else
        reconnect();

//    ui->tableView->setItemDelegate(new TableDelegate);
//    ui->tableView->setIconSize(QSize(16,16));

    createColumnAliases();

    formControl();

    fillDefaultTree();

    createModels();

    setWindowTitle("arcirk");
}

MainWindow::~MainWindow()
{
    if(m_client->isConnected())
        m_client->close();
    delete ui;
}

bool MainWindow::openConnectionDialog()
{
    auto dlg = ConnectionDialog(m_client->conf(), this);
    dlg.setConnectionState(m_client->isConnected());
    connect(this, &MainWindow::setConnectionChanged, &dlg, &ConnectionDialog::connectionChanged);
    connect(&dlg, &ConnectionDialog::closeConnection, this, &MainWindow::closeConnection);
    connect(&dlg, &ConnectionDialog::openConnection, this, &MainWindow::openConnection);

    dlg.setModal(true);
    dlg.exec();
    if(dlg.result() == QDialog::Accepted){
        m_client->write_conf();
        reconnect();
    }

    return true;
}

void MainWindow::reconnect()
{
    if(m_client->isConnected())
            m_client->close();

    m_client->open();
}

void MainWindow::displayError(const QString &what, const QString &err)
{

}

void MainWindow::connectionSuccess()
{
    get_online_users();
    if(m_models.find(arcirk::server::DatabaseUsers) == m_models.end()){
        m_models.insert(arcirk::server::DatabaseUsers, new TreeViewModel(m_client->conf(), this));
        m_models[arcirk::server::DatabaseUsers]->set_columns({"first","second","ref","parent", "is_group", "deletion_mark"});
        m_models[arcirk::server::DatabaseUsers]->set_column_aliases(m_colAliases);
        m_models[arcirk::server::DatabaseUsers]->set_server_object(arcirk::server::DatabaseUsers);
        ui->treeView->setUniformRowHeights(true);
        ui->treeView->setModel(m_models[arcirk::server::DatabaseUsers]);
    }
}

void MainWindow::connectionChanged(bool state)
{
    ui->mnuConnect->setEnabled(!state);
    ui->mnuDisconnect->setEnabled(state);

    auto tree = ui->treeWidget;

    if(!state){
        infoBar->setText("Не подключен");
        tree->topLevelItem(0)->setText(0, "Не подключен");
        tree->clear();
        ui->treeView->setModel(nullptr);
    }else{
        infoBar->setText("Подключен: " + QString(m_client->conf().server_host.data()));
        if(!tree->topLevelItem(0))
            fillDefaultTree();
        tree->topLevelItem(0)->setText(0, QString(m_client->conf().server_host.data()));

    }
}


void MainWindow::formControl()
{
    infoBar = new QLabel(this);
    ui->statusbar->addWidget(infoBar);
    infoBar->setText("Готово");

    app_icons.insert(arcirk::server::application_names::PriceChecker, QIcon(":/img/pricechecker.png"));
    app_icons.insert(arcirk::server::application_names::ServerManager, QIcon(":/img/socket.ico"));

}

void MainWindow::write_conf()
{

}

void MainWindow::openConnection()
{
    if(m_client->isConnected())
            m_client->close();

    m_client->open();
}

void MainWindow::closeConnection()
{
    if(m_client->isConnected())
        m_client->close();
}

void MainWindow::serverResponse(const arcirk::server::server_response &message)
{
    if(message.command == arcirk::enum_synonym(arcirk::server::server_commands::ServerOnlineClientsList)){
        tableResetModel(arcirk::server::OnlineUsers, message.result.data());
    }else if(message.command == arcirk::enum_synonym(arcirk::server::server_commands::ServerConfiguration)){
        tableResetModel(arcirk::server::Root, message.result.data());
    }else if(message.command == arcirk::enum_synonym(arcirk::server::server_commands::GetDatabaseTables)){
        tableResetModel(arcirk::server::DatabaseTables, message.result.data());
    }else if(message.command == arcirk::enum_synonym(arcirk::server::server_commands::ExecuteSqlQuery)){
        auto param = nlohmann::json::parse(QByteArray::fromBase64(message.param.data()));
        auto query_param = param.value("query_param", "");
        if(!query_param.empty()){
            auto param_ = nlohmann::json::parse(QByteArray::fromBase64(query_param.data()));
            auto table_name = param_.value("table_name", "");
            if(table_name == "Devices" || table_name == "DevicesView"){
                tableResetModel(arcirk::server::Devices, message.result.data());
            }
        }

    }else if(message.command == arcirk::enum_synonym(arcirk::server::server_commands::ProfileDeleteFile)){
        if(message.message == "OK"){
            QMessageBox::information(this, "Удаление файла", "Файл успешно удален!");
            auto model = (TreeViewModel*)ui->treeView->model();
            model->clear();
            model->fetchRoot("ProfileDirectory");
        }else if(message.message == "error"){
            QMessageBox::critical(this, "Удаление файла", "Ошибка удаения файла!");
        }
    }
}


void MainWindow::on_mnuConnect_triggered()
{
    openConnectionDialog();
}


void MainWindow::on_mnuExit_triggered()
{
    QApplication::exit();
}


void MainWindow::on_mnuDisconnect_triggered()
{
    if(m_client->isConnected())
            m_client->close();
}

QTreeWidgetItem *MainWindow::addTreeNode(const QString &text, const QVariant &key, const QString &imagePath)
{
    //qDebug() << __FUNCTION__;
    auto * node = new QTreeWidgetItem();
    node-> setText (0, text);
    node->setData(0, Qt::UserRole, key);
    node->setIcon(0, QIcon(imagePath));
    return node;
}

QTreeWidgetItem *MainWindow::findTreeItem(const QString &key)
{
    auto tree = ui->treeWidget;
    auto root = tree->topLevelItem(0);
    return findTreeItem(key, root);
}

QTreeWidgetItem * MainWindow::findTreeItem(const QString& key, QTreeWidgetItem* parent)
{
    int i;

    if (!parent)
        return nullptr;

    if(parent->childCount()>0){
        for(i=0;i<parent->childCount();i++){
            if(parent->child(i)!=0)
            {
                if (parent->child(i)->data(0, Qt::UserRole).toString() == key)
                    return parent->child(i);
                else{
                    auto ch = findTreeItem(key, parent->child(i));
                    if(ch)
                        return ch;
                }
            }

        }
    }

    return nullptr;
}

void MainWindow::tableSetModel(const QString &key)
{
    nlohmann::json m_key = key.toStdString();
    auto e_key = m_key.get<arcirk::server::server_objects>();
    auto model = m_models[e_key];
    auto table = ui->treeView;
    table->setModel(nullptr);
    table->setModel(model);
//    model->reset();
//    table->resizeColumnsToContents();

}

void MainWindow::tableResetModel(server::server_objects key, const QByteArray& resp)
{
    if(!m_client->isConnected())
        return;

    using namespace arcirk::server;

    if(key == OnlineUsers){

        if(!resp.isEmpty() && resp != "error"){
            auto json = QByteArray::fromBase64(resp);
            //qDebug() << qPrintable(json);
            m_models[OnlineUsers]->set_table(nlohmann::json::parse(json));
            update_icons(OnlineUsers, m_models[OnlineUsers]);
            m_models[OnlineUsers]->reset();
        }
    }else if (key == Root){
        //auto srv_conf = pre::json::from_json<server_config>(nlohmann::json::parse(QByteArray::fromBase64(resp).toStdString()));
        auto srv_conf = nlohmann::json::parse(QByteArray::fromBase64(resp).toStdString());
        if(srv_conf.is_object()){
            auto items = srv_conf.items();
            auto columns = nlohmann::json::array();
            //columns += "empty";
            columns += "Параметр";
            columns += "Значение";
            auto rows = nlohmann::json::array();
            for (auto itr = items.begin(); itr != items.end(); ++itr) {
                auto row = nlohmann::json::object();
                //row["empty"] = " ";
                row["Параметр"] = itr.key();
                row["Значение"] = itr.value();
                rows += row;
            }
            auto json = nlohmann::json::object();
            json["columns"] = columns;
            json["rows"] = rows;
            m_models[Root]->set_table(json);
            m_models[Root]->reset();
        }


    }else if (key == DatabaseTables){
        auto tbls = nlohmann::json::parse(QByteArray::fromBase64(resp).toStdString());
        if(tbls.is_array()){
            auto columns = nlohmann::json::array();
            //columns += "empty";
            columns += "Таблица";
            auto rows = nlohmann::json::array();
            for (auto itr = tbls.begin(); itr != tbls.end(); ++itr) {
                auto row = nlohmann::json::object();
                //row["empty"] = " ";
                row["Таблица"] = *itr;
                rows += row;
            }
            auto json = nlohmann::json::object();
            json["columns"] = columns;
            json["rows"] = rows;
            m_models[DatabaseTables]->set_table(json);
            m_models[DatabaseTables]->reset();
        }
    }else if (key == Devices){
        auto tbls = QByteArray::fromBase64(resp);
        m_models[Devices]->set_table(nlohmann::json::parse(tbls));
        update_icons(Devices, m_models[Devices]);
        m_models[Devices]->reset();
    }

}

void MainWindow::fillDefaultTree()
{
    qDebug() << __FUNCTION__;
    auto tree = ui->treeWidget;
    tree->setHeaderHidden(true);

    using namespace arcirk;

    auto root = addTreeNode("root", arcirk::enum_synonym(server::server_objects::Root).c_str(), ":/img/root.png");
    tree->addTopLevelItem(root);
    auto item = addTreeNode("Активные пользователи", arcirk::enum_synonym(server::server_objects::OnlineUsers).c_str(), ":/img/activeUesers16.png");
    root->addChild(item);
//    auto curr_user = addTreeNode("Текущий пользователь", СurrentUser, ":/img/userOptions.ico");
//    root->addChild(curr_user);


//    tree->expandAll();
//    auto wsUsers = addTreeNode("Пользователи", UsersCatalogRoot, ":/img/users1.png");
//    ws->addChild(wsUsers);
    item = addTreeNode("Службы", arcirk::enum_synonym(server::server_objects::Services).c_str(), ":/img/services.png");
    root->addChild(item);
    item = addTreeNode("База данных", arcirk::enum_synonym(server::server_objects::Database).c_str(), ":/img/sqlServer.png");
    root->addChild(item);
    auto tbl = addTreeNode("Таблицы", arcirk::enum_synonym(server::server_objects::DatabaseTables).c_str(), ":/img/socket_16_only.ico");
    item->addChild(tbl);
    tbl = addTreeNode("Устройства", arcirk::enum_synonym(server::server_objects::Devices).c_str(), ":/img/SqlDivaces.png");
    item->addChild(tbl);
    tbl = addTreeNode("Пользователи", arcirk::enum_synonym(server::server_objects::DatabaseUsers).c_str(), ":/img/users1.png");
    item->addChild(tbl);
    tbl = addTreeNode("Каталог профиля", arcirk::enum_synonym(server::server_objects::ProfileDirectory).c_str(), ":/img/users1.png");
    root->addChild(tbl);
}

void MainWindow::on_toolButton_clicked()
{
    auto model = (TreeViewModel*)ui->treeView->model();
    auto index = ui->treeView->currentIndex();
    if(!index.isValid()){
        QMessageBox::critical(this, "Ошибка", "Не выбран элемент!");
        return;
    }

    using namespace arcirk::server;
    using json = nlohmann::json;
    if(model->server_object() == arcirk::server::ProfileDirectory){

    }
}

void MainWindow::createModels()
{
    using namespace arcirk::server;
    QVector<server_objects> vec{
        OnlineUsers,
        Root,
        Services,
        Database,
        DatabaseTables,
        Devices,
        DatabaseUsers,
        ProfileDirectory
    };

    foreach (auto const& itr, vec) {
        auto model = new TreeViewModel(m_client->conf(), this);
        model->set_column_aliases(m_colAliases);
        model->set_server_object(itr);
        if(itr == DatabaseUsers)
            model->set_columns({"first","second","ref","parent", "is_group", "deletion_mark"});
        else if(itr == DatabaseTables){
            model->set_rows_icon(QIcon(":/img/externalDataTable.png"));
        }
        m_models.insert(itr, model);
    }
}

void MainWindow::createColumnAliases()
{
    m_colAliases.insert("uuid", "ID");
    m_colAliases.insert("session_uuid", "ID сессии");
    m_colAliases.insert("name", "Имя");
    m_colAliases.insert("uuid_user", "ID пользователя");
    m_colAliases.insert("user_uuid", "ID пользователя");
    m_colAliases.insert("app_name", "Приложение");
    m_colAliases.insert("user_name", "Имя пользователя");
    m_colAliases.insert("ip_address", "IP адрес");
    m_colAliases.insert("address", "IP адрес");
    m_colAliases.insert("host_name", "Host");
    m_colAliases.insert("Ref", "Ссылка");
    m_colAliases.insert("ref", "Ссылка");
    m_colAliases.insert("FirstField", "Имя");
    m_colAliases.insert("SecondField", "Представление");
    m_colAliases.insert("first", "Имя");
    m_colAliases.insert("second", "Представление");
    m_colAliases.insert("privateKey", "Контейнер");
    m_colAliases.insert("_id", "Иднекс");
    m_colAliases.insert("sid", "SID");
    m_colAliases.insert("cache", "cache");
    m_colAliases.insert("subject", "Кому выдан");
    m_colAliases.insert("issuer", "Кем выдан");
    m_colAliases.insert("container", "Контейнер");
    m_colAliases.insert("notValidBefore", "Начало действия");
    m_colAliases.insert("parentUser", "Владелец");
    m_colAliases.insert("notValidAfter", "Окончание дейтствия");
    m_colAliases.insert("serial", "Серийный номер");
    m_colAliases.insert("volume", "Хранилище");
    m_colAliases.insert("cache", "Кэш");
    m_colAliases.insert("role", "Роль");
    m_colAliases.insert("device_id", "ID устройства");
    m_colAliases.insert("ipadrr", "IP адрес");
    m_colAliases.insert("product", "Продукт");
    m_colAliases.insert("typeIp", "Тип адреса");
    m_colAliases.insert("defPort", "Стандартный порт");
    m_colAliases.insert("notFillWindow", "Оконный режим");
    m_colAliases.insert("password", "Пароль");
    m_colAliases.insert("width", "Ширина");
    m_colAliases.insert("height", "Высота");
    m_colAliases.insert("port", "Порт");
    m_colAliases.insert("user", "Пользователь");
    m_colAliases.insert("updateUser", "Обновлять учетку");
    m_colAliases.insert("start_date", "Дата подключения");
    m_colAliases.insert("organization", "Организация");
    m_colAliases.insert("subdivision", "Подразделение");
    m_colAliases.insert("warehouse", "Склад");
    m_colAliases.insert("price", "Тип цен");
    m_colAliases.insert("workplace", "Рабочее место");
    m_colAliases.insert("device_type", "Тип устройства");
    m_colAliases.insert("is_group", "Это группа");
    m_colAliases.insert("size", "Размер");
}

void MainWindow::on_treeWidget_itemClicked(QTreeWidgetItem *item, int column)
{
    if(!item)
        return;

    QString key = item->data(0, Qt::UserRole).toString();
    qDebug() << __FUNCTION__ << "tree key:" << key;
    tableSetModel(key);

    using namespace arcirk::server;
    if(key == QString(arcirk::enum_synonym(OnlineUsers).data())){
//        ui->treeView->setVisible(false);
//        ui->tableView->setVisible(true);
        get_online_users();
    }else if(key == QString(arcirk::enum_synonym(Root).data())){
//        ui->treeView->setVisible(false);
//        ui->tableView->setVisible(true);
        if(m_client->isConnected())
            m_client->send_command(arcirk::server::server_commands::ServerConfiguration, nlohmann::json{});
    }else if(key == QString(arcirk::enum_synonym(DatabaseTables).data())){
        if(m_models[DatabaseTables]->is_loaded()){
            tableSetModel(arcirk::enum_synonym(DatabaseTables).c_str());
            return;
        }
        if(m_client->isConnected())
            m_client->send_command(arcirk::server::server_commands::GetDatabaseTables, nlohmann::json{});
    }else if(key == QString(arcirk::enum_synonym(Devices).data())){
//        ui->treeView->setVisible(false);
//        ui->tableView->setVisible(true);
        if(m_client->isConnected()){
            auto query_param = nlohmann::json{
                {"table_name", "DevicesView"},
                {"where_values", nlohmann::json{}},
                {"query_type", "select"},
                {"empty_column", false},
                {"line_number", false}
            };
            m_client->send_command(arcirk::server::server_commands::ExecuteSqlQuery, nlohmann::json{
                                       {"query_param", QByteArray(query_param.dump().data()).toBase64().toStdString()}
                                   });
        }
    }else if(key == QString(arcirk::enum_synonym(DatabaseUsers).data())){
//        ui->treeView->setVisible(true);
//        ui->tableView->setVisible(false);
        if(m_models.find(DatabaseUsers) != m_models.end()){
            m_models[DatabaseUsers]->fetchRoot("Users");
            QVector<QString> m_hide{"ref", "parent", "is_group", "deletion_mark"};
            foreach (auto const& itr, m_hide) {
                auto index = m_models[DatabaseUsers]->get_column_index(itr);
                if(index != -1)
                    ui->treeView->hideColumn(index);
            }
            ui->treeView->hideColumn(m_models[DatabaseUsers]->columnCount(QModelIndex()) - 1);
        }
        ui->treeView->resizeColumnToContents(0);


    }else if(key == QString(arcirk::enum_synonym(ProfileDirectory).data())){
        if(m_client->isConnected()){
            m_models[ProfileDirectory]->fetchRoot("ProfileDirectory");
            ui->treeView->resizeColumnToContents(0);
//            using json_nl = nlohmann::json;

//            std::string uuid_form_ = arcirk::uuids::nil_string_uuid();

//            json_nl param = {
//                    {"table", true},
//                    {"uuid_form", uuid_form_},
//                    {"empty_column", false},
//                    {"recursive", true}
//            };

//            m_client->send_command(arcirk::server::server_commands::ProfileDirFileList, param);
        }
    }

}

void MainWindow::get_online_users() {

    if(!m_client->isConnected())
        return;

    using json_nl = nlohmann::json;

    std::string uuid_form_ = arcirk::uuids::nil_string_uuid();

    json_nl param = {
            {"table", true},
            {"uuid_form", uuid_form_},
            {"empty_column", false}
    };

    //std::string param_ = param.dump();
    m_client->send_command(arcirk::server::server_commands::ServerOnlineClientsList, param);

}

void MainWindow::update_icons(arcirk::server::server_objects key, TreeViewModel *model)
{
    using namespace arcirk::server;
    using namespace nlohmann;

    if(key == Devices){
        int ind = model->get_column_index("device_type");
        int indRef = model->get_column_index("ref");
        int indDev = m_models[OnlineUsers]->get_column_index("device_id");
        auto parent = QModelIndex();
        if(ind == -1)
            return;
        for (int i = 0; i < model->rowCount(parent); ++i) {
            auto index = model->index(i, 0, parent);
            json device_type = model->index(i, ind, parent).data(Qt::DisplayRole).toString().trimmed().toStdString();
            QString ref = model->index(i, indRef, parent).data(Qt::DisplayRole).toString().trimmed();
            auto indexRef = findInTable(m_models[OnlineUsers], ref, indDev, false);
            auto type = device_type.get<device_types>();
            if(type == devDesktop){
                if(!indexRef.isValid())
                    model->set_icon(index, QIcon(":/img/desktop.png"));
                else
                    model->set_icon(index, QIcon(":/img/desktopOnline.png"));
            }else if(type == devServer){
                if(!indexRef.isValid())
                    model->set_icon(index, QIcon(":/img/server.png"));
                else
                    model->set_icon(index, QIcon(":/img/serverOnline.png"));
            }else if(type == devPhone){
                if(!indexRef.isValid())
                    model->set_icon(index, QIcon(":/img/mobile.png"));
                else
                    model->set_icon(index, QIcon(":/img/mobileOnline.png"));
            }else if(type == devTablet){
                if(!indexRef.isValid())
                    model->set_icon(index, QIcon(":/img/tablet.png"));
                else
                    model->set_icon(index, QIcon(":/img/tabletOnline.png"));
            }
        }

    }else if(key == OnlineUsers){
        using namespace arcirk::server;

        int ind = model->get_column_index("app_name");
        auto parent = QModelIndex();
        if(ind == -1)
            return;
        for (int i = 0; i < model->rowCount(parent); ++i) {
            json app = model->index(i, ind, parent).data(Qt::DisplayRole).toString().trimmed().toStdString();
            auto type_app = app.get<application_names>();
            if(type_app == PriceChecker){
                model->set_icon(model->index(i,0,parent), app_icons[PriceChecker]);
            }else if(type_app == ServerManager){
                model->set_icon(model->index(i,0,parent), app_icons[ServerManager]);
            }
        }
    }
}


QModelIndex MainWindow::findInTable(QAbstractItemModel * model, const QString &value, int column, bool findData)
{
    int rows =  model->rowCount();
    for (int i = 0; i < rows; ++i) {
        auto index = model->index(i, column);
        if(findData){
            if(value == index.data(Qt::UserRole + 1).toString())
                return index;
        }else{
            QString data = index.data(Qt::UserRole + column).toString();
            if(value == data)
                return index;
        }
    }

    return QModelIndex();
}

void MainWindow::on_btnEdit_clicked()
{

}


void MainWindow::on_treeView_doubleClicked(const QModelIndex &index)
{
    if(!index.isValid()){
        QMessageBox::critical(this, "Ошибка", "Не выбран элемент!");
        return;
    }

    auto model = (TreeViewModel*)ui->treeView->model();

    if(model->server_object() == arcirk::server::DatabaseUsers){
        auto object = model->get_object(index);
        if(object["is_group"].get<int>() == 1){
            //QMessageBox::critical(this, "Ошибка", "Редактирование групп запрещено!");
            return;
        }
        using namespace arcirk::database;
        auto query = builder::query_builder();

        std::string query_text = query.select().from("Users").where(nlohmann::json{
                                                                           {"ref", object["ref"].get<std::string>()}
                                                                       }, true).prepare();

        auto param = nlohmann::json::object();
        param["query_text"] = query_text;
        auto command = arcirk::enum_synonym(arcirk::server::server_commands::ExecuteSqlQuery);
        auto result = m_client->exec_http_query(command, param);

        if(!result.is_object()){
            QMessageBox::critical(this, "Ошибка", "Ошибка на свервере!");
            return;
        }

        auto rows = result["rows"];
        if(!rows.is_array()){
            QMessageBox::critical(this, "Ошибка", "Ошибка на свервере!");
            return;
        }

        if(rows.size() == 0){
            QMessageBox::critical(this, "Ошибка", "Запись не найдена!");
            return;
        }

        auto struct_users = pre::json::from_json<arcirk::database::user_info>(rows[0]);
        auto parent = index.parent();
        QString parentName;
        if(parent.isValid()){
            parentName = parent.data().toString();
        }
        auto dlg = DialogUser(struct_users, parentName, this);
        dlg.setModal(true);
        dlg.exec();

    }
}


void MainWindow::on_btnDataImport_clicked()
{
    auto model = (TreeViewModel*)ui->treeView->model();
    auto index = ui->treeView->currentIndex();
    if(!index.isValid()){
        QMessageBox::critical(this, "Ошибка", "Не выбран элемент!");
        return;
    }

    using namespace arcirk::server;

    if(model->server_object() == arcirk::server::DatabaseTables){
        auto model = new TreeViewModel(m_client->conf(), this);
        model->set_column_aliases(m_colAliases);
        model->set_server_object(arcirk::server::DatabaseTables);
        model->fetchRoot("ProfileDirectory", "shared_files/files");
        auto dlg = DialogProfileFolder(model, this);
        dlg.setModal(true);
        dlg.setWindowTitle("Выбор файла для импорта");
        dlg.exec();

        if(dlg.result() == QDialog::Accepted){
            auto tableName = index.data().toString();
            auto path = dlg.file_name();
            if(QMessageBox::question(this, "Импорт файла", QString("Импортировать данные из файла в таблицу %1?").arg(tableName)) == QMessageBox::No)
                return;
            if(m_client->isConnected()){
                auto param = nlohmann::json{
                    {"file_name", path.toStdString()},
                    {"table_name", tableName.toStdString()},
                };
                m_client->send_command(arcirk::server::server_commands::FileToDatabase, param);
            }
        }

        delete model;
    }
}


void MainWindow::on_btnAdd_clicked()
{
    auto model = (TreeViewModel*)ui->treeView->model();
    auto index = ui->treeView->currentIndex();
    if(!index.isValid()){
        QMessageBox::critical(this, "Ошибка", "Не выбран элемент!");
        return;
    }

    using namespace arcirk::server;
    using json = nlohmann::json;

    if(model->server_object() == arcirk::server::ProfileDirectory){

        auto result = QFileDialog::getOpenFileName(this, "Выбрать файл на диске");
        if(!result.isEmpty()){
            ByteArray data{};
            try {
                arcirk::read_file(result.toStdString(), data);
                if(data.size() > 0){
                    auto destantion = model->current_parent_path();
                    QUrl url(result);
                    auto file_name = url.fileName();
                    json param{
                        {"destantion", destantion.toStdString()},
                        {"file_name", file_name.toStdString()},
                        {"data", data}
                    };
                    auto resp = m_client->exec_http_query(arcirk::enum_synonym(server_commands::DownloadFile), param);
                        if(resp  == "success"){
                            QMessageBox::information(this, "Копирование файла", "Файл успешно скопирован!");
                            model->refresh(index);
                        }
                }

            } catch (const std::exception& e) {
                QMessageBox::critical(this, "Ошибка", e.what());
                return;
            }

        }
    }
}


void MainWindow::on_btnDelete_clicked()
{
    auto model = (TreeViewModel*)ui->treeView->model();
    auto index = ui->treeView->currentIndex();
    if(!index.isValid()){
        QMessageBox::critical(this, "Ошибка", "Не выбран элемент!");
        return;
    }

    using namespace arcirk::server;
    using json = nlohmann::json;
    if(model->server_object() == arcirk::server::ProfileDirectory){

        int ind = model->get_column_index("path");
        auto file_path = model->index(index.row(), ind, index.parent()).data().toString();
        auto file_name = model->index(index.row(), 0, index.parent()).data().toString();
        auto result = QMessageBox::question(this, "Удаление файла", QString("Удалить файл %1").arg(file_name));

        if(result == QMessageBox::Yes){

            model->remove(index);
//            int rows = model->rowCount(index);
//            qDebug() << rows;

//            json param{
//                {"file_name", file_path.toStdString()}
//            };
//            m_client->send_command(server_commands::ProfileDeleteFile, param);



//            try {
//                arcirk::read_file(result.toStdString(), data);
//                if(data.size() > 0){
//                    auto destantion = model->current_parent_path();
//                    QUrl url(result);
//                    auto file_name = url.fileName();
//                    json param{
//                        {"destantion", destantion.toStdString()},
//                        {"file_name", file_name.toStdString()},
//                        {"data", data}
//                    };
//                    auto resp = m_client->exec_http_query(arcirk::enum_synonym(server_commands::DownloadFile), param);
//                }

//            } catch (const std::exception& e) {
//                QMessageBox::critical(this, "Ошибка", e.what());
//                return;
//            }

        }
    }
}

