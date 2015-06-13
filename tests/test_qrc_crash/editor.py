#!/usr/bin/env python
# -*- coding: utf-8 -*-

import pyotherside

class Crasher:
    def __init__(self):
        pass

    def __del__(self):
        print('Finalizing', self)

def foo(arg):
    print('Got arg:', arg)
    return 'hello world'
