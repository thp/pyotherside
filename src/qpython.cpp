
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

#include "qpython.h"
#include "qpython_priv.h"
#include "qpython_worker.h"

#include <QDebug>
#include <QJSEngine>

QPythonPriv *
QPython::priv = NULL;

QPython::QPython(QObject *parent)
    : QObject(parent)
    , worker(new QPythonWorker(this))
    , thread()
    , handlers()
{
    if (priv == NULL) {
        priv = new QPythonPriv;
    }

    worker->moveToThread(&thread);

    QObject::connect(priv, SIGNAL(receive(QVariant)),
                     this, SLOT(receive(QVariant)));

    QObject::connect(this, SIGNAL(process(QString,QVariant,QJSValue)),
                     worker, SLOT(process(QString,QVariant,QJSValue)));
    QObject::connect(worker, SIGNAL(finished(QVariant,QJSValue)),
                     this, SLOT(finished(QVariant,QJSValue)));

    QObject::connect(this, SIGNAL(import(QString,QJSValue)),
                     worker, SLOT(import(QString,QJSValue)));
    QObject::connect(worker, SIGNAL(imported(bool,QJSValue)),
                     this, SLOT(imported(bool,QJSValue)));

    thread.start();
}

QPython::~QPython()
{
    thread.quit();
    thread.wait();

    delete worker;
}

void
QPython::addImportPath(QString path)
{
    priv->enter();
    QByteArray utf8bytes = path.toUtf8();

    PyObject *sys_path = PySys_GetObject((char*)"path");

    PyObject *cwd = PyUnicode_FromString(utf8bytes.constData());
    PyList_Insert(sys_path, 0, cwd);
    Py_DECREF(cwd);
    priv->leave();
}

void
QPython::importModule(QString name, QJSValue callback)
{
    emit import(name, callback);
}

bool
QPython::importModule_sync(QString name)
{
    // Lesson learned: name.toUtf8().constData() doesn't work, as the
    // temporary QByteArray will be destroyed after constData() has
    // returned, so we need to save the toUtf8() result in a local
    // variable that doesn't get destroyed until the function returns.
    QByteArray utf8bytes = name.toUtf8();
    const char *moduleName = utf8bytes.constData();

    priv->enter();
    PyObject *module = PyImport_ImportModule(moduleName);
    if (module == NULL) {
        emit error(priv->formatExc());
        priv->leave();
        return false;
    }

    PyDict_SetItemString(priv->globals, moduleName, module);
    priv->leave();
    return true;
}

void
QPython::receive(QVariant variant)
{
    QVariantList list = variant.toList();
    QString event = list[0].toString();
    if (handlers.contains(event)) {
        QJSValue callback = handlers[event];
        QJSValueList args;
        for (int i=1; i<list.size(); i++) {
            args << callback.engine()->toScriptValue(list[i]);
        }
        callback.call(args);
    } else {
        // Default action
        emit received(variant);
    }
}

void
QPython::setHandler(QString event, QJSValue callback)
{
    if (!callback.isCallable() || callback.isNull() || callback.isUndefined()) {
        handlers.remove(event);
    } else {
        handlers[event] = callback;
    }
}

QVariant
QPython::evaluate(QString expr)
{
    priv->enter();
    PyObject *o = priv->eval(expr);
    if (o == NULL) {
        emit error(priv->formatExc());
        priv->leave();
        return QVariant();
    }

    QVariant v = convertPyObjectToQVariant(o);
    Py_DECREF(o);
    priv->leave();
    return v;
}

void
QPython::call(QString func, QVariant args, QJSValue callback)
{
    emit process(func, args, callback);
}

QVariant
QPython::call_sync(QString func, QVariant args)
{
    priv->enter();
    PyObject *callable = priv->eval(func);

    if (callable == NULL) {
        emit error(priv->formatExc());
        priv->leave();
        return QVariant();
    }

    if (PyCallable_Check(callable)) {
        QVariant v;

        PyObject *argl = convertQVariantToPyObject(args);
        assert(PyList_Check(argl));
        PyObject *argt = PyList_AsTuple(argl);
        Py_DECREF(argl);
        PyObject *o = PyObject_Call(callable, argt, NULL);
        Py_DECREF(argt);

        if (o == NULL) {
            emit error(priv->formatExc());
        } else {
            v = convertPyObjectToQVariant(o);
            Py_DECREF(o);
        }

        Py_DECREF(callable);
        priv->leave();
        return v;
    }

    emit error(QString("Not a callable: ") + func);
    Py_DECREF(callable);
    priv->leave();
    return QVariant();
}

void
QPython::finished(QVariant result, QJSValue callback)
{
    if (!callback.isNull() && !callback.isUndefined() && callback.isCallable()) {
        QJSValueList args;
        QJSValue v = callback.engine()->toScriptValue(result);
        args << v;
        callback.call(args);
    }
}

void
QPython::imported(bool result, QJSValue callback)
{
    if (!callback.isNull() && !callback.isUndefined() && callback.isCallable()) {
        QJSValueList args;
        QJSValue v = callback.engine()->toScriptValue(QVariant(result));
        args << v;
        callback.call(args);
    }
}
