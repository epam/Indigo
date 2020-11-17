from abc import ABCMeta, abstractmethod
from functools import lru_cache
from typing import Dict, List, Optional, Union

from indigo import Indigo

from bingo_elastic.model.record import IndigoRecord
from bingo_elastic.predicates import clauses
from bingo_elastic.utils import PostprocessType, head_by_path


def default_script_score(query: Dict) -> None:
    script_score_head = head_by_path(
        query,
        (
            "query",
            "script_score",
        ),
    )
    if not script_score_head.get("script"):
        script_score_head["script"] = {"source": "_score"}


class CompilableQuery(metaclass=ABCMeta):
    def __init__(self, *args, **kwargs):
        pass

    @abstractmethod
    def compile(
        self, query: Dict, postprocess_actions: PostprocessType = None
    ) -> None:
        pass


class KeywordQuery(CompilableQuery):
    def __init__(
        self, key: str, value: Union[str, IndigoRecord], *args, **kwargs
    ):
        self._key = key
        self._value = value
        super().__init__(*args, **kwargs)

    def compile(
        self, query: Dict, postprocess_actions: PostprocessType = None
    ) -> None:
        bool_head = head_by_path(
            query, ("query", "script_score", "query", "bool")
        )
        if not bool_head.get("must"):
            bool_head["must"] = []
        bool_head["must"].append(
            {
                "term": {
                    f"{self._key}.keyword": {"value": self._value, "boost": 0}
                }
            }
        )
        default_script_score(query)


class SubstructureQuery(CompilableQuery):
    def __init__(self, key: str, value: IndigoRecord, *args, **kwargs):
        if type(value) != IndigoRecord:
            raise AttributeError(
                "Argument for substructure search " "must be IndigoRecord"
            )
        self._key = key
        self._value = value
        super().__init__(*args, **kwargs)

    def postprocess(
        self, record: IndigoRecord, indigo: Indigo
    ) -> Optional[IndigoRecord]:
        if indigo.substructureMatcher(record.as_indigo_object(indigo)).match(
            indigo.loadQueryMolecule(
                self._value.as_indigo_object(indigo).canonicalSmiles()
            )
        ):
            return record

    @property
    @lru_cache(maxsize=None)
    def clauses(self) -> List[Dict]:
        return clauses(self._value.sub_fingerprint, "sub_fingerprint")

    def compile(
        self, query: Dict, postprocess_actions: PostprocessType = None
    ) -> None:
        # This code same as ExactMatch.
        # ExactMatch will use search by hash in next releases
        bool_head = head_by_path(
            query, ("query", "script_score", "query", "bool")
        )
        if not bool_head.get("must"):
            bool_head["must"] = []
        bool_head["must"] += self.clauses
        script_score_head = head_by_path(query, ("query", "script_score"))
        script_score_head["script"] = {
            "source": "_score / doc['sub_fingerprint_len'].value"
        }
        query["min_score"] = 1
        postprocess_actions.append(getattr(self, "postprocess"))


class RangeQuery(CompilableQuery):
    def __init__(
        self, field: str, lower: int, upper: int, *args, **kwargs
    ) -> None:
        self.field = field
        self.lower = lower
        self.upper = upper
        super().__init__(*args, **kwargs)

    def compile(
        self, query: Dict, postprocess_actions: PostprocessType = None
    ) -> None:
        bool_head = head_by_path(
            query, ("query", "script_score", "query", "bool")
        )
        if not bool_head.get("must"):
            bool_head["must"] = []
        bool_head["must"].append(
            {
                "range": {
                    self.field: {
                        "from": self.lower,
                        "to": self.upper,
                        "include_lower": True,
                        "include_upper": True,
                        "boost": 1,
                    }
                }
            }
        )
        default_script_score(query)


class WildcardQuery(CompilableQuery):

    def __init__(self, field: str, wildcard: str, *args, **kwargs) -> None:
        self.field = field
        self.wildcard = wildcard
        super().__init__(*args, **kwargs)

    def compile(
        self, query: Dict, postprocess_actions: PostprocessType = None
    ) -> None:
        bool_head = head_by_path(
            query, ("query", "script_score", "query", "bool")
        )
        if not bool_head.get("must"):
            bool_head["must"] = []
        bool_head["must"].append(
            {
                "wildcard": {
                    f"{self.field}":
                        {"wildcard": self.wildcard,
                         "boost": 1}
                }
            }
        )
        default_script_score(query)


def query_factory(*args, **kwargs) -> CompilableQuery:
    if len(args) < 1:
        raise AttributeError("query_factory accepts 1 or more arguments")
    key = args[0]
    value = args[1] if len(args) > 1 else None
    if key == "substructure":
        return SubstructureQuery(*args, **kwargs)
    elif type(value) == str:
        return KeywordQuery(*args, **kwargs)
    else:
        raise AttributeError(
            f"Unsoported request {args}, {kwargs}",
        )
