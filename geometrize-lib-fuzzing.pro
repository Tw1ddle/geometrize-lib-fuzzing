include($$PWD/geometrize/geometrize/geometrize.pri)

CONFIG += warn_on console

HEADERS += libbmpread/bmpread.h

SOURCES += libbmpread/bmpread.c \
           main.cpp
