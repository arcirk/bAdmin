#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QStandardPaths>
#include <QMessageBox>
#include "query_builder.hpp"
#include "dialoguser.h"
#include "connectiondialog.h"
#include <QVector>
#include <QFileDialog>
#include "dialogabout.h"
#include "dialogseversettings.h"
#include "dialogdevice.h"
#include "dialogselectinlist.h"
#include "dialogtask.h"
#include <QStandardPaths>
#include <QException>
#include "cryptcertificate.h"
#include "cryptcontainer.h"
#include <QStorageInfo>
#include <QSysInfo>
#include "dialogselectdevice.h"
#include <QDirIterator>
#include <QTemporaryFile>
#include "dialoginfo.h"
#include "dialogedit.h"
#include "dialogcertusercache.h"

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

    createColumnAliases();

    formControl();

    fillDefaultTree();;

    current_user = new CertUser(this);
    current_user->setLocalhost(true);

    setWindowTitle(QString("Менеджер сервера (%1)").arg(current_user->getInfo().user.c_str()));

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
    createModels();
    get_online_users();
    if(m_models.find(arcirk::server::DatabaseUsers) == m_models.end()){
        m_models.insert(arcirk::server::DatabaseUsers, new TreeViewModel(m_client->conf(), this));
        m_models[arcirk::server::DatabaseUsers]->set_columns({"first","second","ref","parent", "is_group", "deletion_mark"});
        m_models[arcirk::server::DatabaseUsers]->use_hierarchy("first");
        m_models[arcirk::server::DatabaseUsers]->set_column_aliases(m_colAliases);
        m_models[arcirk::server::DatabaseUsers]->set_server_object(arcirk::server::DatabaseUsers);
        ui->treeView->setUniformRowHeights(true);
        ui->treeView->setModel(m_models[arcirk::server::DatabaseUsers]);
    }
//    auto tree = ui->treeWidget;
//    if(tree->topLevelItem(0))

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
        infoBar->setText(QString("Подключен: %1 (%2)").arg(m_client->conf().server_host.c_str(), m_client->server_conf().ServerName.c_str()));
        if(!tree->topLevelItem(0)){
            fillDefaultTree();          
        }
         tree->topLevelItem(0)->setText(0, QString("%1 (%2)").arg(m_client->conf().server_host.c_str(), m_client->server_conf().ServerName.c_str()));

    }
}


