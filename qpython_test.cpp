
#include "qpython_test.h"

#include "qpython.h"

#include <QDebug>

void
qpython_test()
{
    QPython py;
    py.addImportPath(".");
    py.importModule("pyotherside");

    qDebug() << py.evaluate("pyotherside.demo()");

    QList<QVariant> l;
    l << 1;
    l << "Hello World";
    l << 3;

    QVariantList l2;
    l2 << 4 << QString::fromUtf8("Jössas!") << 6;
    l << QVariant(l2);

    QMap<QString,QVariant> m;
    m["Hello"] = 4711;
    l << QVariant(m);

    qDebug() << l;
    QVariant v(l);
    PyObject *o = py.toPython(v);
    PyObject_Print(o, stdout, 0);
    printf("\n");
    QVariant l3 = py.fromPython(o);
    qDebug() << l3;
    Py_DECREF(o);
}

