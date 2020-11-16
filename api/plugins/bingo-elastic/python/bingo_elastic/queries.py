from abc import ABCMeta, abstractmethod
from typing import Dict, Any

from bingo_elastic.utils import head_by_path


class Compilable(metaclass=ABCMeta):

    def __init__(self, key: str, value: Any):
        self._key = key
        self._value = value

    @abstractmethod
    def compile(self, query: Dict) -> None:
        pass


class KeywordQuery(Compilable):

    def compile(self, query: Dict) -> None:
        bool_head = \
            head_by_path(query, ("query", "script_score", "query", "bool"))
        if not bool_head.get("must"):
            bool_head["must"] = []
        bool_head["must"].append({
            "term": {
                # TODO: think about genius
                f"{self._key}.keyword": {
                    "value": self._value,
                    "boost": 0
                }
            }
        })
        script_score_head = head_by_path(query, ("query", "script_score",))
        if not script_score_head.get("script"):
            script_score_head["script"] = {
                "source": "_score"
            }


def query_factory(key: str, value: Any) -> Compilable:
    if type(value) == str:
        return KeywordQuery(key, value)
