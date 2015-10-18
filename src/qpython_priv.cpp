
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

#include "ensure_gil_state.h"

#include <QImage>
#include <QDebug>
#include <QResource>
#include <QFile>
#include <QDir>

#include <QMetaObject>
#include <QMetaProperty>
#include <QMetaMethod>
#include <QGenericArgument>

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


PyTypeObject pyotherside_QObjectType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "pyotherside.QObject", /* tp_name */
    sizeof(pyotherside_QObject), /* tp_basicsize */
    0, /* tp_itemsize */
    0, /* tp_dealloc */
    0, /* tp_print */
    0, /* tp_getattr */
    0, /* tp_setattr */
    0, /* tp_reserved */
    0, /* tp_repr */
    0, /* tp_as_number */
    0, /* tp_as_sequence */
    0, /* tp_as_mapping */
    0, /* tp_hash  */
    0, /* tp_call */
    0, /* tp_str */
    0, /* tp_getattro */
    0, /* tp_setattro */
    0, /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT, /* tp_flags */
    "Wrapped QObject", /* tp_doc */
};

PyTypeObject pyotherside_QObjectMethodType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "pyotherside.QObjectMethod", /* tp_name */
    sizeof(pyotherside_QObjectMethod), /* tp_basicsize */
    0, /* tp_itemsize */
    0, /* tp_dealloc */
    0, /* tp_print */
    0, /* tp_getattr */
    0, /* tp_setattr */
    0, /* tp_reserved */
    0, /* tp_repr */
    0, /* tp_as_number */
    0, /* tp_as_sequence */
    0, /* tp_as_mapping */
    0, /* tp_hash  */
    0, /* tp_call */
    0, /* tp_str */
    0, /* tp_getattro */
    0, /* tp_setattro */
    0, /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT, /* tp_flags */
    "Bound method of wrapped QObject", /* tp_doc */
};



PyObject *
pyotherside_send(PyObject *self, PyObject *args)
{
    priv->receiveObject(args);
    Py_RETURN_NONE;
}

PyObject *
pyotherside_atexit(PyObject *self, PyObject *o)
{
    priv->atexit_callback = PyObjectRef(o);

    Py_RETURN_NONE;
}

