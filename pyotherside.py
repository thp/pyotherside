
import os
import json

def demo(path='.'):
    return json.dumps(['..'] + os.listdir(path))

