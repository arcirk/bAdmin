#ifndef DIALOGMPLITEM_H
#define DIALOGMPLITEM_H

#include <QDialog>
#include "shared_struct.hpp"

namespace Ui {
class DialogMplItem;
}

using json = nlohmann::json;
using namespace arcirk::client;

class DialogMplItem : public QDialog
{
    Q_OBJECT

public:
    explicit DialogMplItem(mpl_item& item, const QStringList& profiles, QWidget *parent = nullptr);
    ~DialogMplItem();

    void accept() override;

private:
    Ui::DialogMplItem *ui;
    mpl_item& item_;
    QStringList profiles_;

    void setProfilesList();
    void load_data();
 signals:
   void getMozillaProfiles();
   void profilesChanged(const QStringList& lst);

public slots:
   void mozillaProfiles(const QStringList& profiles);

private slots:
   void on_btnResetProfiles_clicked();
};

#endif // DIALOGMPLITEM_H
