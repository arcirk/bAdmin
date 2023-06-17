#include "dialogmstsc.h"
#include "ui_dialogmstsc.h"
#include "websocketclient.h"
#include <QLineEdit>
#include <QToolButton>
#include <QMessageBox>
#include "dialogselectintree.h"
#include <QUuid>
#include <crypter/crypter.hpp>

DialogMstsc::DialogMstsc(mstsc_options& mst, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogMstsc),
    _mstsc(mst)
{
    ui->setupUi(this);

    if(_mstsc.width == 0){
        _mstsc.width = 800;
    }
    if(_mstsc.height == 0){
        _mstsc.height = 600;
    }

    if(mst.uuid.empty()){
        _mstsc.uuid = QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
    }

    ui->txtAddress->setText(_mstsc.address.c_str());
    ui->txtName->setText(_mstsc.name.c_str());
    ui->spinPort->setValue(_mstsc.port == 0 ? DEF_MSTSC_PORT : _mstsc.port);
    ui->chkDefaultPort->setCheckState(_mstsc.def_port ? Qt::Checked : Qt::Unchecked);
    //QString _pwd = _mstsc.password.c_str();

    if(!_mstsc.password.empty()){
       // QString hashKey = QString::fromStdString(WebSocketClient::crypt(_pwd, QString(CRYPT_KEY).toUtf8()));
        auto crypter = arcirk::cryptography::crypt_utils();
        //auto pwd = QByteArray::fromBase64(_pwd.toUtf8()).toStdString();
        QString hashKey = crypter.decrypt_string(_mstsc.password).c_str();
        //QString hashKey = QString::fromStdString(arcirk::to_utf(crypter.decrypt_string(arcirk::from_utf(_mstsc.password))));
        ui->txtPassword->setText(hashKey);
    }
    ui->txtUserName->setText(_mstsc.user_name.c_str());
    ui->spinPort->setEnabled(ui->chkDefaultPort->checkState() != Qt::Checked);
    ui->chkWindowMode->setCheckState(_mstsc.not_full_window ? Qt::Checked : Qt::Unchecked);
    ui->spinX->setValue(_mstsc.width);
    ui->spinY->setValue(_mstsc.height);
    ui->chkEnableCmdKey->setCheckState(_mstsc.reset_user ? Qt::Checked : Qt::Unchecked);

    screens.push_back(qMakePair(800,600));
    screens.push_back(qMakePair(1024,768));
    screens.push_back(qMakePair(1152,864));
    screens.push_back(qMakePair(1280,768));
    screens.push_back(qMakePair(1280,800));
    screens.push_back(qMakePair(1280,960));
    screens.push_back(qMakePair(1360,768));
    screens.push_back(qMakePair(1366,768));
    screens.push_back(qMakePair(1440,900));
    screens.push_back(qMakePair(1600,900));
    screens.push_back(qMakePair(1600,1024));
    screens.push_back(qMakePair(1900,1200));

    createContextMenu();

    formControl();
}

DialogMstsc::~DialogMstsc()
{
    delete ui;
}

void DialogMstsc::accept()
{
    if(ui->txtAddress->text().trimmed().isEmpty()){
        QMessageBox::critical(this, "Ошибка", "Не указан адрес хоста!");
        return;
    }
    if(ui->txtName->text().trimmed().isEmpty()){
        QMessageBox::critical(this, "Ошибка", "Не указано наименование настройки!");
        return;
    }

    _mstsc.address = ui->txtAddress->text().trimmed().toStdString();
    _mstsc.name =  ui->txtName->text().trimmed().toStdString();
    _mstsc.port = ui->spinPort->value();
    _mstsc.def_port = ui->chkDefaultPort->checkState() == Qt::Checked ? true : false;
    _mstsc.not_full_window = ui->chkWindowMode->checkState() == Qt::Checked ? true : false;
    _mstsc.width = ui->spinX->value();
    _mstsc.height = ui->spinY->value();
    _mstsc.reset_user = ui->chkEnableCmdKey->checkState() == Qt::Checked ? true : false;
    QString _pwd = ui->txtPassword->text();
    if(!_pwd.isEmpty()){
        //auto hashKey = WebSocketClient::crypt(_pwd, CRYPT_KEY);
        auto crypter = arcirk::cryptography::crypt_utils();
        auto hashKey = crypter.encrypt_string(_pwd.toStdString());
        _mstsc.password = hashKey;
    }
    _mstsc.user_name = ui->txtUserName->text().trimmed().toStdString();

    QDialog::accept();

}

