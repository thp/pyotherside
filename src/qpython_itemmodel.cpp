
/**
 * PyOtherSide: Asynchronous Python 3 Bindings for Qt 5
 * Copyright (c) 2011, 2013, 2014, Thomas Perl <m@thp.io>
 * Copyright (c) 2015, Robie Basak <robie@justgohome.co.uk>
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

#include "qpython_itemmodel.h"

#include "qml_python_bridge.h"
#include "ensure_gil_state.h"
#include "pyitemmodel_proxy.h"

QPythonItemModel::QPythonItemModel(QObject *parent)
{
    m_proxy = 0;
    if (!connect(this, SIGNAL(needModificationCallback(PyObject *)), this, SLOT(handleModificationCallback(PyObject *))))
        qFatal("Could not connect needModificationCallback signal");
}

QPythonItemModel::~QPythonItemModel()
{
    Py_CLEAR(m_proxy);
}

static PyObject *
qmodelindex_to_pyargs(const QModelIndex &index)
{
    if (index.isValid())
        return Py_BuildValue("(k)", index.internalId());
    else
        return PyTuple_Pack(1, Py_None);
}

static int
call_model_for_int(const PyObjectRef &model, const QModelIndex &parent, const char *method_name)
{
    ENSURE_GIL_STATE;
    if (!model)
        return 0;
    PyObject *method = PyObject_GetAttrString(model.borrow(), method_name);
    if (!method) {
        PyErr_Print();
        PyErr_Clear();
        return 0;
    }
    PyObject *args = qmodelindex_to_pyargs(parent);
    if (!args) {
        Py_DECREF(method);
        PyErr_Print();
        PyErr_Clear();
        return 0;
    }
    PyObject *result = PyObject_CallObject(method, args);
    Py_DECREF(method);
    Py_DECREF(args);
    // XXX: should check result is long first, otherwise segfault
    long result_long = PyLong_AsLong(result);
    Py_DECREF(result);
    return result_long;
}

int
QPythonItemModel::columnCount(const QModelIndex &parent) const {
    return call_model_for_int(m_model, parent, "columnCount");
}

int
QPythonItemModel::rowCount(const QModelIndex &parent) const {
    return call_model_for_int(m_model, parent, "rowCount");
}

/* Note: this Py_DECREFs args for convenience */
QModelIndex
QPythonItemModel::call_model_for_index(PyObject *args, const char *method_name) const
{
    ENSURE_GIL_STATE;
    if (!m_model)
        return QModelIndex();
    PyObject *method = PyObject_GetAttrString(m_model.borrow(), method_name);
    if (!method) {
        PyErr_Print();
        PyErr_Clear();
        return QModelIndex();
    }
    PyObject *result = PyObject_CallObject(method, args);
    Py_DECREF(method);
    Py_DECREF(args);

    if (!result) {
        PyErr_Print();
        PyErr_Clear();
        return QModelIndex();
    } else if (result == Py_None) {
        Py_DECREF(result);
        return QModelIndex();
    } else {
        int row, column;
        unsigned long index_id;
        if (!PyArg_ParseTuple(result, "iik", &row, &column, &index_id)) {
            Py_DECREF(result);
            PyErr_Print();
            PyErr_Clear();
            return QModelIndex();
        }
        Py_DECREF(result);
        return createIndex(row, column, index_id);
    }
}

QModelIndex
QPythonItemModel::parent(const QModelIndex &index) const {
    PyObject *args = qmodelindex_to_pyargs(index);
    if (!args) {
        PyErr_Print();
        PyErr_Clear();
        return QModelIndex();
    }
    return call_model_for_index(args, "parent");
}

QModelIndex
QPythonItemModel::index(int row, int column, const QModelIndex &parent) const {
    PyObject *args;
    if (parent.isValid())
        args = Py_BuildValue("(iik)", row, column, parent.internalId());
    else
        args = Py_BuildValue("(iiO)", row, column, Py_None);

    return call_model_for_index(args, "index");
}

QVariant
QPythonItemModel::data(const QModelIndex &index, int role) const
{
    ENSURE_GIL_STATE;

    PyObject *method;

    if (!m_model)
        return QVariant(QString("1"));  // XXX make empty QVariant()

    PyObject *args;
    if (index.isValid())
        args = Py_BuildValue("(ki)", index.internalId(), role);
    else
        args = Py_BuildValue("(Oi)", Py_None, role);
    if (!args) {
        PyErr_Print();
        PyErr_Clear();
        return QVariant("4");
    }

    method = PyObject_GetAttrString(m_model.borrow(), "data");
    if (!method) {
        Py_DECREF(args);
        PyErr_Print();
        PyErr_Clear();
        return QVariant(QString("2"));
    }
    PyObjectRef result(PyObject_CallObject(method, args));
    Py_DECREF(args);
    Py_DECREF(method);
    if (!result) {
        PyErr_PrintEx(0);
        return QVariant(QString("3"));
    }
    return convertPyObjectToQVariant(result.borrow());
}

