
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

#include "ensure_gil_state.h"

#include <QDebug>

#include <QJSEngine>
#include <QMetaMethod>


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
    , error_connections(0)
{
    if (priv == NULL) {
        priv = new QPythonPriv;
    }

    worker->moveToThread(&thread);

    QObject::connect(priv, SIGNAL(receive(QVariant)),
                     this, SLOT(receive(QVariant)));

    QObject::connect(this, SIGNAL(process(QVariant,QVariant,QJSValue *)),
                     worker, SLOT(process(QVariant,QVariant,QJSValue *)));
    QObject::connect(worker, SIGNAL(finished(QVariant,QJSValue *)),
                     this, SLOT(finished(QVariant,QJSValue *)));

    QObject::connect(this, SIGNAL(import(QString,QJSValue *)),
                     worker, SLOT(import(QString,QJSValue *)));
    QObject::connect(this, SIGNAL(import_names(QString, QVariant, QJSValue *)),
                     worker, SLOT(import_names(QString, QVariant, QJSValue *)));
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
QPython::connectNotify(const QMetaMethod &signal)
{
    if (signal == QMetaMethod::fromSignal(&QPython::error)) {
        error_connections++;
    }
}

void
QPython::disconnectNotify(const QMetaMethod &signal)
{
    if (signal == QMetaMethod::fromSignal(&QPython::error)) {
        error_connections--;
    }
}

void
QPython::addImportPath(QString path)
{
    ENSURE_GIL_STATE;

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
            emitError(errorMessage);
        }
    }

    QByteArray utf8bytes = path.toUtf8();

    PyObject *sys_path = PySys_GetObject((char*)"path");

    PyObjectRef cwd(PyUnicode_FromString(utf8bytes.constData()), true);
    PyList_Insert(sys_path, 0, cwd.borrow());
}

void
QPython::importNames(QString name, QVariant args, QJSValue callback)
{
    QJSValue *cb = 0;
    if (!callback.isNull() && !callback.isUndefined() && callback.isCallable()) {
        cb = new QJSValue(callback);
    }
    emit import_names(name, args, cb);
}

