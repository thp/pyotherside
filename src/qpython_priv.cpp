
/**
 * PyOtherSide: Asynchronous Python 3 Bindings for Qt 5
 * Copyright (c) 2011, 2013, Thomas Perl <m@thp.io>
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

#include "qml_python_bridge.h"

#include "qpython_priv.h"

#include <QImage>

static QPythonPriv *priv = NULL;

PyObject *
pyotherside_send(PyObject *self, PyObject *args)
{
    priv->receiveObject(args);
    Py_RETURN_NONE;
}

PyObject *
pyotherside_atexit(PyObject *self, PyObject *o)
{
    if (priv->atexit_callback != NULL) {
        Py_DECREF(priv->atexit_callback);
    }

    Py_INCREF(o);
    priv->atexit_callback = o;

    Py_RETURN_NONE;
}

PyObject *
pyotherside_set_image_provider(PyObject *self, PyObject *o)
{
    if (priv->image_provider != NULL) {
        Py_DECREF(priv->image_provider);
    }

    Py_INCREF(o);
    priv->image_provider = o;

    Py_RETURN_NONE;
}

static PyMethodDef PyOtherSideMethods[] = {
    {"send", pyotherside_send, METH_VARARGS, "Send data to Qt."},
    {"atexit", pyotherside_atexit, METH_O, "Function to call on shutdown."},
    {"set_image_provider", pyotherside_set_image_provider, METH_O, "Set the QML image provider."},
    {NULL, NULL, 0, NULL},
};

#ifdef PY3K
static struct PyModuleDef PyOtherSideModule = {
    PyModuleDef_HEAD_INIT,
    "pyotherside",   /* name of module */
    NULL,
    -1,
    PyOtherSideMethods,
};

PyMODINIT_FUNC
PyOtherSide_init()
{
    PyObject *pyotherside = PyModule_Create(&PyOtherSideModule);

    // Format constants for the image provider return value format
    // see http://qt-project.org/doc/qt-5.1/qtgui/qimage.html#Format-enum
    PyModule_AddIntConstant(pyotherside, "format_mono", QImage::Format_Mono);
    PyModule_AddIntConstant(pyotherside, "format_mono_lsb", QImage::Format_MonoLSB);
    PyModule_AddIntConstant(pyotherside, "format_rgb32", QImage::Format_RGB32);
    PyModule_AddIntConstant(pyotherside, "format_argb32", QImage::Format_ARGB32);
    PyModule_AddIntConstant(pyotherside, "format_rgb16", QImage::Format_RGB16);
    PyModule_AddIntConstant(pyotherside, "format_rgb666", QImage::Format_RGB666);
    PyModule_AddIntConstant(pyotherside, "format_rgb555", QImage::Format_RGB555);
    PyModule_AddIntConstant(pyotherside, "format_rgb888", QImage::Format_RGB888);
    PyModule_AddIntConstant(pyotherside, "format_rgb444", QImage::Format_RGB444);

    // Custom constant - pixels are to be interpreted as encoded image file data
    PyModule_AddIntConstant(pyotherside, "format_data", -1);

    return pyotherside;
}
#endif

QPythonPriv::QPythonPriv()
    : locals(NULL)
    , globals(NULL)
    , state(NULL)
    , atexit_callback(NULL)
    , image_provider(NULL)
    , traceback_mod(NULL)
    , mutex()
{
#ifdef PY3K
    PyImport_AppendInittab("pyotherside", PyOtherSide_init);
#endif

    Py_Initialize();
    PyEval_InitThreads();

    locals = PyDict_New();
    assert(locals != NULL);

    globals = PyDict_New();
    assert(globals != NULL);

    traceback_mod = PyImport_ImportModule("traceback");
    assert(traceback_mod != NULL);

#ifndef PY3K
    Py_InitModule("pyotherside", PyOtherSideMethods);
#endif

    priv = this;

    if (PyDict_GetItemString(globals, "__builtins__") == NULL) {
        PyDict_SetItemString(globals, "__builtins__",
                PyEval_GetBuiltins());
    }

    // Need to lock mutex here, as it will always be unlocked
    // by leave(). If we don't do that, it will be unlocked
    // once too often resulting in undefined behavior.
    mutex.lock();
    leave();
}

QPythonPriv::~QPythonPriv()
{
    enter();
    Py_DECREF(traceback_mod);
    Py_DECREF(globals);
    Py_DECREF(locals);
    Py_Finalize();
}

void
QPythonPriv::enter()
{
    mutex.lock();
    assert(state != NULL);
    PyEval_RestoreThread(state);
    state = NULL;
}

void
QPythonPriv::leave()
{
    assert(state == NULL);
    state = PyEval_SaveThread();
    mutex.unlock();
}

void
QPythonPriv::receiveObject(PyObject *o)
{
    emit receive(convertPyObjectToQVariant(o));
}

QString
QPythonPriv::formatExc()
{
    PyObject *type = NULL;
    PyObject *value = NULL;
    PyObject *traceback = NULL;
    PyErr_Fetch(&type, &value, &traceback);
    PyErr_Clear();

    if (type == NULL && value == NULL && traceback == NULL) {
        return "No Error";
    }

    if (value != NULL && (type == NULL || traceback == NULL)) {
        return convertPyObjectToQVariant(PyObject_Str(value)).toString();
    }

    PyObject *list = PyObject_CallMethod(traceback_mod,
            "format_exception", "OOO", type, value, traceback);
    Q_ASSERT(list != NULL);
    PyObject *n = PyUnicode_FromString("\n");
    Q_ASSERT(n != NULL);
    PyObject *s = PyUnicode_Join(n, list);
    Q_ASSERT(s != NULL);
    if (s == NULL) {
        PyErr_Print();
        return "Exception";
    }
    QVariant v = convertPyObjectToQVariant(s);
    Q_ASSERT(v.isValid());
    Py_DECREF(s);
    Py_DECREF(n);
    Py_DECREF(list);

    Py_DECREF(type);
    Py_DECREF(value);
    Py_DECREF(traceback);

    return v.toString();
}

PyObject *
QPythonPriv::eval(QString expr)
{
    QByteArray utf8bytes = expr.toUtf8();
    PyObject *result = PyRun_String(utf8bytes.constData(),
            Py_eval_input, globals, locals);

    return result;
}

void
QPythonPriv::closing()
{
    if (!priv) {
        return;
    }

    priv->enter();
    if (priv->atexit_callback != NULL) {
        PyObject *args = PyTuple_New(0);
        PyObject *result = PyObject_Call(priv->atexit_callback, args, NULL);
        Py_DECREF(args);
        Py_XDECREF(result);

        Py_DECREF(priv->atexit_callback);
        priv->atexit_callback = NULL;
    }
    if (priv->image_provider != NULL) {
        Py_DECREF(priv->image_provider);
        priv->image_provider = NULL;
    }
    priv->leave();
}

QPythonPriv *
QPythonPriv::instance()
{
    return priv;
}
