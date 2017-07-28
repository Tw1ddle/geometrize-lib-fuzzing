TEMPLATE = app
CONFIG += console c++14 warn_on
CONFIG += core

include($$PWD/lib/geometrize/geometrize/geometrize.pri)

SOURCES += main.cpp
