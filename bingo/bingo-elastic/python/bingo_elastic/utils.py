from typing import Callable, Dict, List, Optional, Tuple

from bingo_elastic.model.record import IndigoRecord

from indigo import Indigo

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
