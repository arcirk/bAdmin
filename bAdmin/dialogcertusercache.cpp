#include "dialogcertusercache.h"
#include "ui_dialogcertusercache.h"
#include <QFloat16>
#include <QUrl>
#include "dialogselectintree.h"
#include "websocketclient.h"
#include <QStringListModel>
#include <QMap>
#include "dialogmstsc.h"
#include <QMessageBox>
#include "dialogmplitem.h"
#include <QVector>
#include <dialoginfo.h>
#include <QStandardPaths>
#include <QDir>
//#include "crypter/crypter.hpp"

DialogCertUserCache::DialogCertUserCache(arcirk::database::cert_users& obj, TreeViewModel * users_model,
                                         const QString& def_url, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogCertUserCache),
    object(obj),
    current_url(def_url)
{
    ui->setupUi(this);

    column_aliases();

    cache = json::object();

    if(!object.cache.empty())
        cache= json::parse(object.cache);

    users_model_ = users_model;

    read_cache(cache);

    is_localhost_ = false;
}

DialogCertUserCache::~DialogCertUserCache()
{
    delete ui;
}

void DialogCertUserCache::accept()
{

    if(ui->chkStandradPass->isChecked()){
        cli_param.hash = WebSocketClient::generateHash(cli_param.user_name.c_str(), cli_param.user_uuid.c_str()).toStdString();
    }else{
        if(ui->btnPwdEdit->isChecked()){
            cli_param.hash = WebSocketClient::generateHash(cli_param.user_name.c_str(), ui->lblPwd->text()).trimmed().toStdString();
        }
    }
    cache["client_param"] = pre::json::to_json(cli_param);
    cache["server_config"] = pre::json::to_json(srv_conf);

    cache["standard_password"] = ui->chkStandradPass->isChecked();
    cache["auto_connect"] = ui->chkAutoConnect->isChecked();

    cache["use_sid"] = ui->chUseSid->isChecked();

    write_mstsc_param();
    write_mpl_options();
    write_crypt_data();

    object.cache = cache.dump();

    QDialog::accept();
}

void DialogCertUserCache::set_is_localhost(bool value)
{
    is_localhost_ = value;
}


void DialogCertUserCache::on_txtServer_editingFinished()
{
    QUrl url(ui->txtServer->text());
    srv_conf.ServerHost = url.host().toStdString();
    srv_conf.ServerPort = url.port();
    srv_conf.ServerSSL = url.scheme() == "wss" ? true : false;

}


void DialogCertUserCache::on_txtServerUser_editingFinished()
{
    cli_param.user_name = ui->txtServerUser->text().trimmed().toStdString();
}


void DialogCertUserCache::on_btnSelectUser_clicked()
{
    if(!users_model_)
        return;

    auto dlg = DialogSelectInTree(users_model_, {"ref", "parent", "is_group", "deletion_mark"}, this);
    dlg.setModal(true);
    dlg.exec();
    if(dlg.result() == QDialog::Accepted){
        auto sel_object = dlg.selectedObject();
        cli_param.user_uuid = sel_object["ref"];
        ui->txtServerUser->setText(sel_object["first"].get<std::string>().c_str());
        on_txtServerUser_editingFinished();
    }

}


void DialogCertUserCache::on_btnPwdView_toggled(bool checked)
{
    auto echoMode = checked ? QLineEdit::Normal : QLineEdit::Password;
    QString image = checked ? ":/img/viewPwd.svg" : ":/img/viewPwd1.svg";
    auto btn = dynamic_cast<QToolButton*>(sender());
    btn->setIcon(QIcon(image));
    ui->lblPwd->setEchoMode(echoMode);
}


void DialogCertUserCache::on_btnPwdEdit_toggled(bool checked)
{
    if(checked){
        if(ui->lblPwd->text() == "***"){
            ui->lblPwd->setText("");
        }
    }else{
        if(ui->lblPwd->text() == ""){
            ui->lblPwd->setText("***");
        }
    }

    ui->lblPwd->setEnabled(checked);
    ui->btnPwdView->setEnabled(checked);
    if(!checked){
        on_btnPwdView_toggled(false);
        ui->btnPwdView->setChecked(false);
    }
}

