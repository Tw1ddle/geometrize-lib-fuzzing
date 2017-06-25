TEMPLATE = app
CONFIG += console c++14 warn_on
CONFIG -= app_bundle
CONFIG -= qt

include($$PWD/lib/geometrize/geometrize/geometrize.pri)

HEADERS += lib/stb/stb_image.h \
           lib/stb/stb_image_write.h

SOURCES += main.cpp
