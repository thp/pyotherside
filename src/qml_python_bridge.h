
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

#ifndef PYOTHERSIDE_QML_PYTHON_BRIDGE_H
#define PYOTHERSIDE_QML_PYTHON_BRIDGE_H

#include "pyobject_converter.h"
#include "qvariant_converter.h"

inline PyObject *
convertQVariantToPyObject(QVariant v)
{
    return convert<QVariant, PyObject *, QVariantConverter, PyObjectConverter>(v);
}

inline QVariant
convertPyObjectToQVariant(PyObject *o)
{
    return convert<PyObject *, QVariant, PyObjectConverter, QVariantConverter>(o);
}

#endif /* PYOTHERSIDE_QML_PYTHON_BRIDGE_H */
