#ifndef _QPYTHON_H
#define _QPYTHON_H

#include <QVariant>
#include <QObject>
#include <QString>

#include "Python.h"

class QPython : public QObject
{
    Q_OBJECT

    public:
        QPython(QObject *parent=NULL);
        virtual ~QPython();

        Q_INVOKABLE void
        addImportPath(QString path);

        Q_INVOKABLE bool
        importModule(QString name);

        Q_INVOKABLE QVariant
        evaluate(QString expr);

        Q_INVOKABLE QVariant
        call(QString func, QVariant args);

        static void
        registerQML();

        // Convert a Python value to a Qt value
        QVariant fromPython(PyObject *o);

        // Convert a Qt value to a Python value
        PyObject *toPython(QVariant v);

        // Internal function to evaluate a string to a PyObject *
        // Used by evaluate() and call()
        PyObject *eval(QString expr);

    private:
        PyObject *locals;
        PyObject *globals;

        static int instances;
};

#endif