void MainWindow::formControl()
{
    infoBar = new QLabel(this);
    ui->statusbar->addWidget(infoBar);
    infoBar->setText("Готово");

    app_icons.insert(arcirk::server::application_names::PriceChecker, qMakePair(QIcon(":/img/pricechecker.png"), QIcon(":/img/pricechecker.png")));
    app_icons.insert(arcirk::server::application_names::ServerManager, qMakePair(QIcon(":/img/socket.ico"), QIcon(":/img/socket.ico")));
    app_icons.insert(arcirk::server::application_names::ExtendedLib, qMakePair(QIcon(":/img/1cv8.png"), QIcon(":/img/1cv8.png")));

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
        if(message.result == "error"){
            qDebug() << __FUNCTION__ << "error result ExecuteSqlQuery";
            return;
        }
        auto param = nlohmann::json::parse(QByteArray::fromBase64(message.param.data()));
        auto query_param = param.value("query_param", "");
        if(!query_param.empty()){
            auto param_ = nlohmann::json::parse(QByteArray::fromBase64(query_param.data()));
            auto table_name = param_.value("table_name", "");
            if(table_name == "Devices" || table_name == "DevicesView"){
                tableResetModel(arcirk::server::Devices, message.result.data());
            }else if(table_name == "Certificates"){
                tableResetModel(arcirk::server::Certificates, message.result.data());
//            }else if(table_name == "CertUsers"){
//                tableResetModel(arcirk::server::CertUsers, message.result.data());
            }else if(table_name == "Containers"){
                tableResetModel(arcirk::server::Containers, message.result.data());
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
    }else if(message.command == arcirk::enum_synonym(arcirk::server::server_commands::GetTasks)){
        if(message.message == "OK"){
            tableResetModel(arcirk::server::Services, message.result.data());
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
    try {
        nlohmann::json m_key = key.toStdString();
        auto e_key = m_key.get<arcirk::server::server_objects>();
        auto model = m_models[e_key];
        auto table = ui->treeView;
        table->setModel(nullptr);
        table->setModel(model);
        table->resizeColumnToContents(0);
    } catch (const QException& e) {
        qCritical() << e.what();
    }


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
            QVector<QString> col_order{"app_name", "host_name", "address", "user_name", "role", "start_date", "session_uuid", "user_uuid", "device_id"};
            m_models[OnlineUsers]->columns_establish_order(col_order);
            update_icons(OnlineUsers, m_models[OnlineUsers]);
            m_models[OnlineUsers]->reset();
        }
    }else if (key == Root){
        //auto srv_conf = pre::json::from_json<server_config>(nlohmann::json::parse(QByteArray::fromBase64(resp).toStdString()));
        auto srv_conf = nlohmann::json::parse(QByteArray::fromBase64(resp).toStdString());
        if(srv_conf.is_object()){
            auto items = srv_conf.items();
            auto columns = nlohmann::json::array();
            columns += "Параметр";
            columns += "Значение";
            auto rows = nlohmann::json::array();
            for (auto itr = items.begin(); itr != items.end(); ++itr) {
                auto row = nlohmann::json::object();
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
        if(tbls.is_object()){
            m_models[DatabaseTables]->set_table(tbls);
            m_models[DatabaseTables]->reset();
        }
    }else if (key == Devices){
        auto tbls = QByteArray::fromBase64(resp);
        m_models[Devices]->set_table(nlohmann::json::parse(tbls));
        update_icons(Devices, m_models[Devices]);
        m_models[Devices]->reset();
    }else if (key == Services){
        auto tbls = QByteArray::fromBase64(resp);
        m_models[Services]->set_table(nlohmann::json::parse(tbls));
        update_icons(Services, m_models[Services]);
        m_models[Services]->reset();
//    }else if (key == CertUsers){
//        auto tbls = QByteArray::fromBase64(resp);
//        m_models[CertUsers]->set_table(nlohmann::json::parse(tbls));
//        //update_icons(Services, m_models[Services]);
//        //m_models[CertUsers]->set_columns({"first","second","ref","parent", "is_group", "deletion_mark"});
//        m_models[CertUsers]->use_hierarchy("first");
//        m_models[CertUsers]->reset();
    }else{
        auto tbls = QByteArray::fromBase64(resp);
        m_models[key]->set_table(nlohmann::json::parse(tbls));
        m_models[key]->reset();
    }

}

void MainWindow::resetModel(server::server_objects key, const nlohmann::json &data)
{
    if(!m_client->isConnected())
        return;

    using namespace arcirk::server;
    m_models[key]->set_table(data);
    m_models[key]->reset();
    update_icons(key, m_models[key]);

    if (key == LocalhostUserCertificates){

    }else if (key == arcirk::server::LocalhostUserContainersRegistry){
//        m_models[key]->set_rows_icon(QIcon("qrc:/img/cert16NoKey.png"));
//        m_models[key]->set_table(data);
//        m_models[key]->reset();
    }

        ui->treeView->resizeColumnToContents(0);
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
    item = addTreeNode("Зарегистрированные пользователи", arcirk::enum_synonym(server::server_objects::CertUsers).c_str(), ":/img/certUsers.png");
    root->addChild(item);
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
    tbl = addTreeNode("Менеджер сертификатов", arcirk::enum_synonym(server::server_objects::CertManager).c_str(), ":/img/cert.png");
    root->addChild(tbl);
    item = addTreeNode("Контейнеры", arcirk::enum_synonym(server::server_objects::Containers).c_str(), ":/img/cont.png");
    tbl->addChild(item);
    item = addTreeNode("Сертификаты", arcirk::enum_synonym(server::server_objects::Certificates).c_str(), ":/img/certs16.png");
    tbl->addChild(item);
    item = addTreeNode("Текущий пользователь", arcirk::enum_synonym(server::server_objects::LocalhostUser).c_str(), ":/img/userOptions.ico");
    tbl->addChild(item);
    auto item_l = addTreeNode("Сертификаты", arcirk::enum_synonym(server::server_objects::LocalhostUserCertificates).c_str(), ":/img/certs16.png");
    item->addChild(item_l);
    item_l = addTreeNode("Контейнеры", arcirk::enum_synonym(server::server_objects::LocalhostUserContainers).c_str(), ":/img/certUsers.png");
    item->addChild(item_l);
//    auto item_u = addTreeNode("Реестр", arcirk::enum_synonym(server::server_objects::LocalhostUserContainersRegistry).c_str(), ":/img/registry16.png");
//    item_l->addChild(item_u);
//    item_u = addTreeNode("Устройства", arcirk::enum_synonym(server::server_objects::LocalhostUserContainersVolume).c_str(), ":/img/Card_Reader_16.ico");
//    item_l->addChild(item_u);
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
        ProfileDirectory,
        CertManager,
        Containers,
        CertUsers,
        Certificates,
        LocalhostUser,
        LocalhostUserCertificates,
        LocalhostUserContainers

    };
//    LocalhostUserContainersRegistry,
//    LocalhostUserContainersVolume
    m_models.clear();

    foreach (auto const& itr, vec) {
        auto model = new TreeViewModel(m_client->conf(), this);
        model->set_column_aliases(m_colAliases);
        model->set_server_object(itr);
        if(itr == DatabaseUsers){
            model->set_columns({"first","second","ref","parent", "is_group", "deletion_mark"});
            model->use_hierarchy("first");
        }else if(itr == DatabaseTables){
            model->set_rows_icon(QIcon(":/img/externalDataTable.png"));
        }else if(itr == CertUsers){
            //model->set_columns({"first","second","ref","parent", "is_group", "deletion_mark"});
            model->use_hierarchy("first");
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
    m_colAliases.insert("rows_count", "Количество записей");
    m_colAliases.insert("interval", "Интервал");
    m_colAliases.insert("start_task", "Начало выполнения");
    m_colAliases.insert("end_task", "Окончание выполнения");
    m_colAliases.insert("allowed", "Разрешено");
    m_colAliases.insert("predefined", "Предопределенный");
    m_colAliases.insert("days_of_week", "По дням недели");
    m_colAliases.insert("synonum", "Представление");
    m_colAliases.insert("script", "Скрипт");
    m_colAliases.insert("parent_user", "Владелец");
    m_colAliases.insert("not_valid_before", "Начало действия");
    m_colAliases.insert("not_valid_after", "Окончание дейтствия");
    m_colAliases.insert("type", "Тип");
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
        get_online_users();
    }else if(key == QString(arcirk::enum_synonym(Root).data())){
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
        if(m_models.find(DatabaseUsers) != m_models.end()){
            m_models[DatabaseUsers]->fetchRoot("Users");
            QVector<QString> m_hide{"ref", "parent", "is_group", "deletion_mark"};
            foreach (auto const& itr, m_hide) {
                auto index = m_models[DatabaseUsers]->get_column_index(itr);
                if(index != -1)
                    ui->treeView->hideColumn(index);
            }
        }
        ui->treeView->resizeColumnToContents(0);


    }else if(key == QString(arcirk::enum_synonym(ProfileDirectory).data())){
        if(m_client->isConnected()){
            m_models[ProfileDirectory]->fetchRoot("ProfileDirectory");
            QVector<QString> m_hide{"path", "parent", "is_group"};
            foreach (auto const& itr, m_hide) {
                auto index = m_models[ProfileDirectory]->get_column_index(itr);
                if(index != -1)
                    ui->treeView->hideColumn(index);
            }
            ui->treeView->resizeColumnToContents(0);
        }
    }else if(key == QString(arcirk::enum_synonym(Services).data())){
        m_client->send_command(arcirk::server::server_commands::GetTasks, nlohmann::json{});
    }else if(key == QString(arcirk::enum_synonym(Certificates).data())){
        if(m_client->isConnected()){
            //auto values_ = pre::json::to_json(certificates());
            //values_.erase("data");
            //auto values = arcirk::json_keys(values_);
            auto query_param = nlohmann::json{
                {"table_name", "Certificates"},
                {"where_values", nlohmann::json{}},
                {"query_type", "select"},
                {"empty_column", false},
                {"line_number", false},
                {"values", json{
                        "first",
                        "ref",
                        "private_key",
                        "subject",
                        "issuer",
                        "not_valid_before",
                        "not_valid_after",
                        "parent_user",
                        "serial",
                        "sha1",
                        "suffix",
                        "cache"
                    }
                }
            };
            m_client->send_command(arcirk::server::server_commands::ExecuteSqlQuery, nlohmann::json{
                                       {"query_param", QByteArray(query_param.dump().data()).toBase64().toStdString()}
                                   });
        }
    }else if(key == QString(arcirk::enum_synonym(CertUsers).data())){
//        if(m_client->isConnected()){
//            auto query_param = nlohmann::json{
//                {"table_name", "CertUsers"},
//                {"where_values", nlohmann::json{}},
//                {"values", json::array({"first", "second", "ref", "cache", "uuid", "host", "parent", "is_group","deletion_mark"})},
//                {"query_type", "select"}
////                    ,
////                {"empty_column", false},
////                {"line_number", false}
//            };
//            m_client->send_command(arcirk::server::server_commands::ExecuteSqlQuery, nlohmann::json{
//                                       {"query_param", QByteArray(query_param.dump().data()).toBase64().toStdString()}
//                                   });
//        }
        if(m_models.find(CertUsers) != m_models.end()){
            m_models[CertUsers]->fetchRoot("CertUsers");
            QVector<QString> m_hide{"ref", "parent", "is_group", "deletion_mark", "cache", "sid", "uuid"};
            foreach (auto const& itr, m_hide) {
                auto index = m_models[CertUsers]->get_column_index(itr);
                if(index != -1)
                    ui->treeView->hideColumn(index);
            }
        }
        ui->treeView->resizeColumnToContents(0);
    }else if(key == QString(arcirk::enum_synonym(Containers).data())){
        if(m_client->isConnected()){
            auto values_ = pre::json::to_json(containers());
            values_.erase("data");
            auto values = arcirk::json_keys(values_);
            auto query_param = nlohmann::json{
                {"table_name", arcirk::enum_synonym(Containers)},
                {"where_values", nlohmann::json{}},
                {"query_type", "select"},
                {"empty_column", false},
                {"line_number", false},
                {"values", json{
                        "first",
                        "ref",
                        "parent_user",
                        "subject",
                        "issuer",
                        "not_valid_before",
                        "not_valid_after",
                        "cache"
                    }
                }
            };
            m_client->send_command(arcirk::server::server_commands::ExecuteSqlQuery, nlohmann::json{
                                       {"query_param", QByteArray(query_param.dump().data()).toBase64().toStdString()}
                                   });
        }
    }else if(key == QString(arcirk::enum_synonym(LocalhostUserCertificates).data())){
        auto certs = current_user->getCertificates(true);
        if(certs.is_object() && !certs.empty()){
            resetModel(LocalhostUserCertificates, certs);
        }
    }else if(key == QString(arcirk::enum_synonym(LocalhostUserContainers).data())){
        auto conts = current_user->getContainers();
        if(conts.is_object() && !conts.empty()){
            resetModel(LocalhostUserContainers, conts);
        }
    }

    update_columns();
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
    using json = nlohmann::json;

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
                model->set_icon(model->index(i,0,parent), app_icons[PriceChecker].first);
            }else if(type_app == ServerManager){
                model->set_icon(model->index(i,0,parent), app_icons[ServerManager].first);
            }else if(type_app == ExtendedLib){
                model->set_icon(model->index(i,0,parent), app_icons[ExtendedLib].first);
            }
        }
    }else if(key == LocalhostUserCertificates){
        model->set_rows_icon(QIcon(":/img/cert16NoKey.png"));
    }else if(key == arcirk::server::LocalhostUserContainers){
        using namespace arcirk::cryptography;
        int ind = model->get_column_index("type");
        auto parent = QModelIndex();
        for (int i = 0; i < model->rowCount(parent); ++i) {
            json vol = model->index(i, ind, parent).data(Qt::DisplayRole).toString().trimmed().toStdString();
            auto volume = vol.get<TypeOfStorgare>();
            if(volume == TypeOfStorgare::storgareTypeRegistry)
                model->set_icon(model->index(i,0,parent), QIcon(":/img/registry16.png"));
            else if(volume == TypeOfStorgare::storgareTypeLocalVolume)
                model->set_icon(model->index(i,0,parent), QIcon(":/img/desktop.png"));
        }
    }
}

