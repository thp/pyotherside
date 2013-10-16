
/**
 * PyOtherSide: Asynchronous Python 3 Bindings for Qt 5
 * Copyright (c) 2011, 2013, Thomas Perl <m@thp.io>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 **/

#ifndef PYOTHERSIDE_QPYTHON_H
#define PYOTHERSIDE_QPYTHON_H

#include <QVariant>
#include <QObject>
#include <QString>
#include <QStringList>

#include <QMap>
#include <QThread>
#include <QJSValue>

class QPython;
class QPythonPriv;
class QPythonWorker;

class QPython : public QObject {
    Q_OBJECT

    public:
        /**
         * \brief Create a new Python instance
         *
         * A new Python instance can be created in QML as follows:
         *
         * \code
         * import io.thp.pyotherside 1.0
         *
         * Python {
         *     id: py
         * }
         * \endcode
         *
         * \arg parent The parent QObject
         **/
        QPython(QObject *parent=NULL);

        virtual ~QPython();

        /**
         * \brief Add a local filesystem path to Python's sys.path.
         *
         * The import path will be added synchronously. After the call
         * returns, the new import path will already be in effect.
         *
         * \arg path Directory that will be added to the search path
         **/
        Q_INVOKABLE void
        addImportPath(QString path);

        /**
         * \brief Add a handler for events sent with \c pyotherside.send()
         *
         * The provided \a callback will be called whenever the first argument
         * to \c pyotherside.send() matches the \a event argument.
         *
         * For example, when the Python code contains a call like this:
         *
         * \code
         * import pyotherside
         *
         * pyotherside.send('new-entries', 100, 123)
         * \endcode
         *
         * The event can be captured in QML like this:
         *
         * \code
         * Python {
         *     id: py
         *
         *     Component.onCompleted: {
         *         py.setHandler('new-entries', function(first, last) {
         *             console.log('New entries from ' + first + ' to ' + last);
         *         });
         *     }
         * }
         * \endcode
         *
         * All events for which no handler was set will be sent to the
         * received() signal.
         *
         * If a handler for that specific \a event already exist, the
         * new \a callback will replace the old one.
         *
         * \arg event The event name (first argument to pyotherside.send())
         * \arg callback The JS callback to be called when the event occurs
         **/
        Q_INVOKABLE void
        setHandler(QString event, QJSValue callback);

        /**
         * \brief Evaluate a Python expression synchronously
         *
         * Evaluate the string \a expr as Python expression and return the
         * result as Qt data type. The evaluation happens synchronously.
         *
         * \code
         * Python {
         *     Component.onCompleted: {
         *         console.log('Squares: ' + evaluate('[x for x in range(10)]'));
         *     }
         * }
         * \endcode
         *
         * \arg expr Python expression to be evaluated
         * \result The result of the expression as Qt data type
         **/
        Q_INVOKABLE QVariant
        evaluate(QString expr);

        /**
         * \brief Asynchronously import a Python module
         *
         * Imports a Python module by name asynchronously. The function
         * will return immediately. If the module is successfully imported,
         * the supplied \a callback will be called. Only then will the
         * imported module be available:
         *
         * \code
         * Python {
         *     Component.onCompleted: {
         *         importModule('os', function() {
         *             // You can use the "os" module here
         *         });
         *     }
         * }
         * \endcode
         *
         * If an error occurs while trying to import, the signal error()
         * will be emitted with detailed information about the error.
         *
         * \arg name The name of the Python module to import
         * \arg callback The JS callback to be called when the module is
         *               successfully imported
         **/
        Q_INVOKABLE void
        importModule(QString name, QJSValue callback);

        /**
         * \brief Synchronously import a Python module
         *
         * Imports a Python module by name synchronously. This function
         * will block until the module is imported and available. In
         * general, you should use importModule() instead of this function
         * to avoid blocking the QML UI thread. Example use:
         *
         * \code
         * Python {
         *     Component.onCompleted: {
         *         var success = importModule_sync('os');
         *         if (success) {
         *             // You can use the "os" module here
         *         }
         *     }
         * }
         * \endcode
         *
         * \arg name The name of the Python module to import
         * \result \c true if the import was successful, \c false otherwise
         **/
        Q_INVOKABLE bool
        importModule_sync(QString name);

        /**
         * \brief Asynchronously call a Python function
         *
         * Call a Python function asynchronously and call back into QML
         * when the result is available:
         *
         * \code
         * Python {
         *     Component.onCompleted: {
         *         importModule('os', function() {
         *             call('os.getcwd', [], function (result) {
         *                 console.log('Working directory: ' + result);
         *                 call('os.chdir', ['/'], function (result) {
         *                     console.log('Working directory changed.');
         *                 }););
         *             });
         *         });
         *     }
         * }
         * \endcode
         *
         * \arg func The Python function to call
         * \arg args A list of arguments, or \c [] for no arguments
         * \arg callback A callback that receives the function call result
         **/
        Q_INVOKABLE void
        call(QString func,
             QVariant args=QVariantList(),
             QJSValue callback=QJSValue());


        /**
         * \brief Synchronously call a Python function
         *
         * This is the synchronous variant of call(). In general, you should
         * use call() instead of this function to avoid blocking the QML UI
         * thread. Example usage:
         *
         * \code
         * Python {
         *     Component.onCompleted: {
         *         importModule_sync('os');
         *         var cwd = call_sync('os.getcwd', []);
         *         console.log('Working directory: ' + cwd);
         *         call_sync('os.chdir', ['/']);
         *         console.log('Working directory changed.');
         *     }
         * }
         * \endcode
         *
         * \arg func The Python function to call
         * \arg args A list of arguments, or \c [] for no arguments
         * \result The return value of the Python call as Qt data type
         **/
        Q_INVOKABLE QVariant
        call_sync(QString func, QVariant args=QVariantList());

    signals:
        /**
         * \brief Default event handler for \c pyotherside.send()
         *
         * This signal will be emitted for all events from Python for
         * which no specific handler (see setHandler()) is configured.
         *
         * \arg data The argument list of \c pyotherside.send()
         **/
        void received(QVariant data);

        /**
         * \brief Error handler for errors from Python
         *
         * This signal will be emitted when an error happens in the
         * Python interpreter that isn't caught. For example, errors
         * in evaluate(), importModule() and call() will be reported
         * with this signal.
         *
         * \arg traceback A string describing the error
         **/
        void error(QString traceback);

        /* For internal use only */
        void process(QString func, QVariant args, QJSValue callback);
        void import(QString name, QJSValue callback);

    private slots:
        void receive(QVariant data);

        void finished(QVariant result, QJSValue callback);
        void imported(bool result, QJSValue callback);

    private:
        static QPythonPriv *priv;

        QPythonWorker *worker;
        QThread thread;
        QMap<QString,QJSValue> handlers;
};

#endif /* PYOTHERSIDE_QPYTHON_H */
