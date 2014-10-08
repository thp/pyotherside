
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

#include "qpython_priv.h"

#include "qpython_imageprovider.h"
#include "ensure_gil_state.h"

#include <QDebug>


QPythonImageProvider::QPythonImageProvider()
    : QQuickImageProvider(QQmlImageProviderBase::Image)
{
}

QPythonImageProvider::~QPythonImageProvider()
{
}

static void
cleanup_python_qimage(void *data)
{
    QPythonPriv *priv = QPythonPriv::instance();

    ENSURE_GIL_STATE;
    Py_XDECREF(static_cast<PyObject *>(data));
}

QImage
QPythonImageProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    QImage img;

    // Image data (and metadata) returned from Python
    PyObject *pixels = NULL;
    unsigned int width = 0, height = 0;
    int format = 0;

    // For counting the number of required bytes
    int bitsPerPixel = 0;
    size_t requiredBytes = 0;
    size_t actualBytes = 0;

    QPythonPriv *priv = QPythonPriv::instance();
    if (!priv) {
        qWarning() << "Python component not instantiated yet";
        return QImage();
    }

    if (!priv->image_provider) {
        qWarning() << "No image provider set in Python code";
        return QImage();
    }

    QByteArray id_utf8 = id.toUtf8();

    // Image provider implementation in Python:
    //
    // import pyotherside
    //
    // def image_provider(image_id, requested_size):
    //     if requested_size == (-1, -1):
    //         requested_size = 100, 200 # some sane default size
    //     width, height = requested_size
    //     pixels = ...
    //     format = pyotherside.format_argb32 # or some other format
    //     return (bytearray(pixels), (width, height), format)
    //
    // pyotherside.set_image_provider(image_provider)

    ENSURE_GIL_STATE;

    PyObjectRef args(Py_BuildValue("(N(ii))",
            PyUnicode_FromString(id_utf8.constData()),
            requestedSize.width(), requestedSize.height()), true);
    PyObjectRef result(PyObject_Call(priv->image_provider.borrow(), args.borrow(), NULL), true);

    if (!result) {
        qDebug() << "Error while calling the image provider";
        PyErr_Print();
        goto cleanup;
    }

    if (!PyArg_ParseTuple(result.borrow(), "O(ii)i", &pixels, &width, &height, &format)) {
        PyErr_Clear();
        qDebug() << "Image provider must return (pixels, (width, height), format)";
        goto cleanup;
    }

    if (!PyByteArray_Check(pixels)) {
        qDebug() << "Image data must be a Python bytearray()";
        goto cleanup;
    }

    switch (format) {
        case -1: /* pyotherside.format_data */
            break;
        case QImage::Format_Mono:
        case QImage::Format_MonoLSB:
            bitsPerPixel = 1;
            break;
        case QImage::Format_RGB32:
        case QImage::Format_ARGB32:
            bitsPerPixel = 32;
            break;
        case QImage::Format_RGB16:
        case QImage::Format_RGB555:
        case QImage::Format_RGB444:
            bitsPerPixel = 16;
            break;
        case QImage::Format_RGB666:
        case QImage::Format_RGB888:
            bitsPerPixel = 24;
            break;
        default:
            qDebug() << "Invalid format:" << format;
            goto cleanup;
    }

    requiredBytes = (bitsPerPixel * width * height + 7) / 8;
    actualBytes = PyByteArray_Size(pixels);

    // QImage requires scanlines to be 32-bit aligned. Scanlines from Python
    // are considered to be tightly packed, we have to check for alignment.
    // While we could re-pack the data to be aligned, we don't want to do that
    // for performance reasons.
    // If we're using 32-bit data (e.g. ARGB32), it will always be aligned.
    if (format != -1 && bitsPerPixel != 32) {
        if ((bitsPerPixel * width) % 32 != 0) {
            // If actualBytes > requiredBytes, we can check if there are enough
            // bytes to consider the data 32-bit aligned (from Python) and avoid
            // the error (scanlines must be padded to multiples of 4 bytes)
            if ((((width * bitsPerPixel / 8 + 3) / 4) * 4 * height) == actualBytes) {
                qDebug() << "Assuming 32-bit aligned scanlines from Python";
            } else {
                qDebug() << "Each scanline of data must be 32-bit aligned";
                goto cleanup;
            }
        }
    }

    if (format != -1 && requiredBytes > actualBytes) {
        qDebug() << "Format" << (enum QImage::Format)format <<
            "at size" << QSize(width, height) <<
            "requires at least" << requiredBytes <<
            "bytes of image data, got only" << actualBytes << "bytes";
        goto cleanup;
    }


    if (format == -1) {
        // Pixel data is actually encoded image data that we need to decode
        img.loadFromData((const unsigned char*)PyByteArray_AsString(pixels),
                PyByteArray_Size(pixels));
    } else {
        // Need to keep a reference to the byte array object, as it contains
        // the backing store data for the QImage.
        // Will be decref'd by cleanup_python_qimage once the QImage is gone.
        Py_INCREF(pixels);

        img = QImage((const unsigned char *)PyByteArray_AsString(pixels),
                width, height, (enum QImage::Format)format,
                cleanup_python_qimage, pixels);
    }

cleanup:

    *size = img.size();
    return img;
}
