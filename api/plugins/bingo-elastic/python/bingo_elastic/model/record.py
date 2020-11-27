from uuid import uuid4

import indigo
from typing import Dict, List, Tuple
from contextvars import ContextVar

from indigo import Indigo, IndigoObject


class RecordErrorStack:
    def __init__(self):
        self.__stack = []

    def put(self, record_id: str, error: Exception) -> None:
        self.__stack.append((record_id, error))

    def pop(self) -> Tuple[str, Exception]:
        return self.__stack.pop()

    def view(self):
        for record_id, error in self.__stack[::-1]:
            yield record_id, error

    def __len__(self):
        return len(self.__stack)


record_errors: ContextVar[RecordErrorStack] = ContextVar(
    "error_context", default=RecordErrorStack()
)


class WithElasticResponse:
    def __set__(self, instance: object, value: Dict):
        el_src = value["_source"]
        for key, value in el_src.items():
            setattr(instance, key, value)


class WithIndigoObject:
    def __set__(self, instance: object, value: IndigoObject) -> None:
        fps = (
            "sim",
            "sub",
        )
        for fp in fps:
            try:
                setattr(instance, f"{fp}_fingerprint", [])
                setattr(instance, f"{fp}_fingerprint_len", 0)
                fp_ = [
                    int(feature)
                    for feature in value.fingerprint(fp)
                    .oneBitsList()
                    .split(" ")
                ]
                setattr(instance, f"{fp}_fingerprint", fp_)
                setattr(instance, f"{fp}_fingerprint_len", len(fp_))
            except ValueError as err_:
                record_errors.get().put(instance.record_id, err_)
            except indigo.IndigoException as err_:
                record_errors.get().put(instance.record_id, err_)

        setattr(instance, "name", value.name())
        try:
            setattr(
                instance, "cmf", " ".join(map(str, list(value.serialize())))
            )
        except indigo.IndigoException:
            record_errors.get().put(instance.record_id, err_)


class IndigoRecord:

    cmf: bytes = None
    name: str = None
    sim_fingerprint: List[str] = None
    sub_fingerprint: List[str] = None
    indigo_object = WithIndigoObject()
    elastic_response = WithElasticResponse()
    record_id: str = None

    def __init__(self, **kwargs) -> None:
        """
        Constructor accepts only keyword arguments
        :param indigo_object: — create indigo record from IndigoObject
        :type indigo_object: IndigoObject
        :param name: — add name. Rewrites name from IndigoObject
        :type name: str
        :param sim_fingerprint: similarity fingerprint (sim)
        :type sim_fingerprint: List[int]
        :param sub_fingerprint: similarity fingerprint (sub)
        :type sub_fingerprint: List[int]
        """
        self.record_id = uuid4().hex
        for k, v in kwargs.items():
            setattr(self, k, v)

    def as_dict(self) -> Dict:
        return self.__dict__

    def as_indigo_object(self, session: Indigo):
        return session.deserialize(list(map(int, self.cmf.split(" "))))
