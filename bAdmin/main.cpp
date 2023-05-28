#include "mainwindow.h"

#include <QApplication>
#include <QStyleFactory>

#ifdef Q_OS_WINDOWS
#include <Windows.h>
#pragma comment(lib, "advapi32")
#endif

//#ifdef Q_OS_WINDOWS
//    #pragma warning(disable:4100)
//    #pragma warning(disable:4267)
//    #pragma comment(lib, "advapi32")
//#endif

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