void DialogCertUserCache::read_mstsc_param()
{

    QMap<QString, QString> m_aliases{
        qMakePair("name","Наименование"),
        qMakePair("address","Хост"),
        qMakePair("port","Порт"),
        qMakePair("def_port","Стандартный порт"),
        qMakePair("not_full_window","Режим окна"),
        qMakePair("width","Ширина"),
        qMakePair("height","Высота"),
        qMakePair("reset_user","Сброс пользователя"),
        qMakePair("user_name","Пользователь"),
        qMakePair("password","Пароль")
    };

    QVector<QString> m_order{
        "name",
        "address",
        "port",
        "user_name",
        "width",
        "height"
    };

    QVector<QString> m_hide{};
    for (auto itr = m_aliases.constBegin(); itr != m_aliases.constEnd(); ++itr) {
        if(m_order.indexOf(itr.key()) == -1)
           m_hide.append(itr.key());
    }

    mstsc_param = cache.value("mstsc_param", json::object());
    auto model = new TreeViewModel(this);
    model->set_column_aliases(m_aliases);
    model->set_rows_icon(QIcon(":/img/mstsc.png"));

    if(!mstsc_param.empty()){
        ui->chkEnableMstsc->setCheckState(mstsc_param.value("enable_mstsc", false) ? Qt::CheckState::Checked : Qt::CheckState::Unchecked );
        ui->chkEnableMstscUserSessions->setCheckState(mstsc_param.value("enable_mstsc_users_sess", false) ? Qt::CheckState::Checked : Qt::CheckState::Unchecked );
        std::string admin_mstsc = mstsc_param.value("mstsc_admin", "admin");
        if(admin_mstsc == "admin" || admin_mstsc.empty()){
            ui->radioRunAsAdmin->setChecked(true);
        }else{
            ui->radioRunAs->setChecked(true);
            ui->txtRuAsName->setText(admin_mstsc.c_str());
        }

        auto detailed_records = mstsc_param.value("detailed_records", json::array());
        auto model_json = json::object();
        auto columns = json::array();
        auto tmp = pre::json::to_json(arcirk::client::mstsc_options());
        for (auto itr = tmp.items().begin(); itr != tmp.items().end(); ++itr) {
            columns += itr.key();
        }
        model_json["columns"] = columns;
        model_json["rows"] = detailed_records;
        model->set_table(model_json);
        model->columns_establish_order(m_order);
        ui->treeViewMstsc->setModel(model);
        if(ui->treeViewMstsc->model()->columnCount() > 0)
            ui->treeViewMstsc->resizeColumnToContents(0);
    }else{
        ui->chkEnableMstsc->setCheckState( Qt::CheckState::Unchecked );
        ui->chkEnableMstscUserSessions->setCheckState(Qt::CheckState::Unchecked );
        ui->radioRunAsAdmin->setChecked(true);
    }

    ui->treeViewMstsc->setModel(model);
    if (ui->treeViewMstsc->model()->columnCount() > 0) {
        foreach (const auto& itr, m_hide) {
            auto ind = model->get_column_index(itr);
            if(ind != -1)
                ui->treeViewMstsc->hideColumn(ind);
        }
    }

}

void DialogCertUserCache::write_mstsc_param()
{
    mstsc_param["enable_mstsc"] = ui->chkEnableMstsc->checkState() == Qt::CheckState::Checked;
    mstsc_param["enable_mstsc_users_sess"] = ui->chkEnableMstscUserSessions->checkState() == Qt::CheckState::Checked;
    auto mstsc_admin = ui->txtRuAsName->text().trimmed();
    mstsc_param["mstsc_admin"] = mstsc_admin.isEmpty() ? "admin" : mstsc_admin.toStdString();
    auto model = (TreeViewModel*)ui->treeViewMstsc->model();
    if(model)
        mstsc_param["detailed_records"] = model->get_objects(QModelIndex());

    cache["mstsc_param"] = mstsc_param;
}

