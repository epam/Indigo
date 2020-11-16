from abc import ABCMeta, abstractmethod
from typing import Dict, Any


class Compilable(metaclass=ABCMeta):

    def __init__(self, key: str, value: Any):
        self._key = key
        self._value = value

    @abstractmethod
    def compile(self) -> Dict:
        pass


class KeywordQuery(Compilable):

    def compile(self) -> Dict:
        return {}


def query_factory(key: str, value: Any) -> Compilable:
    if type(value) == str:
        return KeywordQuery(key, value)
