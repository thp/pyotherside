
/**
 * PyOtherSide: Asynchronous Python 3 Bindings for Qt 5
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

#ifndef PYOTHERSIDE_QOBJECT_REF_H
#define PYOTHERSIDE_QOBJECT_REF_H

#include <QObject>

class QObjectRef : public QObject {
    Q_OBJECT

public:
    explicit QObjectRef(QObject *obj=0);
    virtual ~QObjectRef();

    QObjectRef(const QObjectRef &other);
    QObjectRef &operator=(const QObjectRef &other);

    QObject *value() const { return qobject; }
    operator bool() const { return (qobject != 0); }

private slots:
    void handleDestroyed(QObject *obj);

private:
    QObject *qobject;
};

class QObjectMethodRef {
public:
    QObjectMethodRef(const QObjectRef &object, const QString &method)
        : m_object(object)
        , m_method(method)
    {
    }

    const QObjectRef &object() { return m_object; }
    const QString &method() { return m_method; }

private:
    QObjectRef m_object;
    QString m_method;
};

#endif // PYOTHERSIDE_QOBJECT_REF_H
