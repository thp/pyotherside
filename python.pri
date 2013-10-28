# Link against Python 3.3
PYTHON_CONFIG = python3.3-config
QMAKE_LIBS += $$system($$PYTHON_CONFIG --ldflags)
QMAKE_CXXFLAGS += $$system($$PYTHON_CONFIG --includes)

# Determinate the python library we link against and set
# a define for it. Its the last entry in the via --libs
# returned list of libraries.
PYTHON_LIBRARIES = $$system($$PYTHON_CONFIG --libs)
PYTHON_LIBRARY = ""
for(lib, PYTHON_LIBRARIES) {
    lib ~= s/^-l/
    PYTHON_LIBRARY="$$lib"
}
DEFINES += PYTHON_LIBRARY=\\\"$$PYTHON_LIBRARY\\\"
