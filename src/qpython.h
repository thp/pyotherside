
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

#ifndef PYOTHERSIDE_QPYTHON_H
#define PYOTHERSIDE_QPYTHON_H

#include <QVariant>
#include <QObject>
#include <QString>
#include <QStringList>

#include <QMap>
#include <QThread>
#include <QJSValue>

class QPython;
class QPythonPriv;

class QPythonWorker : public QObject {
    Q_OBJECT

    public:
        QPythonWorker(QPython *qpython);
        ~QPythonWorker();

    public slots:
        void process(QString func, QVariant args, QJSValue callback);
        void import(QString func, QJSValue callback);

    signals:
        void finished(QVariant result, QJSValue callback);
        void imported(bool result, QJSValue callback);

    private:
        QPython *qpython;
};

class QPython : public QObject {
    Q_OBJECT

    public:
        QPython(QObject *parent=NULL);
        virtual ~QPython();

        Q_INVOKABLE void
        addImportPath(QString path);

        Q_INVOKABLE void
        setHandler(QString event, QJSValue callback);

        Q_INVOKABLE QVariant
        evaluate(QString expr);

        /* Need a callback, as the module can only be used after import */
        Q_INVOKABLE void
        importModule(QString name, QJSValue callback);

        Q_INVOKABLE bool
        importModule_sync(QString name);

        Q_INVOKABLE void
        call(QString func,
             QVariant args=QVariantList(),
             QJSValue callback=QJSValue());

        Q_INVOKABLE QVariant
        call_sync(QString func, QVariant args=QVariantList());

        static void
        closing();

    signals:
        void received(QVariant data);
        void error(QString traceback);

        void process(QString func, QVariant args, QJSValue callback);
        void import(QString name, QJSValue callback);

    private slots:
        void receive(QVariant data);

        void finished(QVariant result, QJSValue callback);
        void imported(bool result, QJSValue callback);

    private:
        static QPythonPriv *priv;

        QPythonWorker worker;
        QThread thread;
        QMap<QString,QJSValue> handlers;
};

#endif /* PYOTHERSIDE_QPYTHON_H */
