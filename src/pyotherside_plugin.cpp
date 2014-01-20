
/**
 * PyOtherSide: Asynchronous Python 3 Bindings for Qt 5
 * Copyright (c) 2011, 2013, Thomas Perl <m@thp.io>
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
#include "qpython_imageprovider.h"

#include "pyotherside_plugin.h"

#include <QLibrary>

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

#if defined(Q_OS_UNIX)
    // Its needed to dlopen the python library using RTLD_GLOBAL so modules
    // dynamic loaded later, which not link against the python library, find
    // its symbols. This is only needed on Unix where our/python symbols may
    // be hidden and not acccessible to libraries/modules we/python loads. By
    // loading the python library here again using RTLD_GLOBAL we make its
    // symbols proper accessible to other libraries/modules.
    QByteArray pythonlib(PYTHON_LIBRARY);
    if (!pythonlib.isEmpty()) {
        QLibrary lib(pythonlib);
        lib.setLoadHints(QLibrary::ExportExternalSymbolsHint | QLibrary::ResolveAllSymbolsHint);
        if (!lib.load())
            qWarning() << "Failed loading python library" << pythonlib << lib.errorString();
    }
#endif

    engine->addImageProvider(PYOTHERSIDE_IMAGEPROVIDER_ID, new QPythonImageProvider);
}

void
PyOtherSideExtensionPlugin::registerTypes(const char *uri)
{
    Q_ASSERT(QString(PYOTHERSIDE_PLUGIN_ID) == uri);
    qmlRegisterType<QPython>(uri, 1, 0, PYOTHERSIDE_QPYTHON_NAME);
}
