#ifndef DIALOGCERTUSERCACHE_H
#define DIALOGCERTUSERCACHE_H

#include <QDialog>
#include "shared_struct.hpp"
#include "treeviewmodel.h"

namespace Ui {
class DialogCertUserCache;
}

using json = nlohmann::json;

class DialogCertUserCache : public QDialog
{
    Q_OBJECT

public:
    explicit DialogCertUserCache(arcirk::database::cert_users& obj, TreeViewModel * users_model,
                                 const nlohmann::json&,
                                 QWidget *parent = nullptr);
    ~DialogCertUserCache();

    void accept() override;

private slots:

    void on_txtServer_editingFinished();

    void on_txtServerUser_editingFinished();

    void on_btnSelectUser_clicked();

    void on_btnPwdView_toggled(bool checked);

    void on_btnPwdEdit_toggled(bool checked);

private:
    Ui::DialogCertUserCache *ui;
    arcirk::database::cert_users& object;

    arcirk::client::client_param cli_param;
    arcirk::server::server_config srv_conf;
    json cache;
    TreeViewModel * users_model_;
};

#endif // DIALOGCERTUSERCACHE_H
