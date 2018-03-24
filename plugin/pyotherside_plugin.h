
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

#ifndef PYOTHERSIDE_PLUGIN_H
#define PYOTHERSIDE_PLUGIN_H

#include <QtQml>
#include <QQmlExtensionPlugin>

#define PYOTHERSIDE_PLUGIN_ID "io.thp.pyotherside"
#define PYOTHERSIDE_IMAGEPROVIDER_ID "python"
#define PYOTHERSIDE_QPYTHON_NAME "Python"
#define PYOTHERSIDE_QPYGLAREA_NAME "PyGLArea"
#define PYOTHERSIDE_PYFBO_NAME "PyFBO"

class Q_DECL_EXPORT PyOtherSideExtensionPlugin : public QQmlExtensionPlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID PYOTHERSIDE_PLUGIN_ID)

    public:
        PyOtherSideExtensionPlugin();
        ~PyOtherSideExtensionPlugin();

        virtual void initializeEngine(QQmlEngine *engine, const char *uri);
        virtual void registerTypes(const char *uri);
};

#endif /* PYOTHERSIDE_PLUGIN_H */
