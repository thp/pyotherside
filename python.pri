isEmpty(PYTHON_CONFIG) {
    PYTHON_CONFIG = python3-config
}

message(PYTHON_CONFIG = $$PYTHON_CONFIG)

QMAKE_LIBS += $$system($$PYTHON_CONFIG --ldflags)
QMAKE_CXXFLAGS += $$system($$PYTHON_CONFIG --includes)
