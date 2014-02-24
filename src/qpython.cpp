
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

#include "qpython.h"
#include "qpython_priv.h"
#include "qpython_worker.h"

#include <QDebug>

#include <QJSEngine>


#define SINCE_API_VERSION(smaj, smin) \
    ((api_version_major > smaj) || (api_version_major == smaj && api_version_minor >= smin))


QPythonPriv *
QPython::priv = NULL;

QPython::QPython(QObject *parent, int api_version_major, int api_version_minor)
    : QObject(parent)
    , worker(new QPythonWorker(this))
    , thread()
    , handlers()
    , api_version_major(api_version_major)
    , api_version_minor(api_version_minor)
{
    if (priv == NULL) {
        priv = new QPythonPriv;
    }

    worker->moveToThread(&thread);

    QObject::connect(priv, SIGNAL(receive(QVariant)),
                     this, SLOT(receive(QVariant)));

    QObject::connect(this, SIGNAL(process(QString,QVariant,QJSValue *)),
                     worker, SLOT(process(QString,QVariant,QJSValue *)));
    QObject::connect(worker, SIGNAL(finished(QVariant,QJSValue *)),
                     this, SLOT(finished(QVariant,QJSValue *)));

    QObject::connect(this, SIGNAL(import(QString,QJSValue *)),
                     worker, SLOT(import(QString,QJSValue *)));
    QObject::connect(worker, SIGNAL(imported(bool,QJSValue *)),
                     this, SLOT(imported(bool,QJSValue *)));

    thread.setObjectName("QPythonWorker");
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

    // Strip leading "file://" (for use with Qt.resolvedUrl())
    if (path.startsWith("file://")) {
#ifdef WIN32
        // On Windows, path would be "file:///C:\...", so strip 8 chars to get
        // a Windows-compatible absolute filename to be used as import path
        path = path.mid(8);
#else
        path = path.mid(7);
#endif
    }

    if (SINCE_API_VERSION(1, 3) && path.startsWith("qrc:")) {
        const char *module = "pyotherside.qrc_importer";
        QString filename = "/io/thp/pyotherside/qrc_importer.py";
        QString errorMessage = priv->importFromQRC(module, filename);
        if (!errorMessage.isNull()) {
            emit error(errorMessage);
        }
    }

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
    QJSValue *cb = 0;
    if (!callback.isNull() && !callback.isUndefined() && callback.isCallable()) {
        cb = new QJSValue(callback);
    }
    emit import(name, cb);
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

    bool use_api_10 = (api_version_major == 1 && api_version_minor == 0);

    PyObject *module = NULL;

    if (use_api_10) {
        // PyOtherSide API 1.0 behavior (star import)
        module = PyImport_ImportModule(moduleName);
    } else {
        // PyOtherSide API 1.2 behavior: "import x.y.z"
        PyObject *fromList = PyList_New(0);
        module = PyImport_ImportModuleEx(const_cast<char *>(moduleName), NULL, NULL, fromList);
        Py_XDECREF(fromList);
    }

    if (module == NULL) {
        emit error(QString("Cannot import module: %1 (%2)").arg(name).arg(priv->formatExc()));
        priv->leave();
        return false;
    }

    if (!use_api_10) {
        // PyOtherSide API 1.2 behavior: "import x.y.z"
        // If "x.y.z" is imported, we need to set "x" in globals
        if (name.indexOf('.') != -1) {
            name = name.mid(0, name.indexOf('.'));
            utf8bytes = name.toUtf8();
            moduleName = utf8bytes.constData();
        }
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
        QJSValue result = callback.call(args);
        if (result.isError()) {
            // Ideally we would throw the error back to Python (so that the
            // pyotherside.send() method fails, as this is where the call
            // originated). We can't do this, because the pyotherside.send()
            // call is asynchronous (it returns before we call into JS), so do
            // the next best thing and report the error to our error handler in
            // QML instead.
            emit error("pyotherside.send() failed handler: " +
                    result.property("fileName").toString() + ":" +
                    result.property("lineNumber").toString() + ": " +
                    result.toString());
        }
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
        emit error(QString("Cannot evaluate '%1' (%2)").arg(expr).arg(priv->formatExc()));
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
    QJSValue *cb = 0;
    if (!callback.isNull() && !callback.isUndefined() && callback.isCallable()) {
        cb = new QJSValue(callback);
    }
    emit process(func, args, cb);
}

QVariant
QPython::call_sync(QString func, QVariant args)
{
    priv->enter();
    PyObject *callable = priv->eval(func);

    if (callable == NULL) {
        emit error(QString("Function not found: '%1' (%2)").arg(func).arg(priv->formatExc()));
        priv->leave();
        return QVariant();
    }

    if (PyCallable_Check(callable)) {
        QVariant v;

        PyObject *argl = convertQVariantToPyObject(args);
        if (!PyList_Check(argl)) {
            Py_DECREF(callable);
            Py_XDECREF(argl);
            emit error(QString("Not a parameter list in call to %1: %2")
                    .arg(func).arg(args.toString()));
            priv->leave();
            return QVariant();
        }

        PyObject *argt = PyList_AsTuple(argl);
        Py_DECREF(argl);
        PyObject *o = PyObject_Call(callable, argt, NULL);
        Py_DECREF(argt);

        if (o == NULL) {
            emit error(QString("Return value of PyObject call is NULL: %1").arg(priv->formatExc()));
        } else {
            v = convertPyObjectToQVariant(o);
            Py_DECREF(o);
        }

        Py_DECREF(callable);
        priv->leave();
        return v;
    }

    emit error(QString("Not a callable: %1").arg(func));
    Py_DECREF(callable);
    priv->leave();
    return QVariant();
}

void
QPython::finished(QVariant result, QJSValue *callback)
{
    QJSValueList args;
    QJSValue v = callback->engine()->toScriptValue(result);
    args << v;
    QJSValue callbackResult = callback->call(args);
    if (SINCE_API_VERSION(1, 2)) {
        if (callbackResult.isError()) {
            emit error(callbackResult.property("fileName").toString() + ":" +
                    callbackResult.property("lineNumber").toString() + ": " +
                    callbackResult.toString());
        }
    }
    delete callback;
}

void
QPython::imported(bool result, QJSValue *callback)
{
    QJSValueList args;
    QJSValue v = callback->engine()->toScriptValue(QVariant(result));
    args << v;
    QJSValue callbackResult = callback->call(args);
    if (SINCE_API_VERSION(1, 2)) {
        if (callbackResult.isError()) {
            emit error(callbackResult.property("fileName").toString() + ":" +
                    callbackResult.property("lineNumber").toString() + ": " +
                    callbackResult.toString());
        }
    }
    delete callback;
}

QString
QPython::pluginVersion()
{
    return QString(PYOTHERSIDE_VERSION);
}

QString
QPython::pythonVersion()
{
    return QString(PY_VERSION);
}
