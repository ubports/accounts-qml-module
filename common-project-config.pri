#-----------------------------------------------------------------------------
# Common configuration for all projects.
#-----------------------------------------------------------------------------

# we don't like warnings...
QMAKE_CXXFLAGS += -Werror -Wno-write-strings
# Disable RTTI
QMAKE_CXXFLAGS += -fno-exceptions -fno-rtti
# Use C++11
QMAKE_CXXFLAGS += -std=c++11

TOP_SRC_DIR     = $$PWD
TOP_BUILD_DIR   = $${TOP_SRC_DIR}/$${BUILD_DIR}

include(coverage.pri)
