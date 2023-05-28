#ifndef DIALOGINFO_H
#define DIALOGINFO_H

#include <QDialog>
#include <shared_struct.hpp>

namespace Ui {
class DialogInfo;
}

using json = nlohmann::json;

class DialogInfo : public QDialog
{
    Q_OBJECT

public:
    explicit DialogInfo(const json& info, const QString& name, QWidget *parent = nullptr);
    ~DialogInfo();

private:
    Ui::DialogInfo *ui;
    void set_info(const json& info);
};

#endif // DIALOGINFO_H
