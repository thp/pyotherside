import QtTest 1.0

PythonListView {
    sorted: true;
    TestCase {
        name: "model_exercise_sorted"
        when: ready

        function get_model_item(row) {
            return view.model.data(view.model.index(row, 0), 258);
        }

        function get_model_list() {
            var result = new Array(view.count);
            for (var i=0; i<view.count; i++) {
                result[i] = get_model_item(i);
            }
            return result;
        }

        function test_model_exercise_sorted() {
            compare(get_model_list(), []);
            py.call_sync(py.getattr(python_side_model, 'add'), ['foo']);
            compare(get_model_list(), ['foo']);
            py.call_sync('test_functions.setitem', [python_side_model, 0, 'bar']);
            compare(get_model_list(), ['bar']);
            py.call_sync(py.getattr(python_side_model, 'add'), ['qux']);
            compare(get_model_list(), ['bar', 'qux']);
            py.call_sync(py.getattr(python_side_model, 'add'), ['baz']);
            compare(get_model_list(), ['bar', 'baz', 'qux']);
            py.call_sync(py.getattr(python_side_model, 'add'), ['first']);
            compare(get_model_list(), ['bar', 'baz', 'first', 'qux']);
            py.call_sync(py.getattr(python_side_model, 'remove'), ['first']);
            compare(get_model_list(), ['bar', 'baz', 'qux']);
        }
    }
}
