import QtQuick 2.9
import io.thp.pyotherside 1.5

Item {
id: obj

    signal callSignal()

    function callFunction() {
        print('Function Called')
        return 'hoho'
    }

    function signalConnection(){
    	print('Signal Called')
    }

    Component.onCompleted: {
        obj.callSignal.connect(signalConnection)
    }

    Python {
        Component.onCompleted: {
            addImportPath(Qt.resolvedUrl('.'));

            importModule('TestModule', function(){
                call('TestModule.makeCalls', [obj])
            })
        }
    }
}
