isEmpty(PYTHON_CONFIG) {
    PYTHON_CONFIG = x86_64-linux-gnu-python3.4-dbg-config
}

message(PYTHON_CONFIG = $$PYTHON_CONFIG)

QMAKE_LIBS += $$system($$PYTHON_CONFIG --ldflags)
QMAKE_CXXFLAGS += $$system($$PYTHON_CONFIG --includes)
