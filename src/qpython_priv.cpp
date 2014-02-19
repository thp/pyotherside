
/**
 * PyOtherSide: Asynchronous Python 3 Bindings for Qt 5
 * Copyright (c) 2011, 2013, 2014, Thomas Perl <m@thp.io>
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
#include <QDebug>
#include <QResource>
#include <QFile>
#include <QDir>

static QPythonPriv *priv = NULL;

static QString
qstring_from_pyobject_arg(PyObject *object)
{
    PyObjectConverter conv;

    if (conv.type(object) != PyObjectConverter::STRING) {
        PyErr_SetString(PyExc_ValueError, "Argument must be a string");
        return QString();
    }

    return QString::fromUtf8(conv.string(object));
}

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

PyObject *
pyotherside_qrc_is_file(PyObject *self, PyObject *filename)
{
    QString qfilename = qstring_from_pyobject_arg(filename);

    if (qfilename.isNull()) {
        return NULL;
    }

    if (QFile(":" + qfilename).exists()) {
        Py_RETURN_TRUE;
    }

    Py_RETURN_FALSE;
}

PyObject *
pyotherside_qrc_is_dir(PyObject *self, PyObject *dirname)
{
    QString qdirname = qstring_from_pyobject_arg(dirname);

    if (qdirname.isNull()) {
        return NULL;
    }

    if (QDir(":" + qdirname).exists()) {
        Py_RETURN_TRUE;
    }

    Py_RETURN_FALSE;
}

PyObject *
pyotherside_qrc_get_file_contents(PyObject *self, PyObject *filename)
{
    QString qfilename = qstring_from_pyobject_arg(filename);

    if (qfilename.isNull()) {
        return NULL;
    }

    QFile file(":" + qfilename);
    if (!file.exists() || !file.open(QIODevice::ReadOnly)) {
        PyErr_SetString(PyExc_ValueError, "File not found");
        return NULL;
    }

    QByteArray ba = file.readAll();
    return PyByteArray_FromStringAndSize(ba.constData(), ba.size());
}

PyObject *
pyotherside_qrc_list_dir(PyObject *self, PyObject *dirname)
{
    QString qdirname = qstring_from_pyobject_arg(dirname);

    if (qdirname.isNull()) {
        return NULL;
    }

    QDir dir(":" + qdirname);
    if (!dir.exists()) {
        PyErr_SetString(PyExc_ValueError, "Directory not found");
        return NULL;
    }

    return convertQVariantToPyObject(dir.entryList());
}

static PyMethodDef PyOtherSideMethods[] = {
    /* Introduced in PyOtherSide 1.0 */
    {"send", pyotherside_send, METH_VARARGS, "Send data to Qt."},
    {"atexit", pyotherside_atexit, METH_O, "Function to call on shutdown."},

    /* Introduced in PyOtherSide 1.1 */
    {"set_image_provider", pyotherside_set_image_provider, METH_O, "Set the QML image provider."},

    /* Introduced in PyOtherSide 1.3 */
    {"qrc_is_file", pyotherside_qrc_is_file, METH_O, "Check if a file exists in Qt Resources."},
    {"qrc_is_dir", pyotherside_qrc_is_dir, METH_O, "Check if a directory exists in Qt Resources."},
    {"qrc_get_file_contents", pyotherside_qrc_get_file_contents, METH_O, "Get file contents from a Qt Resource."},
    {"qrc_list_dir", pyotherside_qrc_list_dir, METH_O, "Get directory entries from a Qt Resource."},

    /* sentinel */
    {NULL, NULL, 0, NULL},
};

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

    // Version of PyOtherSide (new in 1.3)
    PyModule_AddStringConstant(pyotherside, "version", PYOTHERSIDE_VERSION);

    return pyotherside;
}

QPythonPriv::QPythonPriv()
    : locals(NULL)
    , globals(NULL)
    , state(NULL)
    , atexit_callback(NULL)
    , image_provider(NULL)
    , traceback_mod(NULL)
    , mutex()
{
    PyImport_AppendInittab("pyotherside", PyOtherSide_init);

    Py_Initialize();
    PyEval_InitThreads();

    locals = PyDict_New();
    assert(locals != NULL);

    globals = PyDict_New();
    assert(globals != NULL);

    traceback_mod = PyImport_ImportModule("traceback");
    assert(traceback_mod != NULL);

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

    PyObject *list = NULL;
    PyObject *n = NULL;
    PyObject *s = NULL;

    PyErr_Fetch(&type, &value, &traceback);
    PyErr_NormalizeException(&type, &value, &traceback);

    QString message;
    QVariant v;

    if (type == NULL && value == NULL && traceback == NULL) {
        // No exception thrown?
        goto cleanup;
    }

    if (value != NULL) {
        // We can at least format the exception as string
        message = convertPyObjectToQVariant(PyObject_Str(value)).toString();
    }

    if (type == NULL || traceback == NULL) {
        // Cannot get a traceback for this exception
        goto cleanup;
    }

    list = PyObject_CallMethod(traceback_mod,
            "format_exception", "OOO", type, value, traceback);

    if (list == NULL) {
        // Could not format exception, fall back to original message
        PyErr_Print();
        goto cleanup;
    }

    n = PyUnicode_FromString("\n");
    if (n == NULL) {
        PyErr_Print();
        goto cleanup;
    }

    s = PyUnicode_Join(n, list);
    if (s == NULL) {
        PyErr_Print();
        goto cleanup;
    }

    v = convertPyObjectToQVariant(s);
    if (v.isValid()) {
        message = v.toString();
    }

cleanup:
    Py_XDECREF(s);
    Py_XDECREF(n);
    Py_XDECREF(list);

    Py_XDECREF(type);
    Py_XDECREF(value);
    Py_XDECREF(traceback);

    qDebug() << QString("PyOtherSide error: %1").arg(message);
    return message;
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

QString
QPythonPriv::importFromQRC(const char *module, const QString &filename)
{
    PyObject *sys_modules = PySys_GetObject((char *)"modules");
    if (!PyMapping_Check(sys_modules)) {
        return QString("sys.modules is not a mapping object");
    }

    PyObject *qrc_importer = PyMapping_GetItemString(sys_modules,
            (char *)module);

    if (qrc_importer == NULL) {
        PyErr_Clear();

        QFile qrc_importer_code(":" + filename);
        if (!qrc_importer_code.open(QIODevice::ReadOnly)) {
            return QString("Cannot load qrc importer source");
        }

        QByteArray ba = qrc_importer_code.readAll();
        QByteArray fn = QString("qrc:/" + filename).toUtf8();

        PyObject *co = Py_CompileString(ba.constData(), fn.constData(),
                Py_file_input);
        if (co == NULL) {
            QString result = QString("Cannot compile qrc importer: %1")
                .arg(formatExc());
            PyErr_Clear();
            return result;
        }

        qrc_importer = PyImport_ExecCodeModule((char *)module, co);
        if (qrc_importer == NULL) {
            QString result = QString("Cannot exec qrc importer: %1")
                    .arg(formatExc());
            PyErr_Clear();
            return result;
        }
        Py_XDECREF(co);
    }

    Py_XDECREF(qrc_importer);

    return QString();
}
