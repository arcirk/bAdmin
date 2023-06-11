#ifndef DIALOGSELECTINTREE_H
#define DIALOGSELECTINTREE_H

#include <QDialog>
#include "treeviewmodel.h"
#include "shared_struct.hpp"

namespace Ui {
class DialogSelectInTree;
}

class DialogSelectInTree : public QDialog
{
    Q_OBJECT

public:
    explicit DialogSelectInTree(TreeViewModel* model, QWidget *parent = nullptr);
    explicit DialogSelectInTree(TreeViewModel* model, QVector<QString> hideColumns, QWidget *parent = nullptr);

    ~DialogSelectInTree();

    QString file_name() const;

    void accept() override;

    nlohmann::json selectedObject();

    void allow_sel_group(bool value);

    void set_window_text(const QString& value);

private slots:
    void on_treeView_doubleClicked(const QModelIndex &index);

    void on_buttonBox_accepted();

    void on_buttonBox_rejected();

private:
    Ui::DialogSelectInTree *ui;
    QString file_name_;
    nlohmann::json sel_object;
    bool allow_sel_group_;
};

#endif // DIALOGSELECTINTREE_H
