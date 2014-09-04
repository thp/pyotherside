
/**
 * PyOtherSide: Asynchronous Python 3 Bindings for Qt 5
 * Copyright (c) 2014, Dennis Tomas <den.t@gmx.de>
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

#ifndef PYOTHERSIDE_PYGLAREA_H
#define PYOTHERSIDE_PYGLAREA_H

#include "Python.h"

#include <QString>
#include <QtQuick/QQuickItem>

#include "pyglrenderer.h"


class PyGLArea : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(QString initGL READ initGL WRITE setInitGL)
    Q_PROPERTY(QString paintGL READ paintGL WRITE setPaintGL)
    Q_PROPERTY(QString cleanupGL READ cleanupGL WRITE setCleanupGL)
    Q_PROPERTY(bool before READ before WRITE setBefore)
    Q_PROPERTY(qreal t READ t WRITE setT NOTIFY tChanged)

public:
    PyGLArea();
    ~PyGLArea();

    QString initGL() const { return m_initGL; };
    QString paintGL() const { return m_paintGL; };
    QString cleanupGL() const { return m_paintGL; };
    bool before() { return m_before; };
    void setInitGL(QString initGL);
    void setPaintGL(QString paintGL);
    void setCleanupGL(QString cleanupGL);
    void setBefore(bool before);
    qreal t() const { return m_t; }
    void setT(qreal t);

signals:
    void tChanged();

public slots:
    void sync();
    void update();

private slots:
    void handleWindowChanged(QQuickWindow *win);
    void paint();
    void cleanup();

private:
    qreal m_t;
    PyObject *m_paintGLCallable;
    bool m_initialized;
    QString m_initGL;
    QString m_paintGL;
    QString m_cleanupGL;
    bool m_before;
    PyGLRenderer *m_renderer;
};

#endif /* PYOTHERSIDE_PYGLAREA_H */
