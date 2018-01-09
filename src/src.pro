include(../common-project-config.pri)

TEMPLATE = lib
TARGET = Accounts

API_URI = "Ubuntu.OnlineAccounts"

DESTDIR = $$replace(API_URI, \\., /)

CONFIG += \
    link_pkgconfig \
    plugin \
    qt

QT += qml

# Error on undefined symbols
QMAKE_LFLAGS += $$QMAKE_LFLAGS_NOUNDEF

PKGCONFIG += \
    accounts-qt5 \
    libsignon-qt5

CONFIG(debug) {
    DEFINES += \
        DEBUG_ENABLED
}

SOURCES += \
    account-service-model.cpp \
    account-service.cpp \
    account.cpp \
    application-model.cpp \
    application.cpp \
    credentials.cpp \
    debug.cpp \
    manager.cpp \
    plugin.cpp \
    provider-model.cpp

HEADERS += \
    account-service-model.h \
    account-service.h \
    account.h \
    application-model.h \
    application.h \
    credentials.h \
    debug.h \
    manager.h \
    plugin.h \
    provider-model.h

DEFINES += API_URI=\\\"$${API_URI}\\\"

qmldir_gen.input = qmldir.in
qmldir_gen.output = $${DESTDIR}/qmldir
QMAKE_SUBSTITUTES += qmldir_gen
OTHER_FILES += qmldir.in

PLUGIN_INSTALL_BASE = $$[QT_INSTALL_QML]/$$replace(API_URI, \\., /)
target.path = $${PLUGIN_INSTALL_BASE}
INSTALLS += target

qmldir.files = $${DESTDIR}/qmldir plugin.qmltypes
qmldir.path = $${PLUGIN_INSTALL_BASE}
INSTALLS += qmldir

QML_PLUGINS += $${DESTDIR}/lib$${TARGET}.so
qmltypes.commands = export LD_PRELOAD=$${QML_PLUGINS}; $$[QT_INSTALL_BINS]/qmlplugindump -notrelocatable $${API_URI} 0.1 . > $$PWD/plugin.qmltypes
qmltypes.depends = $${QML_PLUGINS}
QMAKE_EXTRA_TARGETS += qmltypes
