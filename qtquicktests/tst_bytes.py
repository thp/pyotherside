import struct

def get_bytes():
    return struct.pack('<bhi', 123, 456, 789)

def set_bytes(bytedata):
    print('Got this raw data from QML:', bytedata)
    a, b, c = struct.unpack('<hhI', bytedata)
    print('Got this data (unpacked) from QML:', a, b, hex(c))
    return a == 1337 and b == -4711 and c == 0xcafebabe
