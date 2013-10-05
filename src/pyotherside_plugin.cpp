
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
    engine->addImageProvider(PYOTHERSIDE_IMAGEPROVIDER_ID, new QPythonImageProvider);
}

void
PyOtherSideExtensionPlugin::registerTypes(const char *uri)
{
    Q_ASSERT(QString(PYOTHERSIDE_PLUGIN_ID) == uri);
    qmlRegisterType<QPython>(uri, 1, 0, PYOTHERSIDE_QPYTHON_NAME);
}
