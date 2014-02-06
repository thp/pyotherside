# This examples shows how to do cleanups and other things when
# the application exists by using pyside.atexit().

import pyotherside

def called_when_exiting():
    print('Now exiting the application...')

pyotherside.atexit(called_when_exiting)
print('python loaded')
