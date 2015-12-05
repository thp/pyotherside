import concurrent.futures
import functools
import itertools
import threading
import time

import pyotherside


def in_ui_thread(f):
    """Decorator used by Model to force a method into the UI thread.

    In Qt, model implementations must only change their contents from the UI
    thread so that views never see inconsistencies.

    pyotherside passes this responsibility to the Python implementation side of
    any PythonItemModel object by providing the call_back_for_modification
    bridge method that takes a callable and queues it to run from the UI
    thread.

    This decorator works with class methods only. It assumes that the class
    instance provides an attribute called _bridge that provides the
    call_back_for_modification method as supplied by pyotherside, uses it to
    switch to the UI thread to run the real function, blocks on the UI thread
    until it is complete, and then returns the real function's result. If the
    real function raises an exception then this is caught and re-raised in the
    caller's thread.
    """

    def wrapper(self, *args, **kwargs):
        # Use an concurrent.futures.Future to store the
        # return-value-or-exception. I'm not sure if this use is strictly this
        # is permitted according to the docs ("for testing only") but it
        # appears to be standalone and usable for this purpose. Otherwise I'd
        # just end up reimplementing the part of it that is needed here
        # identically.
        result_future = concurrent.futures.Future()
        callback_done = threading.Event()
        def callback():
            try:
                inner_result = f(self, *args, **kwargs)
            except Exception as exception:
                result_future.set_exception(exception)
            else:
                result_future.set_result(inner_result)
            callback_done.set()
        self._bridge.call_back_for_modification(callback)
        callback_done.wait()
        return result_future.result()  # this raises the exception if set
    return wrapper


class FlatModelIndexIndex:
    """Integer references to (row, column) tuples (an index of indexes).

    pyotherside's PythonItemModel requires the Python model implementation to
    provide integer references to items to meet the requirement's of Qt's
    QModelIndex in a QAbstractItemModel implementation. The alternative is to
    use pointers which are awkward to reference count because Qt's API does not
    allow provision of a destructor when it is done using them.

    The integer references are invalidated as soon as the model changes. We
    re-use them to avoid overflow, both after invalidation and if an integer
    reference for the same (row, column) is requested again.
    """

    def __init__(self):
        self.invalidate()

    def invalidate(self):
        self._map_forwards = {}
        self._map_backwards = {}
        self._key_counter = iter(itertools.count(0))

    def _new_index_key(self):
        return next(self._key_counter)

    def add(self, row, column):
        try:
            index_key = self._map_backwards[(row, column)]
        except KeyError:
            index_key = self._new_index_key()
            self._map_forwards[index_key] = row, column
            self._map_backwards[(row, column)] = index_key
        else:
            assert self._map_forwards[index_key] == (row, column)
        return index_key

    def __getitem__(self, index_key):
        return self._map_forwards[index_key]


class ListModelOtherSide:
    """Implementation of pyotherside PythonItemModel Python interface"""
    def __init__(self, container, index, role_names=None):
        self._index = index
        self._container = container
        self._role_names = role_names

    def columnCount(self, parent):
        return 1 if parent is None else 0

    def rowCount(self, parent):
        return len(self._container) if parent is None else 0

    def parent(self, index_id):
        return None

    def index(self, row, column, parent):
        if parent is not None:
            return None
        else:
            return row, column, self._index.add(row, column)

    def data(self, index_id, role):
        row, column = self._index[index_id]
        if self._role_names is None:
            return self._container[row].get('display')
        else:
            return self._container[row].get(self._role_names[role])

    def roleNames(self):
        return self._role_names


