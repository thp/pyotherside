import pyotherside
import threading
import time

class Wrapper(object):
    def __init__(self, capsule):
        object.__setattr__(self, '_capsule', capsule)

    def __getattr__(self, attr):
        return pyotherside.get_attribute(self._capsule, attr)

    def __setattr__(self, attr, value):
        return pyotherside.set_attribute(self._capsule, attr, value)

class MethodWrapper(object):
    def __init__(self, capsule):
        self._capsule = capsule

    def __call__(self, *args):
        return pyotherside.call_method(self._capsule, args)

def do_something(bar):
    while True:
        print('got: ', MethodWrapper(Wrapper(bar).dynamicFunction)(1, 2, 3))
        time.sleep(1)

def foo(bar):
    bar2 = Wrapper(bar)
    print(bar2.x, bar2.color, bar2.scale)
    bar2.x = bar2.x * 3

    print(bar2.dynamicFunction)
    method = MethodWrapper(bar2.dynamicFunction)
    result = method(1, 2, 3)
    print('result:', result)

    threading.Thread(target=do_something, args=[bar]).start()

    #print(bar)
    #print(pyotherside.get_attribute(bar, 'x'))
    #print('bar.x: ' + bar.x)
    #return bar.x * 2
    return bar

