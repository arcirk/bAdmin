#ifndef DIALOGPROFILEFOLDER_H
#define DIALOGPROFILEFOLDER_H

#include <QDialog>
#include "treeviewmodel.h"

namespace Ui {
class DialogProfileFolder;
}

class DialogProfileFolder : public QDialog
{
    Q_OBJECT

public:
    explicit DialogProfileFolder(TreeViewModel* model, QWidget *parent = nullptr);
    ~DialogProfileFolder();

    QString file_name() const;

    void accept() override;

private:
    Ui::DialogProfileFolder *ui;
    QString file_name_;
};

#endif // DIALOGPROFILEFOLDER_H
