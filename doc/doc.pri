QDOC = $$[QT_INSTALL_BINS]/qdoc

QMAKE_EXTRA_TARGETS += docs clean-docs docs-html clean-docs-html

docs.depends = docs-html

docs-html.commands = \
    "$${QDOC} $${PWD}/accounts-qml-module.qdocconf"

clean-docs-html.commands = \
    "rm -rf $${PWD}/html"

clean-docs.depends = clean-docs-html
