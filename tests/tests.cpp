
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

    delete conv;
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
