#-------------------------------------------------
#
# Project created by QtCreator 2020-04-27T17:35:52
#
#-------------------------------------------------

QT       += core gui sql axcontainer

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = FileManagerVega
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
        Lemmatizer.cpp \
        agramtab.cpp \
        filesystem.cpp \
        inifile.cpp \
        main.cpp \
        mainwindow.cpp \
        panel.cpp \
        sortableheaderview.cpp \
        tinyxml2.cpp \
        tipdbshell.cpp \
        trmlshell.cpp \
        viewip.cpp \
        workspace.cpp

HEADERS += \
        Lemmatizer.h \
        agramtab.h \
        filesystem.h \
        inifile.h \
        mainwindow.h \
        panel.h \
        sortableheaderview.h \
        tinyxml2.h \
        tipdbshell.h \
        trmlshell.h \
        viewip.h \
        workspace.h

FORMS += \
        mainwindow.ui \
        viewip.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    LIB/libpq.dll \
    LIB/libpq.lib \
    resources/dbf.png \
    resources/folder.png \
    resources/icon \
    resources/icon.ico \
    resources/redo.png \
    resources/undo.png

LIBS += -L$$PWD/LIB

LIBS += -llibpq
