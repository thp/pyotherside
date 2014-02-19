# This example demonstrates the use of pyotherside.send() to send events to Qt.

import pyotherside

import threading
import time

print('Using PyOtherSide version', pyotherside.version)

COLORS = ['red', 'green', 'blue']

def thread_func():
    i = 0
    while True:
        pyotherside.send('append', 'Next Number: ',  i)
        if i % 2 == 0:
            color = COLORS[int((i / 2) % len(COLORS))]
            pyotherside.send('color', color)
        i += 1
        time.sleep(1)

thread = threading.Thread(target=thread_func)
thread.start()
