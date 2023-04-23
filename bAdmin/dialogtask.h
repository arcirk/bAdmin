#ifndef DIALOGTASK_H
#define DIALOGTASK_H

#include <QDialog>
#include <shared_struct.hpp>

namespace Ui {
class DialogTask;
}

class DialogTask : public QDialog
{
    Q_OBJECT

public:
    explicit DialogTask(arcirk::services::task_options& task_data, QWidget *parent = nullptr);
    ~DialogTask();

    void accept() override;

private:
    Ui::DialogTask *ui;
    arcirk::services::task_options& task_data_;

};

#endif // DIALOGTASK_H
