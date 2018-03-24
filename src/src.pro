TARGET = pyotherside

include(../pyotherside.pri)

DEFINES += PYOTHERSIDE_VERSION=\\\"$${VERSION}\\\"

TEMPLATE = lib
CONFIG += qt plugin
QT += qml

target.path = $$[QT_INSTALL_LIBS]
INSTALLS += target

headers.files = qpython.h
headers.path = $$[QT_INSTALL_HEADERS]/pyotherside
INSTALLS += headers

DEPENDPATH += .
INCLUDEPATH += .

# Embedded Python Library (add pythonlib.zip if you want this)
exists (pythonlib.zip) {
    RESOURCES += pythonlib_loader.qrc
    DEFINES *= PYTHONLIB_LOADER_HAVE_PYTHONLIB_ZIP
}

!windows {
    DEFINES *= HAVE_DLADDR
}

HEADERS += pythonlib_loader.h
SOURCES += pythonlib_loader.cpp

# Python QML Object
SOURCES += qpython.cpp
HEADERS += qpython.h
SOURCES += qpython_worker.cpp
HEADERS += qpython_worker.h
SOURCES += qpython_priv.cpp
HEADERS += qpython_priv.h

# Globally Load Python hack
SOURCES += global_libpython_loader.cpp
HEADERS += global_libpython_loader.h

# Reference-counting PyObject wrapper class
SOURCES += pyobject_ref.cpp
HEADERS += pyobject_ref.h

# QObject wrapper class exposed to Python
SOURCES += qobject_ref.cpp
HEADERS += qobject_ref.h
HEADERS += pyqobject.h

# GIL helper
HEADERS += ensure_gil_state.h

# Type System Conversion Logic
HEADERS += converter.h
HEADERS += qvariant_converter.h
HEADERS += pyobject_converter.h
HEADERS += qml_python_bridge.h

include(../python.pri)
