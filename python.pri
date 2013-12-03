# Link against Python 3.3
PYTHON_CONFIG = python3.3-config

unix: {
QMAKE_LIBS += $$system($$PYTHON_CONFIG --ldflags)
QMAKE_CXXFLAGS += $$system($$PYTHON_CONFIG --includes)
}

win32: {
INCLUDEPATH += C:\Python33\include
LIBS += -LC:\Python33\libs -lpython33

QMAKE_LIBS += -IC:\Python33\include -fno-strict-aliasing -DNDEBUG -g -fwrapv -O2 -Wall -g -fstack-protector
}

#RESOURCES +=
