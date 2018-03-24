
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

#include "qpython_priv.h"
#include "pyglarea.h"

#include <QVariant>
#include <QPointF>
#include <QtQuick/qquickwindow.h>
#include <QtGui/QOpenGLShaderProgram>
#include <QtGui/QOpenGLContext>


PyGLArea::PyGLArea()
    : m_before(true)
    , m_renderer(0)
    , m_rendererChanged(false)
    , m_beforeChanged(true)
{
    connect(this, SIGNAL(windowChanged(QQuickWindow*)), this, SLOT(handleWindowChanged(QQuickWindow*)));
}

PyGLArea::~PyGLArea()
{
    if (m_renderer) {
        delete m_renderer;
        m_renderer = 0;
    }
}

void PyGLArea::setRenderer(QVariant renderer)
{
    if (renderer == m_pyRenderer)
        return;
    m_pyRenderer = renderer;

    // Defer creating the PyGLRenderer until sync() is called,
    // when we have an OpenGL context.
    m_rendererChanged = true;
    update();
}

void PyGLArea::setBefore(bool before)
{
    if (before == m_before)
        return;
    m_before = before;

    m_beforeChanged = true;
    update();
}

void PyGLArea::handleWindowChanged(QQuickWindow *win)
{
    if (win) {
        connect(win, SIGNAL(beforeSynchronizing()), this, SLOT(sync()), Qt::DirectConnection);
        connect(win, SIGNAL(sceneGraphInvalidated()), this, SLOT(cleanup()), Qt::DirectConnection);
    }
}

void PyGLArea::update() {
    if (window())
        window()->update();
}

void PyGLArea::sync()
{
    if (m_beforeChanged) {
        disconnect(window(), SIGNAL(beforeRendering()), this, SLOT(render()));
        disconnect(window(), SIGNAL(afterRendering()), this, SLOT(render()));
        if (m_before) {
            // If we allow QML to do the clearing, they would clear what we paint
            // and nothing would show.
            window()->setClearBeforeRendering(false);
            connect(window(), SIGNAL(beforeRendering()), this, SLOT(render()), Qt::DirectConnection);
        } else {
            window()->setClearBeforeRendering(true);
            connect(window(), SIGNAL(afterRendering()), this, SLOT(render()), Qt::DirectConnection);
        }
        m_beforeChanged = false;
    }

    if (m_rendererChanged) {
        if (m_renderer) {
            m_renderer->cleanup();
            delete m_renderer;
            m_renderer = 0;
        }
        if (!m_pyRenderer.isNull()) {
            m_renderer = new PyGLRenderer(m_pyRenderer);
            m_renderer->init();
            window()->resetOpenGLState();
        }
        m_rendererChanged = false;
    }
}

void PyGLArea::render()
{
    if (!m_renderer)
        return;
    QPointF pos = mapToScene(QPointF(.0, .0));
    m_renderer->reshape(
        QRect(
            (long)pos.x(), (long)(window()->height() - this->height() - pos.y()),
            this->width(), this->height()
        )
    );
    m_renderer->render();
    window()->resetOpenGLState();
}

void PyGLArea::cleanup()
{
    if (m_renderer) m_renderer->cleanup();
}
