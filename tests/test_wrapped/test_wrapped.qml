import QtQuick 2.0
import io.thp.pyotherside 1.4

Rectangle {
    id: page
    width: 300
    height: 300

    Python {
        Component.onCompleted: {
            addImportPath(Qt.resolvedUrl('.'));

            importModule('test_wrapped', function () {
                console.log('"test_wrapped" imported successfully');

                var foo = call_sync('test_wrapped.get_foo', []);
                console.log('got foo: ' + foo);

                console.log('attribute bar of foo: ' + getattr(foo, 'bar'));

                var func = getattr(foo, 'methodman');

                call(func, ['the pain'], function (result) {
                    console.log('methodman() result: ' + result);
                });

                var mmr = call_sync(func, ['the pain']);
                console.log('methodman() sync result: ' + mmr);

                var result = call_sync('test_wrapped.set_foo', [foo]);
                console.log('got result: ' + result);
            });
        }

        onError: {
            console.log('Received error: ' + traceback);
        }
    }
}
