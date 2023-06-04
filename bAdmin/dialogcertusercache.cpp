#include "dialogcertusercache.h"
#include "ui_dialogcertusercache.h"
#include <QFloat16>
#include <QUrl>
#include "dialogselectintree.h"
#include "websocketclient.h"
#include <QStringListModel>

DialogCertUserCache::DialogCertUserCache(arcirk::database::cert_users& obj, TreeViewModel * users_model,
                                         const nlohmann::json& hosts, const QString& def_url, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogCertUserCache),
    object(obj)
{
    ui->setupUi(this);

    cache = json::object();

    if(!object.cache.empty()){
        cache= json::parse(object.cache);
        cli_param = pre::json::from_json<arcirk::client::client_param>(cache["client_param"]);
        srv_conf = pre::json::from_json<arcirk::server::server_config>(cache["server_config"]);
    }else{
        cli_param = arcirk::client::client_param();
        srv_conf = arcirk::server::server_config();
    }

    if(!srv_conf.ServerHost.empty()){
        QString scheme = srv_conf.ServerSSL ? "wss" : "ws";
        QUrl url{};
        url.setHost(srv_conf.ServerHost.c_str());
        url.setPort(srv_conf.ServerPort);
        url.setScheme(scheme);
        ui->txtServer->setText(url.toString());
    }else{
        QUrl url(def_url);
        srv_conf.ServerSSL = url.scheme() == "wss" ? true : false;
        srv_conf.ServerHost = url.host().toStdString();
        srv_conf.ServerPort = url.port();
        ui->txtServer->setText(url.toString());
    }

    ui->chkStandradPass->setCheckState(cache.value("standard_password", true) ? Qt::Checked : Qt::Unchecked);
    ui->txtServerUser->setText(cli_param.user_name.c_str());

    users_model_ = users_model;

    QStringList lst{""};
    for (auto itr = hosts.begin(); itr != hosts.end(); ++itr) {
        auto it = *itr;
        lst.push_back(it["first"].get<std::string>().c_str());
    }

    auto lst_ = new QStringListModel(lst, this);
    ui->cmbHost->setModel(lst_);

    if(!cli_param.host_name.empty())
        ui->cmbHost->setCurrentText(cli_param.host_name.c_str());

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
    cli_param.host_name = ui->cmbHost->currentText().toStdString();
    cache["client_param"] = pre::json::to_json(cli_param);
    cache["server_config"] = pre::json::to_json(srv_conf);


    object.cache = cache.dump();

    QDialog::accept();
}


void DialogCertUserCache::on_txtServer_editingFinished()
{
    QUrl url(ui->txtServerUser->text());
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

