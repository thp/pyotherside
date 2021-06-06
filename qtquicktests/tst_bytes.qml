import QtQuick 2.0
import io.thp.pyotherside 1.5
import QtTest 1.2

TestCase {
    Python {
        id: py
        Component.onCompleted: {
            addImportPath(Qt.resolvedUrl('.'));
            importModule_sync('tst_bytes');
        }
    }

    function test_bytes_from_python() {
        var buf = py.evaluate('tst_bytes.get_bytes()');
        var view = new DataView(buf);

        var a = view.getInt8(0, true);
        var b = view.getInt16(1, true);
        var c = view.getInt32(3, true);

        console.log('Data retrieved from Python (as string):', buf);
        console.log('First element decoded:', a);
        console.log('Second element decoded:', b);
        console.log('Third element decoded:', c);

        compare(a, 123);
        compare(b, 456);
        compare(c, 789);
    }

    function test_bytes_to_python() {
        var buf = new ArrayBuffer(8);
        var view = new DataView(buf);

        view.setInt16(0, 1337, true);
        view.setInt16(2, -4711, true);
        view.setUInt32(4, 0xCAFEBABE, true);

        compare(py.call_sync('tst_bytes.set_bytes', [buf]), true);
    }
}
