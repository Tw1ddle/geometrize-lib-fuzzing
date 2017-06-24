include($$PWD/lib/geometrize/geometrize/geometrize.pri)

CONFIG += warn_on console

HEADERS += lib/libbmpread/bmpread.h

SOURCES += lib/libbmpread/bmpread.c \
           main.cpp