void DialogCertUserCache::read_mpl_options()
{
    ui->chUseFirefox->setChecked(mpl_.use_firefox);
    if(mpl_.firefox_path.empty())
        ui->txtFirefoxPath->setText("C:/Program Files/Mozilla Firefox");
    else
        ui->txtFirefoxPath->setText(mpl_.firefox_path.c_str());

    m_moz_profiles.clear();
    if(mpl_.mpl_profiles.size() > 0){
        auto lst_str = arcirk::byte_array_to_string(mpl_.mpl_profiles);
        auto lst_j = json::parse(lst_str);
        if(lst_j.is_array()){
            for (auto itr = lst_j.begin(); itr != lst_j.end(); ++itr) {
                std::string s = *itr;
                m_moz_profiles.append(s.c_str());
            }
        }

    }

    auto model = new TreeViewModel(this);
    model->set_column_aliases(QMap<QString,QString>{
                                  qMakePair("name", "Наименование"),
                                  qMakePair("url", "Ссылка"),
                                  qMakePair("profile", "Профиль")
                              });
    model->use_hierarchy("name");
    if(mpl_.mpl_list.size() > 0){
        auto table_str = arcirk::byte_array_to_string(mpl_.mpl_list);
        auto table_j = json::parse(table_str);
        if(table_j.is_object())
            model->set_table(table_j);
        model->set_columns(QVector<QString>{"name", "profile", "url"});
    }
    ui->treeViewMpl->setModel(model);
}

void DialogCertUserCache::write_mpl_options()
{
    mpl_.use_firefox = ui->chUseFirefox->isChecked();
    mpl_.firefox_path = ui->txtFirefoxPath->text().toStdString();

    auto model = (TreeViewModel*)ui->treeViewMpl->model();
    mpl_.mpl_list.clear();
    if(model){
        auto table = model->get_table_model(QModelIndex());
        auto table_ba = arcirk::string_to_byte_array(table.dump());
        mpl_.mpl_list = ByteArray(table_ba.size());
        std::copy(table_ba.begin(), table_ba.end(), mpl_.mpl_list.begin());
    }

    auto prof = json::array();
    mpl_.mpl_profiles.clear();
    foreach (auto it, m_moz_profiles) {
        prof += it.toStdString();
    }
    auto ba = arcirk::string_to_byte_array(prof.dump());
    mpl_.mpl_profiles = ByteArray(ba.size());
    std::copy(ba.begin(), ba.end(), mpl_.mpl_profiles.begin());

    cache["mpl_options"] = pre::json::to_json(mpl_);
}

void DialogCertUserCache::read_crypt_data()
{
    if(crypt_data.cryptopro_path.empty()){
        crypt_data.cryptopro_path = "C:/Program Files (x86)/Crypto Pro/CSP";
    }
    ui->txtCryptoProPath->setText(crypt_data.cryptopro_path.c_str());
    onCertificates(crypt_data);
    onContainers(crypt_data);
}

void DialogCertUserCache::write_crypt_data()
{
    crypt_data.cryptopro_path = ui->txtCryptoProPath->text().toStdString();
    auto model = (TreeViewModel*)ui->treeCertificates->model();
    crypt_data.certs.clear();
    if(model){
        auto table = model->get_table_model(QModelIndex());
        auto table_ba = arcirk::string_to_byte_array(table.dump());
        crypt_data.certs = ByteArray(table_ba.size());
        std::copy(table_ba.begin(), table_ba.end(), crypt_data.certs.begin());
    }

    model = (TreeViewModel*)ui->treeContainers->model();
    crypt_data.conts.clear();
    if(model){
        auto table = model->get_table_model(QModelIndex());
        auto table_ba = arcirk::string_to_byte_array(table.dump());
        crypt_data.conts = ByteArray(table_ba.size());
        std::copy(table_ba.begin(), table_ba.end(), crypt_data.conts.begin());
    }

    cache["crypt_data"] = pre::json::to_json(crypt_data);
}

template<typename T>
T DialogCertUserCache::getStructData(const std::string &name, const json &source)
{
    auto result = T();
    try {
        auto obg = source.value(name, json::object());
        if(!obg.empty())
            result = pre::json::from_json<T>(obg);
    } catch (const std::exception& e) {
       qCritical() <<  e.what();
    }

    return result;
}

