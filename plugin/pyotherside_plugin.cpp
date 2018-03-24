
/**
 * PyOtherSide: Asynchronous Python 3 Bindings for Qt 5
 * Copyright (c) 2011, 2013, 2014, Thomas Perl <m@thp.io>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 **/

#include "qpython_priv.h"
#include "qpython.h"
#include "pyglarea.h"
#include "pyfbo.h"
#include "qpython_imageprovider.h"
#include "global_libpython_loader.h"
#include "pythonlib_loader.h"

#include "pyotherside_plugin.h"


static void
pyotherside_atexit()
{
    QPythonPriv::closing();
}

PyOtherSideExtensionPlugin::PyOtherSideExtensionPlugin()
{
    atexit(pyotherside_atexit);
}

PyOtherSideExtensionPlugin::~PyOtherSideExtensionPlugin()
{
}

void
PyOtherSideExtensionPlugin::initializeEngine(QQmlEngine *engine, const char *uri)
{
    Q_ASSERT(QString(PYOTHERSIDE_PLUGIN_ID) == uri);

    // In some Linux distributions, the plugin (and subsequently libpython)
    // isn't loaded with the RTLD_GLOBAL flag, so symbols in libpython that
    // are needed by shared Python modules won't be resolved unless we also
    // load libpython again RTLD_GLOBAL again. We do this here.
    GlobalLibPythonLoader::loadPythonGlobally();

    // Extract and load embedded Python Standard Library, if necessary
    PythonLibLoader::extractPythonLibrary();

    engine->addImageProvider(PYOTHERSIDE_IMAGEPROVIDER_ID, new QPythonImageProvider);
}

void
PyOtherSideExtensionPlugin::registerTypes(const char *uri)
{
    Q_ASSERT(QString(PYOTHERSIDE_PLUGIN_ID) == uri);
    qmlRegisterType<QPython10>(uri, 1, 0, PYOTHERSIDE_QPYTHON_NAME);
    // There is no PyOtherSide 1.1 import, as it's the same as 1.0
    qmlRegisterType<QPython12>(uri, 1, 2, PYOTHERSIDE_QPYTHON_NAME);
    qmlRegisterType<QPython13>(uri, 1, 3, PYOTHERSIDE_QPYTHON_NAME);
    qmlRegisterType<QPython14>(uri, 1, 4, PYOTHERSIDE_QPYTHON_NAME);
    qmlRegisterType<QPython15>(uri, 1, 5, PYOTHERSIDE_QPYTHON_NAME);
    qmlRegisterType<PyGLArea>(uri, 1, 5, PYOTHERSIDE_QPYGLAREA_NAME);
    qmlRegisterType<PyFbo>(uri, 1, 5, PYOTHERSIDE_PYFBO_NAME);
}
