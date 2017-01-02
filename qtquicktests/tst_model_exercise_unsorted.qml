import QtTest 1.0

PythonListView {
    TestCase {
        name: "model_exercise_unsorted"
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

        function test_model_exercise_unsorted() {
            compare(get_model_list(), []);
            py.call_sync(py.getattr(python_side_model, 'append'), [1]);
            compare(get_model_list(), [1]);
            py.call_sync('test_functions.setitem', [python_side_model, 0, 2]);
            compare(get_model_list(), [2]);
            py.call_sync(py.getattr(python_side_model, 'insert'), [0, 3]);
            compare(get_model_list(), [3, 2]);
            py.call_sync(py.getattr(python_side_model, 'pop'));
            compare(get_model_list(), [3]);
        }
    }
}
