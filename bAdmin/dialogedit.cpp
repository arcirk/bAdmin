#include "dialogedit.h"
#include "ui_dialogedit.h"
#include <QUuid>
#include "dialogselectintree.h"


DialogEdit::DialogEdit(json& source, const QString& parent_name, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogEdit),
    source_(source)
{
    ui->setupUi(this);

    auto ref = source_.value("ref", NIL_STRING_UUID);
    if(ref.empty() || ref == NIL_STRING_UUID){
        source_["ref"] = QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
    }
    auto first = source_.value("first", "");
    auto second = source_.value("second", "");
    is_group = source_.value("is_group", 0);

    ui->lblUUID->setText(QString::fromStdString(source_["ref"].get<std::string>()));
    ui->txtFirst->setText(QString::fromStdString(first));
    ui->txtSecond->setText(QString::fromStdString(second));
    ui->chIsGroup->setCheckState(is_group == 0 ? Qt::Unchecked : Qt::Checked);
    ui->txtParent->setText(parent_name);

    formControl();

    model_users = nullptr;

    setWindowTitle("Пользователь");
}

DialogEdit::DialogEdit(json &source, const QString &parent_name, TreeViewModel *model, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogEdit),
    source_(source)
{
    ui->setupUi(this);

    auto ref = source_.value("ref", NIL_STRING_UUID);
    if(ref.empty() || ref == NIL_STRING_UUID){
        source_["ref"] = QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
    }
    auto first = source_.value("first", "");
    auto second = source_.value("second", "");
    is_group = source_.value("is_group", 0);

    ui->lblUUID->setText(QString::fromStdString(source_["ref"].get<std::string>()));
    ui->txtFirst->setText(QString::fromStdString(first));
    ui->txtSecond->setText(QString::fromStdString(second));
    ui->chIsGroup->setCheckState(is_group == 0 ? Qt::Unchecked : Qt::Checked);
    ui->txtParent->setText(parent_name);

    formControl();

    model_users = model;

    setWindowTitle("Пользователь");
}

DialogEdit::~DialogEdit()
{
    delete ui;
}

void DialogEdit::accept()
{
    source_["first"] = ui->txtFirst->text().trimmed().toStdString();
    source_["second"] = ui->txtSecond->text().trimmed().toStdString();

//    if(srv_object == arcirk::server::server_objects::CertUsers){
//        source_["first"] = ui->txtFirst->text().toStdString();
//    }

    QDialog::accept();
}

void DialogEdit::setServerObject(arcirk::server::server_objects value)
{
    srv_object = value;
}

void DialogEdit::formControl()
{
    bool v = is_group == 0 ? false : true;
    ui->lblHost->setVisible(!v);
    ui->lblParentUserName->setVisible(!v);
    ui->btnSelectUser->setVisible(!v);
    ui->cmbHosts->setVisible(!v);
    ui->txtParentUserName->setVisible(!v);
}

void DialogEdit::on_btnSelectUser_clicked()
{

    if(!model_users)
        return;

    auto dlg = DialogSelectInTree(model_users, {"ref", "parent", "is_group", "deletion_mark"}, this);
    dlg.setModal(true);
    dlg.exec();
    if(dlg.result() == QDialog::Accepted){
        auto sel_object = dlg.selectedObject();
        source_["uuid"] = sel_object["ref"];
        ui->txtParentUserName->setText(sel_object["first"].get<std::string>().c_str());
        ui->txtFirst->setText(sel_object["first"].get<std::string>().c_str());
        ui->txtSecond->setText(sel_object["second"].get<std::string>().c_str());

    }

}


void DialogEdit::on_buttonBox_accepted()
{
    accept();
}

