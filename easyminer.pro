#-------------------------------------------------
#
# Project created by QtCreator 2013-05-22T00:54:28
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = easyminer
TEMPLATE = app


SOURCES += main.cpp\
        MainWindow.cpp

HEADERS  += MainWindow.h

FORMS    += MainWindow.ui

RESOURCES += \
    resources.qrc

win32:RC_FILE += resources-windows.rc
win32:RC_FILE += resources-windows-qt-plugin.qrc

TRANSLATIONS += ts/fr.ts \
    ts/es.ts
