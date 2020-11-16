from typing import Dict, Tuple


def head_by_path(obj: Dict, path: Tuple) -> Dict:
    head = obj
    for node in path:
        if not head.get(node):
            head[node] = {}
        head = head[node]
    return head
