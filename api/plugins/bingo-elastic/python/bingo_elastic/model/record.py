from typing import Callable, Dict, List, Optional
from uuid import uuid4

import indigo
from indigo import Indigo, IndigoObject


# pylint: disable=unused-argument
def skip_errors(instance: object, err: BaseException) -> None:
    """
    Empty handler to skip errors
    """


def check_error(instance: object, error: BaseException) -> None:
    if instance.error_handler:
        instance.error_handler(instance, error)
    else:
        raise error


class WithElasticResponse:
    def __set__(self, instance: object, value: Dict):
        el_src = value["_source"]
        for arg, val in el_src.items():
            setattr(instance, arg, val)


class WithIndigoObject:
    def __set__(self, instance: object, value: IndigoObject) -> None:
        fingerprints = (
            "sim",
            "sub",
        )
        for f_print in fingerprints:
            try:
                setattr(instance, f"{f_print}_fingerprint", [])
                setattr(instance, f"{f_print}_fingerprint_len", 0)
                fp_ = [
                    int(feature)
                    for feature in value.fingerprint(f_print)
                    .oneBitsList()
                    .split(" ")
                ]
                setattr(instance, f"{f_print}_fingerprint", fp_)
                setattr(instance, f"{f_print}_fingerprint_len", len(fp_))
            except ValueError as err_:
                check_error(instance, err_)
            except indigo.IndigoException as err_:
                check_error(instance, err_)

        setattr(instance, "name", value.name())
        try:
            setattr(
                instance, "cmf", " ".join(map(str, list(value.serialize())))
            )
        except indigo.IndigoException as err_:
            check_error(instance, err_)


class IndigoRecord:

    cmf: bytes = None
    name: str = None
    sim_fingerprint: List[str] = None
    sub_fingerprint: List[str] = None
    indigo_object = WithIndigoObject()
    elastic_response = WithElasticResponse()
    record_id: str = None
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
            self.error_handler = skip_errors
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
        return session.deserialize(list(map(int, self.cmf.split(" "))))
