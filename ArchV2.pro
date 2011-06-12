#-------------------------------------------------
#
# Project created by QtCreator 2011-04-15T12:44:24
#
#-------------------------------------------------

QT       += core gui qt3support

TARGET = ArchV2
TEMPLATE = app

OBJECTS_DIR = .build
MOC_DIR = .build
UI_DIR = .build

SOURCES += main.cpp\
        mainwindow.cpp \
    filesystem.cpp \
    msstyle/windowsmodernstyle.cpp \
    contentview.cpp \
    filesviewwidget.cpp \
    textview.cpp \
    listbuilder.cpp

HEADERS  += mainwindow.h \
    filesystem.h \
    msstyle/windowsmodernstyle.h \
    contentview.h \
    filesviewwidget.h \
    textview.h \
    listbuilder.h

FORMS    += mainwindow.ui \
    contentview.ui \
    textview.ui

RESOURCES += \
    res.qrc

TRANSLATIONS = arch_v2_ru.ts \
                arch_v2_en.ts

debug:DEFINES += FS_DEBUG
