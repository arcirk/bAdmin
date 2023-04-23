#include "dialogtask.h"
#include "ui_dialogtask.h"

DialogTask::DialogTask(arcirk::services::task_options& task_data, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogTask),
    task_data_(task_data)
{
    ui->setupUi(this);
    ui->name->setText(task_data.name.c_str());
    ui->synonum->setText(task_data.synonum.c_str());
    //ui->start_task->setText(task_data.name.c_str());
    //ui->end_task->setText(task_data.name.c_str());
    ui->allowed->setCheckState(task_data.allowed ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
    ui->predefined->setCheckState(task_data.predefined ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
    ui->days_of_week->setText(task_data.days_of_week.c_str());
    ui->interval->setValue(task_data.interval);
    ui->uuid->setText(task_data.uuid.c_str());
    ui->script->setText(task_data.script.c_str());

    ui->name->setEnabled(!task_data.predefined);
    ui->uuid->setEnabled(!task_data.predefined);

}

DialogTask::~DialogTask()
{
    delete ui;
}

void DialogTask::accept()
{
    task_data_.name = ui->name->text().trimmed().toStdString();
    task_data_.synonum = ui->synonum->text().trimmed().toStdString();
    task_data_.days_of_week = ui->days_of_week->text().trimmed().toStdString();
    task_data_.uuid = ui->uuid->text().trimmed().toStdString();
    task_data_.script = ui->script->text().trimmed().toStdString();
    task_data_.interval = ui->interval->value();
    task_data_.allowed = ui->allowed->isChecked();

    QDialog::accept();
}