class ListModelPythonSide:
    """Implementation of Pythonic list-like container for ListModelOtherSide"""
    def __init__(self, bridge, index):
        self._bridge = bridge
        self._index = index
        self._data = []

    def __len__(self):
        return len(self._data)

    def __getitem__(self, i):
        return {'display': self._data[i]}

    @in_ui_thread
    def __setitem__(self, i, v):
        self._data[i] = v
        self._bridge.signal_dataChanged([(i, 0)], [(i, 0)])

    @in_ui_thread
    def append(self, v):
        row = len(self._data)
        self._bridge.beginInsertRows(None, row, row)
        self._index.invalidate()
        self._data.append(v)
        self._bridge.endInsertRows()

    @in_ui_thread
    def insert(self, i, v):
        self._bridge.beginInsertRows(None, i, i)
        self._index.invalidate()
        self._data.insert(i, v)
        self._bridge.endInsertRows()

    @in_ui_thread
    def pop(self, i=None):
        if i is None:
            i = len(self._data) - 1
        self._bridge.beginRemoveRows(None, i, i)
        self._index.invalidate()
        self._data.pop(i)
        self._bridge.endRemoveRows()


class SortedListModelPythonSide:
    """Implementation of Pythonic sorted container for ListModelOtherSide"""
    def __init__(self, bridge, index, key_func=lambda x:x, data_func=lambda x:{'display': x}):
        self._bridge = bridge
        self._index = index
        self._data = []
        self._key_func = key_func
        self._data_func = data_func

    def __len__(self):
        return len(self._data)

    def __getitem__(self, i):
        return self._data_func(self._data[i])

    def _find_insert_pos(self, v, begin=0, end=None):
        if end is None:
            end = len(self._data)

        if begin == end:
            return begin
        else:
            middle = (begin + end) // 2
            if self._key_func(v) < self._key_func(self._data[middle]):
                return self._find_insert_pos(v, begin, middle)
            else:
                return self._find_insert_pos(v, middle + 1, end)

    @in_ui_thread
    def __setitem__(self, i, v):
        self._data[i] = v
        self._bridge.signal_dataChanged([(i, 0)], [(i, 0)])

    @in_ui_thread
    def add(self, v):
        """Add v to list and return chosen index"""
        i = self._find_insert_pos(v)
        self._bridge.beginInsertRows(None, i, i)
        self._index.invalidate()
        self._data.insert(i, v)
        self._bridge.endInsertRows()
        return i

    @in_ui_thread
    def pop(self, i):
        """Remove i-th element from list"""
        self._bridge.beginRemoveRows(None, i, i)
        self._index.invalidate()
        self._data.pop(i)
        self._bridge.endRemoveRows()

    @in_ui_thread
    def remove(self, v):
        """Remove element from list by value"""
        self.pop(self._data.index(v))

    @in_ui_thread
    def remove_object(self, o):
        """Remove element from list by identity"""
        for i, _o in enumerate(self._data):
            if o is _o:
                self.pop(i)
                return
        raise ValueError("%r is not in list" % o)


def construct_list_model(bridge, python_side_constructor, other_side_constructor=ListModelOtherSide):
    index = FlatModelIndexIndex()
    list_model_python_side = python_side_constructor(bridge, index)
    list_model_other_side = other_side_constructor(list_model_python_side, index)
    return list_model_python_side, list_model_other_side


def ListModel(bridge):
    return construct_list_model(bridge, ListModelPythonSide)


def SortedListModel(bridge):
    return construct_list_model(bridge, SortedListModelPythonSide)


def get_model():
    def constructor(bridge):
        python_side, other_side = SortedListModel(bridge)
        start_twiddling(python_side, twiddle_sorted)
        return other_side

    return constructor


def twiddle_unsorted(model):
    time.sleep(1)
    model.append('foo')
    time.sleep(1)
    model[0] = 'bar'
    time.sleep(1)
    model.append('baz')
    time.sleep(1)
    model.insert(0, 'first')
    time.sleep(1)
    model.pop(1)


def twiddle_sorted(model):
    time.sleep(1)
    model.add('foo')
    time.sleep(1)
    model[0] = 'bar'
    time.sleep(1)
    qux = 'qux'
    model.add(qux)
    time.sleep(1)
    model.add('baz')
    time.sleep(1)
    model.add('first')
    time.sleep(1)
    model.remove_object(qux)
    time.sleep(1)
    model.remove('first')
    time.sleep(1)
    model.pop(1)


def start_twiddling(model, f):
    threading.Thread(target=functools.partial(f, model)).start()