void MainWindow::insert_container(CryptContainer &cnt)
{

    Q_ASSERT(cnt.isValid());
    auto org_name = cnt.originalName();
    auto volume = cnt.get_volume();
    auto info = cnt.info(volume + org_name);
    auto cnt_info_ = arcirk::database::containers();
    //auto struct_certs = arcirk::database::certificates();
    cnt_info_.cache = info.dump();
    cnt_info_._id = 0;
    cnt_info_.deletion_mark = 0;
    cnt_info_.is_group = 0;
    cnt_info_.first = org_name.toStdString();
    cnt_info_.not_valid_after = info["Not valid after"];
    cnt_info_.not_valid_before = info["Not valid before"];
    cnt_info_.version = 0;
    cnt_info_.ref = QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
    cnt_info_.parent = arcirk::uuids::nil_string_uuid();
    cnt_info_.parent_user = info["ParentUser"];
    cnt_info_.issuer = info["Issuer"];
    cnt_info_.subject = info["Subject"];

    nlohmann::json query_param = {
        {"table_name", arcirk::enum_synonym(tables::tbContainers)},
        {"query_type", "insert"},
        {"values", pre::json::to_json(cnt_info_)}
    };
    std::string base64_param = QByteArray::fromStdString(query_param.dump()).toBase64().toStdString();
    auto resp = m_client->exec_http_query(arcirk::enum_synonym(arcirk::server::server_commands::ExecuteSqlQuery), json{
                                         {"query_param", base64_param}
                                     });
    if(resp  != "error"){
        //обновим данные
        query_param = {
            {"table_name", arcirk::enum_synonym(tables::tbContainers)},
            {"where_values", json{
                 {"ref", cnt_info_.ref}
             }}
        };
        base64_param = QByteArray::fromStdString(query_param.dump()).toBase64().toStdString();
        json param{
            {"destantion", base64_param},
            {"file_name", org_name.toStdString() + ".bin"}
        };
        auto ba = cnt.to_byate_array();
        resp = m_client->exec_http_query(arcirk::enum_synonym(arcirk::server::server_commands::DownloadFile), param, ba);
        if(resp  != "error"){
            auto query_param = nlohmann::json{
                {"table_name", arcirk::enum_synonym(tables::tbContainers)},
                {"where_values", nlohmann::json{}},
                {"query_type", "select"},
                {"empty_column", false},
                {"line_number", false},
                {"values", json{
                        "first",
                        "ref",
                        "parent_user",
                        "subject",
                        "issuer",
                        "not_valid_before",
                        "not_valid_after",
                        "cache"
                    }
                }
            };
            m_client->send_command(arcirk::server::server_commands::ExecuteSqlQuery, nlohmann::json{
                                       {"query_param", QByteArray(query_param.dump().data()).toBase64().toStdString()}
                                   });
        }
    }
}

void MainWindow::update_columns()
{
    auto model = (TreeViewModel*)ui->treeView->model();
    auto table = ui->treeView;

    QList<QString> m_lst{"_id", "ref", "version", "is_group", "deletion_mark", "data", "cache", "parent"};

    foreach (auto it, m_lst) {
        auto ind = model->get_column_index(it);
        if(ind != -1)
            table->hideColumn(ind);
    }
}

