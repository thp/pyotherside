
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

#include "pythonlib_loader.h"

#include <QStandardPaths>
#include <QDir>
#include <QDebug>

#if defined(HAVE_DLADDR)
#  include <dlfcn.h>
#endif

namespace PythonLibLoader {

static void prependPythonPath(const QString &path)
{
    QString pythonpath(path + ":" + QString::fromUtf8(qgetenv("PYTHONPATH")));
    QByteArray pythonpath_utf8 = pythonpath.toUtf8();
    qputenv("PYTHONPATH", pythonpath_utf8.constData());
}

#if defined(PYTHONLIB_LOADER_HAVE_PYTHONLIB_ZIP)

bool extractPythonLibrary()
{
    QString source(":/io/thp/pyotherside/pythonlib.zip");
    QString destdir(QStandardPaths::writableLocation(QStandardPaths::TempLocation));
    QString destination(QDir(destdir).filePath("pythonlib.zip"));
    prependPythonPath(destination);

    if (QFile::exists(destination)) {
        return true;
    }

    return QFile::copy(source, destination);
}

#else /* PYTHONLIB_LOADER_HAVE_PYTHONLIB_ZIP */

bool extractPythonLibrary()
{
#if defined(HAVE_DLADDR)
    // Add the library into the path in case it has a .zip file appended
    Dl_info info;
    memset(&info, 0, sizeof(info));
    int res = dladdr((void *)&extractPythonLibrary, &info);
    if (!res) {
        qWarning() << "Could not determine library path";
        return false;
    }

    QString fname = QString::fromUtf8(info.dli_fname);
    qDebug() << "Got library name: " << fname;
    // On Android, dladdr() returns only the basename of the file, so we go
    // hunt for the full path in /proc/self/maps, where the shared library is
    // mapped (TODO: We could parse the address range and compare that, too)
    if (!fname.startsWith("/")) {
        QFile mapsf("/proc/self/maps");
        if (mapsf.exists()) {
            mapsf.open(QIODevice::ReadOnly);
            QTextStream maps(&mapsf);
            QString line;
            while (!(line = maps.readLine()).isNull()) {
                QString filename = line.split(' ', QString::SkipEmptyParts).last();
                if (filename.endsWith("/" + fname)) {
                    fname = filename;
                    qDebug() << "Resolved full path:" << fname;
                    break;
                }
            }
        }
    }
    prependPythonPath(fname);
#endif
    return true;
}

#endif /* PYTHONLIB_LOADER_HAVE_PYTHONLIB_ZIP */

}; /* namespace PythonLibLoader */
