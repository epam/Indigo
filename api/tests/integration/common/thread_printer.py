import sys
from collections import defaultdict
from io import BytesIO
from threading import current_thread


class ThreadPrinter:
    def __init__(self):
        self.outputs = defaultdict(BytesIO)

    @property
    def _thread_id(self):
        return current_thread().ident

    def write(self, value):
        stream = self.outputs[self._thread_id]
        stream.write(value.encode("utf-8"))

    def read_and_clean(self):
        result = b""
        if self._thread_id in self.outputs:
            stream = self.outputs.pop(self._thread_id)
            stream.seek(0)
            result = stream.read()
        return result


class ThreadPrinterManager:
    def __init__(self, stdout, stderr):
        self._stdout = stdout
        self._stderr = stderr

    def __enter__(self):
        self.old_stdout, self.old_stderr = sys.stdout, sys.stderr
        sys.stdout, sys.stderr = self._stdout, self._stderr

    def __exit__(self, exc_type, exc_value, traceback):
        sys.stdout, sys.stderr = self.old_stdout, self.old_stderr
