#include "connectiondialog.h"
#include "ui_connectiondialog.h"
#include "dialogservers.h"
#include <QStringListModel>
#include <QStringList>
#include <QList>

ConnectionDialog::ConnectionDialog(arcirk::client::client_conf& conf, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConnectionDialog),
    conf_(conf)
{
    ui->setupUi(this);

    ui->chAutoConnect->setChecked(conf.is_auto_connect);
    ui->edtUser->setText(conf.user_name.data());
    if(conf.servers.size() > 0){
        std::string list = arcirk::byte_array_to_string(conf.servers);
        auto lst = QString::fromStdString(list).split("|");
        for (auto const& itr : lst) {
            ui->comboBox->addItem(itr);
        }
        ui->comboBox->setCurrentText(conf.server_host.data());
    }
    m_connectionState = false;
    formControl();
    if(!conf_.hash.empty()){
        ui->edtPass->setText("***");
    }
    setWindowTitle("Подключение к серверу");
}

ConnectionDialog::~ConnectionDialog()
{
    delete ui;
}

void ConnectionDialog::accept()
{

    conf_.user_name = ui->edtUser->text().toStdString();
    conf_.server_host = ui->comboBox->currentText().toStdString();
    if(ui->btnPwdEdit->isChecked()){
        auto hash = get_hash(ui->edtUser->text(), ui->edtPass->text());
        conf_.hash = hash.toStdString();
    }
    conf_.is_auto_connect = ui->chAutoConnect->isChecked();

    QDialog::accept();
}

void ConnectionDialog::on_toolButton_3_clicked()
{

    QStringList lst;

    if(conf_.servers.size() > 0){
        std::string list = arcirk::byte_array_to_string(conf_.servers);
        lst = QString::fromStdString(list).split("|");
    }

    auto dlg = DialogServers(lst, this);
    dlg.setModal(true);
    dlg.exec();
    if(dlg.result() == QDialog::Accepted){
        ByteArray arr{};
        auto selected = dlg.selected();
        auto lst = dlg.getResult();
        if(lst.size() > 0){
            arr = arcirk::string_to_byte_array(lst.join("|").toStdString());
            conf_.servers = arr;
            ui->comboBox->clear();
            foreach(auto const& itr , lst){
                 ui->comboBox->addItem(itr);
            }
           ui->comboBox->setCurrentIndex(ui->comboBox->findText(selected));
        }
    }

}


void ConnectionDialog::on_btnPwdEdit_toggled(bool checked)
{
    ui->btnViewPwd->setEnabled(checked);
    ui->edtPass->setEnabled(checked);
    if(checked){
        if(ui->edtPass->text() == "***"){
            ui->edtPass->setText("");
        }else{
            if(ui->edtPass->text() != "***" && !conf_.hash.empty()){
                ui->edtPass->setText("***");
            }
        }
    }
}

void ConnectionDialog::formControl()
{
    ui->comboBox->setEnabled(!m_connectionState);
    ui->edtUser->setEnabled(!m_connectionState);
    ui->chAutoConnect->setEnabled(!m_connectionState);
    ui->btnPwdEdit->setEnabled(!m_connectionState);
}

QString ConnectionDialog::get_sha1(const QByteArray& p_arg){
    auto sha = QCryptographicHash::hash(p_arg, QCryptographicHash::Sha1);
    return sha.toHex();
}

QString ConnectionDialog::get_hash(const QString& first, const QString& second){
    QString _usr = first.trimmed();
    _usr = _usr.toUpper();
    return get_sha1(QString(_usr + second).toUtf8());
}

bool ConnectionDialog::connectionState() const
{
    return m_connectionState;
}

void ConnectionDialog::setConnectionState(bool newConnectionState)
{
    m_connectionState = newConnectionState;
    formControl();

}

void ConnectionDialog::connectionChanged(bool state)
{
    Q_UNUSED(state);
    formControl();
}



void ConnectionDialog::on_btnViewPwd_toggled(bool checked)
{
    auto echoMode = checked ? QLineEdit::Normal : QLineEdit::Password;
    QString image = checked ? ":/img/viewPwd.svg" : ":/img/viewPwd1.svg";
    auto btn = dynamic_cast<QToolButton*>(sender());
    btn->setIcon(QIcon(image));
    ui->edtPass->setEchoMode(echoMode);
//    if(ui->edtPass->text().isEmpty() && !conf_.hash.empty()){
//        ui->edtPass->setText("***");
//    }
}

