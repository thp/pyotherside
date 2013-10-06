#
# PyOtherSide: Asynchronous Python 3 Bindings for Qt 5
# Copyright (c) 2011, 2013, Thomas Perl <m@thp.io>
#
# Permission to use, copy, modify, and/or distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
# REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
# FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
# INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
# LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
# OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
# PERFORMANCE OF THIS SOFTWARE.
#

import matplotlib.pyplot as plt
import numpy as np
import io

import pyotherside

def plot_provider(image_id, requested_size):
    plt.figure()
    x = np.arange(0.0, 5.0, 0.1)
    plt.plot(np.sin(x))
    for dataset in image_id.split(';'):
        plt.plot([float(x) for x in dataset.split(',') if x])
    plt.title("Dataset from QML")
    buf = io.BytesIO()
    plt.savefig(buf, format='png')
    plt.close()
    buf.seek(0)
    return bytearray(buf.read()), requested_size, pyotherside.format_data

pyotherside.set_image_provider(plot_provider)
