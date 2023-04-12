#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QStandardPaths>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_client = new WebSocketClient(this);
    connect(m_client, &WebSocketClient::connectionChanged, this, &MainWindow::connectionChanged);
    connect(m_client, &WebSocketClient::connectionSuccess, this, &MainWindow::connectionSuccess);
    connect(m_client, &WebSocketClient::displayError, this, &MainWindow::displayError);

    openConnectionDialog();

    if(!m_client->isConnected())
        ui->mnuConnect->setIcon(QIcon(":/img/disconnect.png"));

    setWindowTitle("arcirk");
}

MainWindow::~MainWindow()
{
    if(m_client->isConnected())
        m_client->close();
    delete ui;
}

bool MainWindow::openConnectionDialog()
{
    auto dlg = ConnectionDialog(m_client->conf(), this);
    dlg.setConnectionState(m_client->isConnected());
    connect(this, &MainWindow::setConnectionChanged, &dlg, &ConnectionDialog::connectionChanged);
    connect(&dlg, &ConnectionDialog::closeConnection, this, &MainWindow::closeConnection);
    connect(&dlg, &ConnectionDialog::openConnection, this, &MainWindow::openConnection);

    dlg.setModal(true);
    dlg.exec();
    if(dlg.result() == QDialog::Accepted){
        m_client->write_conf();
        reconnect();
    }

    return true;
}

void MainWindow::reconnect()
{
    if(m_client->isConnected())
            m_client->close();

    m_client->open();
}

void MainWindow::displayError(const QString &what, const QString &err)
{

}

void MainWindow::connectionSuccess()
{

}

void MainWindow::connectionChanged(bool state)
{
    if(state)
        ui->mnuConnect->setIcon(QIcon(":/img/connect.png"));
    else
        ui->mnuConnect->setIcon(QIcon(":/img/disconnect.png"));
}

void MainWindow::write_conf()
{

}

void MainWindow::openConnection()
{
    if(m_client->isConnected())
            m_client->close();

    m_client->open();
}

void MainWindow::closeConnection()
{
    if(m_client->isConnected())
            m_client->close();
}


void MainWindow::on_mnuConnect_triggered()
{
    openConnectionDialog();
}


void MainWindow::on_mnuExit_triggered()
{
    QApplication::exit();
}

