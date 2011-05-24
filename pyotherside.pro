
QT += declarative

TEMPLATE = app
TARGET = pyotherside
DEPENDPATH += .
INCLUDEPATH += .

SOURCES += main.cpp

SOURCES += qpython.cpp
HEADERS += qpython.h

# Python
QMAKE_CXXFLAGS += -I/usr/include/python2.6 -I/usr/include/python2.6 -fno-strict-aliasing -g -O2 -DNDEBUG -g -fwrapv -O2 -Wall -Wstrict-prototypes
QMAKE_LIBS += -L/usr/lib/python2.6/config -lpthread -ldl -lutil -lm -lpython2.6

