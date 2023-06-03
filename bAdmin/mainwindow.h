#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "websocketclient.h"
#include <QLabel>
#include <QTreeWidgetItem>
#include <QTableWidgetItem>
#include <QToolButton>
#include <QTimer>
#include <QSystemTrayIcon>
#include <QAction>
#include <QMenu>
#include <QCloseEvent>
#include <QStandardItemModel>
#include "shared_struct.hpp"
#include "treeviewmodel.h"
#include "certuser.h"
#include "cryptcontainer.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
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

    void fillDefaultTree();

    void get_online_users();

    //
    void update_icons(arcirk::server::server_objects key, TreeViewModel* model);

    void insert_container(CryptContainer& cnt);

    void update_columns();

    void edit_cert_user(const QModelIndex &index);

signals:
    void setConnectionChanged(bool state);

public slots:
    void openConnection();
    void closeConnection();
    void serverResponse(const arcirk::server::server_response& message);

};
#endif // MAINWINDOW_H
