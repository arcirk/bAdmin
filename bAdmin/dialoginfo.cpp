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
            for(auto it = obj.items().begin(); it != obj.items().end(); ++it){
                QString n = i == 0 ? "" : "\n";
                auto val = obj[it.key()];
                if(val.is_string())
                    str.append(n + QString::fromStdString(it.key()) + ": " + QString::fromStdString(obj[it.key()].get<std::string>()).split(",").join("\n").trimmed());
                else if(val.is_object()){
                    QStringList details;
                    for(auto dt = val.items().begin(); dt != val.items().end(); ++dt){
                        auto d_val = dt.value();
                        if(d_val.is_string())
                            details.append(QString::fromStdString(dt.key() + ": " + d_val.get<std::string>()));
                    }
                    str.append(n + QString::fromStdString(it.key()) + ": " + details.join("\n").trimmed());
                }
                i++;
            }
            lbl->setText(str);
            lbl->setWordWrap(true);
            cnt->addWidget(lbl, row, 1);
        }else
        {
           auto lbl = new QLabel();
           auto lst = QString::fromStdString(obj.get<std::string>()).split('\r');
           if(lst.size() == 1){
              auto index =lst[0].indexOf("=");
              if(index != 1){
                  auto tmp = lst[0];
                  lst = tmp.split(",");
              }
           }
           for (int i = 0; i < lst.size(); ++i) {
               lst[i] = lst[i].trimmed();
           }

           lbl->setText(lst.join('\n').trimmed());
           lbl->setWordWrap(true);
           cnt->addWidget(lbl, row, 1);
        }

        row++;
    }
}
