#-------------------------------------------------
#
# Project created by QtCreator 2014-05-26T10:01:50
#
#-------------------------------------------------

QT += core
QT += concurrent
QT += qml
QT += gui

TARGET = vegascene

CONFIG   += qt
CONFIG   -= app_bundle

TEMPLATE = app

CONFIG(debug, debug|release) {
    DEFINES += DEBUG
    TARGET = vegascene-dbg
}

QMAKE_CXXFLAGS += -std=c++98 -Wall


SOURCES += main.cpp \
    jscallback.cpp \
    jscallbackmanager.cpp \
    vegascene.cpp \
    jsconsole.cpp \
    data.cpp \
    jscontext2d.cpp \
    jsmodule.cpp

HEADERS += \
    vegascene.h \
    jscallback.h \
    jscallbackmanager.h \
    jsconsole.h \
    data.h \
    jscontext2d.h \
    jsmodule.h
