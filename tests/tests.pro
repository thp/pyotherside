QT += testlib qml
CONFIG -= app_bundle

SOURCES += tests.cpp
HEADERS += tests.h

SOURCES += ../src/qpython.cpp
SOURCES += ../src/qpython_priv.cpp

HEADERS += ../src/qpython.h
HEADERS += ../src/qpython_priv.h
HEADERS += ../src/converter.h
HEADERS += ../src/qvariant_converter.h
HEADERS += ../src/pyobject_converter.h

DEPENDPATH += . ../src
INCLUDEPATH += . ../src

include(../python.pri)
