import threading
from contextlib import contextmanager
from contextvars import ContextVar
from indigo import Indigo


__indigo: ContextVar[Indigo] = ContextVar("indigo", default=Indigo())
__cur_thread: ContextVar[int] = ContextVar("thread_id", default=threading.get_ident())


def indigo() -> Indigo:
    ident = threading.get_ident()

    if __cur_thread.get() != ident:
        __indigo.set(Indigo())
        __cur_thread.set(ident)
    return __indigo.get()


async def a_indigo() -> Indigo:
    return indigo()


@contextmanager
def indigo_ctx(**kwargs) -> Indigo:
    # TODO: this implementation doesn't support session copy
    # TODO: Implement copy/restore functionality
    indigo_sess = Indigo()
    for option in kwargs.get("setOption", []):
        setattr(indigo_sess, "setOption", option)
    token = __indigo.set(indigo_sess)
    try:
        yield
    finally:
        __indigo.reset(token)
