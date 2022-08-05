PyOtherSide: Python 3 QML Plugin for Qt 5 and Qt 6
==================================================

[![Build and test](https://github.com/thp/pyotherside/actions/workflows/build.yaml/badge.svg)](https://github.com/thp/pyotherside/actions/workflows/build.yaml)

A Qt plugin providing access to a Python 3 interpreter from QML
for creating asynchronous mobile and Desktop UIs with Python.


Requirements
------------

* Qt 5.1.0 or newer (Qt 6.x also supported)
* Python 3.3.0 or newer


Building
--------

To build and install the QML plugin:

```
qmake   # use "qmake6" for Qt 6
make
make install
```

To build against a specific Python version, use:

```
qmake PYTHON_CONFIG=python3.3-config   # use "qmake6" for Qt 6
make
make install
```

To manually update the qmltypes file on x64 Linux (TODO: make this automated):

```
qmake   # use "qmake6" for Qt 6
make
make INSTALL_ROOT=$(pwd)/tmp/
QML2_IMPORT_PATH=$(pwd)/tmp/usr/lib/x86_64-linux-gnu/qt5/qml \
    make -C src qmltypes
```

Unit Testing
------------

To run the included unit tests after building, use:

```
./tests/tests
```

Static Linking
--------------

If you want to link PyOtherSide statically against Python 3, you can include
the Python Standard Library in PyOtherSide as Qt Resource and have it extracted
automatically on load, for this, zip up the Standard Library and place the .zip
file as "pythonlib.zip" into src/ before running qmake.


More information
----------------

- Project page: https://thp.io/2011/pyotherside/
- Git repo: http://github.com/thp/pyotherside/
- Bug tracker: https://github.com/thp/pyotherside/issues
- Documentation: http://pyotherside.readthedocs.org/
