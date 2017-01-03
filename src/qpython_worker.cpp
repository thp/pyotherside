
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

#include "qpython.h"

#include "qpython_worker.h"


QPythonWorker::QPythonWorker(QPython *qpython)
    : QObject()
    , qpython(qpython)
{
}

QPythonWorker::~QPythonWorker()
{
}

void
QPythonWorker::process(QVariant func, QVariant unboxed_args, QJSValue *callback)
{
    QVariant result = qpython->call_internal(func, unboxed_args, false);
    if (callback) {
        emit finished(result, callback);
    }
}

void
QPythonWorker::import(QString name, QJSValue *callback)
{
    bool result = qpython->importModule_sync(name);
    if (callback) {
        emit imported(result, callback);
    }
}

void
QPythonWorker::import_names(QString name, QVariant args, QJSValue *callback)
{
    bool result = qpython->importNames_sync(name, args);
    if (callback) {
        emit imported(result, callback); // using the same imported signal at the end
    }
}
