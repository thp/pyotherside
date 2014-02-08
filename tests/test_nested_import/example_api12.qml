import QtQuick 2.0
import io.thp.pyotherside 1.2

Python {
    Component.onCompleted: {
        addImportPath(Qt.resolvedUrl('.'));
        importModule('thp_io.pyotherside.nested', function () {
            console.log('"nested" imported successfully');
            console.log('repr of the module: ' + evaluate('repr(thp_io.pyotherside.nested)'));
            call('thp_io.pyotherside.nested.info', [], function (result) {
                console.log('from nested.info(): ' + result);
            });
            importModule('thp_io.pyotherside.nested.module', function () {
                console.log('"nested.module" imported successfully');
                call('thp_io.pyotherside.nested.module.info', [], function (result) {
                    console.log('from nested.module.info(): ' + result);
                    console.log('nested.module.value: ' + evaluate('thp_io.pyotherside.nested.module.value'));
                    Qt.quit();
                });
            });
        });
    }
}
