#include "mainwindow.h"

#include <QApplication>
#include <QStyleFactory>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    qApp->setStyle(QStyleFactory::create("Fusion"));
    QStringList cmdline_args = QCoreApplication::arguments();
    bool bHidden = false;
    foreach(auto param, cmdline_args){
        if(param == "-h")
            bHidden = true;
    }
    MainWindow w;
    w.show();
    if(bHidden)
        w.hide();
    return a.exec();
}
