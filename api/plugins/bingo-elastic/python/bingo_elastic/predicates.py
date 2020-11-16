import math
from abc import ABCMeta, abstractmethod
from functools import lru_cache
from typing import Dict, List

from bingo_elastic.model.record import IndigoRecord


def clauses(fingerprint, fingerprint_name) -> List[Dict]:
    return [
        {
            "term": {
                fingerprint_name: {
                    "value": clause,
                }
            }
        }
        for clause in fingerprint
    ]


class BaseMatch(metaclass=ABCMeta):
    def __init__(self, target: IndigoRecord, threshold: float):
        self._target = target
        self._threshold = threshold

    @property
    @lru_cache(maxsize=None)
    def clauses(self) -> List[Dict]:
        return clauses(self._target.sim_fingerprint, "sim_fingerprint")

    def compile(self) -> Dict:
        return {
            "query": {
                "script_score": {
                    "query": {
                        "bool": {
                            "should": self.clauses,
                            "minimum_should_match": self.min_should_match(
                                len(self.clauses)
                            )
                        }
                    },
                    "script": self.script,
                }
            },
            "min_score": self._threshold,
        }

    @abstractmethod
    def min_should_match(self, length: int):
        pass

    @property
    @abstractmethod
    def script(self) -> Dict:
        pass


class TanimotoSimilarityMatch(BaseMatch):
    @property
    def target(self) -> IndigoRecord:
        return self._target

    @property
    def script(self) -> Dict:
        return {
            "source": "_score / (params.a + "
            "doc['sim_fingerprint_len'].value - _score)",
            "params": {"a": len(self._target.sim_fingerprint)},
        }

    def min_should_match(self, length: int) -> str:
        mm = (
            math.floor(
                (self._threshold * (len(self.target.sim_fingerprint) + 1))
                / (1.0 + self._threshold)
            )
            / length
        )

        return f"{int(mm*100)}%"


class EuclidSimilarityMatch(BaseMatch):
    @property
    def script(self) -> Dict:
        return {
            "source": "_score / params.a",
            "params": {"a": len(self._target.sim_fingerprint)},
        }

    def min_should_match(self, length: int):
        mm = (math.floor(self._threshold * len(self._target.sim_fingerprint))) / length

        return f"{int(mm*100)}%"


class TverskySimilarityMatch(BaseMatch):
    def __init__(
        self,
        target: IndigoRecord,
        threshold: float,
        alpha: float = 0.5,
        beta: float = 0.5,
    ):
        super().__init__(target, threshold)
        self._alpha = alpha
        self._beta = beta

    @property
    def script(self) -> Dict:
        return {
            "source": "_score / ((params.a - _score) * "
            "params.alpha + (doc['sim_fingerprint_len'].value - "
            "_score) * params.beta + _score)",
            "params": {
                "a": len(self._target.sim_fingerprint),
                "alpha": self._alpha,
                "beta": self._beta,
            },
        }

    def min_should_match(self, length: int) -> str:
        top = self._alpha * len(self._target.sim_fingerprint) + self._beta
        down = self._threshold + self._alpha + self._beta - 1.0
        mm = math.floor((top / down)) / length
        return f"{int(mm*100)}%"


class ExactMatch:
    def __init__(self, target):
        self._target = target

    @property
    @lru_cache(maxsize=None)
    def clauses(self) -> List[Dict]:
        return clauses(self._target.sub_fingerprint, "sub_fingerprint")

    def compile(self) -> Dict:
        return {
            "query": {
                "script_score": {
                    "query": {
                        "bool": {
                            "must": self.clauses
                        }
                    },
                    "script": {
                        "source": "_score / doc['sub_fingerprint_len'].value"
                    },
                }
            },
            "min_score": 1,
        }


# Alias to default similarity match
SimilarityMatch = TanimotoSimilarityMatch
