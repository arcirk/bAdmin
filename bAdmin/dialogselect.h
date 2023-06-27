#ifndef DIALOGSELECT_H
#define DIALOGSELECT_H

#include <QDialog>

namespace Ui {
class DialogSelect;
}

class DialogSelect : public QDialog
{
    Q_OBJECT

public:
    explicit DialogSelect(QWidget *parent = nullptr);
    ~DialogSelect();

    void accept() override;

    int get_result();

private:
    Ui::DialogSelect *ui;
    int m_result;
};

#endif // DIALOGSELECT_H
