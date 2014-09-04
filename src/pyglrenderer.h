
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

#ifndef PYOTHERSIDE_PYGLRENDERER_H
#define PYOTHERSIDE_PYGLRENDERER_H

#include "Python.h"

#include <QtCore/QObject>
#include <QString>
#include <QRect>


class PyGLRenderer : public QObject {
    Q_OBJECT

public:
    PyGLRenderer();
    ~PyGLRenderer();

    void init();
    void paint();
    void cleanup();

    void setInitGL(QString initGL);
    void setPaintGL(QString paintGL);
    void setCleanupGL(QString cleanupGL);
    void setRect(QRect rect);

private:
    QRect m_rect;
    PyObject *m_paintGLCallable;
    bool m_initialized;
    QString m_initGL;
    QString m_paintGL;
    QString m_cleanupGL;
};

#endif /* PYOTHERSIDE_PYGLRENDERER_H */
