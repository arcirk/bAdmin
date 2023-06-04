#include "dialogedit.h"
#include "ui_dialogedit.h"
#include <QUuid>
#include "dialogselectintree.h"
#include <QStringListModel>

DialogEditCertUser::DialogEditCertUser(arcirk::database::cert_users& source, const QString& parent_name, const json& dev, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogEdit),
    source_(source)
{
    ui->setupUi(this);

    auto ref = source_.ref;
    if(ref.empty() || ref == NIL_STRING_UUID){
        source_.ref = QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
    }
    auto first = source_.first;
    auto second = source_.second;
    is_group = source_.is_group;

    ui->lblUUID->setText(QString::fromStdString(source_.ref));
    ui->txtFirst->setText(QString::fromStdString(first));
    ui->txtSecond->setText(QString::fromStdString(second));
    ui->chIsGroup->setCheckState(is_group == 0 ? Qt::Unchecked : Qt::Checked);
    ui->txtParent->setText(parent_name);

    formControl();

    model_users = nullptr;

    QStringList lst{""};
    for (auto itr = dev.begin(); itr != dev.end(); ++itr) {
        auto it = *itr;
        lst.push_back(it["first"].get<std::string>().c_str());
    }

    auto lst_ = new QStringListModel(lst, this);
    ui->cmbHosts->setModel(lst_);

    setWindowTitle("Пользователь");
}

DialogEditCertUser::DialogEditCertUser(arcirk::database::cert_users &source, const QString &parent_name, TreeViewModel *model, const json& dev, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogEdit),
    source_(source)
{
    ui->setupUi(this);

    auto ref = source_.ref;
    if(ref.empty() || ref == NIL_STRING_UUID){
        source_.ref = QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
    }
    auto first = source_.first;
    auto second = source_.second;
    is_group = source_.is_group;

    ui->lblUUID->setText(QString::fromStdString(source_.ref));
    ui->txtFirst->setText(QString::fromStdString(first));
    ui->txtSecond->setText(QString::fromStdString(second));
    ui->chIsGroup->setCheckState(is_group == 0 ? Qt::Unchecked : Qt::Checked);
    ui->txtParent->setText(parent_name);

    formControl();

    model_users = model;

    QStringList lst{""};
    for (auto itr = dev.begin(); itr != dev.end(); ++itr) {
        auto it = *itr;
        lst.push_back(it["first"].get<std::string>().c_str());
    }

    auto lst_ = new QStringListModel(lst, this);
    ui->cmbHosts->setModel(lst_);

    setWindowTitle("Пользователь");
}

DialogEditCertUser::~DialogEditCertUser()
{
    delete ui;
}

void DialogEditCertUser::accept()
{
    source_.first = ui->txtFirst->text().trimmed().toStdString();
    source_.second = ui->txtSecond->text().trimmed().toStdString();
    source_.host = ui->cmbHosts->currentText().toStdString();

//    if(srv_object == arcirk::server::server_objects::CertUsers){
//        source_["first"] = ui->txtFirst->text().toStdString();
//    }

    QDialog::accept();
}

void DialogEditCertUser::setServerObject(arcirk::server::server_objects value)
{
    srv_object = value;
}

void DialogEditCertUser::formControl()
{
    bool v = is_group == 0 ? false : true;
    ui->lblHost->setVisible(!v);
    ui->lblParentUserName->setVisible(!v);
    ui->btnSelectUser->setVisible(!v);
    ui->cmbHosts->setVisible(!v);
    ui->txtParentUserName->setVisible(!v);
}

void DialogEditCertUser::on_btnSelectUser_clicked()
{

    if(!model_users)
        return;

    auto dlg = DialogSelectInTree(model_users, {"ref", "parent", "is_group", "deletion_mark"}, this);
    dlg.setModal(true);
    dlg.exec();
    if(dlg.result() == QDialog::Accepted){
        auto sel_object = dlg.selectedObject();
        source_.uuid = sel_object["ref"].get<std::string>();
        ui->txtParentUserName->setText(sel_object["first"].get<std::string>().c_str());
        ui->txtFirst->setText(sel_object["first"].get<std::string>().c_str());
        ui->txtSecond->setText(sel_object["second"].get<std::string>().c_str());

    }

}


void DialogEditCertUser::on_buttonBox_accepted()
{
    accept();
}

