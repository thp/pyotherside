
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
    : m_before(false)
    , m_renderer(0)
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
    if (m_renderer) {
        delete m_renderer;
        m_renderer = 0;
    }
    update();
}

void PyGLArea::setBefore(bool before)
{
    if (before == m_before)
        return;
    m_before = before;
    if (m_renderer) {
        delete m_renderer;
        m_renderer = 0;
    }
    update();
}

void PyGLArea::handleWindowChanged(QQuickWindow *win)
{
    if (win) {
        connect(win, SIGNAL(beforeSynchronizing()), this, SLOT(sync()), Qt::DirectConnection);
        connect(win, SIGNAL(sceneGraphInvalidated()), this, SLOT(cleanup()), Qt::DirectConnection);
        // If we allow QML to do the clearing, they would clear what we paint
        // and nothing would show.
        win->setClearBeforeRendering(false);
    }
}

void PyGLArea::update() {
    if (window())
        window()->update();
}

void PyGLArea::sync()
{
    if (!m_renderer && !m_pyRenderer.isNull()) {
        disconnect(window(), SIGNAL(beforeRendering()), this, SLOT(render()));
        disconnect(window(), SIGNAL(afterRendering()), this, SLOT(render()));
        m_renderer = new PyGLRenderer(m_pyRenderer);
        if (m_before)
            connect(window(), SIGNAL(beforeRendering()), this, SLOT(render()), Qt::DirectConnection);
        else
            connect(window(), SIGNAL(afterRendering()), this, SLOT(render()), Qt::DirectConnection);
    }
}

void PyGLArea::render()
{
    QPointF pos = mapToScene(QPointF(.0, .0));
    m_renderer->setRect(
        QRect(
            (long)pos.x(), (long)(window()->height() - this->height() - pos.y()),
            (long)this->width(), (long)this->height()
        )
    );
    m_renderer->init();
    m_renderer->render();
    window()->resetOpenGLState();
}

void PyGLArea::cleanup()
{
    m_renderer->cleanup();
}