void MainWindow::edit_cert_user(const QModelIndex &index)
{

    if(!index.isValid()){
        QMessageBox::critical(this, "Ошибка", "Не выбран элемент!");
        return;
    }

    auto model = (TreeViewModel*)ui->treeView->model();
    using namespace arcirk::database;
    using json = nlohmann::json;
    using namespace arcirk::server;


    auto object = model->get_object(index);
    auto query = builder::query_builder();

    std::string query_text = query.select().from("CertUsers").where(nlohmann::json{
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

    auto struct_users = pre::json::from_json<arcirk::database::cert_users>(rows[0]);
    auto parent = index.parent();
    QString parentName;
    if(parent.isValid()){
        parentName = parent.data().toString();
    }

    auto dlg = DialogEdit(object, parentName, this);
    dlg.setModal(true);
    dlg.exec();

    if(dlg.result() == QDialog::Accepted){
        struct_users.first = object["first"];
        struct_users.second = object["second"];
        nlohmann::json query_param = {
            {"table_name", arcirk::enum_synonym(tables::tbCertUsers)},
            {"query_type", "update"},
            {"values", pre::json::to_json(struct_users)},
            {"where_values", nlohmann::json({
                 {"ref", struct_users.ref}
             })}
        };

        std::string base64_param = QByteArray::fromStdString(query_param.dump()).toBase64().toStdString();
        auto resp = m_client->exec_http_query(arcirk::enum_synonym(server_commands::ExecuteSqlQuery), json{
                                             {"query_param", base64_param}
                                         });
        if(resp.is_string()){
            if(resp.get<std::string>() == "success"){
                model->set_object(index ,pre::json::to_json(struct_users));
                update_icons(server_objects::CertUsers, model);
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
            QString data = index.data().toString();
            if(value == data)
                return index;
        }
    }

    return QModelIndex();
}

void MainWindow::on_btnEdit_clicked()
{
    auto index = ui->treeView->currentIndex();

    if(!index.isValid()){
        QMessageBox::critical(this, "Ошибка", "Не выбран элемент!");
        return;
    }

    auto model = (TreeViewModel*)ui->treeView->model();
    if(model->server_object() == arcirk::server::CertUsers){
        edit_cert_user(index);
    }

}


void MainWindow::on_treeView_doubleClicked(const QModelIndex &index)
{
    if(!index.isValid()){
        QMessageBox::critical(this, "Ошибка", "Не выбран элемент!");
        return;
    }

    auto model = (TreeViewModel*)ui->treeView->model();
    using namespace arcirk::database;
    using json = nlohmann::json;
    using namespace arcirk::server;

    try {
        if(model->server_object() == arcirk::server::DatabaseUsers){
            auto object = model->get_object(index);
            if(object["is_group"].get<int>() == 1){
                //QMessageBox::critical(this, "Ошибка", "Редактирование групп запрещено!");
                return;
            }

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

            if(dlg.result() == QDialog::Accepted){
                nlohmann::json query_param = {
                    {"table_name", arcirk::enum_synonym(tables::tbUsers)},
                    {"query_type", "update"},
                    {"values", pre::json::to_json(struct_users)},
                    {"where_values", nlohmann::json({
                         {"ref", struct_users.ref}
                     })}
                };

                std::string base64_param = QByteArray::fromStdString(query_param.dump()).toBase64().toStdString();
                auto resp = m_client->exec_http_query(arcirk::enum_synonym(server_commands::ExecuteSqlQuery), json{
                                                     {"query_param", base64_param}
                                                 });
                if(resp.is_string()){
                    if(resp.get<std::string>() == "success"){
                        model->set_object(index ,pre::json::to_json(struct_users));
                        update_icons(server_objects::DatabaseUsers, model);
                    }
                }
            }

        }else if(model->server_object() == arcirk::server::Devices){

            auto ref = model->data(model->index(index.row(), model->get_column_index("ref"), index.parent()), Qt::DisplayRole).toString();
            json param{
                {"device", ref.toStdString()}
            };
            //m_client->send_command(server_commands::DeviceGetFullInfo, param);
            auto resp = m_client->exec_http_query(arcirk::enum_synonym(server_commands::DeviceGetFullInfo), param);
            if(resp.is_object()){
                auto dlg = DialogDevice(resp, this);
                dlg.setModal(true);
                dlg.exec();

                if(dlg.result() == QDialog::Accepted){
                    auto dev = dlg.get_result();
                    nlohmann::json query_param = {
                        {"table_name", "Devices"},
                        {"query_type", "update"},
                        {"values", pre::json::to_json(dev)},
                        {"where_values", nlohmann::json({
                             {"ref", ref.toStdString()}
                         })}
                    };

                    std::string base64_param = QByteArray::fromStdString(query_param.dump()).toBase64().toStdString();
                    resp = m_client->exec_http_query(arcirk::enum_synonym(server_commands::ExecuteSqlQuery), json{
                                                         {"query_param", base64_param}
                                                     });
                    if(resp.is_string()){
                        if(resp.get<std::string>() == "success"){
                            model->set_object(index ,pre::json::to_json(dlg.get_view_result()));
                            update_icons(server_objects::Devices, model);
                        }
                    }
                }
            }
        }else if(model->server_object() == arcirk::server::Services){
            auto obj = pre::json::from_json<arcirk::services::task_options>(model->get_object(index));
            auto dlg = DialogTask(obj, this);
            dlg.setModal(true);
            dlg.exec();
            if(dlg.result() == QDialog::Accepted){
                model->set_object(index, pre::json::to_json(obj));
                auto arr_service = model->get_objects(index.parent());
                json param{};
                param["task_options"] = arr_service;
                m_client->send_command(server_commands::UpdateTaskOptions, param);
            }
        }else if(model->server_object() == arcirk::server::CertUsers){
            auto is_group = model->data(model->index(index.row(), model->get_column_index("is_group"), index.parent()), Qt::DisplayRole).toInt();
            if(is_group == 1)
                return;
            auto obj = model->get_object(index);
            auto struct_user = pre::json::from_json<arcirk::database::cert_users>(obj);
            m_models[DatabaseUsers]->fetchRoot("Users");

            auto types = json::array({enum_synonym(device_types::devDesktop), enum_synonym(device_types::devServer)});
            json query_param = {
                {"table_name", arcirk::enum_synonym(tables::tbDevices)},
                {"query_type", "select"},
                {"values", json{"first", "ref"}},
                {"where_values", json{{"deviceType", types}}}
            };
            std::string base64_param = QByteArray::fromStdString(query_param.dump()).toBase64().toStdString();
            auto dev = m_client->exec_http_query(arcirk::enum_synonym(arcirk::server::server_commands::ExecuteSqlQuery), json{
                                                 {"query_param", base64_param}
                                             });
            auto rows = dev.value("rows", json::array());
            auto dlg = DialogCertUserCache(struct_user, m_models[server_objects::DatabaseUsers], rows, this);
            dlg.setModal(true);
            dlg.exec();
            if(dlg.result() == QDialog::Accepted){
                auto query_param = nlohmann::json{
                    {"table_name", enum_synonym(tables::tbCertUsers)},
                    {"where_values", nlohmann::json{{"ref", struct_user.ref}}},
                    {"values", pre::json::to_json(struct_user)},
                    {"query_type", "update"}
                };

                m_client->send_command(arcirk::server::server_commands::ExecuteSqlQuery, nlohmann::json{
                                           {"query_param", QByteArray(query_param.dump().data()).toBase64().toStdString()}
                                       });
            }
        }
    } catch (const std::exception& e) {
        qCritical() << __FUNCTION__ << e.what();
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
        auto model_ = new TreeViewModel(m_client->conf(), this);
        model_->set_column_aliases(m_colAliases);
        model_->set_server_object(arcirk::server::ProfileDirectory);
        model_->fetchRoot("ProfileDirectory", "shared_files/files");
        delete model_;
    }
}


void MainWindow::on_btnAdd_clicked()
{
    auto model = (TreeViewModel*)ui->treeView->model();

    using namespace arcirk::server;
    using json = nlohmann::json;

    if(model->server_object() == arcirk::server::ProfileDirectory){
        auto index = ui->treeView->currentIndex();
        if(!index.isValid()){
            QMessageBox::critical(this, "Ошибка", "Не выбран элемент!");
            return;
        }
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
                        {"destantion", destantion.toUtf8().toBase64().toStdString()},
                        {"file_name", file_name.toStdString()}//,
                        //{"data", data}
                    };
                    auto resp = m_client->exec_http_query(arcirk::enum_synonym(server_commands::DownloadFile), param, data);
                    if(resp.is_object()){
                        int is_folder = model->index(index.row(), model->get_column_index("is_group"), index.parent()).data().toInt();
                        if(is_folder == 0)
                            model->add(resp,  index.parent());
                        else
                            model->add(resp, index);
                    }
                }

            } catch (const std::exception& e) {
                QMessageBox::critical(this, "Ошибка", e.what());
                return;
            }

        }
    }else if(model->server_object() == arcirk::server::Certificates){
        auto result = QFileDialog::getOpenFileName(this, "Выбрать файл на диске", "", tr("Файлы сетрификатов (*.cer *.crt)"));
        if(!result.isEmpty()){
            //ByteArray data{};
            try {
                auto cert = CryptCertificate(this);
                cert.fromFile(result);
                if(cert.isValid()){
                    ByteArray ba = cert.getData().data;
                    if(ba.size() > 0){
                        //сначала запись добавим
                        auto struct_certs = arcirk::database::certificates();
                        struct_certs._id = 0;
                        struct_certs.suffix = cert.getData().suffix;
                        struct_certs.deletion_mark = 0;
                        struct_certs.is_group = 0;
                        struct_certs.first = cert.sinonym();
                        struct_certs.not_valid_after = cert.getData().not_valid_after;
                        struct_certs.not_valid_before = cert.getData().not_valid_before;
                        struct_certs.serial = cert.getData().serial;
                        struct_certs.version = 0;
                        struct_certs.ref = QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
                        struct_certs.parent = arcirk::uuids::nil_string_uuid();
                        struct_certs.parent_user = cert.parent();
                        struct_certs.issuer = cert.issuer_briefly();
                        struct_certs.subject = cert.subject_briefly();
                        struct_certs.cache = cert.getData().cache;
                        struct_certs.sha1 = cert.sha1();

                        nlohmann::json query_param = {
                            {"table_name", arcirk::enum_synonym(tables::tbCertificates)},
                            {"query_type", "insert"},
                            {"values", pre::json::to_json(struct_certs)}
                        };
                        std::string base64_param = QByteArray::fromStdString(query_param.dump()).toBase64().toStdString();
                        auto resp = m_client->exec_http_query(arcirk::enum_synonym(server_commands::ExecuteSqlQuery), json{
                                                             {"query_param", base64_param}
                                                         });

                        if(resp  != "error"){
                            //обновим данные
                            query_param = {
                                {"table_name", arcirk::enum_synonym(tables::tbCertificates)},
                                {"where_values", json{
                                     {"ref", struct_certs.ref}
                                 }}
                            };
                            base64_param = QByteArray::fromStdString(query_param.dump()).toBase64().toStdString();
                            json param{
                                {"destantion", base64_param},
                                {"file_name", result.toStdString()}
                            };
                            resp = m_client->exec_http_query(arcirk::enum_synonym(server_commands::DownloadFile), param, ba);
                            if(resp  != "error"){
                                auto query_param = nlohmann::json{
                                    {"table_name", "Certificates"},
                                    {"where_values", nlohmann::json{}},
                                    {"query_type", "select"},
                                    {"empty_column", false},
                                    {"line_number", false},
                                    {"values", json{
                                            "first",
                                            "ref",
                                            "private_key",
                                            "subject",
                                            "issuer",
                                            "not_valid_before",
                                            "not_valid_after",
                                            "parent_user",
                                            "serial",
                                            "sha1",
                                            "cache"
                                        }
                                    }
                                };
                                m_client->send_command(arcirk::server::server_commands::ExecuteSqlQuery, nlohmann::json{
                                                           {"query_param", QByteArray(query_param.dump().data()).toBase64().toStdString()}
                                                       });
                            }
                        }
                    }
                }
            } catch (const std::exception& e) {
                QMessageBox::critical(this, "Ошибка", e.what());
                return;
            }

        }
    }else if(model->server_object() == arcirk::server::Containers){

        using namespace arcirk::cryptography;

        QStringList select = {
            "Добавить из каталога",
            "Добавить из устройства"
        };

        auto dlg = DialogSelectDevice(this, select, "Добавить контейнер");
        dlg.setModal(true);
        dlg.exec();

        if(dlg.result() != QDialog::Accepted){
            return;
        }

        int selection = dlg.currentSelection();

        if(selection == 0){

            QString dir = QFileDialog::getExistingDirectory(this, tr("Выбрать каталог"),
                                                         QDir::homePath(),
                                                         QFileDialog::ShowDirsOnly
                                                         | QFileDialog::DontResolveSymlinks);
            if(dir != ""){
//                QFile file(dir + QDir::separator() + "name.key");
//                if(file.open(QIODevice::ReadOnly)){
//                    QString data = QString::fromLocal8Bit(file.readAll());
//                    int ind = data.indexOf("\026");
//                    if(ind == -1){
//                        QMessageBox::critical(this, "Ошибка", "В выбранном каталоге не найдены данные контейнера!");
//                        delete cnt;
//                        return;
//                    }
//                }else{
//                    QMessageBox::critical(this, "Ошибка", "В выбранном каталоге не найдены данные контейнера!");
//                    delete cnt;
//                    return;
//                }

//                container = dir;
//                cnt->fromContainerName(dir);
            }
        }else if(selection == 2){

        }else{
            auto model_loc_cnt = m_models[arcirk::server::LocalhostUserContainers];
            if(model_loc_cnt->rowCount(QModelIndex()) == 0){
                auto model_json = current_user->getContainers();
                resetModel(arcirk::server::LocalhostUserContainers, model_json);
            }

            auto dlgSel = new DialogSelectInList(model_loc_cnt, "Выбор устройства", this);
            dlgSel->setModal(true);
            dlgSel->exec();
            if(dlgSel->result() == QDialog::Accepted){
                QStringList dlgResult = dlgSel->dialogResult();
                auto m_type = json(dlgResult[2].toStdString()).get<TypeOfStorgare>();
                if(m_type == TypeOfStorgare::storgareTypeLocalVolume){
                    auto vol = CryptContainer::get_local_volume(dlgResult[1]);
                    Q_ASSERT(!vol.isEmpty());
                    auto index = dlgResult[0].indexOf("@");
                    Q_ASSERT(index!=-1);
                    auto code = dlgResult[0].left(index);
                    QDirIterator it(vol, {code + ".*"}, QDir::Dirs);
                    //Ищем все каталоги с именем начинающимся с code
                    QString dir{};
                    auto cnt = CryptContainer(this);
                    cnt.set_volume(dlgResult[1]);
                    while (it.hasNext()) {
                        dir = it.next();
                        qDebug() << dir;
                        //получаем имя контейнера из файла name.key
                        auto org_name = CryptContainer::get_local_original_name(dir);
                        if(org_name.isEmpty()){
                            dir = "";
                            continue;
                        }else{
                            if(org_name == dlgResult[0]){
                                cnt.from_dir(dir);
                                insert_container(cnt);
                            }
                        }
                    }
                }else if(m_type == TypeOfStorgare::storgareTypeRegistry){
                    if(current_user->getInfo().sid.empty())
                        return;
                    auto cnt = CryptContainer(this);
                    cnt.set_volume(dlgResult[1]);
                    cnt.from_registry(current_user->getInfo().sid.c_str(), dlgResult[0]);
                    insert_container(cnt);
                }

            }
        }

    }else if(model->server_object() == arcirk::server::CertUsers){
        auto index = ui->treeView->currentIndex();
        QModelIndex current_parent{};
        auto user_struct = arcirk::database::table_default_struct<arcirk::database::cert_users>(arcirk::database::tbCertUsers);
        if(index.isValid()){

            auto parent_object = model->get_object(index);
            current_parent = index;

            if(parent_object["is_group"] == 0){
                auto parent = index.parent();                
                if(!parent.isValid())
                {
                    QMessageBox::critical(this, "Ошибка", "Выберете группу!");
                    return;
                }
                parent_object = model->get_object(parent);
                current_parent = parent;
            }

            std::string parent_name = parent_object["first"];

            auto struct_users = arcirk::database::table_default_json(arcirk::database::tables::tbCertUsers);
            struct_users["parent"] = parent_object["ref"];
            struct_users["ref"] = QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
            struct_users["is_group"] = 0;
            m_models[DatabaseUsers]->fetchRoot("Users");
            auto dlg = DialogEdit(struct_users, parent_name.c_str(), m_models[arcirk::server::server_objects::DatabaseUsers], this);
            dlg.setModal(true);
            dlg.exec();
            if(dlg.result() == QDialog::Accepted){
                nlohmann::json query_param = {
                    {"table_name", "CertUsers"},
                    {"query_type", "insert"},
                    {"values", struct_users},
                    {"where_values", {}}
                };

                std::string base64_param = QByteArray::fromStdString(query_param.dump()).toBase64().toStdString();
                auto resp = m_client->exec_http_query(arcirk::enum_synonym(server_commands::ExecuteSqlQuery), json{
                                                     {"query_param", base64_param}
                                                 });
                if(resp.is_string()){
                    if(resp.get<std::string>() == "success"){
                        model->add(struct_users, current_parent);
                    }
                }
            }
        }else
            QMessageBox::critical(this, "Ошибка", "Выберете группу!");
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
    if(model->server_object() == server_objects::ProfileDirectory){

        int ind = model->get_column_index("path");
        auto file_path = model->index(index.row(), ind, index.parent()).data().toString();
        auto file_name = model->index(index.row(), 0, index.parent()).data().toString();
        auto result = QMessageBox::question(this, "Удаление файла", QString("Удалить файл %1").arg(file_name));

        if(result == QMessageBox::Yes) {
            json param{
                    {"file_name", file_path.toStdString()}
            };
            auto command = arcirk::enum_synonym(arcirk::server::server_commands::ProfileDeleteFile);
            auto resp = m_client->exec_http_query(command, param);
            if (resp.get<std::string>() == "success")
                model->remove(index);
        }
    }else if(model->server_object() == server_objects::DatabaseUsers){
        int ind = model->get_column_index("first");
        int indRef = model->get_column_index("ref");
        auto name = model->index(index.row(), ind, index.parent()).data().toString();
        auto ref = model->index(index.row(), indRef, index.parent()).data().toString();
        auto result = QMessageBox::question(this, "Удаление пользователя или группы", QString("Удалить пользователя или группу пользователей %1").arg(name));

        if(result == QMessageBox::Yes){
            using namespace arcirk::database::builder;
            auto command = arcirk::enum_synonym(arcirk::server::server_commands::ExecuteSqlQuery);

            json query_param = {
                {"query_text", QString("DELETE FROM Users WHERE ref = '%1' OR parent = '%1';").arg(ref).toStdString()}
            };
            std::string base64_param = QByteArray::fromStdString(query_param.dump()).toBase64().toStdString();
            auto resp = m_client->exec_http_query(command, query_param);
            model->remove(index);

        }
    }else if(model->server_object() == server_objects::Devices){
        int ind = model->get_column_index("first");
        int indRef = model->get_column_index("ref");
        auto name = model->index(index.row(), ind, index.parent()).data().toString();
        auto ref = model->index(index.row(), indRef, index.parent()).data().toString();
        auto result = QMessageBox::question(this, "Удаление устройства", QString("Удалить устройство %1").arg(name));

        if(result == QMessageBox::Yes){
            using namespace arcirk::database::builder;
            auto command = arcirk::enum_synonym(arcirk::server::server_commands::ExecuteSqlQuery);

            json query_param = {
                {"query_text", QString("DELETE FROM Devices WHERE ref = '%1';").arg(ref).toStdString()}
            };
            std::string base64_param = QByteArray::fromStdString(query_param.dump()).toBase64().toStdString();
            auto resp = m_client->exec_http_query(command, query_param);
            model->remove(index);

        }
    }else if(model->server_object() == server_objects::Certificates){
        int ind = model->get_column_index("first");
        int indRef = model->get_column_index("ref");
        auto name = model->index(index.row(), ind, index.parent()).data().toString();
        auto ref = model->index(index.row(), indRef, index.parent()).data().toString();
        auto result = QMessageBox::question(this, "Удаление сертификата", QString("Удалить сертификат %1").arg(name));

        if(result == QMessageBox::Yes){
            using namespace arcirk::database::builder;
            auto command = arcirk::enum_synonym(arcirk::server::server_commands::ExecuteSqlQuery);

            json query_param = {
                {"query_text", QString("DELETE FROM Certificates WHERE ref = '%1';").arg(ref).toStdString()}
            };
            std::string base64_param = QByteArray::fromStdString(query_param.dump()).toBase64().toStdString();
            auto resp = m_client->exec_http_query(command, query_param);
            model->remove(index);

        }
    }else if(model->server_object() == server_objects::Containers){
        int ind = model->get_column_index("first");
        int indRef = model->get_column_index("ref");
        auto name = model->index(index.row(), ind, index.parent()).data().toString();
        auto ref = model->index(index.row(), indRef, index.parent()).data().toString();
        auto result = QMessageBox::question(this, "Удаление контейнера", QString("Удалить контейнер %1").arg(name));

        if(result == QMessageBox::Yes){
            using namespace arcirk::database::builder;
            auto command = arcirk::enum_synonym(arcirk::server::server_commands::ExecuteSqlQuery);

            json query_param = {
                {"query_text", QString("DELETE FROM Containers WHERE ref = '%1';").arg(ref).toStdString()}
            };
            std::string base64_param = QByteArray::fromStdString(query_param.dump()).toBase64().toStdString();
            auto resp = m_client->exec_http_query(command, query_param);
            model->remove(index);

        }
    }else if(model->server_object() == server_objects::CertUsers){
        int ind = model->get_column_index("first");
        int indRef = model->get_column_index("ref");
        int ind_is_group = model->get_column_index("is_group");
        auto name = model->index(index.row(), ind, index.parent()).data().toString();
        auto ref = model->index(index.row(), indRef, index.parent()).data().toString();
        auto is_group = model->index(index.row(), ind_is_group, index.parent()).data().toInt();
        auto result = QMessageBox::question(this, "Удаление пользователя или группы", QString("Удалить  %1").arg(name));

        if(result == QMessageBox::Yes){
            using namespace arcirk::database::builder;
            auto command = arcirk::enum_synonym(arcirk::server::server_commands::ExecuteSqlQuery);

            QString query_text;
            if(is_group == 0)
                query_text  = QString("DELETE FROM CertUsers WHERE ref = '%1';").arg(ref);
            else
                query_text  = QString("DELETE FROM CertUsers WHERE ref = '%1' or parent = '%2';").arg(ref).arg(ref);

            json query_param = {
                {"query_text", query_text.arg(ref).toStdString()}
            };
            std::string base64_param = QByteArray::fromStdString(query_param.dump()).toBase64().toStdString();
            auto resp = m_client->exec_http_query(command, query_param);
            model->remove(index);

        }
    }
}
void MainWindow::on_btnSetLinkDevice_clicked()
{
    auto model = (TreeViewModel*)ui->treeView->model();
    auto index = ui->treeView->currentIndex();
    if(!index.isValid()){
        QMessageBox::critical(this, "Ошибка", "Не выбран элемент!");
        return;
    }
    using namespace arcirk::server;
    using json = nlohmann::json;
    if(model->server_object() == arcirk::server::OnlineUsers){
        auto dlg = DialogSelectInList(m_models[Devices], "Выбор устройства", this);
        dlg.setModal(true);
        dlg.exec();
        if(dlg.result() == QDialog::Accepted){
            auto res = dlg.dialogResult();
            auto ind = m_models[Devices]->get_column_index("ref");
            auto indUuid = model->get_column_index("session_uuid");
            auto ref = res[ind];
            auto uuid = model->data(model->index(index.row(), indUuid, index.parent()), Qt::DisplayRole).toString();
            json param{
                    {"remote_session", uuid.toStdString()},
                    {"device_id", ref.toStdString()}
            };
            m_client->send_command(arcirk::server::server_commands::SetNewDeviceId, param);
        }
    }
}


