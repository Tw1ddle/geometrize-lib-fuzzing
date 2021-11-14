TEMPLATE = app
CONFIG += console warn_on
CONFIG += core

QMAKE_CXXFLAGS += -std=c++1y

include($$PWD/geometrize-lib/geometrize/geometrize.pri)

SOURCES += main.cpp

## Run the tests after successful linking
#isEmpty(TARGET_EXT) {
#    win32 {
#        TARGET_CUSTOM_EXT = .exe
#    }
#    macx {
#        #TARGET_CUSTOM_EXT = .app # Seems like no file extension is added
#    }
#} else {
#    TARGET_CUSTOM_EXT = $${TARGET_EXT}
#}

## For some reason the executables are placed in debug/release folders on Windows (MSVC)
## but in one directory up on Linux (gcc)
#win32 {
#    CONFIG(debug, debug|release) {
#        TARGET_DIR = $$shell_path($${OUT_PWD}/debug)
#    } else {
#        TARGET_DIR = $$shell_path($${OUT_PWD}/release)
#    }
#}
#unix {
#    TARGET_DIR = $$shell_path($${OUT_PWD}/)
#}

#win32 {
#    RUN_TESTS = $$shell_quote($$shell_path($${TARGET_DIR}/$${TARGET}$${TARGET_CUSTOM_EXT}))
#}
#unix {
#    RUN_TESTS = $$shell_quote($$shell_path($${TARGET_DIR}/./$${TARGET}$${TARGET_CUSTOM_EXT}))
#}

## Comment this out if you do not want to run the tests during development
#QMAKE_POST_LINK = $${RUN_TESTS}
