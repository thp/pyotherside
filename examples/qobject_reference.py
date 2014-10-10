import threading
import time
import pyotherside

# If you try to instantiate a QObject, it's unbound
unbound = pyotherside.QObject()
print(unbound)
try:
    unbound.a = 1
except Exception as e:
    print('Got exception:', e)

def do_something(bar):
    while True:
        print('got: ', bar.dynamicFunction(1, 2, 3))
        time.sleep(1)

def foo(bar, py):
    # Printing the objects will give some info on the
    # QObject class and memory address
    print('got:', bar, py)

    # Ok, this is pretty wicked - we can now call into
    # the PyOtherSide QML element from within Python
    # (not that it's a good idea to do this, mind you..)
    print(py.evaluate)
    print(py.evaluate('3*3'))

    try:
        bar.i_am_pretty_sure_this_attr_does_not_exist = 147
    except Exception as e:
        print('Got exception (as expected):', e)

    try:
        bar.x = 'i dont think i can set this to a string'
    except Exception as e:
        print('Got exception (as expected):', e)

    # This doesn't work yet, because we can't convert a bound
    # member function to a Qt/QML type yet (fallback to None)
    try:
        bar.dynamicFunction(bar.dynamicFunction, 2, 3)
    except Exception as e:
        print('Got exception (as expected):', e)

    # Property access works just like expected
    print(bar.x, bar.color, bar.scale)
    bar.x *= 3

    # Printing a member function gives a bound method
    print(bar.dynamicFunction)

    # Calling a member function is just as easy
    result = bar.dynamicFunction(1, 2, 3)
    print('result:', result)

    try:
        bar.dynamicFunction(1, 2, 3, unexpected=123)
    except Exception as e:
        print('Got exception (as expected):', e)

    threading.Thread(target=do_something, args=[bar]).start()

    # Returning QObject references from Python also works
    return bar
