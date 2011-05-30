#ifndef _QPYTHON_H
#define _QPYTHON_H

#include <QtCore>

#include "Python.h"

class QPython : public QObject
{
    Q_OBJECT

    public:
        QPython() : locals(PyDict_New()), globals(PyDict_New())
        {
            if (instances == 0) {
                Py_Initialize();
            }
            instances++;

            if (PyDict_GetItemString(globals, "__builtins__") == NULL) {
                PyDict_SetItemString(globals, "__builtins__",
                        PyEval_GetBuiltins());
            }
        }

        ~QPython() {
            instances--;
            if (instances == 0) {
                Py_Finalize();
            }
        }

        Q_INVOKABLE
        bool importModule(QString name) {
            const char *moduleName = name.toUtf8().constData();

            /**
             * FIXME: Use PyImport_ImportModule() or something,
             * but right now this crashes, so we use a more lame
             * approach..
             **/

            /*PyObject *result =*/PyRun_String(
                    (QString("import ") + name).toUtf8().constData(),
                    Py_single_input, globals, locals);
            return true;

            /*PyObject *module;
            module = PyImport_ImportModule(moduleName);
            if (module != NULL) {
                PyDict_SetItemString(globals, moduleName, module);
                return true;
            }

            return false;*/
        }

        Q_INVOKABLE
        QString evaluate(QString expr) {
            PyObject *result;
            result = PyRun_String(expr.toUtf8().constData(),
                    Py_eval_input, globals, locals);

            /* FIXME: Error checking, hey! */

            /**
             * TODO: Maybe auto-serialize stuff into QVariant or JSON for use
             * in QML's JavaScript
             **/

            return QString(PyString_AsString(result));
        }

        static void registerQML();

    private:
        PyObject *locals;
        PyObject *globals;
        static int instances;
};

#endif