void MainWindow::on_mnuAbout_triggered()
{
    auto dlg = DialogAbout(this);
    dlg.setModal(true);
    dlg.exec();
}


void MainWindow::on_mnuOptions_triggered()
{
    if(!m_client->isConnected())
        return;
    auto result_http = m_client->exec_http_query(arcirk::enum_synonym(arcirk::server::server_commands::ServerConfiguration), nlohmann::json{});
    auto conf = pre::json::from_json<arcirk::server::server_config>(result_http);
    auto dlg = DialogSeverSettings(conf, m_client->conf(), this);
    dlg.setModal(true);
    dlg.exec();
    if(dlg.result() == QDialog::Accepted){

        m_client->write_conf();
        auto result = dlg.getResult();
        m_client->send_command(arcirk::server::server_commands::UpdateServerConfiguration, nlohmann::json{
                                   {"config", pre::json::to_json(result)}
                               });

    }
}

void MainWindow::on_btnTaskRestart_clicked()
{
    auto model = (TreeViewModel*)ui->treeView->model();
    if(model->server_object() == arcirk::server::Services){
        m_client->send_command(arcirk::server::server_commands::TasksRestart, nlohmann::json{});
    }
}


void MainWindow::on_btnStartTask_clicked()
{
    auto model = (TreeViewModel*)ui->treeView->model();
    auto index = ui->treeView->currentIndex();
    if(!index.isValid()){
        QMessageBox::critical(this, "Ошибка", "Не выбран элемент!");
        return;
    }
    if(QMessageBox::question(this, "Выполнить задачу", "Выполнить текущую задачу,") == QMessageBox::No)
        return;

    auto indUuid = model->get_column_index("uuid");
    auto uuid = model->data(model->index(index.row(), indUuid, index.parent()), Qt::DisplayRole).toString();

    m_client->send_command(arcirk::server::server_commands::RunTask, nlohmann::json{
                               {"task_uuid", uuid.toStdString()},
                               {"custom_interval", 0}
                           });
}


