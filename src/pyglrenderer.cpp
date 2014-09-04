
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
#include "pyglrenderer.h"

#include <QDebug>


PyGLRenderer::PyGLRenderer()
    : m_paintGLCallable(0)
    , m_initialized(false)
    , m_initGL("")
    , m_paintGL("")
{
}

PyGLRenderer::~PyGLRenderer()
{
    if (m_paintGLCallable) {
        QPythonPriv *priv = QPythonPriv::instance();
        priv->enter();
        Py_DECREF(m_paintGLCallable);
        priv->leave();
        m_paintGLCallable = 0;
    }
}

void PyGLRenderer::setInitGL(QString initGL)
{
    if (initGL == m_initGL)
        return;
    m_initGL = initGL;
    m_initialized = false;
}

void PyGLRenderer::setPaintGL(QString paintGL)
{
    if (paintGL == m_paintGL)
        return;
    m_paintGL = paintGL;
    if (m_paintGLCallable) {
        QPythonPriv *priv = QPythonPriv::instance();
        priv->enter();
        Py_DECREF(m_paintGLCallable);
        priv->leave();
    }
}

void PyGLRenderer::setCleanupGL(QString cleanupGL)
{
    if (cleanupGL == m_cleanupGL)
        return;
    m_cleanupGL = cleanupGL;
}

void PyGLRenderer::setRect(QRect rect) {
    m_rect = rect;
}

void PyGLRenderer::init() {
    if (m_initialized)
        return;

    if (!m_initGL.isEmpty()) {
        QPythonPriv *priv = QPythonPriv::instance();
        priv->enter();
        PyObject *initGLCallable = priv->eval(m_initGL);
        if (!initGLCallable) {
            qWarning() << "Init callback " << m_initGL << " not defined.";
            priv->leave();
            return;
        }
        PyObject *args = PyTuple_New(0);
        PyObject *o = PyObject_Call(initGLCallable, args, NULL);
        if (o) Py_DECREF(o); else PyErr_PrintEx(0);
        Py_DECREF(args);
        Py_DECREF(initGLCallable);
        priv->leave();
    }
    m_initialized = true;
}


void PyGLRenderer::paint()
{
    if (!m_initialized || m_paintGL.isEmpty())
        return;

    QPythonPriv *priv = QPythonPriv::instance();
    priv->enter();

    if (!m_paintGLCallable) {
        m_paintGLCallable = priv->eval(m_paintGL);
        if (!m_paintGLCallable) {
            qWarning() << "Paint callback " << m_paintGL << " not defined.";
            priv->leave();
            return;
        }
    }

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
    PyObject *o = PyObject_Call(m_paintGLCallable, args, NULL);
    if (o) Py_DECREF(o); else PyErr_PrintEx(0);
    Py_DECREF(x);
    Py_DECREF(y);
    Py_DECREF(width);
    Py_DECREF(height);
    Py_DECREF(args);
    priv->leave();
}

void PyGLRenderer::cleanup()
{
    if (m_cleanupGL.isEmpty())
        return;

    QPythonPriv *priv = QPythonPriv::instance();
    priv->enter();
    PyObject *cleanupGLCallable = priv->eval(m_cleanupGL);
    if (!cleanupGLCallable) {
        qWarning() << "Cleanup callback " << m_cleanupGL << " not defined.";
        priv->leave();
        return;
    }
    PyObject *args = PyTuple_New(0);
    PyObject *o = PyObject_Call(cleanupGLCallable, args, NULL);
    if (o) Py_DECREF(o); else PyErr_PrintEx(0);
    m_initialized = true;
    Py_DECREF(args);
    Py_DECREF(cleanupGLCallable);
    priv->leave();
}
