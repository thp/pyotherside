
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

#include "pyitemmodel_proxy.h"
#include "qpython_itemmodel.h"
#include "qobject_ref.h"
#include <QCoreApplication>
#include <QDebug>
#include <QThread>

typedef struct {
    PyObject_HEAD
    QObjectRef *m_qpythonitemmodel_ref;
} pyotherside_QPythonItemModelProxy;

static bool is_gui_thread(void) {
    QCoreApplication *p = QCoreApplication::instance();
    return p && (p->thread() == QThread::currentThread());
}

PyObject *
call_back_for_modification(PyObject *self, PyObject *cb)
{
    if (is_gui_thread()) {
        // call back immediately since we're already in the GUI thread
        PyObject *result = PyObject_CallObject(cb, NULL);
        if (result) {
            // though we could return this, doing so would cause different
            // behaviour between the two is_gui_thread modes.
            Py_DECREF(result);
            Py_RETURN_NONE;
        } else {
            return NULL;  // pass back the exception
        }
    } else {
        // send a signal to handle the callback from the GUI thread.
        pyotherside_QPythonItemModelProxy *proxy = (pyotherside_QPythonItemModelProxy *)self;
        if (!proxy->m_qpythonitemmodel_ref) {
            qFatal("proxy no longer exists");  // XXX handle with exception
            return NULL;  // never gets here
        }
        QPythonItemModel *model = (QPythonItemModel *)proxy->m_qpythonitemmodel_ref->value();
        model->sendModificationCallback(cb);

        // can't return anything useful right now
        Py_RETURN_NONE;
    }
}

PyObject *
signal_dataChanged(PyObject *self, PyObject *args)
{
    pyotherside_QPythonItemModelProxy *proxy = (pyotherside_QPythonItemModelProxy *)self;
    if (!proxy->m_qpythonitemmodel_ref) {
        qFatal("proxy no longer exists");  // XXX handle with exception
        return NULL;  // never gets here
    }
    QPythonItemModel *model = (QPythonItemModel *)proxy->m_qpythonitemmodel_ref->value();
    model->sendDataChanged(args);
    Py_RETURN_NONE;
}

PyObject *
beginInsertRows(PyObject *self, PyObject *args)
{
    pyotherside_QPythonItemModelProxy *proxy = (pyotherside_QPythonItemModelProxy *)self;
    if (!proxy->m_qpythonitemmodel_ref) {
        qFatal("proxy no longer exists");  // XXX handle with exception
        return NULL;  // never gets here
    }
    QPythonItemModel *model = (QPythonItemModel *)proxy->m_qpythonitemmodel_ref->value();
    PyObject *parent_path;
    int first, last;
    if (!PyArg_ParseTuple(args, "Oii", &parent_path, &first, &last))
	return NULL;  // XXX failure here doesn't lead to traceback for some reason
    QModelIndex parent(model->pysequence_to_qmodelindex(parent_path));
    model->do_beginInsertRows(parent, first, last);
    Py_RETURN_NONE;
}

PyObject *
beginRemoveRows(PyObject *self, PyObject *args)
{
    pyotherside_QPythonItemModelProxy *proxy = (pyotherside_QPythonItemModelProxy *)self;
    if (!proxy->m_qpythonitemmodel_ref) {
        qFatal("proxy no longer exists");  // XXX handle with exception
        return NULL;  // never gets here
    }
    QPythonItemModel *model = (QPythonItemModel *)proxy->m_qpythonitemmodel_ref->value();
    PyObject *parent_path;
    int first, last;
    if (!PyArg_ParseTuple(args, "Oii", &parent_path, &first, &last))
	return NULL;  // XXX failure here doesn't lead to traceback for some reason
    QModelIndex parent(model->pysequence_to_qmodelindex(parent_path));
    model->do_beginRemoveRows(parent, first, last);
    Py_RETURN_NONE;
}

PyObject *
endInsertRows(PyObject *self, PyObject *null)
{
    pyotherside_QPythonItemModelProxy *proxy = (pyotherside_QPythonItemModelProxy *)self;
    if (!proxy->m_qpythonitemmodel_ref) {
        qFatal("proxy no longer exists");  // XXX handle with exception
        return NULL;  // never gets here
    }
    QPythonItemModel *model = (QPythonItemModel *)proxy->m_qpythonitemmodel_ref->value();
    model->do_endInsertRows();
    Py_RETURN_NONE;
}

PyObject *
endRemoveRows(PyObject *self, PyObject *null)
{
    pyotherside_QPythonItemModelProxy *proxy = (pyotherside_QPythonItemModelProxy *)self;
    if (!proxy->m_qpythonitemmodel_ref) {
        qFatal("proxy no longer exists");  // XXX handle with exception
        return NULL;  // never gets here
    }
    QPythonItemModel *model = (QPythonItemModel *)proxy->m_qpythonitemmodel_ref->value();
    model->do_endRemoveRows();
    Py_RETURN_NONE;
}

PyMethodDef pyotherside_QPythonItemModelProxyType_methods[] = {
    {"call_back_for_modification", (PyCFunction)call_back_for_modification, METH_O,
     "Request callback from UI thread to change data in the model safely"
    },
    {"signal_dataChanged", (PyCFunction)signal_dataChanged, METH_VARARGS,
     "Send dataChanged signal"
    },
    {"beginInsertRows", (PyCFunction)beginInsertRows, METH_VARARGS,
     "Call beginInsertRows on model"
    },
    {"endInsertRows", (PyCFunction)endInsertRows, METH_NOARGS,
     "Call endInsertRows on model"
    },
    {"beginRemoveRows", (PyCFunction)beginRemoveRows, METH_VARARGS,
     "Call beginRemoveRows on model"
    },
    {"endRemoveRows", (PyCFunction)endRemoveRows, METH_NOARGS,
     "Call endRemoveRows on model"
    },
    {NULL}  /* Sentinel */
};

PyTypeObject pyotherside_QPythonItemModelProxyType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "pyotherside.ItemModelProxy", /* tp_name */
    sizeof(pyotherside_QPythonItemModelProxy), /* tp_basicsize */
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
    "Interface to QPythonItemModel", /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    0,                         /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    pyotherside_QPythonItemModelProxyType_methods, /* tp_methods */
};

int done_init = 0;

static void
dealloc(pyotherside_QPythonItemModelProxy *self)
{
    delete self->m_qpythonitemmodel_ref;
    Py_TYPE(self)->tp_free((PyObject *)self);
}

static void create_type(void) {
   if (!done_init) {
    // QPythonItemModel proxy
    pyotherside_QPythonItemModelProxyType.tp_dealloc = (destructor)dealloc;
    if (PyType_Ready(&pyotherside_QPythonItemModelProxyType) < 0) {
        qFatal("Could not initialize QPythonItemModelProxyType");
        // Not reached
        return;
    }
    Py_INCREF(&pyotherside_QPythonItemModelProxyType);
    done_init = 1;
   }
}

void
init_QPythonItemModelProxyType(PyObject *module)
{
    create_type();
    PyModule_AddObject(module, "ItemModelProxy", (PyObject *)(&pyotherside_QPythonItemModelProxyType));
}

PyObject *
create_QPythonItemModelProxy(QObject *obj)
{
    create_type();
    pyotherside_QPythonItemModelProxy *result = (pyotherside_QPythonItemModelProxy *)pyotherside_QPythonItemModelProxyType.tp_alloc(&pyotherside_QPythonItemModelProxyType, 0);
    if (result)
        result->m_qpythonitemmodel_ref = new QObjectRef(obj);
    return (PyObject *)result;
}
