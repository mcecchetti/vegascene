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

DEFINES += WITH_TOPOJSON
DEFINES += WITH_EXAMPLE_LIBS


QMAKE_CXXFLAGS += -std=c++98 -Wall


SOURCES += \
    data.cxx \
    jscallback.cxx \
    jscallbackmanager.cxx \
    jsconsole.cxx \
    jscontext2d.cxx \
    jsmodule.cxx \
    main.cxx \
    vegascene.cxx

HEADERS += \
    vegascene.h \
    jscallback.h \
    jscallbackmanager.h \
    jsconsole.h \
    data.h \
    jscontext2d.h \
    jsmodule.h
