
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
    : m_t(0)
    , m_before(false)
    , m_renderer(0)
{
    connect(this, SIGNAL(windowChanged(QQuickWindow*)), this, SLOT(handleWindowChanged(QQuickWindow*)));
}

PyGLArea::~PyGLArea()
{
    delete m_renderer;
}

void PyGLArea::setInitGL(QString initGL)
{
    if (initGL == m_initGL)
        return;
    m_renderer->setInitGL(initGL);
    m_initGL = initGL;
    if (window())
        window()->update();
}

void PyGLArea::setPaintGL(QString paintGL)
{
    if (paintGL == m_paintGL)
        return;
    m_paintGL = paintGL;
    m_renderer->setPaintGL(paintGL);
    if (window())
        window()->update();
}

void PyGLArea::setCleanupGL(QString cleanupGL)
{
    if (cleanupGL == m_cleanupGL)
        return;
    m_cleanupGL = cleanupGL;
    m_renderer->setCleanupGL(cleanupGL);
    if (window())
        window()->update();
}

void PyGLArea::setBefore(bool before)
{
    if (before == m_before)
        return;
    m_before = before;
}

void PyGLArea::setT(qreal t)
{
    if (t == m_t)
        return;
    m_t = t;
    emit tChanged();
    if (window())
        window()->update();
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
    window()->update();
}

void PyGLArea::sync()
{
    if (!m_renderer) {
        m_renderer = new PyGLRenderer();
        if (m_before)
            connect(window(), SIGNAL(beforeRendering()), this, SLOT(paint()), Qt::DirectConnection);
        else
            connect(window(), SIGNAL(afterRendering()), this, SLOT(paint()), Qt::DirectConnection);
    }
}

void PyGLArea::paint()
{
    QPointF pos = mapToScene(QPointF(.0, .0));
    m_renderer->setRect(
        QRect(
            (long)pos.x(), (long)(window()->height() - this->height() - pos.y()),
            (long)this->width(), (long)this->height()
        )
    );
    m_renderer->init();
    m_renderer->paint();
    window()->resetOpenGLState();
}

void PyGLArea::cleanup()
{
    m_renderer->cleanup();
}
