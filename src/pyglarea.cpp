
/**
 * PyOtherSide: Asynchronous Python 3 Bindings for Qt 5 and Qt 6
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
#include <QOpenGLShaderProgram>
#include <QOpenGLContext>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#  include <QQuickOpenGLUtils>
#endif

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
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            // If we allow QML to do the clearing, they would clear what we paint
            // and nothing would show.

            window()->setClearBeforeRendering(false);
            connect(window(), SIGNAL(beforeRendering()), this, SLOT(render()), Qt::DirectConnection);
#else
            // For Qt 6, we don't support rendering as underlay; see also:
            // https://doc.qt.io/qt-6/qquickwindow.html#integration-with-accelerated-3d-graphics-apis
            // See also (search for "setClearBeforeRendering" on that page):
            // https://doc.qt.io/qt-6/quick-changes-qt6.html
            qWarning() << "PyGLArea doesn't work properly in Qt 6 yet, please use PyFBO instead.";
            connect(window(), SIGNAL(beforeRendering()), this, SLOT(render()), Qt::DirectConnection);
#endif
        } else {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            window()->setClearBeforeRendering(true);
#endif
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
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            window()->resetOpenGLState();
#else
            QQuickOpenGLUtils::resetOpenGLState();
#endif
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
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    window()->resetOpenGLState();
#else
    QQuickOpenGLUtils::resetOpenGLState();
#endif
}

void PyGLArea::cleanup()
{
    if (m_renderer) m_renderer->cleanup();
}
