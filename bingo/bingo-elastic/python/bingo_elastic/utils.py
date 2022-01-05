from typing import Callable, Dict, List, Optional, Tuple

from indigo import Indigo  # type: ignore

from bingo_elastic.model.record import IndigoRecord

PostprocessType = List[
    Callable[[IndigoRecord, Indigo], Optional[IndigoRecord]]
]


def head_by_path(obj: Dict, path: Tuple) -> Dict:
    head = obj
    for node in path:
        if not head.get(node):
            head[node] = {}
        head = head[node]
    return head
