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

LIBS += -L../src/ -lpyotherside
DEPENDPATH += . ../src
INCLUDEPATH += . ../src

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

!windows {
    DEFINES *= HAVE_DLADDR
}

include(../python.pri)
