import sys

DECODE_ENCODING = "utf-8"


class IndigoException(Exception):
    """Docstring for class IndigoException."""

    def __init__(self, value):
        if sys.version_info > (3, 0) and not isinstance(value, str):
            self.value = value.decode(DECODE_ENCODING)
        else:
            self.value = value

    def __str__(self):
        return self.value
