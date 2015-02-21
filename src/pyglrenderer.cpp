
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
#include "ensure_gil_state.h"

#include <QDebug>
#include <QMetaType>


PyGLRenderer::PyGLRenderer(QVariant pyRenderer)
    : m_pyRendererObject(0)
    , m_initMethod(0)
    , m_reshapeMethod(0)
    , m_renderMethod(0)
    , m_cleanupMethod(0)
    , m_initialized(false)
{

    ENSURE_GIL_STATE;

    if (pyRenderer.userType() != qMetaTypeId<PyObjectRef>()) {
        qWarning() << "Renderer must be of type PyObjectRef (got "
            << pyRenderer << ").";
        return;
    }

    m_pyRendererObject = pyRenderer.value<PyObjectRef>().newRef();

    if (PyObject_HasAttrString(m_pyRendererObject, "render")) {
        m_renderMethod = PyObject_GetAttrString(m_pyRendererObject, "render");
        if (!m_renderMethod) {
            qWarning() << "Error getting render method of renderer.";
            PyErr_PrintEx(0);
        }
    } else {
        qWarning() << "Renderer has no render method.";
    }

    if (PyObject_HasAttrString(m_pyRendererObject, "init")) {
        m_initMethod = PyObject_GetAttrString(m_pyRendererObject, "init");
        if (!m_initMethod) {
            qWarning() << "Error getting init method of renderer.";
            PyErr_PrintEx(0);
        }
    }

    if (PyObject_HasAttrString(m_pyRendererObject, "reshape")) {
        m_reshapeMethod = PyObject_GetAttrString(m_pyRendererObject, "reshape");
        if (!m_reshapeMethod) {
            qWarning() << "Error getting reshape method of renderer.";
            PyErr_PrintEx(0);
        }
    }

    if (PyObject_HasAttrString(m_pyRendererObject, "cleanup")) {
        m_cleanupMethod = PyObject_GetAttrString(m_pyRendererObject, "cleanup");
        if (!m_cleanupMethod) {
            qWarning() << "Error getting cleanup method of renderer.";
            PyErr_PrintEx(0);
        }
    }
}

PyGLRenderer::~PyGLRenderer()
{
    ENSURE_GIL_STATE;
    Py_CLEAR(m_initMethod);
    Py_CLEAR(m_reshapeMethod);
    Py_CLEAR(m_renderMethod);
    Py_CLEAR(m_cleanupMethod);
    Py_CLEAR(m_pyRendererObject);
}

void PyGLRenderer::init() {
    if (m_initialized || !m_initMethod)
        return;

    ENSURE_GIL_STATE;

    PyObject *args = PyTuple_New(0);
    PyObject *o = PyObject_Call(m_initMethod, args, NULL);
    if (o) Py_DECREF(o); else PyErr_PrintEx(0);
    Py_DECREF(args);
    m_initialized = true;
}

void PyGLRenderer::reshape(QRect geometry)
{
    if (!m_initialized || !m_reshapeMethod)
        return;

    ENSURE_GIL_STATE;

    // Call the reshape callback with arguments x, y, width, height.
    // These are the boundaries in which the callback should render,
    // though it may choose to ignore them and simply paint anywhere over
    // (or below) the QML scene.
    // (x, y) is the bottom left corner.
    // (x + width, y + height) is the top right corner.
    PyObject *args = Py_BuildValue(
        "llll", geometry.x(), geometry.y(), geometry.width(), geometry.height()
    );
    PyObject *o = PyObject_Call(m_reshapeMethod, args, NULL);
    Py_DECREF(args);
    if (o) Py_DECREF(o); else PyErr_PrintEx(0);
}

void PyGLRenderer::render()
{
    if (!m_initialized || !m_renderMethod)
        return;

    ENSURE_GIL_STATE;

    PyObject *args = PyTuple_New(0);
    PyObject *o = PyObject_Call(m_renderMethod, args, NULL);
    Py_DECREF(args);
    if (o) Py_DECREF(o); else PyErr_PrintEx(0);
}

void PyGLRenderer::cleanup()
{
    if (!m_initialized || !m_cleanupMethod)
        return;

    ENSURE_GIL_STATE;

    PyObject *args = PyTuple_New(0);
    PyObject *o = PyObject_Call(m_cleanupMethod, args, NULL);
    if (o) Py_DECREF(o); else PyErr_PrintEx(0);
    m_initialized = false;
    Py_DECREF(args);
}