void DialogCertUserCache::read_cache(const nlohmann::json &data)
{
    if(!data.empty()){
        cli_param = getStructData<arcirk::client::client_param>("client_param", data);
        srv_conf = getStructData<arcirk::server::server_config>("server_config", data);
        mpl_ = getStructData<arcirk::client::mpl_options>("mpl_options", data);
        crypt_data = getStructData<arcirk::client::cryptopro_data>("crypt_data", data);
    }else{
        cli_param = arcirk::client::client_param();
        srv_conf = arcirk::server::server_config();
        mpl_ = arcirk::client::mpl_options();
        crypt_data = arcirk::client::cryptopro_data();
    }

    if(!srv_conf.ServerHost.empty()){
        QString scheme = srv_conf.ServerSSL ? "wss" : "ws";
        QUrl url{};
        url.setHost(srv_conf.ServerHost.c_str());
        url.setPort(srv_conf.ServerPort);
        url.setScheme(scheme);
        ui->txtServer->setText(url.toString());
    }else{
        QUrl url(current_url);
        srv_conf.ServerSSL = url.scheme() == "wss" ? true : false;
        srv_conf.ServerHost = url.host().toStdString();
        srv_conf.ServerPort = url.port();
        ui->txtServer->setText(url.toString());
    }


    read_mstsc_param();

    ui->chkStandradPass->setCheckState(cache.value("standard_password", true) ? Qt::Checked : Qt::Unchecked);
    ui->chkAutoConnect->setCheckState(cache.value("auto_connect", true) ? Qt::Checked : Qt::Unchecked);
    ui->txtServerUser->setText(cli_param.user_name.c_str());
    ui->chUseSid->setChecked(cache.value("use_sid", true));

    read_mpl_options();

    read_crypt_data();

    form_control();
}

void DialogCertUserCache::form_control()
{
    bool as_admin = ui->radioRunAsAdmin->isChecked();
    ui->txtRuAsName->setEnabled(!as_admin);
    ui->btnSetMstscPwd->setEnabled(!as_admin);

    update_mpl_items_icons();
}

void DialogCertUserCache::edit_row(const QModelIndex &current_index)
{
    QModelIndex index;

    if(!current_index.isValid())
        index = ui->treeViewMstsc->currentIndex();
    else
        index = current_index;

    if(!index.isValid()){
        QMessageBox::critical(this, "Ошибка", "Не выбрана строка!");
        return;
    }

    auto model = (TreeViewModel*)ui->treeViewMstsc->model();
    auto object = model->get_object(index);
    Q_ASSERT(!object.empty());

    auto mstsc = arcirk::secure_serialization<arcirk::client::mstsc_options>(object);
    auto dlg = DialogMstsc(mstsc, this);
    connect(&dlg, &DialogMstsc::selectHost, this, &DialogCertUserCache::doSelectHosts);
    connect(this, &DialogCertUserCache::setSelectHosts, &dlg, &DialogMstsc::onSelectHost);
    connect(&dlg, &DialogMstsc::selectDatabaseUser, this, &DialogCertUserCache::doSelectDatabaseUser);
    connect(this, &DialogCertUserCache::setSelectDatabaseUser, &dlg, &DialogMstsc::onSelectDatabaseUser);
    dlg.setModal(true);
    dlg.exec();
    if(dlg.result() == QDialog::Accepted){
        auto row = pre::json::to_json(mstsc);
        auto model = (TreeViewModel*)ui->treeViewMstsc->model();
        model->set_object(index, row);
    }
}

