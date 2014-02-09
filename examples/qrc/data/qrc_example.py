import pyotherside
import os.path
import sys

print('Hello from module!')
print(sys.path)
print('file exists?', pyotherside.qrc_is_file('qrc_example.qml'))
print('file exists?', pyotherside.qrc_is_file('qrc_example.qml.nonexistent'))
print('dir exists?', pyotherside.qrc_is_dir('/'))
print('dir exists?', pyotherside.qrc_is_dir('/nonexistent'))

print('='*30)
def walk(root):
    for entry in pyotherside.qrc_list_dir(root):
        name = os.path.join(root, entry)
        if pyotherside.qrc_is_dir(name):
            walk(name)
        else:
            print(name, '=', len(pyotherside.qrc_get_file_contents(name)), 'bytes')
walk('/')
print('='*30)
print(pyotherside.qrc_get_file_contents('qrc_example.py').decode('utf-8'))
print('='*30)

try:
    print('dir exists with number', pyotherside.qrc_is_dir(123))
except Exception as e:
    print('got exception (as expected):', e)

try:
    print('file exists with none', pyotherside.qrc_is_file(None))
except Exception as e:
    print('got exception (as expected):', e)

try:
    print('dir entries with invalid', pyotherside.qrc_list_dir('/nonexistent'))
except Exception as e:
    print('got exception (as expected):', e)

try:
    print('file contents with invalid', pyotherside.qrc_get_file_contents('/qrc_example.qml.nonexistent'))
except Exception as e:
    print('got exception (as expected):', e)
