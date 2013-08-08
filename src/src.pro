TARGET = pyothersideplugin

PLUGIN_IMPORT_PATH = io/thp/pyotherside

TEMPLATE = lib
CONFIG += qt plugin
QT += qml

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

# Python QML Object
SOURCES += qpython.cpp
HEADERS += qpython.h
SOURCES += qpython_worker.cpp
HEADERS += qpython_worker.h
SOURCES += qpython_priv.cpp
HEADERS += qpython_priv.h

# Type System Conversion Logic
HEADERS += converter.h
HEADERS += qvariant_converter.h
HEADERS += pyobject_converter.h
HEADERS += qml_python_bridge.h

include(../python.pri)
