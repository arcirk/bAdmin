#ifndef DIALOGUSER_H
#define DIALOGUSER_H

#include <QDialog>
#include <shared_struct.hpp>

namespace Ui {
class DialogUser;
}

class DialogUser : public QDialog
{
    Q_OBJECT

public:
    explicit DialogUser(arcirk::database::user_info& info, const QString& parentName, QWidget *parent = nullptr);
    ~DialogUser();

    void accept() override;

private slots:
    void on_btnPwdView_toggled(bool checked);

    void on_btnPwdEdit_toggled(bool checked);

private:
    Ui::DialogUser *ui;
    arcirk::database::user_info& info_;
};

#endif // DIALOGUSER_H
