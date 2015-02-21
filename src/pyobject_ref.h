
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

#ifndef PYOTHERSIDE_PYOBJECT_REF_H
#define PYOTHERSIDE_PYOBJECT_REF_H

#include "Python.h"

#include <QMetaType>

class PyObjectRef {
    public:
        explicit PyObjectRef(PyObject *obj=0, bool consume=false);
        PyObjectRef(const PyObjectRef &other);
        virtual ~PyObjectRef();
        PyObjectRef &operator=(const PyObjectRef &other);
        bool operator==(const PyObjectRef &other);

        PyObject *newRef() const;

        PyObject *borrow() const;
        operator bool() const { return (pyobject != 0); }

    private:
        PyObject *pyobject;
};

Q_DECLARE_METATYPE(PyObjectRef)

#endif // PYOTHERSIDE_PYOBJECT_REF_H