void DialogCertUserCache::update_mpl_items_icons()
{
    auto model = (TreeViewModel*)ui->treeViewMpl->model();
    if(!model)
        return;

    for (int i = 0; i < model->rowCount(QModelIndex()); ++i) {
        auto index = model->index(i, 0, QModelIndex());
        auto object = model->get_object(index);
        auto row = pre::json::from_json<arcirk::client::mpl_item>(object);
        QString link = row.url.c_str();
        if(link.indexOf("markirovka") != -1){
            model->set_icon(index, QIcon(":/img/markirowka.png"));
        }else if(link.indexOf("diadoc.kontur.ru") != -1){
            model->set_icon(index, QIcon(":/img/diadoc.png"));
        }else if(link.indexOf("ofd.kontur.ru") != -1){
            model->set_icon(index, QIcon(":/img/ofd.png"));
        }else if(link.indexOf("extern.kontur.ru") != -1){
            model->set_icon(index, QIcon(":/img/extern.png"));
        }else if(link.indexOf("sberbank.ru") != -1){
            model->set_icon(index, QIcon(":/img/sberbank.png"));
        }else
            model->set_icon(index, QIcon(":/img/link.png"));
    }

    ui->treeViewMpl->resizeColumnToContents(0);
}

void DialogCertUserCache::onCertUserCache(const QString &host, const QString &system_user, const nlohmann::json &data)
{
    Q_UNUSED(host);
    Q_UNUSED(system_user);
    cache = data;
    read_cache(cache);
}

void DialogCertUserCache::onMozillaProfiles()
{
    emit getMozillaProfiles(object.host.c_str(), object.system_user.c_str());
}

void DialogCertUserCache::doMozillaProfiles(const QStringList &lst)
{
    emit setMozillaProfiles(lst);
}

void DialogCertUserCache::onProfilesChanged(const QStringList &lst)
{
    m_moz_profiles = lst;
}

void DialogCertUserCache::onCertificates(const arcirk::client::cryptopro_data &data)
{
    ui->treeCertificates->setModel(nullptr);

    if(data.certs.size() > 0){
        QVector<QString> m_order{
            "first",
            "subject",
            "issuer",
            "not_valid_before",
            "not_valid_after",
            "parent_user",
            "sha1"
        };
        auto model = new TreeViewModel(this);
        auto data_s = arcirk::byte_array_to_string(data.certs);
        auto table = json::parse(data_s);
        model->set_table(table);
        model->columns_establish_order(m_order);
        model->set_rows_icon(QIcon(":/img/cert16NoKey.png"));
        model->set_column_aliases(m_colAliases);
        ui->treeCertificates->setModel(model);
        ui->treeCertificates->hideColumn(model->get_column_index("cache"));
        ui->treeCertificates->hideColumn(model->get_column_index("second"));
        ui->treeCertificates->resizeColumnToContents(0);
    }
}

void DialogCertUserCache::onContainers(const arcirk::client::cryptopro_data &data)
{
    ui->treeContainers->setModel(nullptr);

    if(data.conts.size() > 0){
        QVector<QString> m_order{
            "name",
            "volume",
            "type"
        };
        auto model = new TreeViewModel(this);
        auto data_s = arcirk::byte_array_to_string(data.conts);
        auto table = json::parse(data_s);
        model->set_table(table);
        model->columns_establish_order(m_order);
        model->set_column_aliases(m_colAliases);
        ui->treeContainers->setModel(model);

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

        ui->treeContainers->resizeColumnToContents(0);
    }
}

void DialogCertUserCache::onSelectHosts(const json &hosts)
{
    emit setSelectHosts(hosts);
}

void DialogCertUserCache::doSelectHosts()
{
    emit selectHosts();
}

void DialogCertUserCache::doSelectDatabaseUser()
{
    emit selectDatabaseUser();
}

void DialogCertUserCache::onSelectDatabaseUser(const json &user)
{
    emit setSelectDatabaseUser(user);
}


void DialogCertUserCache::on_radioRunAsAdmin_toggled(bool checked)
{
    Q_UNUSED(checked);
    form_control();
}


void DialogCertUserCache::on_radioRunAs_toggled(bool checked)
{
    Q_UNUSED(checked);
    form_control();
}


