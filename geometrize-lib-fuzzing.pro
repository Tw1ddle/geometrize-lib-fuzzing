TEMPLATE = app
CONFIG += console warn_on
CONFIG -= app_bundle
CONFIG -= qt

QMAKE_CXXFLAGS += -std=c++1y

linux-g++ {
    LIBS += -lstdc++fs
}

include($$PWD/lib/geometrize/geometrize/geometrize.pri)

HEADERS += lib/stb/stb_image.h \
           lib/stb/stb_image_write.h

SOURCES += main.cpp
