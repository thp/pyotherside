
/**
 * PyOtherSide: Asynchronous Python 3 Bindings for Qt 5
 * Copyright (c) 2011, 2013, 2014, Thomas Perl <m@thp.io>
 * Copyright (c) 2015, Robie Basak <robie@justgohome.co.uk>
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

#ifndef PYOTHERSIDE_QPYTHON_ITEMMODEL_H
#define PYOTHERSIDE_QPYTHON_ITEMMODEL_H

#include "Python.h"

#include <QAbstractItemModel>

#include "pyobject_ref.h"

class QPythonItemModel : public QAbstractItemModel {
    Q_OBJECT
    Q_PROPERTY(QVariant model READ model WRITE setModel)

public:
    QPythonItemModel(QObject *parent = 0);
    virtual ~QPythonItemModel();

    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int rowCount(const QModelIndex & parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex &index) const;

    QVariant model() const { return QVariant::fromValue(m_model); };
    void setModel(QVariant model);
    void sendModificationCallback(PyObject *cb) { Py_INCREF(cb); emit needModificationCallback(cb); };
    void sendDataChanged(PyObject *args);
    QModelIndex pysequence_to_qmodelindex(PyObject *list);
    QHash<int, QByteArray> roleNames(void) const;

    // Pretty hacky. I can't make pyitemmodel_proxy a friend because it isn't a
    // class, and it can't be a class because Python needs it to have
    // particular function prototypes that don't include "this". Need to find a
    // better way.
    void do_beginInsertRows(const QModelIndex & parent, int first, int last) { beginInsertRows(parent, first, last); };
    void do_endInsertRows(void) { endInsertRows(); };
    void do_beginRemoveRows(const QModelIndex & parent, int first, int last) { beginRemoveRows(parent, first, last); };
    void do_endRemoveRows(void) { endRemoveRows(); };

private slots:
    void handleModificationCallback(PyObject *cb);

signals:
    void needModificationCallback(PyObject *cb);

private:
    PyObjectRef m_model;
    PyObject *m_proxy;
    QModelIndex call_model_for_index(PyObject *args, const char *method_name) const;
};

#endif /* PYOTHERSIDE_QPYTHON_ITEMMODEL_H */
