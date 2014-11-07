
/**
 * PyOtherSide: Asynchronous Python 3 Bindings for Qt 5
 * Copyright (c) 2013, Thomas Perl <m@thp.io>
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

#include "global_libpython_loader.h"

namespace GlobalLibPythonLoader {

#if defined(__linux__) && !defined(ANDROID)

#define _GNU_SOURCE
#include <link.h>

#include <stdio.h>
#include <string.h>

static int load_python_globally_callback(struct dl_phdr_info *info, size_t size, void *data)
{
    int major, minor;
    const char *basename = strrchr(info->dlpi_name, '/');
    int *success = (int *)data;

    if (basename != NULL) {
        if (sscanf(basename, "/libpython%d.%d.so", &major, &minor) != 2) {
            if (sscanf(basename, "/libpython%d.%dm.so", &major, &minor) != 2) {
                return 0;
            }
        }

        void *pylib = dlopen(info->dlpi_name, RTLD_GLOBAL | RTLD_NOW);
        if (pylib != NULL) {
            *success = 1;
        } else {
            fprintf(stderr, "Could not load python library '%s': %s\n",
                    info->dlpi_name, dlerror());
        }
    }

    return 0;
}

bool loadPythonGlobally()
{
    int success = 0;
    dl_iterate_phdr(load_python_globally_callback, &success);
    return success;
}


#else /* __linux__ */

bool loadPythonGlobally()
{
    /* On non-Linux systems, no need to load globally */
    return true;
}

#endif /* __linux__ */

}; /* namespace GlobalLibPythonLoader */
