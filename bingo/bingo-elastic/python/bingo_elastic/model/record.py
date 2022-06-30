from __future__ import annotations

import traceback
from typing import Callable, Dict, List, Optional
from uuid import uuid4

from indigo import Indigo, IndigoException, IndigoObject  # type: ignore
from indigo.inchi import IndigoInchi


# pylint: disable=unused-argument
def skip_errors(instance: IndigoRecord, err: BaseException) -> None:
    """
    Empty handler to skip errors
    """


def check_error(instance: IndigoRecord, error: BaseException) -> None:
    if instance.error_handler:
        instance.error_handler(instance, error)
    else:
        pass
        # raise error


class WithElasticResponse:
    def __set__(self, instance: IndigoRecord, value: Dict):
        el_src = value["_source"]
        for arg, val in el_src.items():
            setattr(instance, arg, val)


class WithIndigoObject:
    def __set__(self, instance: IndigoRecord, value: IndigoObject) -> None:
        value.aromatize()

        fingerprints = (
            "sim",
            "sub",
        )

        for f_print in fingerprints:
            try:
                setattr(instance, f"{f_print}_fingerprint", [])
                setattr(instance, f"{f_print}_fingerprint_len", 0)

                fp_list = value.fingerprint(f_print).oneBitsList()
                if fp_list:
                    fp_ = [
                        int(feature)
                        for feature in fp_list.split(" ")
                    ]
                    setattr(instance, f"{f_print}_fingerprint", fp_)
                    setattr(instance, f"{f_print}_fingerprint_len", len(fp_))
            except ValueError as err_:
                check_error(instance, err_)
            except IndigoException as err_:
                check_error(instance, err_)

        try:
            cmf = " ".join(map(str, list(value.serialize())))
            setattr(instance, "cmf", cmf)
        except IndigoException as err_:
            setattr(instance, "cmf", "")
            check_error(instance, err_)

        try:
            setattr(instance, "name", value.name())
        except IndigoException as err_:
            setattr(instance, "name", "")
            check_error(instance, err_)

        try:
            hash_ = [
                component.clone().hash()
                for component in value.iterateComponents()
            ]
            if hash_:
                setattr(instance, "hash", sorted(set(hash_)))
        except IndigoException as err_:
            setattr(instance, "hash", [])
            check_error(instance, err_)


class IndigoRecord:
    """
    Base class for IndigoObject representation.
    This class could not be instantiated directly, use one of the following
    subclasses:
        - IndigoRecordMolecule
        - IndigoRecordReaction
    """

    cmf: Optional[str] = None
    name: Optional[str] = None
    rawData: Optional[str] = None
    sim_fingerprint: Optional[List[str]] = None
    sub_fingerprint: Optional[List[str]] = None
    indigo_object = WithIndigoObject()
    indigo_inchi: IndigoInchi = None
    elastic_response = WithElasticResponse()
    record_id: Optional[str] = None
    error_handler: Optional[Callable[[object, BaseException], None]] = None

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
        :param error_handler: lambda for catching exceptions
        :type error_handler: Optional[Callable[[object, BaseException], None]]
        :param skip_errors: if True, all errors will be skipped,
                            no error_handler is required
        :type skip_errors: bool
        """

        # First check if skip_errors flag passed
        # If no flag passed add error_handler function from arguments
        if kwargs.get("skip_errors", False):
            self.error_handler = skip_errors  # type: ignore
        else:
            self.error_handler = kwargs.get("error_handler", None)

        self.record_id = uuid4().hex
        for arg, val in kwargs.items():
            setattr(self, arg, val)

    def as_dict(self) -> Dict:
        # Add system fields here to exclude from indexing
        filtered_fields = {"error_handler", "skip_errors"}
        return {
            key: value
            for key, value in self.__dict__.items()
            if key not in filtered_fields
        }

    def as_indigo_object(self, session: Indigo):
        assert self.cmf != ""
        return session.deserialize(list(map(int, self.cmf.split(" "))))


class IndigoRecordMolecule(IndigoRecord):
    pass


class IndigoRecordReaction(IndigoRecord):
    pass


def as_iob(indigo_record: IndigoRecord, session: Indigo) -> IndigoObject:
    """Function extracts IndigoObject from IndigoRecord
    Short alias to IndigoRecord.as_indigo_object
    :param indigo_record:
    :param session:
    :return:
    """
    return indigo_record.as_indigo_object(session)
