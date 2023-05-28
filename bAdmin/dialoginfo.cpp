#include "dialoginfo.h"
#include "ui_dialoginfo.h"

DialogInfo::DialogInfo(const json& info, const QString& name, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogInfo)
{
    ui->setupUi(this);

    ui->lblTitle->setText(name);
    set_info(info);

}

DialogInfo::~DialogInfo()
{
    delete ui;
}

void DialogInfo::set_info(const json &info)
{
    auto cnt = ui->gridLayout;
    int row = 0;
    for(auto itr = info.items().begin(); itr != info.items().end(); ++itr){
        auto item = new QLabel();
        item->setText(QString::fromStdString(itr.key()) + ":");
        cnt->addWidget(item, row, 0, Qt::AlignTop);
        auto obj = itr.value();
        if (obj.is_object()){
            auto lbl = new QLabel();
            QString str;
            int i = 0;
            for(auto it = info.items().begin(); it != info.items().end(); ++it){
                QString n = i == 0 ? "" : "\n";
                str.append(n + QString::fromStdString(it.key()) + ": " + QString::fromStdString(obj[it.key()].get<std::string>()).split(",").join("\n").trimmed());
                i++;
            }
            lbl->setText(str);
            cnt->addWidget(lbl, row, 1);
        }else
        {
           auto lbl = new QLabel();
           lbl->setText(QString::fromStdString(obj.get<std::string>()).split(",").join("\n").trimmed());
           cnt->addWidget(lbl, row, 1);
        }

        row++;
    }
}
