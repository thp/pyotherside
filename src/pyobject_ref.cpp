
/**
 * PyOtherSide: Asynchronous Python 3 Bindings for Qt 5
 * Copyright (c) 2014, Felix Krull <f_krull@gmx.de>
 * Copyright (c) 2014, Thomas Perl <m@thp.io>
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

#include "pyobject_ref.h"
#include "ensure_gil_state.h"

PyObjectRef::PyObjectRef(PyObject *obj, bool consume)
    : pyobject(obj)
{
    if (pyobject && !consume) {
        ENSURE_GIL_STATE;
        Py_INCREF(pyobject);
    }
}

PyObjectRef::PyObjectRef(const PyObjectRef &other)
    : pyobject(other.pyobject)
{
    if (pyobject) {
        ENSURE_GIL_STATE;
        Py_INCREF(pyobject);
    }
}

PyObjectRef::~PyObjectRef()
{
    if (pyobject) {
        ENSURE_GIL_STATE;
        Py_CLEAR(pyobject);
    }
}

PyObjectRef &
PyObjectRef::operator=(const PyObjectRef &other)
{
    if (this != &other) {
        if (pyobject || other.pyobject) {
            ENSURE_GIL_STATE;

            if (pyobject) {
                Py_CLEAR(pyobject);
            }

            if (other.pyobject) {
                pyobject = other.pyobject;
                Py_INCREF(pyobject);
            }
        }
    }

    return *this;
}

bool
PyObjectRef::operator==(const PyObjectRef &other)
{
    return pyobject == other.pyobject;
}

PyObject *
PyObjectRef::newRef() const
{
    if (pyobject) {
        ENSURE_GIL_STATE;
        Py_INCREF(pyobject);
    }

    return pyobject;
}

PyObject *
PyObjectRef::borrow() const
{
    // Borrowed reference
    return pyobject;
}
