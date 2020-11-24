import logging
from typing import Dict, List

from indigo import Indigo, IndigoObject

logger = logging.getLogger("bingo_elastic")


class WithElasticResponse:
    def __set__(self, instance: object, value: Dict):
        el_src = value["_source"]
        setattr(instance, "name", el_src.get("name"))
        setattr(instance, "cmf", el_src.get("cmf"))


class WithIndigoObject:
    def __set__(self, instance: object, value: IndigoObject) -> None:
        fps = (
            "sim",
            "sub",
        )
        for fp in fps:
            try:
                fp_ = [
                    int(feature)
                    for feature in value.fingerprint(fp)
                    .oneBitsList()
                    .split(" ")
                ]
                setattr(instance, f"{fp}_fingerprint", fp_)
                setattr(instance, f"{fp}_fingerprint_len", len(fp_))
            except ValueError:
                raise ValueError(
                    "Building IndigoRecords from empty "
                    "IndigoObject is not supported"
                )
        setattr(instance, "name", value.name())
        setattr(instance, "cmf", " ".join(map(str, list(value.serialize()))))


class IndigoRecord:

    cmf = None
    name = None
    sim_fingerprint = None
    sub_fingerprint = None
    indigo_object = WithIndigoObject()
    elastic_response = WithElasticResponse()

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

        for k, v in kwargs.items():
            setattr(self, k, v)

    def as_dict(self) -> Dict:
        return self.__dict__

    def as_indigo_object(self, session: Indigo):
        return session.unserialize(list(map(int, self.cmf.split(" "))))
