#include "dialogseversettings.h"
#include "ui_dialogseversettings.h"
#include "websocketclient.h"
#include <QDir>
#include <QFileDialog>
#include <QToolButton>

DialogSeverSettings::DialogSeverSettings(arcirk::server::server_config& conf, arcirk::client::client_conf& conf_cl, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogSeverSettings),
    conf_(conf),
    conf_cl_(conf_cl)
{
    ui->setupUi(this);


    ui->edtServerName->setText(conf.ServerName.c_str());
    ui->edtServerHost->setText(conf.ServerHost.c_str());
    ui->spinPort->setValue(conf.ServerPort);
    ui->chAllowDelayedAuth->setCheckState(conf.AllowDelayedAuthorization == 0 ? Qt::CheckState::Unchecked : Qt::CheckState::Checked);
    ui->chAutorizationMode->setCheckState(conf.UseAuthorization == 0 ? Qt::CheckState::Unchecked : Qt::CheckState::Checked);
    ui->edtProfileDir->setText(conf.ServerWorkingDirectory.c_str());
    ui->edtHSHost->setText(conf.HSHost.c_str());
    ui->edtHSUser->setText(conf.HSUser.c_str());
    ui->edtHSPassword->setText(WebSocketClient::crypt(conf.HSPassword.c_str(), "my_key").c_str());
    ui->edtWebDavHost->setText(conf.WebDavHost.c_str());
    ui->edtWebDavUser->setText(conf.WebDavUser.c_str());
    ui->edtWebDavPwd->setText(WebSocketClient::crypt(conf.WebDavPwd.c_str(), "my_key").c_str());
    if(conf.SQLFormat == 0)
        ui->radioSqlite->setChecked(true);
    else
        ui->radioSqlServer->setChecked(true);

    ui->edtSQLHost->setText(conf.SQLHost.c_str());
    ui->edtSQLUser->setText(conf.SQLUser.c_str());
    ui->edtSQLPassword->setText(WebSocketClient::crypt(conf.SQLPassword.c_str(), "my_key").c_str());
    ui->edtExchangePlan->setText(conf.ExchangePlan.c_str());

    ui->edtPriceCheckerRepo->setText(conf_cl.price_checker_repo.c_str());
    ui->edtServerRepo->setText(conf_cl.server_repo.c_str());
    ui->chWriteLog->setCheckState(conf.WriteJournal == 0 ? Qt::CheckState::Unchecked : Qt::CheckState::Checked);

    setWindowTitle(windowTitle() + QString("(%1").arg(conf.ServerName.c_str()));
}

DialogSeverSettings::~DialogSeverSettings()
{
    delete ui;
}

void DialogSeverSettings::accept()
{
    conf_.AllowDelayedAuthorization = ui->chAllowDelayedAuth->checkState() == Qt::CheckState::Checked ? 1 : 0;
    conf_.UseAuthorization = ui->chAutorizationMode->checkState() == Qt::CheckState::Checked ? 1 : 0;
    conf_.ServerName = ui->edtServerName->text().toStdString();
    conf_.ServerHost = ui->edtServerHost->text().toStdString();
    conf_.ServerPort = ui->spinPort->value();
    conf_.ServerWorkingDirectory = ui->edtProfileDir->text().toStdString();
    conf_.HSHost = ui->edtHSHost->text().toStdString();
    conf_.HSUser = ui->edtHSUser->text().toStdString();
    if(ui->btnEditHSPassword->isChecked())
        conf_.HSPassword = WebSocketClient::crypt(ui->edtHSPassword->text(), "my_key");
    conf_.WebDavHost = ui->edtWebDavHost->text().toStdString();
    conf_.WebDavUser = ui->edtWebDavUser->text().toStdString();
    if(ui->btnEditWebDavPwd->isChecked())
        conf_.WebDavPwd = WebSocketClient::crypt(ui->edtWebDavPwd->text(), "my_key");
    conf_.SQLFormat = ui->radioSqlite->isChecked() ? 0 : 1;
    conf_.SQLHost = ui->edtSQLHost->text().toStdString();
    conf_.SQLUser = ui->edtSQLUser->text().toStdString();
    if(ui->btnEditSQLPassword->isChecked())
        conf_.SQLPassword = WebSocketClient::crypt( ui->edtSQLPassword->text(), "my_key");
    conf_.ExchangePlan = ui->edtExchangePlan->text().toStdString();
    conf_.WriteJournal = ui->chWriteLog->isChecked();

    conf_cl_.server_repo = ui->edtServerRepo->text().toStdString();
    conf_cl_.price_checker_repo = ui->edtPriceCheckerRepo->text().toStdString();

    QDialog::accept();
}

arcirk::server::server_config DialogSeverSettings::getResult()
{
    return conf_;
}

void DialogSeverSettings::onPwdEditToggled(bool checked, QToolButton* sender)
{
    if(sender->objectName() == "btnEditHSPassword"){
        ui->btnViewHSPassword->setEnabled(checked);
        ui->edtHSPassword->setEnabled(checked);
    }else if(sender->objectName() == "btnEditWebDavPwd"){
        ui->btnViewWebDavPwd->setEnabled(checked);
        ui->edtWebDavPwd->setEnabled(checked);
    }else if(sender->objectName() == "btnEditSQLPassword"){
        ui->btnViewSQLPassword->setEnabled(checked);
        ui->edtSQLPassword->setEnabled(checked);
    }
}

void DialogSeverSettings::onViewPwdToggled(bool checked, QToolButton* sender, QLineEdit* pwd)
{
    auto echoMode = checked ? QLineEdit::Normal : QLineEdit::Password;
    QString image = checked ? ":/img/viewPwd.svg" : ":/img/viewPwd1.svg";
    sender->setIcon(QIcon(image));
    pwd->setEchoMode(echoMode);
}

void DialogSeverSettings::on_btnEditHSPassword_toggled(bool checked)
{
    auto btn = dynamic_cast<QToolButton*>(sender());
    onPwdEditToggled(checked, btn);
}


void DialogSeverSettings::on_btnViewHSPassword_toggled(bool checked)
{
    auto btn = dynamic_cast<QToolButton*>(sender());
    onViewPwdToggled(checked, btn, ui->edtHSPassword);
}


void DialogSeverSettings::on_btnEditWebDavPwd_toggled(bool checked)
{
    auto btn = dynamic_cast<QToolButton*>(sender());
    onPwdEditToggled(checked, btn);
}


void DialogSeverSettings::on_btnViewWebDavPwd_toggled(bool checked)
{
    auto btn = dynamic_cast<QToolButton*>(sender());
    onViewPwdToggled(checked, btn, ui->edtWebDavPwd);
}


void DialogSeverSettings::on_btnEditSQLPassword_toggled(bool checked)
{
    auto btn = dynamic_cast<QToolButton*>(sender());
    onPwdEditToggled(checked, btn);
}


void DialogSeverSettings::on_btnViewSQLPassword_toggled(bool checked)
{
    auto btn = dynamic_cast<QToolButton*>(sender());
    onViewPwdToggled(checked, btn, ui->edtSQLPassword);
}


void DialogSeverSettings::on_btnSelPriceCheckerRepo_clicked()
{
    auto result = QFileDialog::getExistingDirectory(this, "Выбор каталога", ui->edtPriceCheckerRepo->text());
    if(!result.isEmpty()){
        ui->edtPriceCheckerRepo->setText(result);
    }
}