PyObject *
pyotherside_set_image_provider(PyObject *self, PyObject *o)
{
    priv->image_provider = PyObjectRef(o);

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

void
pyotherside_QObject_dealloc(pyotherside_QObject *self)
{
    delete self->m_qobject_ref;
    Py_TYPE(self)->tp_free((PyObject *)self);
}

PyObject *
pyotherside_QObject_repr(PyObject *o)
{
    if (!PyObject_TypeCheck(o, &pyotherside_QObjectType)) {
        return PyErr_Format(PyExc_TypeError, "Not a pyotherside.QObject");
    }

    pyotherside_QObject *pyqobject = reinterpret_cast<pyotherside_QObject *>(o);
    QObjectRef *ref = pyqobject->m_qobject_ref;
    if (ref) {
        QObject *qobject = ref->value();
        const QMetaObject *metaObject = qobject->metaObject();

        return PyUnicode_FromFormat("<pyotherside.QObject wrapper for %s at %p>",
                metaObject->className(), qobject);
    }

    return PyUnicode_FromFormat("<dangling pyotherside.QObject wrapper>");
}

PyObject *
pyotherside_QObject_getattro(PyObject *o, PyObject *attr_name)
{
    if (!PyObject_TypeCheck(o, &pyotherside_QObjectType)) {
        return PyErr_Format(PyExc_TypeError, "Not a pyotherside.QObject");
    }

    if (!PyUnicode_Check(attr_name)) {
        return PyErr_Format(PyExc_TypeError, "attr_name must be a string");
    }

    pyotherside_QObject *pyqobject = reinterpret_cast<pyotherside_QObject *>(o);
    QObjectRef *ref = pyqobject->m_qobject_ref;
    if (!ref) {
        return PyErr_Format(PyExc_ValueError, "Dangling QObject");
    }
    QObject *qobject = ref->value();
    if (!qobject) {
        return PyErr_Format(PyExc_ReferenceError, "Referenced QObject was deleted");
    }

    const QMetaObject *metaObject = qobject->metaObject();
    QString attrName = convertPyObjectToQVariant(attr_name).toString();

    for (int i=0; i<metaObject->propertyCount(); i++) {
        QMetaProperty property = metaObject->property(i);
        if (attrName == property.name()) {
            return convertQVariantToPyObject(property.read(qobject));
        }
    }

    for (int i=0; i<metaObject->methodCount(); i++) {
        QMetaMethod method = metaObject->method(i);
        if (attrName == method.name()) {
            pyotherside_QObjectMethod *result = PyObject_New(pyotherside_QObjectMethod,
                    &pyotherside_QObjectMethodType);
            result->m_method_ref = new QObjectMethodRef(*ref, attrName);
            return reinterpret_cast<PyObject *>(result);
        }
    }

    return PyErr_Format(PyExc_AttributeError, "Not a valid attribute");
}

int
pyotherside_QObject_setattro(PyObject *o, PyObject *attr_name, PyObject *v)
{
    if (!PyObject_TypeCheck(o, &pyotherside_QObjectType)) {
        PyErr_Format(PyExc_TypeError, "Not a pyotherside.QObject");
        return -1;
    }

    if (!PyUnicode_Check(attr_name)) {
        PyErr_Format(PyExc_TypeError, "attr_name must be a string");
        return -1;
    }

    pyotherside_QObject *pyqobject = reinterpret_cast<pyotherside_QObject *>(o);
    QObjectRef *ref = pyqobject->m_qobject_ref;
    if (!ref) {
        PyErr_Format(PyExc_ValueError, "Dangling QObject");
        return -1;
    }
    QObject *qobject = ref->value();
    if (!qobject) {
        PyErr_Format(PyExc_ReferenceError, "Referenced QObject was deleted");
        return -1;
    }

    const QMetaObject *metaObject = qobject->metaObject();
    QString attrName = convertPyObjectToQVariant(attr_name).toString();

    for (int i=0; i<metaObject->propertyCount(); i++) {
        QMetaProperty property = metaObject->property(i);
        if (attrName == property.name()) {
            QVariant variant(convertPyObjectToQVariant(v));
            if (!property.write(qobject, variant)) {
                PyErr_Format(PyExc_AttributeError, "Could not set property %s to %s(%s)",
                        attrName.toUtf8().constData(),
                        variant.typeName(),
                        variant.toString().toUtf8().constData());
                return -1;
            }

            return 0;
        }
    }

    PyErr_Format(PyExc_AttributeError, "Property does not exist: %s",
            attrName.toUtf8().constData());
    return -1;
}

void
pyotherside_QObjectMethod_dealloc(pyotherside_QObjectMethod *self)
{
    delete self->m_method_ref;
    Py_TYPE(self)->tp_free((PyObject *)self);
}

PyObject *
pyotherside_QObjectMethod_repr(PyObject *o)
{
    if (!PyObject_TypeCheck(o, &pyotherside_QObjectMethodType)) {
        return PyErr_Format(PyExc_TypeError, "Not a pyotherside.QObjectMethod");
    }

    pyotherside_QObjectMethod *pyqobjectmethod = reinterpret_cast<pyotherside_QObjectMethod *>(o);

    QObjectMethodRef *ref = pyqobjectmethod->m_method_ref;
    if (!ref) {
        return PyUnicode_FromFormat("<dangling pyotherside.QObjectMethod>");
    }

    QObjectRef oref = ref->object();
    QObject *qobject = oref.value();
    if (!qobject) {
        return PyUnicode_FromFormat("<pyotherside.QObjectMethod '%s' bound to deleted QObject>",
                ref->method().toUtf8().constData());
    }

    const QMetaObject *metaObject = qobject->metaObject();
    return PyUnicode_FromFormat("<pyotherside.QObjectMethod '%s' bound to %s at %p>",
            ref->method().toUtf8().constData(), metaObject->className(), qobject);
}


PyObject *
pyotherside_QObjectMethod_call(PyObject *callable_object, PyObject *args, PyObject *kw)
{
    if (!PyObject_TypeCheck(callable_object, &pyotherside_QObjectMethodType)) {
        return PyErr_Format(PyExc_TypeError, "Not a pyotherside.QObjectMethod");
    }

    if (!PyTuple_Check(args)) {
        return PyErr_Format(PyExc_TypeError, "Argument list not a tuple");
    }

    if (kw) {
        if (!PyMapping_Check(kw)) {
            return PyErr_Format(PyExc_TypeError, "Keyword arguments not a mapping");
        }

        if (PyMapping_Size(kw) > 0) {
            return PyErr_Format(PyExc_ValueError, "Keyword arguments not supported");
        }
    }

    QList<QVariant> qargs = convertPyObjectToQVariant(args).toList();

    pyotherside_QObjectMethod *pyqobjectmethod = reinterpret_cast<pyotherside_QObjectMethod *>(callable_object);
    QObjectMethodRef *ref = pyqobjectmethod->m_method_ref;
    if (!ref) {
        return PyErr_Format(PyExc_ValueError, "Dangling QObject");
    }

    QList<QGenericArgument> genericArguments;
    for (int j=0; j<qargs.size(); j++) {
        const QVariant& argument = qargs[j];
        genericArguments.append(QGenericArgument(argument.typeName(), argument.constData()));
    }

    QObject *o = ref->object().value();
    if (!o) {
        return PyErr_Format(PyExc_ReferenceError, "Referenced QObject was deleted");
    }
    const QMetaObject *metaObject = o->metaObject();

    for (int i=0; i<metaObject->methodCount(); i++) {
        QMetaMethod method = metaObject->method(i);

        if (method.name() == ref->method()) {
            QVariant result;
            if (method.invoke(o, Qt::DirectConnection,
                    Q_RETURN_ARG(QVariant, result), genericArguments.value(0),
                    genericArguments.value(1), genericArguments.value(2),
                    genericArguments.value(3), genericArguments.value(4),
                    genericArguments.value(5), genericArguments.value(6),
                    genericArguments.value(7), genericArguments.value(8),
                    genericArguments.value(9))) {
                return convertQVariantToPyObject(result);
            }

            return PyErr_Format(PyExc_RuntimeError, "QObject method call failed");
        }
    }

    return PyErr_Format(PyExc_RuntimeError, "QObject method not found: %s",
            ref->method().toUtf8().constData());
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
    PyModule_AddIntConstant(pyotherside, "format_data", PYOTHERSIDE_IMAGE_FORMAT_ENCODED);
    PyModule_AddIntConstant(pyotherside, "format_svg_data", PYOTHERSIDE_IMAGE_FORMAT_SVG);

    // Version of PyOtherSide (new in 1.3)
    PyModule_AddStringConstant(pyotherside, "version", PYOTHERSIDE_VERSION);

    // QObject wrappers (new in 1.4)
    pyotherside_QObjectType.tp_new = PyType_GenericNew;
    pyotherside_QObjectType.tp_repr = pyotherside_QObject_repr;
    pyotherside_QObjectType.tp_getattro = pyotherside_QObject_getattro;
    pyotherside_QObjectType.tp_setattro = pyotherside_QObject_setattro;
    pyotherside_QObjectType.tp_dealloc = (destructor)pyotherside_QObject_dealloc;
    if (PyType_Ready(&pyotherside_QObjectType) < 0) {
        qFatal("Could not initialize QObjectType");
        // Not reached
        return NULL;
    }
    Py_INCREF(&pyotherside_QObjectType);
    PyModule_AddObject(pyotherside, "QObject", (PyObject *)(&pyotherside_QObjectType));

    pyotherside_QObjectMethodType.tp_new = PyType_GenericNew;
    pyotherside_QObjectMethodType.tp_repr = pyotherside_QObjectMethod_repr;
    pyotherside_QObjectMethodType.tp_call = pyotherside_QObjectMethod_call;
    pyotherside_QObjectMethodType.tp_dealloc = (destructor)pyotherside_QObjectMethod_dealloc;
    if (PyType_Ready(&pyotherside_QObjectMethodType) < 0) {
        qFatal("Could not initialize QObjectMethodType");
        // Not reached
        return NULL;
    }
    Py_INCREF(&pyotherside_QObjectMethodType);
    PyModule_AddObject(pyotherside, "QObjectMethod", (PyObject *)(&pyotherside_QObjectMethodType));

    return pyotherside;
}

QPythonPriv::QPythonPriv()
    : locals()
    , globals()
    , atexit_callback()
    , image_provider()
    , traceback_mod()
    , pyotherside_mod()
    , thread_state(NULL)
{
    PyImport_AppendInittab("pyotherside", PyOtherSide_init);

    Py_InitializeEx(0);
    PyEval_InitThreads();

    locals = PyObjectRef(PyDict_New(), true);
    assert(locals);

    globals = PyObjectRef(PyDict_New(), true);
    assert(globals);

    traceback_mod = PyObjectRef(PyImport_ImportModule("traceback"), true);
    assert(traceback_mod);

    priv = this;

    if (PyDict_GetItemString(globals.borrow(), "__builtins__") == NULL) {
        PyDict_SetItemString(globals.borrow(), "__builtins__",
                PyEval_GetBuiltins());
    }

    // Need to "self-import" the pyotherside module here, so that Python code
    // can use objects wrapped with pyotherside.QObject without crashing when
    // the user's Python code doesn't "import pyotherside"
    pyotherside_mod = PyObjectRef(PyImport_ImportModule("pyotherside"), true);
    assert(pyotherside_mod);

    // Release the GIL
    thread_state = PyEval_SaveThread();
}

QPythonPriv::~QPythonPriv()
{
    // Re-acquire the previously-released GIL
    PyEval_RestoreThread(thread_state);

    Py_Finalize();
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

    list = PyObject_CallMethod(traceback_mod.borrow(),
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
            Py_eval_input, globals.borrow(), locals.borrow());

    return result;
}

void
QPythonPriv::closing()
{
    if (!priv) {
        return;
    }

    ENSURE_GIL_STATE;

    if (priv->atexit_callback) {
        PyObjectRef args(PyTuple_New(0), true);
        PyObjectRef result(PyObject_Call(priv->atexit_callback.borrow(), args.borrow(), NULL), true);
        Q_UNUSED(result);
    }

    priv->atexit_callback = PyObjectRef();
    priv->image_provider = PyObjectRef();
}

QPythonPriv *
QPythonPriv::instance()
{
    return priv;
}

QString
QPythonPriv::importFromQRC(const char *module, const QString &filename)
{
    PyObjectRef sys_modules(PySys_GetObject((char *)"modules"), true);
    if (!PyMapping_Check(sys_modules.borrow())) {
        return QString("sys.modules is not a mapping object");
    }

    PyObjectRef qrc_importer(PyMapping_GetItemString(sys_modules.borrow(),
            (char *)module), true);

    if (!qrc_importer) {
        PyErr_Clear();

        QFile qrc_importer_code(":" + filename);
        if (!qrc_importer_code.open(QIODevice::ReadOnly)) {
            return QString("Cannot load qrc importer source");
        }

        QByteArray ba = qrc_importer_code.readAll();
        QByteArray fn = QString("qrc:/" + filename).toUtf8();

        PyObjectRef co(Py_CompileString(ba.constData(), fn.constData(),
                Py_file_input), true);
        if (!co) {
            QString result = QString("Cannot compile qrc importer: %1")
                .arg(formatExc());
            PyErr_Clear();
            return result;
        }

        qrc_importer = PyObjectRef(PyImport_ExecCodeModule((char *)module, co.borrow()), true);
        if (!qrc_importer) {
            QString result = QString("Cannot exec qrc importer: %1")
                    .arg(formatExc());
            PyErr_Clear();
            return result;
        }
    }

    return QString();
}

QString
QPythonPriv::call(PyObject *callable, QString name, QVariant args, QVariant *v)
{
    if (!PyCallable_Check(callable)) {
        return QString("Not a callable: %1").arg(name);
    }

    PyObjectRef argl(convertQVariantToPyObject(args), true);
    if (!PyList_Check(argl.borrow())) {
        return QString("Not a parameter list in call to %1: %2")
                .arg(name).arg(args.toString());
    }

    PyObjectRef argt(PyList_AsTuple(argl.borrow()), true);
    PyObjectRef o(PyObject_Call(callable, argt.borrow(), NULL), true);

    if (!o) {
        return QString("Return value of PyObject call is NULL: %1").arg(priv->formatExc());
    } else {
        if (v != NULL) {
            *v = convertPyObjectToQVariant(o.borrow());
        }
    }
    return QString();
}
