
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

#include "pyobject_converter.h"
#include "qvariant_converter.h"

#include "qpython.h"
#include "converter.h"
#include "qml_python_bridge.h"

#include "tests.h"

TestPyOtherSide::TestPyOtherSide()
    : QObject()
{
}

QTEST_MAIN(TestPyOtherSide)


template<class V>
void
test_converter_for(Converter<V> *conv)
{
    V v, w, x;

    /* Convert from/to Integer */
    v = conv->fromInteger(123);
    QVERIFY(conv->type(v) == Converter<V>::INTEGER);
    QVERIFY(conv->integer(v) == 123);

    /* Convert from/to Float */
    v = conv->fromFloating(42.23);
    QVERIFY(conv->type(v) == Converter<V>::FLOATING);
    QVERIFY(conv->floating(v) == 42.23);

    /* Convert from/to Bool */
    v = conv->fromBoolean(true);
    QVERIFY(conv->type(v) == Converter<V>::BOOLEAN);
    QVERIFY(conv->boolean(v));
    v = conv->fromBoolean(false);
    QVERIFY(conv->type(v) == Converter<V>::BOOLEAN);
    QVERIFY(!conv->boolean(v));

    /* Convert from/to String */
    v = conv->fromString("Hello World");
    QVERIFY(conv->type(v) == Converter<V>::STRING);
    QVERIFY(strcmp(conv->string(v), "Hello World") == 0);

    /* Convert from/to List */
    ListBuilder<V> *builder = conv->newList();
    v = conv->fromInteger(444);
    builder->append(v);
    v = conv->fromString("Hello");
    builder->append(v);
    v = builder->value();
    delete builder;
    ListIterator<V> *iterator = conv->list(v);
    QVERIFY(iterator->next(&w));
    QVERIFY(conv->type(w) == Converter<V>::INTEGER);
    QVERIFY(conv->integer(w) == 444);
    QVERIFY(iterator->next(&w));
    QVERIFY(conv->type(w) == Converter<V>::STRING);
    QVERIFY(strcmp(conv->string(w), "Hello") == 0);
    delete iterator;

    /* Convert from/to Dict */
    DictBuilder<V> *builder2 = conv->newDict();
    v = conv->fromBoolean(true);
    builder2->set(conv->fromString("a"), v);
    v = builder2->value();
    delete builder2;
    DictIterator<V> *iterator2 = conv->dict(v);
    QVERIFY(iterator2->next(&w, &x));
    QVERIFY(conv->type(w) == Converter<V>::STRING);
    QVERIFY(strcmp(conv->string(w), "a") == 0);
    QVERIFY(conv->type(x) == Converter<V>::BOOLEAN);
    QVERIFY(conv->boolean(x) == true);
    delete iterator2;

    /* Convert from/to generic PyObject */
    PyObject *obj = PyCapsule_New(conv, "test", NULL);
    v = conv->fromPyObject(PyObjectRef(obj));
    QVERIFY(conv->type(v) == Converter<V>::PYOBJECT);

    // Check if getting a new reference works
    PyObject *o = conv->pyObject(v).newRef();
    QVERIFY(o == obj);
    Py_DECREF(o);

    Py_CLEAR(obj);

    delete conv;
}

void destruct(PyObject *obj) {
    bool *destructor_called = (bool *)PyCapsule_GetPointer(obj, "test");
    *destructor_called = true;
}

void TestPyOtherSide::testPyObjectRefAssignment()
{
    // Test assignment operator of PyObjectRef
    bool destructor_called_foo = false;
    PyObject *foo = PyCapsule_New(&destructor_called_foo, "test", destruct);

    bool destructor_called_bar = false;
    PyObject *bar = PyCapsule_New(&destructor_called_bar, "test", destruct);

    QVERIFY(foo);
    QVERIFY(foo->ob_refcnt == 1);

    QVERIFY(bar);
    QVERIFY(bar->ob_refcnt == 1);

    {
        PyObjectRef a(foo);
        PyObjectRef b(bar);
        PyObjectRef c; // empty

        // foo got a new reference in a
        QVERIFY(foo->ob_refcnt == 2);
        // bar got a new reference in b
        QVERIFY(bar->ob_refcnt == 2);

        // Overwrite empty reference with reference to bar
        c = b;
        // bar got a new reference in c
        QVERIFY(bar->ob_refcnt == 3);
        // reference count for foo is unchanged
        QVERIFY(foo->ob_refcnt == 2);

        // Overwrite reference to bar with reference to foo
        b = a;
        // bar lost a reference in b
        QVERIFY(bar->ob_refcnt == 2);
        // foo got a new reference in b
        QVERIFY(foo->ob_refcnt == 3);

        // Overwrite reference to foo with empty reference
        a = PyObjectRef();
        // foo lost a reference in a
        QVERIFY(foo->ob_refcnt == 2);

        Py_DECREF(foo);

        // there is still a reference to foo in b
        QVERIFY(foo->ob_refcnt == 1);
        QVERIFY(!destructor_called_foo);

        // a falls out of scope (but is empty)
        // b falls out of scope, foo loses a reference
        // c falls out of scope, bar loses a reference
    }

    // Now that b fell out of scope, foo was destroyed
    QVERIFY(destructor_called_foo);

    // But we still have a single reference to bar
    QVERIFY(!destructor_called_bar);
    QVERIFY(bar->ob_refcnt == 1);
    Py_CLEAR(bar);

    // Now bar is also gone
    QVERIFY(destructor_called_bar);
}

