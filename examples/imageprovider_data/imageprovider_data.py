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
import base64


# base64-encoded image data (PNG)
IMAGE_DATA = b"""
iVBORw0KGgoAAAANSUhEUgAAAGAAAABgCAMAAADVRocKAAABRFBMVEX////C0d5+pcZklb1JhbZD
g7ZDf7JQhbBkkbV9ob/X3uOdutFDhbs4gr44fLU3ea83dqtkjrI4f7o3c6VCc6VJiLtEicDN3+73
9/+CrdFwlrWbv9329fU8cp6Cr9Q2bZybvdo3apRakb/v7+//7WD/5ln44W5/qMr/617/41T51ls4
hcP/20z12oL7zkewx9n/1UTx5cNyoMVQfKH16Mb/4FL/0ED12Y1Sjb341Wj4zVd/nrjx7Nr/zDv7
yED71U/8xTmAo8DG1OD/2En/xTJDeafc4ef145j64mL7wDf15qb/vCn5wkSApsb3zXTz8vJCfa76
vkCQr8n/tSH22Jf5wEmhus9Jf6z/zVf/4Jf/1nv/9+X/03b9vi7/+/L/1oP33pL/z2r/7cj/5Kz/
uC330oT4x2L236D/tTH31Hvr6urm5uaTuO3WAAAAAWJLR0QAiAUdSAAAAAlwSFlzAAAASAAAAEgA
RslrPgAAAnBJREFUaN7tlltz0kAYQBcVRcsmbdDWKooppDZNRFRAUWhtoViKFjXWe73f/f/v7iVZ
toG8ZD9mdNzz2plzyJdvt0FIo9FoNBrNv0bm2PET2ZOEU7nTZ+D1c3mMsWEYpmnOExbA/RhblmXQ
AtEXCoWzwIFzeHHp/DJ7ANMsUC7ABvL4IkLFcEAscAk2gPFlhJYM4S+VStABvHwlKwZUAg+wF3zE
Dx0QGxr5gQOCXBSwgYSZFXrCcHxAtm2XKxXHcVYV/VcxTvDb5bU1x3XddSX/nOd59Ahb0gkQAYf6
ff+aSmCR+6UrQvjtssv8flXBf91LHBALEL1fq91IH7jpefIdKhaIcqveaFB9rXk7feAOThyQnWn5
3N+8mz6QTx7QvXbo73Q20gcsa/KK4PPJtDepn+g7nfvpA/Lv39ouj6m3ut0u9/d6PaUnMHhhp/9g
t8Kh68/XJ/QrBLJiQP1VoXca0fqE/kH6wErk39qb4m9y//Bh+sCj6AT0d8fjccX69Jh/uJ8+sB2e
gPmRE/NH4yH+YTt9AD0ON3SU4Cf64RMFP3rKP7N4YHJ9qD94phJAOX5FjKauD/MfKPkRev5ih4DI
P5fQX61WNxgDhsIbPoIYfwtIGMedWB+F4zU9EH+9wAE6/uj27L189Zq8XtiAL6/PG4TeBoewgU12
O4fXwzuE3gfAI/og3z4fP33+EnyFDaB16fYJCN+A/Qh9r0anl+h/qN0PCdR/HgwO6e22r/AlpPlr
KY4mmIU8/OoCjRSLsloGpiHrf42REwC/X+h/c0QCojDjJ5j9O2CJ4pQlJdA/qOulSgwwt0aj0Wg0
Gs1/wR/yOd+mI4x7LgAAACV0RVh0ZGF0ZTpjcmVhdGUAMjAxMy0xMC0wNFQwMzo1OTozNCswMDow
MLgfTfgAAAAldEVYdGRhdGU6bW9kaWZ5ADIwMTMtMTAtMDRUMDM6NTk6MzQrMDA6MDDJQvVEAAAA
AElFTkSuQmCC
"""

def load_data(image_id, requested_size):
    # If you return data in the format "pyotherside.format_data", the size is
    # ignored; the data is interpreted as image file data (e.g. PNG, jpeg, ..)
    return bytearray(base64.b64decode(IMAGE_DATA)), (-1, -1), pyotherside.format_data

pyotherside.set_image_provider(load_data)
