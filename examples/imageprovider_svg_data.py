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

def load_data(image_id, requested_size):
    # If you return data in the format "pyotherside.format_svg_data" requested_size
    # is is used as the target size when rendering the SVG image

    # We use the image id to get name of the SVG file to render
    with open(image_id, 'rb') as f:
        svg_image_data = f.read()

    return bytearray(svg_image_data), requested_size, pyotherside.format_svg_data

pyotherside.set_image_provider(load_data)
