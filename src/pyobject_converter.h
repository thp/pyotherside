
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

#ifndef PYOTHERSIDE_PYOBJECT_CONVERTER_H
#define PYOTHERSIDE_PYOBJECT_CONVERTER_H

#include "converter.h"
#include "pyqobject.h"

#include "Python.h"
#include "datetime.h"
#include <QDebug>


class PyObjectListBuilder : public ListBuilder<PyObject *> {
    public:
        PyObjectListBuilder() : list(PyList_New(0)) {}
        virtual ~PyObjectListBuilder() {}

        virtual void append(PyObject *o) {
            PyList_Append(list, o);
            Py_DECREF(o);
        }

        virtual PyObject * value() {
            return list;
        }

    private:
        PyObject *list;
};

class PyObjectDictBuilder : public DictBuilder<PyObject *> {
    public:
        PyObjectDictBuilder() : dict(PyDict_New()) {}
        virtual ~PyObjectDictBuilder() {}

        virtual void set(PyObject *key, PyObject *value) {
            PyDict_SetItem(dict, key, value);
            Py_DECREF(value);
        }

        virtual PyObject * value() {
            return dict;
        }

    private:
        PyObject *dict;
};

class PyObjectListIterator : public ListIterator<PyObject *> {
    public:
        PyObjectListIterator(PyObject *&v)
            : list(v)
            , iter(PyObject_GetIter(list))
            , ref(NULL)
        {
            if (iter == NULL) {
                // TODO: Handle error
            }
        }

        virtual ~PyObjectListIterator()
        {
            Py_XDECREF(ref);
            Py_XDECREF(iter);

            if (PyErr_Occurred()) {
                // TODO: Handle error
            }
        }

        virtual bool next(PyObject **v) {
            if (!iter) {
                return false;
            }

            Py_XDECREF(ref);
            ref = PyIter_Next(iter);

            if (ref) {
                *v = ref;
                return true;
            }

            return false;
        }

    private:
        PyObject *list;
        PyObject *iter;
        PyObject *ref;
};

class PyObjectDictIterator : public DictIterator<PyObject *> {
    public:
        PyObjectDictIterator(PyObject *&v) : dict(v), pos(0) {}
        virtual ~PyObjectDictIterator() {}

        virtual bool next(PyObject **key, PyObject **value) {
            return PyDict_Next(dict, &pos, key, value);
        }

    private:
        PyObject *dict;
        Py_ssize_t pos;
};



class PyObjectConverter : public Converter<PyObject *> {
    public:
        PyObjectConverter() : stringcontainer(NULL) {
            if (!PyDateTimeAPI) {
                PyDateTime_IMPORT;
            }
        }

        virtual ~PyObjectConverter() {
            if (stringcontainer != NULL) {
                Py_DECREF(stringcontainer);
            }
        }

        virtual enum Type type(PyObject * const & o) {
            if (PyObject_TypeCheck(o, &pyotherside_QObjectType)) {
                return QOBJECT;
            } else if (PyObject_TypeCheck(o, &pyotherside_QObjectMethodType)) {
                qWarning("Cannot convert QObjectMethod yet - falling back to None");
                // TODO: Implement passing QObjectMethod references around
                return NONE;
            } else if (PyBool_Check(o)) {
                return BOOLEAN;
            } else if (PyLong_Check(o)) {
                return INTEGER;
            } else if (PyFloat_Check(o)) {
                return FLOATING;
            } else if (PyUnicode_Check(o) || PyBytes_Check(o)) {
                return STRING;
            } else if (PyDateTime_Check(o)) {
                // Need to check PyDateTime before PyDate, because
                // it is a subclass of PyDate.
                return DATETIME;
            } else if (PyDate_Check(o)) {
                return DATE;
            } else if (PyTime_Check(o)) {
                return TIME;
            } else if (PyList_Check(o) || PyTuple_Check(o) || PySet_Check(o) || PyIter_Check(o)) {
                return LIST;
            } else if (PyDict_Check(o)) {
                return DICT;
            } else if (o == Py_None) {
                return NONE;
            } else {
                return PYOBJECT;
            }
        }

