# A very simple notetaking application that uses Python to load
# and save a string in a text file in the user's home directory.

import os
import threading
import time

import pyotherside

class Notes:
    def __init__(self):
        self.filename = os.path.expanduser('~/pyotherside_notes.txt')
        self.thread = None
        self.new_text = self.get_contents()

    def save_now(self):
        print('Saving file right away at exit')
        self._update_file(now=True)

    def _update_file(self, now=False):
        if not now:
            time.sleep(3)
        print('Saving file now')
        open(self.filename, 'w').write(self.new_text)
        self.thread = None

    def get_contents(self):
        if os.path.exists(self.filename):
            return open(self.filename).read()
        else:
            return '<new file>'

    def set_contents(self, text):
        self.new_text = text
        if self.thread is None:
            print('Scheduling saving of file')
            self.thread = threading.Thread(target=self._update_file)
            self.thread.start()

notes = Notes()
pyotherside.atexit(notes.save_now)
