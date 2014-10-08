# -*- coding: utf-8 -*-

class Foo(object):
    def __init__(self, name):
        print('new Foo(', name, ')')
        self.name = name
        self.bar = 4711

    def methodman(self, something):
        return 'I came to bring {}'.format(something)

    def __del__(self):
        print('__del__ called on', self.name)


def get_foo():
    print('get_foo()')
    return Foo('Hello World!')

def set_foo(foo):
    print('set_foo(', foo, ')')
    return foo.name

