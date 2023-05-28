#ifndef DIALOGSELECTDEVICE_H
#define DIALOGSELECTDEVICE_H

#include <QDialog>
#include <QRadioButton>

namespace Ui {
class DialogSelectDevice;
}

class DialogSelectDevice : public QDialog
{
    Q_OBJECT

public:
    explicit DialogSelectDevice(QWidget *parent = nullptr, const QStringList& labels = {"Каталог", "Файл", "Реестр"}, const QString& tTitle = "Выбор");
    ~DialogSelectDevice();

    void accept() override;

    int currentSelection();

private slots:

private:
    Ui::DialogSelectDevice *ui;
    int _currentSelection;
    QList<QRadioButton*> ctrl;
};

#endif // DIALOGSELECTDEVICE_H
