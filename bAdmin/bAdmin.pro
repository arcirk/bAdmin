QT       += core gui
QT += websockets network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    connectiondialog.cpp \
    dialogservers.cpp \
    main.cpp \
    mainwindow.cpp \
    websocketclient.cpp

HEADERS += \
    connectiondialog.h \
    dialogservers.h \
    mainwindow.h \
    shared_struct.hpp \
    websocketclient.h

FORMS += \
    connectiondialog.ui \
    dialogservers.ui \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

windows:DEFINES += _CRT_SECURE_NO_WARNINGS

INCLUDEPATH += $(BOOST_INCLDUE)

RESOURCES += \
    resurses.qrc