void MainWindow::on_btnSendClientRelease_clicked()
{
    auto model = (TreeViewModel*)ui->treeView->model();
    auto index = ui->treeView->currentIndex();
    if(!index.isValid()){
        QMessageBox::critical(this, "Ошибка", "Не выбран элемент!");
        return;
    }

    if(model->server_object() == arcirk::server::ProfileDirectory){

        QString result = QString::fromStdString(m_client->conf().price_checker_repo) + "/android-build-release-signed.apk";
        if(!QFile(result).exists())
        {
            QMessageBox::critical(this, "Ошибка", "Не верный путь к репозиторию!");
            return;
        }else{
            if(QMessageBox::question(this, "Загрузка версии", "Загузить новую версию мобильного приложения?") == QMessageBox::No)
                return;
        }
        if(!result.isEmpty()){
            ByteArray data{};
            try {
                arcirk::read_file(result.toStdString(), data);
                if(data.size() > 0){
                    auto destantion = model->current_parent_path();
                    //QUrl url(result);
                    //auto file_name = url.fileName();
                    auto ver = WebSocketClient::get_version();
                    json param{
                        {"destantion", destantion.toUtf8().toBase64().toStdString()},
                        {"file_name", QString("checker_v%1_%2_%3.apk").arg(QString::number(ver.major)).arg( QString::number(ver.major)).arg( QString::number(CLIENT_VERSION)).toStdString()}
                        //,
                        //{"data", data}
                    };
                    auto resp = m_client->exec_http_query(arcirk::enum_synonym(arcirk::server::server_commands::DownloadFile), param, data);
                        if(resp.is_object()){
                            //auto resp_ = json::parse(QByteArray::fromBase64(resp.result.data()).toStdString());
                                int is_folder = model->index(index.row(), model->get_column_index("is_group"), index.parent()).data().toInt();
                                if(is_folder == 0)
                                    model->add(resp,  index.parent());
                                else
                                    model->add(resp, index);
                       }
                }

            } catch (const std::exception& e) {
                QMessageBox::critical(this, "Ошибка", e.what());
                return;
            }

        }
    }else if(model->server_object() == arcirk::server::Certificates){
        auto ind = model->get_column_index("ref");
        auto ind_suffix = model->get_column_index("suffix");
        auto ind_first = model->get_column_index("first");
        if(ind == -1 || ind_suffix == -1 || ind_first == -1)
            return;
        auto ref = model->index(index.row(), ind, index.parent()).data(Qt::DisplayRole).toString();
        auto suffix = model->index(index.row(), ind_suffix, index.parent()).data(Qt::DisplayRole).toString();
        auto first = model->index(index.row(), ind_first, index.parent()).data(Qt::DisplayRole).toString();
        if(ref.isEmpty() && suffix.isEmpty())
            return;
        using json = nlohmann::json;
        auto destantion = QFileDialog::getSaveFileName(this, "Сохранить как", QStandardPaths::writableLocation(QStandardPaths::HomeLocation), "Файлы сертификаторв (*." + suffix + ")");
        if(!destantion.isEmpty()){
            auto file_name = first + "." + suffix;
            json query_param{
                {"table_name", arcirk::enum_synonym(tables::tbCertificates)},
                {"where_values", json{
                        {"ref", ref.toStdString()}
                    }}
            };
            json param{
                {"file_name", file_name.toStdString()},
                {"destantion", QByteArray(query_param.dump().data()).toBase64().toStdString()}
            };

            auto ba = m_client->exec_http_query_get("GetBlob", param);

            if(ba.isEmpty())
                return;

            QFile file(destantion);
            if(file.open(QIODevice::WriteOnly)){
                file.write(ba);
                file.close();
            }

        }

    }else if(model->server_object() == arcirk::server::Containers){
        auto ind = model->get_column_index("ref");
        auto ind_first = model->get_column_index("first");
        if(ind == -1 || ind_first == -1)
            return;
        auto ref = model->index(index.row(), ind, index.parent()).data(Qt::DisplayRole).toString();
        auto first = model->index(index.row(), ind_first, index.parent()).data(Qt::DisplayRole).toString();

        if(ref.isEmpty())
            return;

        auto m_name = first.split("@");
        Q_ASSERT(m_name.size() == 2);

        QString def_folder = m_name[0];

        using json = nlohmann::json;
        auto destantion = QFileDialog::getExistingDirectory(this, "Выгрузить контейнер", QStandardPaths::writableLocation(QStandardPaths::HomeLocation));
        if(!destantion.isEmpty()){

            json query_param{
                {"table_name", arcirk::enum_synonym(tables::tbContainers)},
                {"where_values", json{
                        {"ref", ref.toStdString()}
                    }}
            };
            json param{
                {"file_name", m_name[0].toStdString() + ".bin"},
                {"destantion", QByteArray(query_param.dump().data()).toBase64().toStdString()}
            };

            auto ba = m_client->exec_http_query_get("GetBlob", param);

            if(ba.isEmpty())
                return;

            QTemporaryFile file;
            if(file.open()){
                file.write(ba);
                file.close();
            }

            auto cnt = CryptContainer(this);
            cnt.from_file(file.fileName().toStdString());
            Q_ASSERT(cnt.isValid());

            bool exists = true;
            int max = 999;
            int count = 0;
            QString result;
            while (exists) {
                auto suffix = std::format("{:03}", count);
                if(count >= max)
                    break;
                QDir dir(destantion + "/" + def_folder + "." + suffix.c_str());
                if(!dir.exists()){
                    exists = false;
                    result = dir.path();
                }
            }

            if(!exists){
                cnt.to_dir(result);
            }

        }

    }else if(model->server_object() == arcirk::server::LocalhostUserCertificates){
        QString fileName = QFileDialog::getSaveFileName(this, tr("Сохранить как"),
                                                     QDir::homePath(),
                                                     "Файл сертификата (*.cer)");
        if(fileName != ""){
            int ind = model->get_column_index("sha1");
            QString sha1 = model->index(index.row(), ind, index.parent()).data(Qt::DisplayRole).toString();
            if(!sha1.isEmpty()){
                CryptCertificate::save_as(sha1, fileName, this);
            }
        }
    }
}


