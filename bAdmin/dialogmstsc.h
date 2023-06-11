#ifndef DIALOGMSTSC_H
#define DIALOGMSTSC_H

#include <QDialog>
#include <QAction>
#include <QMenu>
#include "shared_struct.hpp"

#define DEF_MSTSC_PORT 3389

namespace Ui {
class DialogMstsc;
}

using json = nlohmann::json;
using namespace arcirk::client;

class DialogMstsc : public QDialog
{
    Q_OBJECT

public:
    explicit DialogMstsc(mstsc_options& mst, QWidget *parent = nullptr);
    ~DialogMstsc();

    void accept() override;

private slots:
    void on_chkDefaultPort_toggled(bool checked);
    void onContextMenuTriggered();

    void on_chkWindowMode_toggled(bool checked);

    void on_btnView_toggled(bool checked);

    void on_btnSelectHost_clicked();

    void on_btnSelectUserName_clicked();

private:
    Ui::DialogMstsc *ui;
    mstsc_options& _mstsc;
    QVector<QPair<int,int>> screens;
    QMenu *contextMenu;

    void formControl();
    void createContextMenu();

signals:
    void selectHost();
    void selectDatabaseUser();

public slots:
    void onSelectHost(const json& hosts);
    void onSelectDatabaseUser(const json& user);

};

#endif // DIALOGMSTSC_H
