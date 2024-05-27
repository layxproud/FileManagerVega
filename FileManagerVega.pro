#-------------------------------------------------
#
# Project created by QtCreator 2020-04-27T17:35:52
#
#-------------------------------------------------

QT       += core gui sql axcontainer xml

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
        classifywindow.cpp \
        filesystem.cpp \
        indexwindow.cpp \
        inifile.cpp \
        ipcompare.cpp \
        loginwindow.cpp \
        main.cpp \
        mainwindow.cpp \
        panel.cpp \
        portraitlistwidget.cpp \
        servicehandler.cpp \
        tinyxml2.cpp \
        tipdbshell.cpp \
        trmlshell.cpp \
        viewip.cpp \
        workspace.cpp \
        xmlparser.cpp

HEADERS += \
        Lemmatizer.h \
        agramtab.h \
        classifywindow.h \
        filesystem.h \
        indexwindow.h \
        inifile.h \
        ipcompare.h \
        loginwindow.h \
        mainwindow.h \
        panel.h \
        portraitlistwidget.h \
        servicehandler.h \
        tinyxml2.h \
        tipdbshell.h \
        trmlshell.h \
        viewip.h \
        workspace.h \
        xmlparser.h

FORMS += \
        classifywindow.ui \
        indexwindow.ui \
        ipcompare.ui \
        loginwindow.ui \
        mainwindow.ui \
        viewip.ui

LIBCURL_DEBUG_PATH = C:/lib/install/libcurl-debug
LIBCURL_RELEASE_PATH = C:/lib/install/libcurl-release

CONFIG(debug, debug|release) {
    LIBS += -L$$LIBCURL_DEBUG_PATH/lib -llibcurl_debug
    INCLUDEPATH += $$LIBCURL_DEBUG_PATH/include
} else {
    LIBS += -L$$LIBCURL_RELEASE_PATH/lib -llibcurl
    INCLUDEPATH += $$LIBCURL_RELEASE_PATH/include
}

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    res.qrc