void MainWindow::on_btnSendoToClient_clicked()
{
//    auto model = (TreeViewModel*)ui->treeView->model();
//    auto index = ui->treeView->currentIndex();
//    if(!index.isValid()){
//        QMessageBox::critical(this, "Ошибка", "Не выбран элемент!");
//        return;
//    }
//    if(model->server_object() == arcirk::server::OnlineUsers){
//        auto ind = model->get_column_index("session_uuid");
//        auto rec = model->data(model->index(index.row(), ind, index.parent()), Qt::DisplayRole).toString();
//        m_client->command_to_client(rec.toStdString(), "GetForms");
//    }
}


void MainWindow::on_btnInfo_clicked()
{
    auto model = (TreeViewModel*)ui->treeView->model();
    auto index = ui->treeView->currentIndex();
    if(!index.isValid()){
        QMessageBox::critical(this, "Ошибка", "Не выбран элемент!");
        return;
    }

    using json = nlohmann::json;

    if(model->server_object() == arcirk::server::Containers
            || model->server_object() == arcirk::server::Certificates
            || model->server_object() == arcirk::server::LocalhostUserCertificates){
        auto ind = model->get_column_index("cache");
        auto ind_first = model->get_column_index("first");
        if(ind == -1 || ind_first == -1)
            return;
        auto cache = model->index(index.row(), ind, index.parent()).data(Qt::DisplayRole).toString().toStdString();
        auto first = model->index(index.row(), ind_first, index.parent()).data(Qt::DisplayRole).toString();

        if(json::accept(cache)){
            auto cache_ = nlohmann::json::parse(cache);
            auto dlg = DialogInfo(cache_, first, this);
            dlg.setModal(true);
            dlg.exec();
        }

    }
}