void
TestPyOtherSide::testPyObjectRefRoundTrip()
{
    // Simulate a complete round-trip of a PyObject reference, from PyOtherSide
    // to QML and back.

    // Create a Python object, i.e. in a Python function.
    bool destructor_called = false;
    PyObject *o = PyCapsule_New(&destructor_called, "test", destruct);
    QVERIFY(o->ob_refcnt == 1);

    // Convert the object to a QVariant and increment its refcount.
    QVariant v = convertPyObjectToQVariant(o);

    // Decrement refcount and pass QVariant to QML.
    QVERIFY(o->ob_refcnt == 2);
    Py_DECREF(o);
    QVERIFY(o->ob_refcnt == 1);

    // Pass QVariant back to PyOtherSide, which converts it to a PyObject,
    // incrementing its refcount.
    PyObject *o2 = convertQVariantToPyObject(v);
    QVERIFY(o->ob_refcnt == 2);

    // The QVariant is deleted, i.e. by a JS variable falling out of scope.
    // This deletes the PyObjectRef and thus decrements the object's refcount.
    v = QVariant();

    // At this point, we only have one reference (the one from o2)
    QVERIFY(o->ob_refcnt == 1);

    // There's still a reference, so the destructor must not have been called
    QVERIFY(!destructor_called);

    // Now, at this point, the last remaining reference is removed, which
    // will cause the destructor to be called
    Py_DECREF(o2);

    // There are no references left, so the capsule's destructor is called.
    QVERIFY(destructor_called);
}

void
TestPyOtherSide::testQObjectRef()
{
    QObject *o = new QObject();
    QObjectRef ref(o);

    QVERIFY(ref.value() == o);

    delete o;

    QVERIFY(ref.value() == NULL);
}

void
TestPyOtherSide::testQVariantConverter()
{
    test_converter_for<QVariant>(new QVariantConverter);
}

void
TestPyOtherSide::testPyObjectConverter()
{
    test_converter_for<PyObject *>(new PyObjectConverter);
}

void
TestPyOtherSide::testConvertToPythonAndBack()
{
    QVariantList l;
    l << "Hello" << 123 << 45.667 << true;
    QVariantList l2;
    l2 << "A" << QVariant() << "B" << 4711;
    l << QVariant(l2);
    QVariant v(l);

    PyObject *o = convertQVariantToPyObject(v);
    QVariant v2 = convertPyObjectToQVariant(o);
    QVERIFY(v == v2);
}

static void testEvaluateWith(QPython *py)
{
    QVariant squares = py->evaluate("[x*x for x in range(10)]");
    QVERIFY(squares.canConvert(QMetaType::QVariantList));

    QVariantList squares_list = squares.toList();
    QVERIFY(squares_list.size() == 10);
    QVERIFY(squares_list[0] == 0);
    QVERIFY(squares_list[1] == 1);
    QVERIFY(squares_list[2] == 4);
    QVERIFY(squares_list[3] == 9);
    QVERIFY(squares_list[4] == 16);
    QVERIFY(squares_list[5] == 25);
    QVERIFY(squares_list[6] == 36);
    QVERIFY(squares_list[7] == 49);
    QVERIFY(squares_list[8] == 64);
    QVERIFY(squares_list[9] == 81);
}

void
TestPyOtherSide::testEvaluate()
{
    // PyOtherSide API 1.0
    QPython10 py10;
    testEvaluateWith(&py10);

    // PyOtherSide API 1.2
    QPython12 py12;
    testEvaluateWith(&py12);

    // PyOtherSide API 1.3
    QPython13 py13;
    testEvaluateWith(&py13);
}

void
TestPyOtherSide::testSetToList()
{
    // Test if a Python set is converted to a list
    PyObject *set = PySet_New(NULL);
    QVERIFY(set != NULL);
    PyObject *o = NULL;

    o = PyLong_FromLong(123);
    QVERIFY(o != NULL);
    QVERIFY(PySet_Add(set, o) == 0);

    o = PyLong_FromLong(321);
    QVERIFY(o != NULL);
    QVERIFY(PySet_Add(set, o) == 0);

    o = PyLong_FromLong(444);
    QVERIFY(o != NULL);
    QVERIFY(PySet_Add(set, o) == 0);

    // This will not be added (no duplicates in a set)
    o = PyLong_FromLong(123);
    QVERIFY(o != NULL);
    QVERIFY(PySet_Add(set, o) == 0);

    // At this point, we should have 3 items (123, 321 and 444)
    QVERIFY(PySet_Size(set) == 3);

    QVariant v = convertPyObjectToQVariant(set);
    QVERIFY(v.canConvert(QMetaType::QVariantList));

    QList<QVariant> l = v.toList();
    QVERIFY(l.size() == 3);
    QVERIFY(l.contains(123));
    QVERIFY(l.contains(321));
    QVERIFY(l.contains(444));
}
