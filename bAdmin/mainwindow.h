#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "connectiondialog.h"
#include "websocketclient.h"

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

private:
    Ui::MainWindow *ui;
    WebSocketClient * m_client;

    bool openConnectionDialog();

    void reconnect();
    void displayError(const QString& what, const QString& err);
    void connectionSuccess(); //при успешной авторизации
    void connectionChanged(bool state);

    void write_conf();

signals:
    void setConnectionChanged(bool state);

public slots:
    void openConnection();
    void closeConnection();

};
#endif // MAINWINDOW_H
