from bingo_elastic.model.record import IndigoRecord
from abc import ABCMeta, abstractmethod
from typing import Dict


class BaseMatch(metaclass=ABCMeta):
    def __init__(self, target: IndigoRecord, threshold: float):
        self._target = target
        self._threshold = threshold

    def compile(self) -> Dict:
        return {
            "query": {
                "script_score": {
                    "query": {
                        "bool": {
                            "should": [
                                {
                                    "term": {
                                        "sim_fingerprint": {
                                            "value": clause,
                                            "boost": 1.0,
                                        }
                                    }
                                }
                                for clause in self._target.sim_fingerprint
                            ],
                            "adjust_pure_negative": True,
                            "minimum_should_match": self.min_should_match,
                            "boost": 1.0,
                        }
                    },
                    "script": self.script,
                }
            },
            "min_score": self._threshold,
        }

    @property
    def min_should_match(self) -> str:
        return "100%"

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
        r = {
            "source": "_score / (params.a + "
                      "doc['sim_fingerprint_len'].value - _score)",
            "lang": "painless",
            "params": {"a": len(self._target.sim_fingerprint)},
        }
        return r


class EuclidSimilarityMatch(BaseMatch):
    @property
    def script(self) -> Dict:
        return {
            "source": "_score / params.a",
            "lang": "painless",
            "params": {"a": len(self._target.sim_fingerprint)},
        }


class TverskySimilarityMatch(BaseMatch):
    def __init__(
        self, target: IndigoRecord, threshold: float, alpha: float, beta: float
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
            "lang": "painless",
            "params": {
                "a": len(self._target.sim_fingerprint),
                "alpha": self._alpha,
                "beta": self._beta,
            },
        }


# Alias to default similarity match
SimilarityMatch = TanimotoSimilarityMatch
