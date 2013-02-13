TEMPLATE = lib
TARGET = Accounts

API_URI = "Ubuntu.OnlineAccounts"

DESTDIR = $$replace(API_URI, \\., /)

CONFIG += \
    plugin \
    qt

QT += qml

SOURCES += \
    plugin.cpp

HEADERS += \
    plugin.h

DEFINES += API_URI=\\\"$${API_URI}\\\"

QMLDIR_FILES += qmldir
QMAKE_SUBSTITUTES += qmldir.in
OTHER_FILES += qmldir.in

copy2build.output = $${DESTDIR}/${QMAKE_FILE_IN}
copy2build.input = QMLDIR_FILES
copy2build.commands = $$QMAKE_COPY ${QMAKE_FILE_IN} ${QMAKE_FILE_OUT}
copy2build.name = COPY ${QMAKE_FILE_IN}
copy2build.variable_out = PRE_TARGETDEPS
copy2build.CONFIG += no_link
QMAKE_EXTRA_COMPILERS += copy2build

PLUGIN_INSTALL_BASE = $$[QT_INSTALL_QML]/$$replace(API_URI, \\., /)
target.path = $${PLUGIN_INSTALL_BASE}
INSTALLS += target

qmldir.files = qmldir
qmldir.path = $${PLUGIN_INSTALL_BASE}
INSTALLS += qmldir
