include(../common-project-config.pri)
include($${TOP_SRC_DIR}/common-vars.pri)
include($${TOP_SRC_DIR}/common-installs-config.pri)

TARGET = tst_plugin

CONFIG += \
    debug \
    link_pkgconfig

QT += \
    core \
    qml \
    testlib

PKGCONFIG += \
    accounts-qt5

SOURCES += \
    tst_plugin.cpp

HEADERS += \
    tst_plugin.h

INCLUDEPATH += \
    $$TOP_SRC_DIR/src

DATA_PATH = $${TOP_SRC_DIR}/tests/data/

DEFINES += \
    SERVICES_DIR=\\\"$$DATA_PATH\\\" \
    SERVICE_TYPES_DIR=\\\"$$DATA_PATH\\\" \
    PROVIDERS_DIR=\\\"$$DATA_PATH\\\"

check.commands = "LD_LIBRARY_PATH=mock:${LD_LIBRARY_PATH} xvfb-run -a dbus-test-runner -t ./$${TARGET}"
check.depends = $${TARGET}
QMAKE_EXTRA_TARGETS += check
