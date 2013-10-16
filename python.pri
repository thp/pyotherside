# Link against Python 3.3
PYTHON_CONFIG = python3.3-config

unix: {
QMAKE_LIBS += $$system($$PYTHON_CONFIG --ldflags)
QMAKE_CXXFLAGS += $$system($$PYTHON_CONFIG --includes)
}

win32: {
INCLUDEPATH += C:\Python27\include
LIBS += -LC:\Python27\libs -lpython27
}

#RESOURCES +=
