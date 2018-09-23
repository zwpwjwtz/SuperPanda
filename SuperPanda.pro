#-------------------------------------------------
#
# Project created by QtCreator 2018-06-13T20:48:02
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = SuperPanda
TEMPLATE = app

VER_MAJ = 1
VER_MIN = 1
VER_PAT = 0
VERSION = 1.1.0

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS APP_VERSION=\\\"$$VERSION\\\"

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES +=\
    main.cpp \
    Interfaces/configfileeditor.cpp \
    Components/magickonfug.cpp \
    Interfaces/exelauncher.cpp \
    Utils/diskutils.cpp \
    Utils/bootutils.cpp \
    Utils/dialogutils.cpp \
    mainwindow.cpp \
    global.cpp \
    aboutwindow.cpp \
    Utils/environment.cpp \
    Utils/environmentmodel.cpp \
    Utils/hostosinfo.cpp \
    Utils/qtcassert.cpp \
    Utils/fileutils.cpp \
    Utils/savefile.cpp \
    Widgets/headerviewstretcher.cpp \
    Widgets/itemviews.cpp \
    Widgets/environmentdialog.cpp \
    Widgets/environmentwidget.cpp \
    Utils/gsettingseditor.cpp

HEADERS  += \
    Interfaces/configfileeditor.h \
    Components/magickonfug.h \
    Interfaces/exelauncher.h \
    Utils/diskutils.h \
    Utils/bootutils.h \
    Utils/dialogutils.h \
    mainwindow.h \
    global.h \
    aboutwindow.h \
    Utils/environment.h \
    Utils/environmentmodel.h \
    Utils/hostosinfo.h \
    Utils/fileutils.h \
    Utils/qtcassert.h \
    Utils/osspecificaspects.h \
    Utils/savefile.h \
    Widgets/headerviewstretcher.h \
    Widgets/itemviews.h \
    Widgets/environmentdialog.h \
    Widgets/environmentwidget.h \
    Utils/gsettingseditor.h

FORMS    += \
    Components/magickonfug.ui \
    mainwindow.ui \
    aboutwindow.ui

RESOURCES += \
    icons.qrc \
    translations.qrc \

TRANSLATIONS += \
    Translations/SuperPanda_fr_FR.ts \
    Translations/SuperPanda_zh_CN.ts \
    Translations/SuperPanda_zh_TW.ts

include(translation.pri)


target.path = $${PREFIX}/bin/

INSTALLS += target