void DialogCertUserCache::on_btnMstscAdd_clicked()
{
    auto mstsc = arcirk::client::mstsc_options();
    auto dlg = DialogMstsc(mstsc, this);
    connect(&dlg, &DialogMstsc::selectHost, this, &DialogCertUserCache::doSelectHosts);
    connect(this, &DialogCertUserCache::setSelectHosts, &dlg, &DialogMstsc::onSelectHost);
    connect(&dlg, &DialogMstsc::selectDatabaseUser, this, &DialogCertUserCache::doSelectDatabaseUser);
    connect(this, &DialogCertUserCache::setSelectDatabaseUser, &dlg, &DialogMstsc::onSelectDatabaseUser);
    dlg.setModal(true);
    dlg.exec();
    if(dlg.result() == QDialog::Accepted){
        auto row = pre::json::to_json(mstsc);
        auto model = (TreeViewModel*)ui->treeViewMstsc->model();
        model->add(row);
    }
}


void DialogCertUserCache::on_btnMstscEdit_clicked()
{
    edit_row();
}


void DialogCertUserCache::on_treeViewMstsc_doubleClicked(const QModelIndex &index)
{
    edit_row(index);
}


void DialogCertUserCache::on_btnMstscMoveUp_clicked()
{
    auto index = ui->treeViewMstsc->currentIndex();
    if(!index.isValid()){
        QMessageBox::critical(this, "Ошибка", "Не выбрана строка!");
        return;
    }

    if(index.row() < 1)
        return;

    auto model = (TreeViewModel*)ui->treeViewMstsc->model();
    model->move_up(index);
}


void DialogCertUserCache::on_btnMstscMoveDown_clicked()
{
    auto index = ui->treeViewMstsc->currentIndex();
    if(!index.isValid()){
        QMessageBox::critical(this, "Ошибка", "Не выбрана строка!");
        return;
    }

    auto model = (TreeViewModel*)ui->treeViewMstsc->model();

    if(index.row() >= model->rowCount(index.parent()) - 1)
        return;

    model->move_down(index);
}


void DialogCertUserCache::on_btnImportSettings_clicked()
{
    if(QMessageBox::question(this, "Импорт настроек", "Перезаполнить данные?") == QMessageBox::No)
        return;
    QUrl ws(ui->txtServer->text());
    Q_ASSERT(ws.isValid());
    emit getData(ws, object.host.c_str(), object.system_user.c_str(), this);
}

void DialogCertUserCache::on_btnMplItemAdd_clicked()
{
    auto row = arcirk::client::mpl_item();
    auto dlg = DialogMplItem(row, m_moz_profiles, this);
    connect(&dlg, &DialogMplItem::getMozillaProfiles, this, &DialogCertUserCache::onMozillaProfiles);
    connect(this, &DialogCertUserCache::setMozillaProfiles, &dlg, &DialogMplItem::mozillaProfiles);
    connect(&dlg, &DialogMplItem::profilesChanged, this, &DialogCertUserCache::onProfilesChanged);
    dlg.setWindowTitle("Новая запись");
    dlg.setModal(true);
    dlg.exec();

    if(dlg.result() == QDialog::Accepted){
        auto model = (TreeViewModel*)ui->treeViewMpl->model();
        if(!model){
            model = new TreeViewModel(this);
            model->set_column_aliases(QMap<QString,QString>{
                                          qMakePair("name", "Наименование"),
                                          qMakePair("url", "Ссылка"),
                                          qMakePair("profile", "Профиль")
                                      });
        }
        auto object = pre::json::to_json(row);
        if(model->columnCount(QModelIndex()) == 0){
            auto columns = json::array();
            auto rows = json::array();
            rows += object;
            for (auto it = object.items().begin(); it != object.items().end(); ++it) {
                columns += it.key();
            }
            json table = json::object();
            table["columns"]  = columns;
            table["rows"] = rows;
            model->set_table(table);
            model->set_columns(QVector<QString>{"name", "profile", "url"});
        }else
            model->add(object);
    }

    update_mpl_items_icons();
}