void MainWindow::on_btnAddGroup_clicked()
{

    auto model = (TreeViewModel*)ui->treeView->model();

    auto index = ui->treeView->currentIndex();

    if(model->server_object() == arcirk::server::CertUsers){
        auto st = arcirk::database::table_default_json(arcirk::database::tables::tbCertUsers);
        st["is_group"] = 1;

        QString parent_name;
        QString parent_uuid = NIL_STRING_UUID;
        QModelIndex parent;
        int is_group = 0;
        if(index.isValid()){
            parent = index.parent();
            auto ind_is_group = model->get_column_index("is_group");
            auto ind_first = model->get_column_index("first");
            auto ind_ref = model->get_column_index("ref");
            is_group = model->index(index.row(), ind_is_group, index.parent()).data(Qt::DisplayRole).toInt();
            if(is_group == 1){
                parent_name = model->index(index.row(), ind_first, index.parent()).data(Qt::DisplayRole).toString();
                parent_uuid = model->index(index.row(), ind_ref, index.parent()).data(Qt::DisplayRole).toString();
            }else{

                if(parent.isValid()){
                    parent_name = model->index(parent.row(), ind_first, parent.parent()).data(Qt::DisplayRole).toString();
                    parent_uuid = model->index(parent.row(), ind_ref, parent.parent()).data(Qt::DisplayRole).toString();
                }
            }
        }else
            parent = QModelIndex();

        st["parent"] = parent_uuid.toStdString();

        auto dlg = DialogEdit(st, parent_name, this);
        dlg.setModal(true);
        dlg.exec();

        if(dlg.result() == QDialog::Accepted){
            nlohmann::json query_param = {
                {"table_name", arcirk::enum_synonym(tables::tbCertUsers)},
                {"query_type", "insert"},
                {"values", st}
            };
            std::string base64_param = QByteArray::fromStdString(query_param.dump()).toBase64().toStdString();
            auto resp = m_client->exec_http_query(arcirk::enum_synonym(arcirk::server::server_commands::ExecuteSqlQuery), json{
                                                 {"query_param", base64_param}
                                             });
            std::string result = "success";
            if(resp.is_string())
                result = resp.get<std::string>();
            if(resp != "error"){
                if(is_group == 0)
                    model->add(st, index.parent());
                else
                    model->add(st, index);
            }
        }
    }

}


void MainWindow::on_btnRegistryDevice_clicked()
{
    auto model = (TreeViewModel*)ui->treeView->model();
    auto index = ui->treeView->currentIndex();
    if(!index.isValid()){
        QMessageBox::critical(this, "Ошибка", "Не выбран элемент!");
        return;
    }

    using namespace arcirk::database;

    if(model->server_object() == arcirk::server::OnlineUsers){

        auto sess_info = pre::json::from_json<arcirk::client::session_info>(model->get_object(index));

        if(sess_info.device_id == NIL_STRING_UUID){
            QMessageBox::critical(this, "Ошибка", "Устройство имеет пустой идентификатор!");
            return;
        }

        if(QMessageBox::question(this, "Регистрация устройства", QString("Зарегистрировать устройство %1").arg(sess_info.host_name.c_str())) == QMessageBox::No)
            return;

        auto dev = table_default_struct<devices>(tables::tbDevices);
        dev.address = sess_info.address;
        dev.deviceType = arcirk::enum_synonym(arcirk::server::device_types::devDesktop);
        dev.first = sess_info.host_name;
        dev.second = sess_info.product;
        dev.ref = sess_info.device_id;

        nlohmann::json query_param = {
            {"table_name", arcirk::enum_synonym(tables::tbDevices)},
            {"query_type", "insert"},
            {"values", pre::json::to_json(dev)}
        };
        std::string base64_param = QByteArray::fromStdString(query_param.dump()).toBase64().toStdString();
        auto resp = m_client->exec_http_query(arcirk::enum_synonym(arcirk::server::server_commands::ExecuteSqlQuery), json{
                                             {"query_param", base64_param}
                                         });

        std::string result = "success";
        if(resp.is_string())
            result = resp.get<std::string>();

        if(result == "success")
            QMessageBox::information(this, "Регистрация устройства", "Устройство успешно зарегистрировано!");

    }
}

