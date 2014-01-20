import pyotherside
import threading
import time

def run():
    while True:
        pyotherside.send("test-errors")
        time.sleep(3)

thread = threading.Thread(target=run)
thread.start()
