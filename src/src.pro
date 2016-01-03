TARGET = pyothersideplugin

include(../pyotherside.pri)

DEFINES += PYOTHERSIDE_VERSION=\\\"$${VERSION}\\\"

PLUGIN_IMPORT_PATH = io/thp/pyotherside

TEMPLATE = lib
CONFIG += qt plugin
QT += qml quick svg

target.path = $$[QT_INSTALL_QML]/$$PLUGIN_IMPORT_PATH
INSTALLS += target

qmldir.files += $$_PRO_FILE_PWD_/qmldir $$_PRO_FILE_PWD_/pyotherside.qmltypes
qmldir.path += $$target.path
INSTALLS += qmldir

DEPENDPATH += .
INCLUDEPATH += .

# PyOtherSide QML Plugin
SOURCES += pyotherside_plugin.cpp
HEADERS += pyotherside_plugin.h

# QML Image Provider
SOURCES += qpython_imageprovider.cpp
HEADERS += qpython_imageprovider.h

# PyGLArea
SOURCES += pyglarea.cpp pyglrenderer.cpp
HEADERS += pyglarea.h pyglrenderer.h

# PyFBO
SOURCES += pyfbo.cpp
HEADERS += pyfbo.h

# Importer from Qt Resources
RESOURCES += qrc_importer.qrc

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
