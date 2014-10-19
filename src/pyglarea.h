
/**
 * PyOtherSide: Asynchronous Python 3 Bindings for Qt 5
 * Copyright (c) 2014, Dennis Tomas <den.t@gmx.de>
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

#ifndef PYOTHERSIDE_PYGLAREA_H
#define PYOTHERSIDE_PYGLAREA_H

#include "Python.h"

#include <QString>
#include <QVariant>
#include <QtQuick/QQuickItem>

#include "pyobject_ref.h"

#include "pyglrenderer.h"


class PyGLArea : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(QVariant renderer READ renderer WRITE setRenderer)
    Q_PROPERTY(bool before READ before WRITE setBefore)

public:
    PyGLArea();
    ~PyGLArea();

    QVariant renderer() const { return m_pyRenderer; };
    bool before() { return m_before; };
    void setRenderer(QVariant renderer);
    void setBefore(bool before);

public slots:
    void sync();
    void update();

private slots:
    void handleWindowChanged(QQuickWindow *win);
    void render();
    void cleanup();

private:
    QVariant m_pyRenderer;
    bool m_before;
    PyGLRenderer *m_renderer;
    bool m_rendererChanged;
    bool m_beforeChanged;
};

#endif /* PYOTHERSIDE_PYGLAREA_H */
