#ifndef DIALOGEDIT_H
#define DIALOGEDIT_H

#include <QDialog>
#include "shared_struct.hpp"
#include "treeviewmodel.h"

namespace Ui {
class DialogEdit;
}

using json = nlohmann::json;

class DialogEditCertUser : public QDialog
{
    Q_OBJECT

public:
    explicit DialogEditCertUser(arcirk::database::cert_users& source, const QString& parent_name, const json& dev, QWidget *parent = nullptr);
    explicit DialogEditCertUser(arcirk::database::cert_users& source, const QString& parent_name, TreeViewModel* model, const json& dev, QWidget *parent = nullptr);
    ~DialogEditCertUser();

    void accept() override;

    void setServerObject(arcirk::server::server_objects value);

private slots:
    void on_btnSelectUser_clicked();

    void on_buttonBox_accepted();

private:
    Ui::DialogEdit *ui;
    arcirk::database::cert_users& source_;
    arcirk::server::server_objects srv_object;
    int is_group;
    TreeViewModel* model_users;

    void formControl();

};

#endif // DIALOGEDIT_H
