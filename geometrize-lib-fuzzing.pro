TEMPLATE = app
CONFIG += console warn_on
CONFIG += core

QMAKE_CXXFLAGS += -std=c++1y

include($$PWD/lib/geometrize/geometrize/geometrize.pri)

SOURCES += main.cpp
