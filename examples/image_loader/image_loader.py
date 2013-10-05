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

import os
import random

from PIL import Image
from PIL import ImageDraw

def render(image_id, requested_size):
    # width and height will be -1 if not set in QML
    if requested_size == (-1, -1):
        requested_size = (300, 300)

    img = Image.new('RGBA', requested_size)

    filename = os.path.join(os.path.dirname(__file__), image_id)
    logo = Image.open(filename)

    for x in range(100):
        img.paste(logo, (random.randint(0, requested_size[0]), random.randint(0, requested_size[1])))

    draw = ImageDraw.Draw(img)
    for x in range(100):
        draw.text((random.randint(0, requested_size[0]), random.randint(0, requested_size[1])),
                'PyOtherSide PIL Test',
                fill=(random.randint(10, 255), random.randint(10, 255), random.randint(10, 255)))
    del draw

    b, g, r, a = img.split()
    img = Image.merge("RGBA", (r, g, b, a))
    return bytearray(img.tostring()), img.size, pyotherside.format_argb32

pyotherside.set_image_provider(render)
