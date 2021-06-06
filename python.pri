isEmpty(PYTHON_CONFIG) {
    PYTHON_CONFIG = python3-config
}

message(PYTHON_CONFIG = $$PYTHON_CONFIG)

QMAKE_LIBS += $$system($$PWD/python-config-wrapper $$PYTHON_CONFIG --libs)
QMAKE_CXXFLAGS += $$system($$PWD/python-config-wrapper $$PYTHON_CONFIG --includes)
