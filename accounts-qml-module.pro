include(common-vars.pri)
include(common-project-config.pri)

TEMPLATE = subdirs
CONFIG += ordered
SUBDIRS = \
    src \
    tests

include(common-installs-config.pri)

DISTNAME = $${PROJECT_NAME}-$${PROJECT_VERSION}
EXCLUDES = \
    --exclude-vcs \
    --exclude-from .bzrignore
    --exclude=$${DISTNAME}.tar.bz2
dist.commands = "tar -cvjf $${DISTNAME}.tar.bz2 $$EXCLUDES --transform='s,^,$$DISTNAME/,' *"
dist.depends = distclean
QMAKE_EXTRA_TARGETS += dist

include(doc/doc.pri)
