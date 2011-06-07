#-------------------------------------------------
#
# Project created by QtCreator 2011-04-15T12:44:24
#
#-------------------------------------------------

QT       += core gui

TARGET = ArchV2
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    filesystem.cpp \
    msstyle/windowsmodernstyle.cpp \
    contentview.cpp \
    filesviewwidget.cpp \
    textview.cpp

HEADERS  += mainwindow.h \
    filesystem.h \
    msstyle/windowsmodernstyle.h \
    contentview.h \
    filesviewwidget.h \
    textview.h

FORMS    += mainwindow.ui \
    contentview.ui \
    textview.ui

RESOURCES += \
    res.qrc

debug:DEFINES += FS_DEBUG
