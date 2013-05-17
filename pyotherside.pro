
QT += declarative

CONFIG -= app_bundle

TEMPLATE = app
TARGET = pyotherside
DEPENDPATH += .
INCLUDEPATH += .

SOURCES += main.cpp

SOURCES += qpython.cpp
HEADERS += qpython.h

SOURCES += qpython_test.cpp
HEADERS += qpython_test.h

# Python
QMAKE_CXXFLAGS += -I/usr/include/python2.6
QMAKE_LIBS += -lpython2.6
#QMAKE_CXXFLAGS += -fno-strict-aliasing -g -O2 -DNDEBUG -g -fwrapv -O2 -Wall -Wstrict-prototypes

