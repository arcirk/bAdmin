QT += core gui sql svg
QT += websockets network
QT += core5compat

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++20

#QMAKE_CXXFLAGS_DEBUG += /MTd
#QMAKE_CXXFLAGS_RELEASE += /MT

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    certuser.cpp \
    commandline.cpp \
    commandlineparser.cpp \
    cryptcertificate.cpp \
    cryptcontainer.cpp \
    dialoginfo.cpp \
    dialogselectdevice.cpp \
    dialogselectinlist.cpp \
    connectiondialog.cpp \
    dialogabout.cpp \
    dialogdevice.cpp \
    dialogimporttodatabase.cpp \
    dialogprofilefolder.cpp \
    dialogservers.cpp \
    dialogseversettings.cpp \
    dialogtask.cpp \
    dialoguser.cpp \
    main.cpp \
    mainwindow.cpp \
    qjsontablemodel.cpp \
    qproxymodel.cpp \
    tabledelegate.cpp \
    treeviewmodel.cpp \
    websocketclient.cpp \
    winreg/WinReg.cpp

HEADERS += \
    certuser.h \
    commandline.h \
    commandlineparser.h \
    cryptcertificate.h \
    cryptcontainer.h \
    dialoginfo.h \
    dialogselectdevice.h \
    dialogselectinlist.h \
    connectiondialog.h \
    dialogdevice.h \
    dialogabout.h \
    dialogimporttodatabase.h \
    dialogprofilefolder.h \
    dialogservers.h \
    dialogseversettings.h \
    dialogtask.h \
    dialoguser.h \
    mainwindow.h \
    qjsontablemodel.h \
    qproxymodel.h \
    query_builder.hpp \
    shared_struct.hpp \
    tabledelegate.h \
    treeviewmodel.h \
    websocketclient.h \
    winreg/WinReg.hpp

FORMS += \
    dialoginfo.ui \
    dialogselectdevice.ui \
    dialogselectinlist.ui \
    connectiondialog.ui \
    dialogdevice.ui \
    dialogabout.ui \
    dialogimporttodatabase.ui \
    dialogprofilefolder.ui \
    dialogservers.ui \
    dialogseversettings.ui \
    dialogtask.ui \
    dialoguser.ui \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

windows:DEFINES += _CRT_SECURE_NO_WARNINGS
windows:DEFINES += _WINDOWS

Boost_USE_STATIC_LIBS = ON
windows:LIBS += -lbcrypt

INCLUDEPATH += $(BOOST_INCLDUE)
#LIBS += -L$(BOOST_LIB) -lboost_locale-vc140-mt

CONFIG(debug, debug|release) {
    LIBS += -LC:/lib/vcpkg/installed/x64-windows/debug/lib -lboost_locale-vc140-mt-gd
    LIBS += -LC:/lib/vcpkg/installed/x64-windows/debug/bin
} else {
    LIBS += -LC:/lib/vcpkg/installed/x64-windows/lib -lboost_locale-vc140-mt
    LIBS += -LC:/lib/vcpkg/installed/x64-windows/bin
#    -lboost_locale-vc140-mt
#
}


RESOURCES += \
    resurses.qrc

DEFINES += IS_USE_QT_LIB

win32 {
    QMAKE_TARGET_PRODUCT = "arcirk Websocket Server Manager"
    QMAKE_TARGET_DESCRIPTION = "Managing servers"
}

VERSION = 1.1.0
QMAKE_TARGET_COPYRIGHT = (c)(arcirk) Arcady Borisoglebsky
