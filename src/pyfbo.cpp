
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

#include "pyfbo.h"

#include <QtGui/QOpenGLFramebufferObject>


class PyFboRenderer : public QQuickFramebufferObject::Renderer
{
public:
    PyFboRenderer()
        : m_renderer(0)
        , m_oldRenderer(0)
    {
    }

    ~PyFboRenderer()
    {
        if (m_renderer) {
            delete m_renderer;
            m_renderer = 0;
        }
        if (m_oldRenderer) {
            delete m_oldRenderer;
            m_oldRenderer = 0;
        }
    }

    void render() {
        if (m_oldRenderer) {
            m_oldRenderer->cleanup();
            delete m_oldRenderer;
            m_oldRenderer = 0;
        }
        if (!m_renderer) {
            return;
        }
        m_renderer->init();
        m_renderer->render();
    }

    void setRenderer(QVariant rendererRef) {
        // Defer deleting the old renderer until render() is called,
        // when we have an OpenGL context.
        m_oldRenderer = m_renderer;
        m_renderer = new PyGLRenderer(rendererRef, false);
        update();
    }

    QOpenGLFramebufferObject *createFramebufferObject(const QSize &size) {
        QOpenGLFramebufferObjectFormat format;
        // TODO: get the FBO format from the PyGLRenderer.
        return new QOpenGLFramebufferObject(size, format);
    }
private:
    PyGLRenderer *m_renderer;
    PyGLRenderer *m_oldRenderer;
};

PyFbo::PyFbo()
    : m_fboRenderer(0)
{
}

void PyFbo::setRenderer(QVariant rendererRef)
{
    if (rendererRef == m_rendererRef)
        return;
    m_rendererRef = rendererRef;

    // If we already have a PyFboRenderer, set its python GL renderer.
    // Otherwise it will be set when createRenderer() is called.
    if (m_fboRenderer) {
        static_cast<PyFboRenderer *>(m_fboRenderer)->setRenderer(m_rendererRef);
    }
}

QQuickFramebufferObject::Renderer *PyFbo::createRenderer() const
{
    m_fboRenderer = new PyFboRenderer();

    // If we already have a python GL renderer, set it now.
    if (!m_rendererRef.isNull()) {
        static_cast<PyFboRenderer *>(m_fboRenderer)->setRenderer(m_rendererRef);
    }
    return m_fboRenderer;
}
