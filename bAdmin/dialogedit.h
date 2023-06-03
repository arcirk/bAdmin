#ifndef DIALOGEDIT_H
#define DIALOGEDIT_H

#include <QDialog>
#include "shared_struct.hpp"
#include "treeviewmodel.h"

namespace Ui {
class DialogEdit;
}

using json = nlohmann::json;

class DialogEdit : public QDialog
{
    Q_OBJECT

public:
    explicit DialogEdit(json& source, const QString& parent_name, QWidget *parent = nullptr);
    explicit DialogEdit(json& source, const QString& parent_name, TreeViewModel* model, QWidget *parent = nullptr);
    ~DialogEdit();

    void accept() override;

    void setServerObject(arcirk::server::server_objects value);

private slots:
    void on_btnSelectUser_clicked();

    void on_buttonBox_accepted();

private:
    Ui::DialogEdit *ui;
    json& source_;
    arcirk::server::server_objects srv_object;
    int is_group;
    TreeViewModel* model_users;

    void formControl();

};

#endif // DIALOGEDIT_H
