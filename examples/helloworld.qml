import QtQuick 2.0
import io.thp.pyotherside 1.4

Rectangle {
    width: 200
    height: 200
    color: 'blue'

    ListView {
        id: listView
        anchors.fill: parent
        delegate: Text { color: 'white'; text: modelData }
    }

    Python {
        id: python

        Component.onCompleted: {
            // Print version of plugin and Python interpreter
            console.log('PyOtherSide version: ' + pluginVersion());
            console.log('Python version: ' + pythonVersion());

            // Asynchronous module importing
            importModule('os', function() {
                console.log('Python module "os" is now imported');

                // Asynchronous function calls
                call('os.listdir', [], function(result) {
                    console.log('dir listing: ' + result);
                    listView.model = result;
                });

                // Synchronous calls - avoid these, they block the UI
                call_sync('os.chdir', ['/']);
                console.log('files in /: ' + call_sync('os.listdir', ['.']));
            });

            // sychronous imports and calls - again, avoid!
            importModule_sync('pyotherside');
            call_sync('pyotherside.send', ['hello world!', 123]);

            // error handling
            importModule_sync('thismoduledoesnotexisthopefully');
            evaluate('[ 123 [.syntax234-error!');
        }

        onError: {
            // when an exception is raised, this error handler will be called
            console.log('python error: ' + traceback);
        }

        onReceived: {
            // asychronous messages from Python arrive here
            // in Python, this can be accomplished via pyotherside.send()
            console.log('got message from python: ' + data);
        }
    }
}
