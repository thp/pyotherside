def get_set():
    return set((1, 2, 3))

def get_iterable_generator_expression():
    return (x * 2 for x in range(4))

def get_iterable_generator():
    for i in range(5):
        yield i * 3
