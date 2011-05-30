
import os
import json

# For a given filename and a (relative) base path, generate a dictionary
# containing the filename (key "name") without the path and a boolean value
# (key "directory") that is true if the filename is a directory
def mkdict(fn, path):
    return {'name': fn, 'directory': os.path.isdir(os.path.join(path, fn))}

# This gets called by the QML expression - it takes an optional parameter
# that when supplied tells for which directory we want to get the file list
def demo(path='.'):
    files = ['..'] + os.listdir(path)
    result = [mkdict(fn, path) for fn in files]
    return json.dumps(result)

