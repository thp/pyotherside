import pyotherside

def called_when_exiting():
    print('Now exiting the application...')

pyotherside.atexit(called_when_exiting)
print('python loaded')

