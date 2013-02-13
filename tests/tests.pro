include(../common-project-config.pri)
include($${TOP_SRC_DIR}/common-vars.pri)
include($${TOP_SRC_DIR}/common-installs-config.pri)

TARGET = tst_plugin

CONFIG += \
    debug

QT += \
    core \
    qml \
    testlib

SOURCES += \
    tst_plugin.cpp

HEADERS += \
    tst_plugin.h

INCLUDEPATH += \
    $$TOP_SRC_DIR/src

check.commands = "./$${TARGET}"
check.depends = $${TARGET}
QMAKE_EXTRA_TARGETS += check