// XXX now that this uses a constructor, it should no longer be a property but
// a method instead
void
QPythonItemModel::setModel(QVariant model)
{
    ENSURE_GIL_STATE;

    if (!m_proxy) {
        m_proxy = create_QPythonItemModelProxy(this);
        if (!m_proxy)
            qFatal("Failed to create QPythonItemModelProxy");
    }

    PyObjectRef model_constructor(model.value<PyObjectRef>());
    if (model_constructor && PyCallable_Check(model_constructor.borrow())) {
        PyObject *args = PyTuple_Pack(1, m_proxy);
        if (!args)
            return;  /* spec doesn't say that this sets an exception */
        PyObject *result = PyObject_CallObject(model_constructor.borrow(), args);
        Py_DECREF(args);
        if (result == NULL) {
            PyErr_Print();
            PyErr_Clear();
        } else {
            beginResetModel();
            m_model = PyObjectRef(result);
            endResetModel();
        }
    }
}

void
QPythonItemModel::handleModificationCallback(PyObject *cb)
{
    ENSURE_GIL_STATE;
    PyObject *result = PyObject_CallObject(cb, NULL);
    if (!result) {
        PyErr_Print();
        PyErr_Clear();
    }
    Py_DECREF(cb);
    Py_XDECREF(result);
}

QModelIndex
QPythonItemModel::pysequence_to_qmodelindex(PyObject *list)
{
    if (list == Py_None)
	return QModelIndex();

    PyObject *fast = PySequence_Fast(list, "Argument is not a sequence when converting to QModelIndex");
    if (!fast) {
	PyErr_Print();
	PyErr_Clear();
	return QModelIndex();
    }
    PyObject **items = PySequence_Fast_ITEMS(fast);
    Py_ssize_t n = PySequence_Fast_GET_SIZE(fast);

    QModelIndex result;
    int row, column;
    for (Py_ssize_t i=0; i<n; i++) {
	if (!PyTuple_Check(items[i])) {
	    // Seems a bit harsh to require a tuple but not a sequence but I
	    // don't see it documented that PyArg_ParseTuple will accept a
	    // sequence.
	    // XXX need better handling
	    qFatal("Sequence item is not tuple");
	    return QModelIndex();  // never reached
	}
	if (!PyArg_ParseTuple(items[i], "ii", &row, &column)) {
	    Py_DECREF(fast);
	    PyErr_Print();
	    PyErr_Clear();
	    qFatal("2");
	    return QModelIndex();
	}
	result = index(row, column, result);
    }
    Py_DECREF(fast);
    return result;
}

void
QPythonItemModel::sendDataChanged(PyObject *args)
{
    PyObject *topleft_path, *bottomright_path;
    if (!PyArg_ParseTuple(args, "OO", &topleft_path, &bottomright_path)) {
	PyErr_Print();
	PyErr_Clear();
	qFatal("1");
	return;
    }
    QModelIndex topleft(pysequence_to_qmodelindex(topleft_path));
    QModelIndex bottomright(pysequence_to_qmodelindex(bottomright_path));
    emit dataChanged(topleft, bottomright);
}

QHash<int, QByteArray>
QPythonItemModel::roleNames(void) const
{
    ENSURE_GIL_STATE;

    if (!m_model)
        return QAbstractItemModel::roleNames();

    PyObject *method = PyObject_GetAttrString(m_model.borrow(), "roleNames");
    if (!method)
	return QAbstractItemModel::roleNames();

    PyObject *py_result = PyObject_CallObject(method, NULL);
    Py_DECREF(method);
    if (!py_result) {
	PyErr_Print();
	PyErr_Clear();
	return QAbstractItemModel::roleNames();
    }

    if (py_result == Py_None) {
	Py_DECREF(py_result);
	return QAbstractItemModel::roleNames();
    }

    if (!PyDict_Check(py_result)) {
	Py_DECREF(py_result);
	qFatal("not a dict"); // XXX handle properly
	return QAbstractItemModel::roleNames();
    }

    PyObject *py_key, *py_value;
    Py_ssize_t pos = 0;

    QHash<int, QByteArray> q_result;
    while (PyDict_Next(py_result, &pos, &py_key, &py_value)) {
	long key_long = PyLong_AsLong(py_key);
	if (PyErr_Occurred()) {
	    PyErr_Print();
	    PyErr_Clear();
	    qFatal("dict key not an int"); // XXX handle properly
	    continue;
	}
	if (key_long < INT_MIN || key_long > INT_MAX) {
	    qFatal("int overflow"); // XXX handle properly
	    continue;
	}
	int key_int = (int)key_long;
	if (!PyUnicode_Check(py_value)) {
	    qFatal("dict value is not a str"); // XXX handle properly
	    continue;
	}
	/* Not sure what encoding to use here. What encoding does Qt use for
	 * matching against unicode role names defined in QML? It probably
	 * doesn't matter - role names are internal, don't need l10n and can
	 * be required to be in ASCII for now. */
	PyObject *py_bytes_value = PyUnicode_EncodeLocale(py_value, NULL);
	if (!py_bytes_value) {
	    PyErr_Print();
	    PyErr_Clear();
	    qFatal("could not convert dict value to bytes"); // XXX handle properly
	    continue;
	}
	q_result.insert(key_int, QByteArray(PyBytes_AS_STRING(py_bytes_value), PyBytes_GET_SIZE(py_bytes_value)));
	Py_DECREF(py_bytes_value);
    }

    Py_DECREF(py_result);
    return q_result;
}
