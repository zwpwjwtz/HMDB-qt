#-------------------------------------------------
#
# Project created by QtCreator 2020-12-25T19:13:02
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = HMDB-qt
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
        main.cpp \
        mainwindow.cpp \
    dialogimport.cpp \
    hmdb/hmdbxmlimporter.cpp \
    utils/stdc.c \
    formquery.cpp \
    hmdb/hmdbquery.cpp \
    hmdb/hmdbsearch.cpp \
    hmdb/hmdbrecordgenerator.cpp \
    utils/filesystem.cpp \
    utils/uconfigentryobject.cpp \
    utils/uconfigfile.cpp \
    utils/uconfigio.cpp \
    utils/uconfigxml.cpp

HEADERS += \
        mainwindow.h \
    dialogimport.h \
    hmdb/hmdbxmlimporter.h \
    hmdb/hmdbxml_def.h \
    utils/stdc.h \
    formquery.h \
    hmdb/hmdbquery.h \
    hmdb/hmdbquery_p.h \
    hmdb/hmdbsearch.h \
    hmdb/hmdbsearch_p.h \
    hmdb/hmdbrecordgenerator.h \
    utils/filesystem.h \
    utils/uconfigxml.h \
    utils/uconfigentry.h \
    utils/uconfigentryobject.h \
    utils/uconfigfile.h \
    utils/uconfigio.h \
    utils/uconfigxml_p.h

FORMS += \
        mainwindow.ui \
    dialogimport.ui \
    formquery.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target