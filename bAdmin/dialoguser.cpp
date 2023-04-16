#include "dialoguser.h"
#include "ui_dialoguser.h"
#include "websocketclient.h"
#include <QMessageBox>

DialogUser::DialogUser(arcirk::database::user_info& info, const QString& parentName,  QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogUser),
    info_(info)
{
    ui->setupUi(this);

    ui->lblGroup->setText(parentName);
    ui->lblName->setText(info.first.c_str());
    ui->lblPresent->setText(info.second.c_str());
    ui->lblPwd->setText("***");

    setWindowTitle(info.first.c_str());
}

DialogUser::~DialogUser()
{
    delete ui;
}

void DialogUser::accept()
{
    if(ui->btnPwdEdit->isChecked() && ui->lblPwd->text().isEmpty()){
        QMessageBox::critical(this, "Ошибка", "Пароль не должен быть пустым!");
        return;
    }
    info_.first = ui->lblName->text().toStdString();
    info_.second = ui->lblPresent->text().toStdString();

    if(ui->btnPwdEdit->isChecked()){
        info_.hash = WebSocketClient::generateHash(ui->lblName->text(), ui->lblPwd->text()).toStdString();
    }

    QDialog::accept();
}

void DialogUser::on_btnPwdView_toggled(bool checked)
{
    auto echoMode = checked ? QLineEdit::Normal : QLineEdit::Password;
    QString image = checked ? ":/img/viewPwd.svg" : ":/img/viewPwd1.svg";
    auto btn = dynamic_cast<QToolButton*>(sender());
    btn->setIcon(QIcon(image));
    ui->lblPwd->setEchoMode(echoMode);
}


void DialogUser::on_btnPwdEdit_toggled(bool checked)
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

