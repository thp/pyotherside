
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
#include "converter.h"
#include "pyobject_ref.h"
#include "pyglrenderer.h"

#include <QDebug>
#include <QMetaType>


PyGLRenderer::PyGLRenderer(QVariant pyRenderer, bool useRect)
    : m_pyRendererObject(0)
    , m_renderMethod(0)
    , m_initialized(false)
{
    m_pyRenderer = pyRenderer;
    m_useRect = useRect;
}

PyGLRenderer::~PyGLRenderer()
{
    if (m_pyRendererObject) {
        QPythonPriv *priv = QPythonPriv::instance();
        priv->enter();
        Py_CLEAR(m_pyRendererObject);
        if (m_renderMethod)
            Py_DECREF(m_renderMethod);
        priv->leave();
        m_pyRendererObject = 0;
        m_renderMethod = 0;
    }
}

void PyGLRenderer::setRect(QRect rect) {
    m_rect = rect;
}

void PyGLRenderer::init() {
    if (m_initialized)
        return;

    QPythonPriv *priv = QPythonPriv::instance();
    priv->enter();

    PyObject *pyRendererObject = getPyRendererObject();
    if (!pyRendererObject || pyRendererObject == Py_None) {
        priv->leave();
        return;
    }

    if (!PyObject_HasAttrString(m_pyRendererObject, "init")) {
        // Optional init() method not found, consider the renderer initialized.
        priv->leave();
        m_initialized = true;
        return;
    }

    PyObject *initMethod = PyObject_GetAttrString(m_pyRendererObject, "init");
    if (!initMethod) {
        qWarning() << "Failed to get init method of renderer.";
        PyErr_PrintEx(0);
        priv->leave();
        return;
    }

    PyObject *args = PyTuple_New(0);
    PyObject *o = PyObject_Call(initMethod, args, NULL);
    if (o) Py_DECREF(o); else PyErr_PrintEx(0);
    Py_DECREF(args);
    Py_DECREF(initMethod);
    priv->leave();
    m_initialized = true;
}


void PyGLRenderer::render()
{
    if (!m_initialized)
        return;

    QPythonPriv *priv = QPythonPriv::instance();
    priv->enter();

    PyObject *renderMethod = getRenderMethod();
    if (!renderMethod) {
        qWarning() << "Failed to get render method of renderer.";
        PyErr_PrintEx(0);
        priv->leave();
        return;
    }

    PyObject *o = NULL;
    if (m_useRect) {
        // Call the paintGL callback with arguments x, y, width, height.
        // These are the boundaries in which the callback should render,
        // though it may choose to ignore them and simply paint anywhere over
        // (or below) the QML scene.
        // (x, y) is the bottom left corner.
        // (x + width, y + height) is the top right corner.
        PyObject *x = PyLong_FromLong(m_rect.x());
        PyObject *y = PyLong_FromLong(m_rect.y());
        PyObject *width = PyLong_FromLong(m_rect.width());
        PyObject *height = PyLong_FromLong(m_rect.height());
        PyObject *args = PyTuple_Pack(4, x, y, width, height);
        o = PyObject_Call(renderMethod, args, NULL);
        Py_DECREF(x);
        Py_DECREF(y);
        Py_DECREF(width);
        Py_DECREF(height);
        Py_DECREF(args);
    } else {
        PyObject *args = PyTuple_New(0);
        o = PyObject_Call(renderMethod, args, NULL);
        Py_DECREF(args);
    }
    if (o) Py_DECREF(o); else PyErr_PrintEx(0);
    priv->leave();
}

void PyGLRenderer::cleanup()
{
    if (!m_initialized)
        return;

    ENSURE_GIL_STATE;

    PyObject *pyRendererObject = getPyRendererObject();
    if (!pyRendererObject || pyRendererObject == Py_None ||
            !PyObject_HasAttrString(m_pyRendererObject, "cleanup")) {
        priv->leave();
        return;
    }

    PyObject *cleanupMethod = PyObject_GetAttrString(m_pyRendererObject, "cleanup");
    if (!cleanupMethod) {
        qWarning() << "Failed to get cleanup method of renderer.";
        PyErr_PrintEx(0);
        priv->leave();
        return;
    }

    PyObject *args = PyTuple_New(0);
    PyObject *o = PyObject_Call(cleanupMethod, args, NULL);
    if (o) Py_DECREF(o); else PyErr_PrintEx(0);
    m_initialized = false;
    Py_DECREF(args);
    Py_DECREF(cleanupMethod);
    priv->leave();
}

PyObject *PyGLRenderer::getPyRendererObject() {
    if (m_pyRendererObject) {
        return m_pyRendererObject;
    }
    if (m_pyRenderer.userType() != qMetaTypeId<PyObjectRef>()) {
        qWarning() << "Renderer must be of type PyObjectRef (got "
            << m_pyRenderer << ").";
        return NULL;
    }
    m_pyRendererObject = m_pyRenderer.value<PyObjectRef>().newRef();
    return m_pyRendererObject;
}

PyObject *PyGLRenderer::getRenderMethod() {
    if (m_renderMethod) {
        return m_renderMethod;
    }

    PyObject *pyRendererObject = getPyRendererObject();
    if (!pyRendererObject || pyRendererObject == Py_None) {
        return NULL;
    }

    if (!PyObject_HasAttrString(m_pyRendererObject, "render")) {
        qWarning() << "Renderer has no render method.";
        return NULL;
    }

    PyObject *m_renderMethod = PyObject_GetAttrString(m_pyRendererObject, "render");
    if (!m_renderMethod) {
        qWarning() << "Failed to get render method of renderer.";
        PyErr_PrintEx(0);
        return NULL;
    }

    return m_renderMethod;
}
