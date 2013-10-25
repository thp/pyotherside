# Link against Python 3.3
PYTHON_CONFIG = python3.3-config
QMAKE_LIBS += $$system($$PYTHON_CONFIG --ldflags)
QMAKE_CXXFLAGS += $$system($$PYTHON_CONFIG --includes)

PYTHON_LIBRARIES = $$system($$PYTHON_CONFIG --libs)
PYTHON_LIBRARY = ""
for(lib, PYTHON_LIBRARIES) {
    lib ~= s/^-l/
    PYTHON_LIBRARY="$$lib"
}
DEFINES += PYTHON_LIBRARY=\\\"$$PYTHON_LIBRARY\\\"
