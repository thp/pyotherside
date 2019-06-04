isEmpty(PYTHON_CONFIG) {
    PYTHON_CONFIG = python3-config
}

message(PYTHON_CONFIG = $$PYTHON_CONFIG)

QMAKE_LIBS += $$system($$PYTHON_CONFIG --ldflags --libs)
QMAKE_CXXFLAGS += $$system($$PYTHON_CONFIG --includes)