void DialogMstsc::on_chkDefaultPort_toggled(bool checked)
{
    if(checked){
         ui->spinPort->setValue(3389);
    }
    ui->spinPort->setEnabled(!checked);
}

void DialogMstsc::onContextMenuTriggered()
{
    auto *action = dynamic_cast<QAction*>( sender() );

    int index = action->property("index").toInt();
    auto p = screens[index];
    ui->spinX->setValue(p.first);
    ui->spinY->setValue(p.second);

}

void DialogMstsc::formControl()
{
    bool enable = ui->chkWindowMode->checkState() == Qt::Checked;
    ui->spinX->setEnabled(enable);
    ui->spinY->setEnabled(enable);
    ui->btnSelectScreenSize->setEnabled(enable);
}

void DialogMstsc::createContextMenu()
{
    contextMenu = new QMenu(this);

    int index = 0;
    foreach(auto itr, screens){
        QString name = QString::number(itr.first) + "x" + QString::number(itr.second);
        auto action = new QAction(name, this);
        action->setProperty("index",  index);
        contextMenu->addAction(action);

        connect(action, &QAction::triggered, this, &DialogMstsc::onContextMenuTriggered);

        index++;
    }

    ui->btnSelectScreenSize->setMenu(contextMenu);
}

void DialogMstsc::onSelectHost(const json &hosts)
{
    if(!hosts.is_object())
        return;
    using namespace arcirk::server;

    auto model = new TreeViewModel(this);
    model->set_table(hosts);

    QMap<QString, QString> m_aliases{
        qMakePair("first","Наименование"),
        qMakePair("address","Хост")
    };
    model->set_column_aliases(m_aliases);

    QVector<QString> m_hide{
        "ref",
        "deviceType"
    };
    for (int i = 0; i < model->rowCount(QModelIndex()); ++i) {
        auto index = model->index(i, 0, QModelIndex());
        auto obj = model->get_object(index);
        auto type = obj["deviceType"].get<arcirk::server::device_types>();
        if(type == devDesktop){
            model->set_icon(index, QIcon(":/img/desktop.png"));
        }else if(type == devServer){
            model->set_icon(index, QIcon(":/img/server.png"));
        }
    }
    auto dlg = DialogSelectInTree(model, m_hide, this);
    dlg.setModal(true);
    dlg.exec();

    if(dlg.result() == QDialog::Accepted){
        auto dev = dlg.selectedObject();
        ui->txtName->setText(dev["first"].get<std::string>().c_str());
        ui->txtAddress->setText(dev["address"].get<std::string>().c_str());
    }
}

void DialogMstsc::onSelectDatabaseUser(const json &user)
{
    if(user.is_object()){
        ui->txtUserName->setText(user["first"].get<std::string>().c_str());
    }
}


void DialogMstsc::on_chkWindowMode_toggled(bool checked)
{
    Q_UNUSED(checked);
    formControl();
}


void DialogMstsc::on_btnView_toggled(bool checked)
{
    auto echoMode = checked ? QLineEdit::Normal : QLineEdit::Password;
    QString image = checked ? ":/img/viewPwd.svg" : ":/img/viewPwd1.svg";
    auto btn = dynamic_cast<QToolButton*>(sender());
    btn->setIcon(QIcon(image));
    ui->txtPassword->setEchoMode(echoMode);
}


void DialogMstsc::on_btnSelectHost_clicked()
{
    emit selectHost();
}


void DialogMstsc::on_btnSelectUserName_clicked()
{
    emit selectDatabaseUser();
}

