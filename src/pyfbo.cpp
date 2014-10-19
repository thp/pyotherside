
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
    {
    }

    void render() {
        if (!m_renderer) {
            return;
        }
        m_renderer->init();
        m_renderer->render();
        update();
    }

    void setRenderer(PyGLRenderer *renderer) {
        m_renderer = renderer;
        update();
    }

    QOpenGLFramebufferObject *createFramebufferObject(const QSize &size) {
        QOpenGLFramebufferObjectFormat format;
        // TODO: get the FBO format from the PyGLRenderer.
        return new QOpenGLFramebufferObject(size, format);
    }
private:
    PyGLRenderer *m_renderer;
};

PyFbo::PyFbo()
    : m_fboRenderer(0)
    , m_renderer(0)
{
}

PyFbo::~PyFbo()
{
    if (m_renderer) delete m_renderer;
    // Do not delete m_fboRenderer, because we don't own it!
}

void PyFbo::setRenderer(QVariant rendererRef)
{
    if (rendererRef == m_rendererRef)
        return;
    m_rendererRef = rendererRef;

    // Delete the old python GL renderer.
    if (m_renderer) {
        delete m_renderer;
        m_renderer = 0;
    }

    // Create a renderer object from the reference.
    m_renderer = new PyGLRenderer(m_rendererRef, false);

    // If we already have a PyFboRenderer, set its python GL renderer.
    // Otherwise it will be set when createRenderer() is called.
    if (m_fboRenderer) {
        static_cast<PyFboRenderer *>(m_fboRenderer)->setRenderer(m_renderer);
    }
}

QQuickFramebufferObject::Renderer *PyFbo::createRenderer() const
{
    m_fboRenderer = new PyFboRenderer();

    // If we already have a python GL renderer, set it now.
    if (m_renderer) {
        static_cast<PyFboRenderer *>(m_fboRenderer)->setRenderer(m_renderer);
    }
    return m_fboRenderer;
}
