include($$PWD/geometrize/geometrize/geometrize.pri)

CONFIG += warn_on console

DEFINES += AFL_FUZZING

HEADERS += libbmpread/bmpread.h

SOURCES += libbmpread/bmpread.c \
           main.cpp
