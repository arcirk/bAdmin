#ifndef DIALOGSERVERS_H
#define DIALOGSERVERS_H

#include <QDialog>
#include <QStringListModel>
#include <QStringList>

namespace Ui {
class DialogServers;
}

class DialogServers : public QDialog
{
    Q_OBJECT

public:
    explicit DialogServers(QStringList& lst, QWidget *parent = nullptr);
    ~DialogServers();

    QStringList getResult() const;
    QString selected();

    void accept() override;

private slots:
    void on_tableView_doubleClicked(const QModelIndex &index);
    void on_btnSelect_clicked();

    void on_btnAdd_clicked();

    void on_btnDelete_clicked();

private:
    Ui::DialogServers *ui;
    QString m_selected;
    QStringList list_;

};

#endif // DIALOGSERVERS_H
