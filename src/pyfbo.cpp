
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

    ~PyFboRenderer()
    {
        if (m_renderer) {
            delete m_renderer;
            m_renderer = 0;
        }
    }

    void render()
    {
        if (!m_renderer)
            return;
        m_renderer->render();
    }

    void synchronize(QQuickFramebufferObject *item)
    {
        PyFbo *pyFbo = static_cast<PyFbo *>(item);

        if (pyFbo->renderer() == m_rendererRef)
            return;

        if (m_renderer) {
            m_renderer->cleanup();
            delete m_renderer;
            m_renderer = 0;
        }

        m_rendererRef = pyFbo->renderer();
        if (!m_rendererRef.isNull()) {
            m_renderer = new PyGLRenderer(m_rendererRef, false);
            m_renderer->init();
        }
    }

    QOpenGLFramebufferObject *createFramebufferObject(const QSize &size)
    {
        QOpenGLFramebufferObjectFormat format;
        // TODO: get the FBO format from the PyGLRenderer.
        return new QOpenGLFramebufferObject(size, format);
    }
private:
    QVariant m_rendererRef;
    PyGLRenderer *m_renderer;
};

PyFbo::PyFbo()
{
}

void PyFbo::setRenderer(QVariant rendererRef)
{
    if (rendererRef == m_rendererRef)
        return;
    m_rendererRef = rendererRef;
    update();
}

QQuickFramebufferObject::Renderer *PyFbo::createRenderer() const
{
    return new PyFboRenderer();
}
