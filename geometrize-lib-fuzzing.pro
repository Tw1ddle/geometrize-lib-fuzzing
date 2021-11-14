TEMPLATE = app
CONFIG += console warn_on
CONFIG += core

CONFIG += std=c++17

include($$PWD/geometrize-lib/geometrize/geometrize.pri)

SOURCES += main.cpp