        virtual long long integer(PyObject *&o) { return PyLong_AsLong(o); }
        virtual double floating(PyObject *&o) { return PyFloat_AsDouble(o); }
        virtual bool boolean(PyObject *&o) { return (o == Py_True); }
        virtual const char *string(PyObject *&o) {
            if (PyBytes_Check(o)) {
                return PyBytes_AsString(o);
            }

            // XXX: In Python 3.3, we can use PyUnicode_UTF8()
            if (stringcontainer != NULL) {
                Py_DECREF(stringcontainer);
            }
            stringcontainer = PyUnicode_AsUTF8String(o);
            return PyBytes_AsString(stringcontainer);
        }
        virtual ListIterator<PyObject *> *list(PyObject *&o) { return new PyObjectListIterator(o); }
        virtual DictIterator<PyObject *> *dict(PyObject *&o) { return new PyObjectDictIterator(o);; }
        virtual ConverterDate date(PyObject *&o) {
            return ConverterDate(PyDateTime_GET_YEAR(o),
                    PyDateTime_GET_MONTH(o),
                    PyDateTime_GET_DAY(o));
        }
        virtual ConverterTime time(PyObject *&o) {
            return ConverterTime(PyDateTime_TIME_GET_HOUR(o),
                    PyDateTime_TIME_GET_MINUTE(o),
                    PyDateTime_TIME_GET_SECOND(o),
                    PyDateTime_TIME_GET_MICROSECOND(o) / 1000);
        }
        virtual ConverterDateTime dateTime(PyObject *&o) {
            return ConverterDateTime(PyDateTime_GET_YEAR(o),
                    PyDateTime_GET_MONTH(o),
                    PyDateTime_GET_DAY(o),
                    PyDateTime_DATE_GET_HOUR(o),
                    PyDateTime_DATE_GET_MINUTE(o),
                    PyDateTime_DATE_GET_SECOND(o),
                    PyDateTime_DATE_GET_MICROSECOND(o) / 1000);
        }
        virtual PyObjectRef pyObject(PyObject *&o) { return PyObjectRef(o); }
        virtual QObjectRef qObject(PyObject *&o) {
            if (PyObject_TypeCheck(o, &pyotherside_QObjectType)) {
                pyotherside_QObject *result = reinterpret_cast<pyotherside_QObject *>(o);
                return QObjectRef(*(result->m_qobject_ref));
            }

            return QObjectRef();
        }

        virtual PyObject * fromInteger(long long v) { return PyLong_FromLong((long)v); }
        virtual PyObject * fromFloating(double v) { return PyFloat_FromDouble(v); }
        virtual PyObject * fromBoolean(bool v) { return PyBool_FromLong((long)v); }
        virtual PyObject * fromString(const char *v) { return PyUnicode_FromString(v); }
        virtual PyObject * fromDate(ConverterDate v) { return PyDate_FromDate(v.y, v.m, v.d); }
        virtual PyObject * fromTime(ConverterTime v) { return PyTime_FromTime(v.h, v.m, v.s, 1000 * v.ms); }
        virtual PyObject * fromDateTime(ConverterDateTime v) {
            return PyDateTime_FromDateAndTime(v.y, v.m, v.d, v.time.h, v.time.m, v.time.s, v.time.ms * 1000);
        }
        virtual PyObject * fromPyObject(const PyObjectRef &pyobj) { return pyobj.newRef(); }
        virtual PyObject * fromQObject(const QObjectRef &qobj) {
            pyotherside_QObject *result = PyObject_New(pyotherside_QObject, &pyotherside_QObjectType);
            result->m_qobject_ref = new QObjectRef(qobj);
            return reinterpret_cast<PyObject *>(result);
        }
        virtual ListBuilder<PyObject *> *newList() { return new PyObjectListBuilder(); }
        virtual DictBuilder<PyObject *> *newDict() { return new PyObjectDictBuilder(); }
        virtual PyObject * none() { Py_RETURN_NONE; }

    private:
        PyObject *stringcontainer;
};

#endif /* PYOTHERSIDE_PYOBJECT_CONVERTER_H */
