TARGET = pyothersideplugin

include(../pyotherside.pri)

DEFINES += PYOTHERSIDE_VERSION=\\\"$${VERSION}\\\"

PLUGIN_IMPORT_PATH = io/thp/pyotherside

TEMPLATE = lib
CONFIG += qt plugin
QT += qml quick

target.path = $$[QT_INSTALL_QML]/$$PLUGIN_IMPORT_PATH
INSTALLS += target

qmldir.files += $$_PRO_FILE_PWD_/qmldir
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

# Type System Conversion Logic
HEADERS += converter.h
HEADERS += qvariant_converter.h
HEADERS += pyobject_converter.h
HEADERS += qml_python_bridge.h

include(../python.pri)
