QT += core gui sql svg
QT += websockets network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    dialogselectinlist.cpp \
    connectiondialog.cpp \
    dialogabout.cpp \
    dialogdevice.cpp \
    dialogimporttodatabase.cpp \
    dialogprofilefolder.cpp \
    dialogservers.cpp \
    dialogseversettings.cpp \
    dialoguser.cpp \
    main.cpp \
    mainwindow.cpp \
    qjsontablemodel.cpp \
    qproxymodel.cpp \
    tabledelegate.cpp \
    treeviewmodel.cpp \
    websocketclient.cpp

HEADERS += \
    dialogselectinlist.h \
    connectiondialog.h \
    dialogdevice.h \
    dialogabout.h \
    dialogimporttodatabase.h \
    dialogprofilefolder.h \
    dialogservers.h \
    dialogseversettings.h \
    dialoguser.h \
    mainwindow.h \
    qjsontablemodel.h \
    qproxymodel.h \
    query_builder.hpp \
    shared_struct.hpp \
    tabledelegate.h \
    treeviewmodel.h \
    websocketclient.h

FORMS += \
    dialogselectinlist.ui \
    connectiondialog.ui \
    dialogdevice.ui \
    dialogabout.ui \
    dialogimporttodatabase.ui \
    dialogprofilefolder.ui \
    dialogservers.ui \
    dialogseversettings.ui \
    dialoguser.ui \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

windows:DEFINES += _CRT_SECURE_NO_WARNINGS

INCLUDEPATH += $(BOOST_INCLDUE)

RESOURCES += \
    resurses.qrc

DEFINES += IS_USE_QT_LIB
