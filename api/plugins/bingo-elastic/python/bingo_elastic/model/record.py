import logging
from typing import Dict, List, Union

from indigo import IndigoObject

logger = logging.getLogger("bingo_elastic")


class WithElasticResponse:
    def __set__(self, instance: object, value: Dict):
        pass


class WithIndigoObject:
    def __set__(self, instance: object, value: IndigoObject) -> None:
        fps = (
            "sim",
            "sub",
        )
        for fp in fps:
            try:
                setattr(
                    instance,
                    f"{fp}_fingerprint",
                    [
                        int(feature)
                        for feature in value.fingerprint("sim")
                        .oneBitsList()
                        .split(" ")
                    ],
                )
            except ValueError:
                raise ValueError(
                    "Building IndigoRecords from empty "
                    "IndigoObject is not supported"
                )
        setattr(instance, "name", value.name())
        setattr(instance, "cmf", str(bytes(value.serialize())))


class IndigoRecord:

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