void DialogCertUserCache::on_btnMplItemEdit_clicked()
{
    auto model = (TreeViewModel*)ui->treeViewMpl->model();
    if(!model)
        return;
    auto index = ui->treeViewMpl->currentIndex();
    if(!index.isValid()){
        QMessageBox::critical(this, "Ошибка", "Не выбрана строка!");
        return;
    }

    auto row = pre::json::from_json<arcirk::client::mpl_item>(model->get_object(index));
    auto dlg = DialogMplItem(row, m_moz_profiles, this);
    connect(&dlg, &DialogMplItem::getMozillaProfiles, this, &DialogCertUserCache::onMozillaProfiles);
    connect(this, &DialogCertUserCache::setMozillaProfiles, &dlg, &DialogMplItem::mozillaProfiles);
    connect(&dlg, &DialogMplItem::profilesChanged, this, &DialogCertUserCache::onProfilesChanged);
    dlg.setWindowTitle(row.name.c_str());
    dlg.setModal(true);
    dlg.exec();

    if(dlg.result() == QDialog::Accepted){
        auto model = (TreeViewModel*)ui->treeViewMpl->model();
        auto object = pre::json::to_json(row);
        model->set_object(index, object);
    }

    update_mpl_items_icons();
}


void DialogCertUserCache::on_btnMplItemDelete_clicked()
{
    auto model = (TreeViewModel*)ui->treeViewMpl->model();
    if(!model)
        return;
    auto index = ui->treeViewMpl->currentIndex();
    if(!index.isValid()){
        QMessageBox::critical(this, "Ошибка", "Не выбрана строка!");
        return;
    }

    auto row = pre::json::from_json<arcirk::client::mpl_item>(model->get_object(index));
    if(QMessageBox::question(this, "Удаление", QString("Удалить настройку '%1'").arg(row.name.c_str())) == QMessageBox::Yes){
        model->remove(index);
    }
}


void DialogCertUserCache::on_btnMplItemMoveUp_clicked()
{
    auto index = ui->treeViewMpl->currentIndex();
    if(!index.isValid()){
        QMessageBox::critical(this, "Ошибка", "Не выбрана строка!");
        return;
    }

    if(index.row() < 1)
        return;

    auto model = (TreeViewModel*)ui->treeViewMpl->model();
    model->move_up(index);
    update_mpl_items_icons();
}


void DialogCertUserCache::on_btnMplItemMoveDown_clicked()
{
    auto index = ui->treeViewMpl->currentIndex();
    if(!index.isValid()){
        QMessageBox::critical(this, "Ошибка", "Не выбрана строка!");
        return;
    }

    auto model = (TreeViewModel*)ui->treeViewMpl->model();

    if(index.row() >= model->rowCount(index.parent()) - 1)
        return;

    model->move_down(index);
    update_mpl_items_icons();
}


void DialogCertUserCache::on_treeViewMpl_doubleClicked(const QModelIndex &index)
{
    on_btnMplItemEdit_clicked();
}


void DialogCertUserCache::on_btnResetCertIlst_clicked()
{
    auto currentTab = ui->tabCrypt->currentIndex();
    if(currentTab == 0)
        emit getCertificates(object.host.c_str(), object.system_user.c_str());
    else
        emit getContainers(object.host.c_str(), object.system_user.c_str());
}


void DialogCertUserCache::on_btnCertInfo_clicked()
{
    auto currentTab = ui->tabCrypt->currentIndex();
    if(currentTab == 0){
        auto model = (TreeViewModel*)ui->treeCertificates->model();
        if(!model)
            return;
        auto index = ui->treeCertificates->currentIndex();
        if(!index.isValid()){
            QMessageBox::critical(this, "Ошибка", "Не выбран элемент!");
            return;
        }

        using json = nlohmann::json;


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

void DialogCertUserCache::column_aliases()
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
    m_colAliases.insert("private_key", "Контейнер");
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


void DialogCertUserCache::on_btnCertAdd_clicked()
{

}


void DialogCertUserCache::on_btnCertDelete_clicked()
{

}


void DialogCertUserCache::on_btnMstscRemove_clicked()
{
    auto index = ui->treeViewMstsc->currentIndex();
    if(!index.isValid()){
        QMessageBox::critical(this, "Ошибка", "Не выбрана строка!");
        return;
    }

    if(QMessageBox::question(this, "Удаление", "Удалить выбранную настройку?") == QMessageBox::No)
        return;

    auto model = (TreeViewModel*)ui->treeViewMstsc->model();
    model->remove(index);
}


void DialogCertUserCache::on_btnMstsc_clicked()
{

}

