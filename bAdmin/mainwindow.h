#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "websocketclient.h"
#include <QLabel>
#include <QTreeWidgetItem>
#include <QTableWidgetItem>
#include <QToolButton>
#include <QTimer>


#include <QStandardItemModel>
#include "shared_struct.hpp"
#include "treeviewmodel.h"
#include "certuser.h"
#include "cryptcontainer.h"

#include <QSystemTrayIcon>
#include <QAction>
#include <QMenu>
#include <QCloseEvent>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void onTrayTriggered();
    void onAppExit();
    void onWindowShow();
    void trayMessageClicked();
    void trayIconActivated(QSystemTrayIcon::ActivationReason reason);
    void trayShowMessage(const QString& msg, int isError = false);

    void on_mnuConnect_triggered();
    void on_mnuExit_triggered();
    void on_mnuDisconnect_triggered();
    void on_toolButton_clicked();
    void on_treeWidget_itemClicked(QTreeWidgetItem *item, int column);
    void on_btnEdit_clicked();
    void on_treeView_doubleClicked(const QModelIndex &index);
    void on_btnDataImport_clicked();
    void on_btnAdd_clicked();
    void on_btnDelete_clicked();
    void on_btnSetLinkDevice_clicked();
    void on_mnuAbout_triggered();
    void on_mnuOptions_triggered();
    void on_btnTaskRestart_clicked();
    void on_btnStartTask_clicked();
    void on_btnSendClientRelease_clicked();
    void on_btnSendoToClient_clicked();
    void on_btnInfo_clicked();
    void on_btnAddGroup_clicked();
    void on_btnRegistryDevice_clicked();
    void on_btnRegistryUser_clicked();
    void on_btnEditCache_clicked();

private:
    Ui::MainWindow *                         ui;
    WebSocketClient *                        m_client;
    QLabel *                                 infoBar;
    QMap<arcirk::server::server_objects,
    TreeViewModel*>                          m_models;
    QMap<QString, QString>                   m_colAliases;
    QMap<arcirk::server::application_names,
    QPair<QIcon, QIcon>>                     app_icons;
    CertUser *                               current_user;
    bool                                     use_firefox;
    std::string                              firefox_path;

    QSystemTrayIcon *trayIcon;
    QMenu           *trayIconMenu;
    QAction         *quitAction;
    QAction         *showAction;

    QMap<arcirk::server::server_objects,
    QVector<QString>>   m_order_columns{
        qMakePair(arcirk::server::server_objects::Certificates, QVector<QString>{
                    "first", "subject", "issuer", "not_valid_before", "not_valid_after", "private_key", "parent_user"
                }),
        qMakePair(arcirk::server::server_objects::Containers, QVector<QString>{
                    "first", "subject", "issuer", "not_valid_before", "not_valid_after", "parent_user"
                }),
        qMakePair(arcirk::server::server_objects::OnlineUsers, QVector<QString>{
                    "app_name", "host_name", "address", "user_name", "role", "start_date", "session_uuid", "user_uuid", "device_id"
                }),
    };

    void createModels();
    void createColumnAliases();
    bool openConnectionDialog();

    void reconnect();
    void displayError(const QString& what, const QString& err);
    void connectionSuccess(); //при успешной авторизации
    void connectionChanged(bool state);
    void formControl();

    void write_conf();

    QTreeWidgetItem * addTreeNode(const QString &text, const QVariant &key, const QString &imagePath);
    QTreeWidgetItem * findTreeItem(const QString& key);
    QTreeWidgetItem * findTreeItem(const QString& key, QTreeWidgetItem * parent);
    QModelIndex findInTable(QAbstractItemModel * model, const QString& value, int column, bool findData = true);

    void tableSetModel(const QString& key);
    void tableResetModel(arcirk::server::server_objects key, const QByteArray& resp = "");
    void resetModel(arcirk::server::server_objects key, const nlohmann::json& data);

    void createTrayActions();
    void createTrayIcon();
    void createDynamicMenu();

    void fillDefaultTree();

    void get_online_users();

    //
    void update_icons(arcirk::server::server_objects key, TreeViewModel* model);

    void insert_container(CryptContainer& cnt);

    void update_columns();

    void edit_cert_user(const QModelIndex &index);
    void add_cert_user(const arcirk::database::cert_users& parent, const QString& host = "", const QString& name = "", const QString& uuid = "", const QString& system_user = "");
    bool is_cert_user_exists(const QString& host, const QString& system_user);
    void setVisible(bool visible) override;

    QIcon get_link_icon(const QString& link);

    void run_mstsc_link(const arcirk::client::mstsc_options& opt);
    QString rdp_file_text();
    QString cache_mstsc_directory();
    void update_rdp_files(const nlohmann::json& data);

    void database_get_containers_synch();
    void database_get_containers_asynch();
    void database_get_certificates_asynch();
    void database_get_certificates_synch();
    void database_get_deviceview_asynch();

    void database_insert_certificate();

    void set_enable_form(bool value);
signals:
    void setConnectionChanged(bool state);
    void certUserData(const QString& host, const QString& system_user, const nlohmann::json& data);
    void mozillaProfiles(const QStringList& lst);
    void certUserCertificates(const arcirk::client::cryptopro_data& data);
    void certUserContainers(const arcirk::client::cryptopro_data& data);
    void selectHosts(const nlohmann::json& hosts);
    void selectDatabaseUser(const nlohmann::json &user);

public slots:
    void openConnection();
    void closeConnection();
    void serverResponse(const arcirk::server::server_response& message);

    void onCertUserCache(const QUrl &ws, const QString& host, const QString& system_user, QWidget *parent);
    void onMozillaProfiles(const QString& host, const QString& system_user);

    void doGetCertUserCertificates(const QString& host, const QString& system_user);
    void doGetCertUserContainers(const QString& host, const QString& system_user);

    void doSelectHosts();
    void doSelectDatabaseUser();

    void onTreeFetch();

    void wsError(const QString& what, const QString& command, const QString& err);


};
#endif // MAINWINDOW_H
