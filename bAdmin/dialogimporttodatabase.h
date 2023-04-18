#ifndef DIALOGIMPORTTODATABASE_H
#define DIALOGIMPORTTODATABASE_H

#include <QDialog>

namespace Ui {
class DialogImportToDatabase;
}

class DialogImportToDatabase : public QDialog
{
    Q_OBJECT

public:
    explicit DialogImportToDatabase(QWidget *parent = nullptr);
    ~DialogImportToDatabase();

private:
    Ui::DialogImportToDatabase *ui;
};

#endif // DIALOGIMPORTTODATABASE_H
