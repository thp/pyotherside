
#include "qpython.h"

#include <QtDeclarative>

int
QPython::instances = 0;

QPython::QPython(QObject *parent)
    : QObject(parent)
    , locals(PyDict_New())
    , globals(PyDict_New())
{
    if (instances == 0) {
        Py_Initialize();
    }

    instances++;

    if (PyDict_GetItemString(globals, "__builtins__") == NULL) {
        PyDict_SetItemString(globals, "__builtins__",
                PyEval_GetBuiltins());
    }
}

QPython::~QPython()
{
    instances--;
    if (instances == 0) {
        Py_Finalize();
    }
}

void
QPython::addImportPath(QString path)
{
    QByteArray utf8bytes = path.toUtf8();

    PyObject *sys_path = PySys_GetObject("path");

    PyObject *cwd = PyString_FromString(utf8bytes.constData());
    PyList_Insert(sys_path, 0, cwd);
    Py_DECREF(cwd);
}

bool
QPython::importModule(QString name)
{
    // Lesson learned: name.toUtf8().constData() doesn't work, as the
    // temporary QByteArray will be destroyed after constData() has
    // returned, so we need to save the toUtf8() result in a local
    // variable that doesn't get destroyed until the function returns.
    QByteArray utf8bytes = name.toUtf8();
    const char *moduleName = utf8bytes.constData();

    PyObject *module = PyImport_ImportModule(moduleName);
    if (module == NULL) {
        PyErr_Print();
        return false;
    }

    PyDict_SetItemString(globals, moduleName, module);
    return true;
}

QVariant
QPython::evaluate(QString expr)
{
    PyObject *o = eval(expr);
    QVariant v = fromPython(o);
    Py_DECREF(o);
    return v;
}

PyObject *
QPython::eval(QString expr)
{
    QByteArray utf8bytes = expr.toUtf8();
    PyObject *result = PyRun_String(utf8bytes.constData(),
            Py_eval_input, globals, locals);

    if (result == NULL) {
        PyErr_Print();
    }

    return result;
}

QVariant
QPython::call(QString func, QVariant args)
{
    PyObject *callable = eval(func);

    if (callable == NULL) {
        return QVariant();
    }

    if (PyCallable_Check(callable)) {
        QVariant v;

        PyObject *argl = toPython(args);
        assert(PyList_Check(argl));
        PyObject *argt = PyList_AsTuple(argl);
        Py_DECREF(argl);
        PyObject *o = PyObject_Call(callable, argt, NULL);
        Py_DECREF(argt);

        if (o == NULL) {
            PyErr_Print();
        } else {
            v = fromPython(o);
            Py_DECREF(o);
        }

        Py_DECREF(callable);
        return v;
    }

    qDebug() << "Not a callable:" << func;
    Py_DECREF(callable);
    return QVariant();
}

void
QPython::registerQML()
{
    qmlRegisterType<QPython>("com.thpinfo.python", 1, 0, "Python");
}

QVariant
QPython::fromPython(PyObject *o)
{
    if (PyInt_Check(o)) {
        return QVariant((int)PyInt_AsLong(o));
    } else if (PyBool_Check(o)) {
        return QVariant(o == Py_True);
    } else if (PyLong_Check(o)) {
        return QVariant((qlonglong)PyLong_AsLong(o));
    } else if (PyFloat_Check(o)) {
        return QVariant(PyFloat_AsDouble(o));
    } else if (PyList_Check(o)) {
        QVariantList result;

        Py_ssize_t count = PyList_Size(o);
        for (int i=0; i<count; i++) {
            result << fromPython(PyList_GetItem(o, i));
        }

        return result;
    } else if (PyUnicode_Check(o)) {
        PyObject *string = PyUnicode_AsUTF8String(o);
        QVariant result = fromPython(string);
        Py_DECREF(string);
        return result;
    } else if (PyString_Check(o)) {
        // We always assume UTF-8 encoding here
        return QString::fromUtf8(PyString_AsString(o));
    } else if (PyDict_Check(o)) {
        QMap<QString,QVariant> result;

        PyObject *key, *value;
        Py_ssize_t pos = 0;
        while (PyDict_Next(o, &pos, &key, &value)) {
            result[fromPython(key).toString()] = fromPython(value);
        }

        return result;
    }

    qDebug() << "XXX Python -> Qt conversion not handled yet";
    return QVariant();
}

PyObject *
QPython::toPython(QVariant v)
{
    QVariant::Type type = v.type();

    if (type == QVariant::Bool) {
        if (v.toBool()) {
            Py_RETURN_TRUE;
        } else {
            Py_RETURN_FALSE;
        }
    } else if (type == QVariant::Int) {
        return PyLong_FromLong(v.toInt());
    } else if (type == QVariant::Double) {
        return PyFloat_FromDouble(v.toDouble());
    } else if (type == QVariant::List) {
        QVariantList l = v.toList();

        PyObject *result = PyList_New(l.size());
        for (int i=0; i<l.size(); i++) {
            PyList_SetItem(result, i, toPython(l[i]));
        }
        return result;
    } else if (type == QVariant::String) {
        QByteArray utf8bytes = v.toString().toUtf8();
        return PyUnicode_FromString(utf8bytes.constData());
    } else if (type == QVariant::Map) {
        QMap<QString,QVariant> m = v.toMap();
        QList<QString> keys = m.keys();

        PyObject *result = PyDict_New();
        for (int i=0; i<keys.size(); i++) {
            PyObject *o = toPython(m[keys[i]]);
            QByteArray utf8bytes = keys[i].toUtf8();
            PyDict_SetItemString(result, utf8bytes.constData(), o);
            Py_DECREF(o);
        }
        return result;
    }

    qDebug() << "XXX Qt -> Python converstion not handled yet";
    return NULL;
}

