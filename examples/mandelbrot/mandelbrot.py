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

import pyotherside
import random

COLORS = [(255, 255, 255, 255), (255, 0, 0, 0)]

for i in range(30):
    COLORS.insert(1, (
        255, # alpha
        random.randint(30, 255), # red
        random.randint(30, 255), # green
        random.randint(30, 255), # blue
    ))

# Of course, in production code, you don't want to calculate your mandelbrot
# set in CPython for performance reasons. This is just an example of what is
# possible with the image provider support in PyOtherSide.
def mandelbrot(x, y, steps):
    a = b = 0
    for i in range(steps):
        a, b = a**2 - b**2 + x, 2*a*b + y
        if a**2 + b**2 >= 4:
            break
    return i

def render_mandelbrot(image_id, requested_size):
    # when the url is: "image://python/xmin/xmax/ymin/ymax"
    # the image_id is: "xmin/xmax/ymin/ymax"

    parts = [float(x) for x in image_id.split('/')]
    x_range = parts[0:2]
    y_range = parts[2:4]

    # width and height will be -1 if not set in QML
    if requested_size == (-1, -1):
        requested_size = (200, 200)

    width, height = requested_size

    pixels = []
    for y in range(height):
        yy = y_range[0] + (y_range[1] - y_range[0]) * y / height
        for x in range(width):
            xx = x_range[0] + (x_range[1] - x_range[0]) * x / width
            pixels.extend(reversed(COLORS[mandelbrot(xx, yy, len(COLORS))]))
    return bytearray(pixels), (width, height), pyotherside.format_argb32

pyotherside.set_image_provider(render_mandelbrot)
