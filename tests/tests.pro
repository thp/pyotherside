QT += testlib qml
CONFIG -= app_bundle

include(../pyotherside.pri)
DEFINES += PYOTHERSIDE_VERSION=\\\"$${VERSION}\\\"

SOURCES += tests.cpp
HEADERS += tests.h

SOURCES += ../src/qpython.cpp
SOURCES += ../src/qpython_worker.cpp
SOURCES += ../src/qpython_priv.cpp

HEADERS += ../src/qpython.h
HEADERS += ../src/qpython_worker.h
HEADERS += ../src/qpython_priv.h
HEADERS += ../src/converter.h
HEADERS += ../src/qvariant_converter.h
HEADERS += ../src/pyobject_converter.h

DEPENDPATH += . ../src
INCLUDEPATH += . ../src

include(../python.pri)
