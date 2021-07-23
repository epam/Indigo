import os
import tempfile
import threading
from contextlib import contextmanager
from contextvars import ContextVar

from indigo import Indigo

__indigo: ContextVar[Indigo] = ContextVar("indigo", default=Indigo())
__cur_thread: ContextVar[int] = ContextVar("thread_id", default=threading.get_ident())


def indigo(thread_local=False) -> Indigo:
    if thread_local:
        ident = threading.get_ident()
        if __cur_thread.get() != ident:
            __indigo.set(Indigo())
            __cur_thread.set(ident)

    return __indigo.get()


@contextmanager
def indigo_new() -> Indigo:
    indigo_sess = Indigo()
    token = __indigo.set(indigo_sess)
    try:
        yield
    finally:
        __indigo.reset(token)


def create_temp_png_file():
    _, path = tempfile.mkstemp(suffix=".png")
    try:
        yield path
    finally:
        os.unlink(path)
