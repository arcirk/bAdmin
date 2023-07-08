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
                                 const QString& def_url = "ws://localhost:8080",
                                 QWidget *parent = nullptr);
    ~DialogCertUserCache();

    void accept() override;

    void set_is_localhost(bool value);

private slots:

    void on_txtServer_editingFinished();
    void on_txtServerUser_editingFinished();
    void on_btnSelectUser_clicked();
    void on_btnPwdView_toggled(bool checked);
    void on_btnPwdEdit_toggled(bool checked);
    void on_radioRunAsAdmin_toggled(bool checked);
    void on_radioRunAs_toggled(bool checked);
    void on_btnMstscAdd_clicked();
    void on_btnMstscEdit_clicked();
    void on_treeViewMstsc_doubleClicked(const QModelIndex &index);
    void on_btnMstscMoveUp_clicked();
    void on_btnMstscMoveDown_clicked();
    void on_btnImportSettings_clicked();
    void on_btnMplItemAdd_clicked();
    void on_btnMplItemEdit_clicked();
    void on_btnMplItemDelete_clicked();
    void on_btnMplItemMoveUp_clicked();
    void on_btnMplItemMoveDown_clicked();
    void on_treeViewMpl_doubleClicked(const QModelIndex &index);
    void on_btnResetCertIlst_clicked();
    void on_btnCertInfo_clicked();
    void on_btnCertAdd_clicked();
    void on_btnCertDelete_clicked();
    void on_btnMstscRemove_clicked();
    void on_btnMstsc_clicked();

    void on_btnSelectPathFirefox_clicked();

private:
    Ui::DialogCertUserCache *ui;
    arcirk::database::cert_users& object;
    json mstsc_param;
    arcirk::client::client_param cli_param;
    arcirk::server::server_config srv_conf;
    arcirk::client::mpl_options mpl_;
    arcirk::client::cryptopro_data crypt_data;
    json cache;
    TreeViewModel * users_model_;
    QString current_url;
    QStringList m_moz_profiles{};
    QMap<QString, QString> m_colAliases;
    bool is_localhost_;

    void column_aliases();

    void read_mstsc_param();
    void write_mstsc_param();

    void read_mpl_options();
    void write_mpl_options();

    void read_crypt_data();
    void write_crypt_data();

    void read_cache(const nlohmann::json& data);

    void form_control();
    void edit_row(const QModelIndex &current_index = QModelIndex());

    void reset_data();

    void update_mpl_items_icons();

    template<typename T>
    T getStructData(const std::string& name, const json& source);

signals:
    void getData(const QUrl &ws, const QString& host, const QString& system_user, QWidget *parent);
    void getMozillaProfiles(const QString& host, const QString& system_user);
    void setMozillaProfiles(const QStringList& lst);
    void getCertificates(const QString& host, const QString& system_user);
    void getContainers(const QString& host, const QString& system_user);
    void selectHosts();
    void setSelectHosts(const json& hosts);
    void selectDatabaseUser();
    void setSelectDatabaseUser(const json& user);

public slots:
   void onCertUserCache(const QString& host, const QString& system_user, const nlohmann::json& data);
   void onMozillaProfiles();
   void doMozillaProfiles(const QStringList& lst);
   void onProfilesChanged(const QStringList& lst);
   void onCertificates(const arcirk::client::cryptopro_data& crypt_data);
   void onContainers(const arcirk::client::cryptopro_data& crypt_data);
   void onSelectHosts(const json& hosts);
   void doSelectHosts();
   void doSelectDatabaseUser();
   void onSelectDatabaseUser(const json& user);
};

#endif // DIALOGCERTUSERCACHE_H
