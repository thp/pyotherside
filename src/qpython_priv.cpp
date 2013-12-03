
/**
 * PyOtherSide: Asynchronous Python 3 Bindings for Qt 5
 * Copyright (c) 2011, 2013, Thomas Perl <m@thp.io>
 * Copyright (c) 2013 Benoît HERVIER <khertan@khertan.net>
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
#include "marshal.h"
#include <QImage>
#include <QFile>
#include <QDir>

static QPythonPriv *priv = NULL;

enum pyotherside_module_info {
    MI_ERROR,
    MI_NOT_FOUND,
    MI_MODULE,
    MI_PACKAGE
};

/* Given a buffer, return the long that is represented by the first
   4 bytes, encoded as little endian. This partially reimplements
   marshal.c:r_long() */
static long
get_long(unsigned char *buf) {
    long x;
    x =  buf[0];
    x |= (long)buf[1] <<  8;
    x |= (long)buf[2] << 16;
    x |= (long)buf[3] << 24;
#if SIZEOF_LONG > 4
    /* Sign extension for 64-bit machines */
    x |= -(x & 0x80000000L);
#endif
    return x;
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

/* Given the contents of a .py[co] file in a buffer, unmarshal the data
   and return the code object. Return None if it the magic word doesn't
   match (we do this instead of raising an exception as we fall back
   to .py if available and we don't want to mask other errors).
   Returns a new reference. */
static PyObject *
unmarshal_code(char *pathname, PyObject *data)
{
    PyObject *code;
    char *buf = PyBytes_AsString(data);
    Py_ssize_t size = PyBytes_Size(data);

    if (size <= 9) {
        PyErr_SetString(PyExc_TypeError,
                        "bad pyc data");
        return NULL;
    }

    if (get_long((unsigned char *)buf) != PyImport_GetMagicNumber()) {
        if (Py_VerboseFlag)
            PySys_WriteStderr("# %s has bad magic\n",
                              pathname);
        Py_INCREF(Py_None);
        return NULL;
    }

    code = PyMarshal_ReadObjectFromString(buf + 8, size - 8);
    if (code == NULL)
        return NULL;
    if (!PyCode_Check(code)) {
        Py_DECREF(code);
        PyErr_Format(PyExc_TypeError,
                     "compiled module %.200s is not a code object",
                     pathname);
        return NULL;
    }
    return code;
}

static PyObject *
pyotherside_get_code_from_data(QString filename) {

    QString buf;
    QDir qrc(":/");
    PyObject *module_code;
    FILE *fh;

    if (qrc.exists(filename+"/__init__.py")) {
        QFile module_data(":/"+filename+"/__init__.py");
        if(module_data.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream data(&module_data);
            buf = data.readAll();
            module_data.close();
            if (buf == NULL) {
                qDebug() << "Can t get code of "+filename.toUtf8();
            }

            // Compile module code
            module_code = Py_CompileString(buf.toUtf8().constData(), filename.toUtf8().constData(), Py_file_input);
            if (module_code != NULL){
                return module_code;
            }
            else {
                qDebug() << "Can t compile code of "+filename.toUtf8();
                return NULL;
            }
        } else {
            qDebug() << "Can't get content from qrc (__init__) for "+filename.toUtf8();
        }
    }

    // WIP
    /*if (qrc.exists(filename+".pyc")) {
        QFile module_data(":/"+filename+".pyc");
        qDebug() << "Opening compiled module pyc : " + filename.toUtf8();
        if(module_data.open(QIODevice::ReadOnly | QIODevice::Text)) {
            int h = module_data.handle();          
            FILE* fh = fdopen(h, "rb");
            qDebug() << "Read MarchalObject from fh : " + filename.toUtf8();
            module_code = PyMarshal_ReadObjectFromFile(fh);
            fclose(fh);
            qDebug() << "Close MarchalObject from fh : " + filename.toUtf8();

            //Load bytecode
            qDebug() << "Loading bytecode from fh : " + filename.toUtf8();
            module_code = unmarshal_code(filename.toUtf8().data(), PyBytes_FromString(buf.toUtf8().data()));
            if (module_code != NULL){
                qDebug() << "Bytecode loaded from fh : " + filename.toUtf8();
                return module_code;
            }

        }
    }*/

    // WIP
    /*if (qrc.exists(filename+".pyo")) {
        QFile module_data(":/"+filename+".pyo");
        if(module_data.open(QIODevice::ReadOnly | QIODevice::Text)) {
            buf = module_data.readAll().constData();

            //Load bytecode
            module_code = unmarshal_code(filename.toUtf8().data(), PyBytes_FromString(buf));
            if (module_code != NULL){
                return module_code;
            }
        }
    }*/

    if (qrc.exists(filename+".py")) {
        QFile module_data(":/"+filename+".py");
        if(module_data.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream data(&module_data);
            buf = data.readAll();
            module_data.close();

            qDebug() << "Content:";
            qDebug() << buf;

            // Compile module code
            module_code = Py_CompileString(buf.toUtf8().constData(), filename.toUtf8().constData(), Py_file_input);
            if (module_code != NULL){
                return module_code;
            }
            else {
                qDebug() << "Can t compile code of "+filename.toUtf8();
                return NULL;

            }
        } else {
            qDebug() << "Can't get qrc content for " + filename.toUtf8();
        }
    }

    qDebug() << filename.toUtf8() + ' not found';
    return NULL;
}

PyObject *
pyotherside_find_module(PyObject *self, PyObject *args) {

    char *fullname, *path;
    int err = PyArg_ParseTuple(args, "s|z", &fullname, &path);

    QString filename(fullname);
    qDebug() << "Search for module " + filename.toUtf8();
    if(err == 0)
    {
        PyObject_Print(PyErr_Occurred(), stdout, Py_PRINT_RAW);
        PySys_WriteStdout("\n");
        PyErr_Print();
        PySys_WriteStdout("\n");
        return Py_None;
    }

    QDir qrc(":/");
    if (((qrc.exists(filename+".py"))
         | (qrc.exists(filename+".pyc"))
         | (qrc.exists(filename+".pyo")))
            | (QDir(":/"+filename).exists())) {
        Py_INCREF(self);
        qDebug() << "Found module "+filename.toUtf8();
        return self;
    }
    qDebug() << "Can't found module "+filename.toUtf8();
    Py_INCREF(Py_None);
    return Py_None;
}

PyObject *
pyotherside_load_module(PyObject *self, PyObject *args) {

    qDebug() << "pyotherside_load_module called";


    char *fullname;
    PyArg_ParseTuple(args, "s", &fullname);
    PyObject *mod, *dict;
    PyObject *module_code;

    QString filename(fullname);

    mod = PyImport_AddModule(fullname);
    if (mod == NULL) {
        qDebug() << "Can't add module " + filename.toUtf8();
        Py_RETURN_NONE;
    }

    module_code = pyotherside_get_code_from_data(filename);
    if (module_code == NULL) {
        qDebug() << "Can't get module code " + filename.toUtf8();
        Py_RETURN_NONE;
    }

    // Set the __loader__ object to pyotherside module
    dict = PyModule_GetDict(mod);
    if (PyDict_SetItemString(dict, "__loader__", (PyObject *)self) != 0) {
        Py_RETURN_NONE;
    }

    mod = PyImport_ExecCodeModuleEx(fullname, module_code, fullname);
    Py_DECREF(module_code);
    Py_DECREF(dict);
    if (Py_VerboseFlag)
        PySys_WriteStderr("import %s # loaded from QRC\n",
                          fullname);

    return mod;
}

static PyMethodDef PyOtherSideMethods[] = {
    {"send", pyotherside_send, METH_VARARGS, "Send data to Qt."},
    {"atexit", pyotherside_atexit, METH_O, "Function to call on shutdown."},
    {"set_image_provider", pyotherside_set_image_provider, METH_O, "Set the QML image provider."},
    {"find_module", pyotherside_find_module, METH_VARARGS},
    {"load_module", pyotherside_load_module, METH_VARARGS},
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

    PyObject *meta_path = PySys_GetObject("meta_path");
    if (meta_path != NULL)
    {
        PyList_Append(meta_path, pyotherside);
    }

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

    qDebug() << v.toString();
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