bool
QPython::importNames_sync(QString module_name, QVariant args)
{
    // The plan is to "from module_name import a, b, c". And args is the list with a, b, c.
    // The module_name can be a packaged module "x.y.z" -- "from x.y.z import a, b, c".
    // Thus:
    //  - import the module, given by module_name,
    //  - get the objects from the module, given by names in args,
    //  - put the objects into globals of priv

    QByteArray utf8bytes = module_name.toUtf8();
    const char *moduleName = utf8bytes.constData();

    ENSURE_GIL_STATE;

    // PyOtherSide API 1.2 behavior: "import x.y.z" -- where the module 'z' is needed
    PyObjectRef module = PyObjectRef(PyImport_ImportModule(moduleName), true);

    if (!module) {
        emitError(QString("Cannot import module: %1 (%2)").arg(module_name).arg(priv->formatExc()));
        return false;
    }

    // at this point the module with the target objects is in PyObjectRef module,
    // it should be well imported in Python

    // Get the names of functions/objects to import
    QVariantList vl = args.toList();

    QString obj_name;   // object name to import
    PyObjectRef result; // the object, obtained from globals_temp

    // for each object name try to get it from the module
    //  - on success put it into priv.globals
    //  - on failure emit the error and continue
    for (QVariantList::const_iterator obj = vl.begin(); obj != vl.end(); ++obj)
    {
        obj_name = obj->toString();
        utf8bytes = obj_name.toUtf8();
        PyObject *res = PyObject_GetAttrString(module.borrow(), utf8bytes);
        result = PyObjectRef(res, true);
        if (!result) {
            emitError(QString("Object '%1' is not found in '%2': (%3)").arg(obj_name).arg(module_name).arg(priv->formatExc()));
            continue;
        }
        PyDict_SetItemString(priv->globals.borrow(), utf8bytes.constData(), result.borrow());
    }

    return true;
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

    ENSURE_GIL_STATE;

    bool use_api_10 = (api_version_major == 1 && api_version_minor == 0);

    PyObjectRef module;

    if (use_api_10) {
        // PyOtherSide API 1.0 behavior (star import)
        module = PyObjectRef(PyImport_ImportModule(moduleName), true);
    } else {
        // PyOtherSide API 1.2 behavior: "import x.y.z"
        PyObjectRef fromList(PyList_New(0), true);
        module = PyObjectRef(PyImport_ImportModuleEx(const_cast<char *>(moduleName),
                    NULL, NULL, fromList.borrow()), true);
    }

    if (!module) {
        emitError(QString("Cannot import module: %1 (%2)").arg(name).arg(priv->formatExc()));
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

    PyDict_SetItemString(priv->globals.borrow(), moduleName, module.borrow());
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
            emitError("pyotherside.send() failed handler: " +
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
    ENSURE_GIL_STATE;

    PyObjectRef o(priv->eval(expr), true);
    if (!o) {
        emitError(QString("Cannot evaluate '%1' (%2)").arg(expr).arg(priv->formatExc()));
        return QVariant();
    }

    return convertPyObjectToQVariant(o.borrow());
}

QVariantList
QPython::unboxArgList(QVariant &args)
{
    // Unbox QJSValue from QVariant
    QVariantList vl = args.toList();
    for (int i = 0, c = vl.count(); i < c; ++i) {
        QVariant &v = vl[i];
        if (v.userType() == qMetaTypeId<QJSValue>()) {
            // TODO: Support boxing a QJSValue as reference in Python
            v = v.value<QJSValue>().toVariant();
        }
    }
    return vl;
}

void
QPython::call(QVariant func, QVariant boxed_args, QJSValue callback)
{
    QJSValue *cb = 0;
    if (!callback.isNull() && !callback.isUndefined() && callback.isCallable()) {
        cb = new QJSValue(callback);
    }
    // Unbox QJSValue from QVariant, since QJSValue::toVariant() can cause calls into
    // QML engine and we don't want that to happen from non-GUI thread
    QVariantList unboxed_args = unboxArgList(boxed_args);

    emit process(func, unboxed_args, cb);
}

QVariant
QPython::call_sync(QVariant func, QVariant boxed_args)
{
    return call_internal(func, boxed_args, true);
}

QVariant
QPython::call_internal(QVariant func, QVariant args, bool unbox)
{
    ENSURE_GIL_STATE;

    PyObjectRef callable;
    QString name;

    if (SINCE_API_VERSION(1, 4)) {
        if (static_cast<QMetaType::Type>(func.type()) == QMetaType::QString) {
            // Using version >= 1.4, but func is a string
            callable = PyObjectRef(priv->eval(func.toString()), true);
            name = func.toString();
        } else {
            // Try to interpret "func" as a Python object
            callable = PyObjectRef(convertQVariantToPyObject(func), true);
            PyObjectRef repr = PyObjectRef(PyObject_Repr(callable.borrow()), true);
            name = convertPyObjectToQVariant(repr.borrow()).toString();
        }
    } else {
        // Versions before 1.4 only support func as a string
        callable = PyObjectRef(priv->eval(func.toString()), true);
        name = func.toString();
    }

    if (!callable) {
        emitError(QString("Function not found: '%1' (%2)").arg(name).arg(priv->formatExc()));
        return QVariant();
    }

    // Unbox QJSValue from QVariant if requested. QPython::call may have done
    // this already, but call_sync is also exposed directly, so it does not
    // happen in this case otherwise
    QVariant args_unboxed;
    if (unbox) {
        args_unboxed = unboxArgList(args);
    } else {
        args_unboxed = args;
    }

    QVariant v;
    QString errorMessage = priv->call(callable.borrow(), name, args_unboxed, &v);
    if (!errorMessage.isNull()) {
        emitError(errorMessage);
    }
    return v;
}

QVariant
QPython::getattr(QVariant obj, QString attr) {
    if (!SINCE_API_VERSION(1, 4)) {
        emitError(QString("Import PyOtherSide 1.4 or newer to use getattr()"));
        return QVariant();
    }

    ENSURE_GIL_STATE;

    PyObjectRef pyobj(convertQVariantToPyObject(obj), true);

    if (!pyobj) {
        emitError(QString("Failed to convert %1 to python object: '%1' (%2)").arg(obj.toString()).arg(priv->formatExc()));
        return QVariant();
    }

    QByteArray byteArray = attr.toUtf8();
    const char *attrStr = byteArray.data();

    PyObjectRef o(PyObject_GetAttrString(pyobj.borrow(), attrStr), true);

    if (!o) {
        emitError(QString("Attribute not found: '%1' (%2)").arg(attr).arg(priv->formatExc()));
        return QVariant();
    }

    return convertPyObjectToQVariant(o.borrow());
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
            emitError(callbackResult.property("fileName").toString() + ":" +
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
            emitError(callbackResult.property("fileName").toString() + ":" +
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
    if (SINCE_API_VERSION(1, 5)) {
        ENSURE_GIL_STATE;

        PyObjectRef version_info(PySys_GetObject("version_info"));
        if (version_info && PyTuple_Check(version_info.borrow()) &&
                PyTuple_Size(version_info.borrow()) >= 3) {

            QStringList parts;
            for (int i=0; i<3; i++) {
                PyObjectRef part(PyTuple_GetItem(version_info.borrow(), i));
                parts << convertPyObjectToQVariant(part.borrow()).toString();
            }
            return parts.join('.');
        }

        // Fallback to the compile-time version below
        qWarning("Could not determine runtime Python version");
    }

    return QString(PY_VERSION);
}

void
QPython::emitError(const QString &message)
{
    if (error_connections) {
        emit error(message);
    } else {
        // We should only print the error if SINCE_API_VERSION(1, 4), but as
        // the error messages are useful for debugging (especially if users
        // don't import the latest API version), we do it unconditionally
        qWarning("Unhandled PyOtherSide error: %s", message.toUtf8().constData());
    }
}
