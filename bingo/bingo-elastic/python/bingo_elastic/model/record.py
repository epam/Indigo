from __future__ import annotations

from typing import Callable, Dict, FrozenSet, Iterable, List, Optional
from uuid import uuid4

from indigo import Indigo, IndigoException, IndigoObject  # type: ignore

MOL_TYPES = ["#02: <molecule>", "#03: <query reaction>", "#12: <RDFMolecule>"]
REAC_TYPES = ["#04: <reaction>", "#05: <query reaction>"]
RESERVED_FIELDS = frozenset(
    {
        "cmf",
        "name",
        "hash",
        "has_error",
        "rawData",
        "sim_fingerprint",
        "sim_fingerprint_len",
        "sub_fingerprint",
        "sub_fingerprint_len",
        "tau_fingerprint",
        "tau_fingerprint_len",
        "record_id",
        "error_handler",
        "skip_errors",
        "tau_search",
        "indigo_object",
        "elastic_response",
    }
)


# pylint: disable=unused-argument
def skip_errors(instance: IndigoRecord, err: BaseException) -> None:
    """
    Empty handler to skip errors
    """


def check_error(instance: IndigoRecord, error: BaseException) -> None:
    if instance.error_handler:
        instance.error_handler(instance, error)
    else:
        raise error


class WithElasticResponse:
    def __set__(self, instance: IndigoRecord, value: Dict):
        el_src = value["_source"]
        for arg, val in el_src.items():
            setattr(instance, arg, val)


class WithIndigoObject:
    def __set__(  # pylint: disable=too-many-statements, too-many-branches, too-many-locals
        self, instance: IndigoRecord, value: IndigoObject
    ) -> None:
        try:
            value_dup = value.clone()
        except IndigoException as err_:
            check_error(instance, err_)
            # Can't create IndigoObject
            return

        value_dup.aromatize()
        fingerprints = [("sim", "sim"), ("sub", "sub")]
        if instance.tau_search:
            fingerprints += [("sub-tau", "tau")]

        for fp_type, prefix in fingerprints:
            try:
                setattr(instance, f"{prefix}_fingerprint", [])
                setattr(instance, f"{prefix}_fingerprint_len", 0)
                fp_list = value_dup.fingerprint(fp_type).oneBitsList()
                if fp_list:
                    fp_ = [int(b) for b in fp_list.split(" ")]
                    setattr(instance, f"{prefix}_fingerprint", fp_)
                    setattr(instance, f"{prefix}_fingerprint_len", len(fp_))
            except (ValueError, IndigoException) as err_:
                check_error(instance, err_)

        try:
            cmf = " ".join(map(str, list(value_dup.serialize())))
            setattr(instance, "cmf", cmf)
        except IndigoException as err_:
            setattr(instance, "cmf", "")
            check_error(instance, err_)

        try:
            setattr(instance, "name", value_dup.name())
        except IndigoException as err_:
            setattr(instance, "name", "")
            check_error(instance, err_)

        try:
            internal_type = value_dup.dbgInternalType()
            if internal_type in MOL_TYPES:
                hash_ = [
                    component.clone().hash()
                    for component in value_dup.iterateComponents()
                ]
                setattr(instance, "hash", sorted(set(hash_)))
            elif internal_type in REAC_TYPES:
                setattr(instance, "hash", [value_dup.hash()])
        except IndigoException as err_:
            check_error(instance, err_)

        try:
            if value_dup.checkBadValence():
                setattr(instance, "has_error", 1)
            else:
                setattr(instance, "has_error", 0)
        except IndigoException as err_:
            check_error(instance, err_)

        allowed = getattr(instance, "_custom_properties", None)
        if allowed:
            try:
                for prop in value_dup.iterateProperties():
                    prop_name = prop.name()
                    if prop_name in RESERVED_FIELDS:
                        continue
                    if prop_name not in allowed:
                        continue
                    setattr(instance, prop_name, prop.rawData())
            except IndigoException as err_:
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
    tau_search: bool = False
    tau_fingerprint: Optional[List[int]] = None
    tau_fingerprint_len: Optional[int] = None
    sim_fingerprint: Optional[List[str]] = None
    sub_fingerprint: Optional[List[str]] = None
    indigo_object = WithIndigoObject()
    elastic_response = WithElasticResponse()
    record_id: Optional[str] = None
    error_handler: Optional[Callable[[object, BaseException], None]] = None
    _custom_properties: Optional[FrozenSet[str]] = None

    def __new__(cls, *args, **kwargs):
        if cls is IndigoRecord:
            raise TypeError(
                "Cannot instantiate IndigoRecord directly. "
                "Use IndigoRecordMolecule or IndigoRecordReaction instead."
            )
        return super().__new__(cls)

    def __init__(self, **kwargs) -> None:
        """
        Constructor accepts only keyword arguments
        :param indigo_object: — create indigo record from IndigoObject
        :type indigo_object: IndigoObject
        :param name: — add name. Rewrites name from IndigoObject
        :type name: str
        :param tau_search: include tautomers in the search
        :type tau_search: boolean
        :param tau_fingerprint: tautomer fingerprint (sub-tau)
        :type tau_fingerprint: List[int]
        :param sim_fingerprint: similarity fingerprint (sim)
        :type sim_fingerprint: List[int]
        :param sub_fingerprint: similarity fingerprint (sub)
        :type sub_fingerprint: List[int]
        :param error_handler: lambda for catching exceptions
        :type error_handler: Optional[Callable[[object, BaseException], None]]
        :param skip_errors: if True, all errors will be skipped,
                            no error_handler is required
        :type skip_errors: bool
        :param custom_properties: iterable of SDF tag names to extract from the
                            IndigoObject. If None or empty (default), no SDF
                            tags are extracted. Pass the keys of the
                            ElasticRepository's custom mapping here so the
                            indexed schema matches what the index mapping
                            declares.
        :type custom_properties: Optional[Iterable[str]]
        """

        # First check if skip_errors flag passed
        # If no flag passed add error_handler function from arguments
        if kwargs.get("skip_errors", False):
            self.error_handler = skip_errors  # type: ignore
        else:
            self.error_handler = kwargs.get("error_handler", None)

        self.record_id = uuid4().hex

        self.tau_search = kwargs.pop("tau_search", False)
        # Must be set before indigo_object assignment so the descriptor sees it
        custom_properties: Optional[Iterable[str]] = kwargs.pop(
            "custom_properties", None
        )
        self._custom_properties = (
            frozenset(custom_properties)
            if custom_properties is not None
            else None
        )
        for arg, val in kwargs.items():
            setattr(self, arg, val)

    def as_dict(self) -> Dict:
        # Add system fields here to exclude from indexing
        filtered_fields = {
            "error_handler",
            "skip_errors",
            "tau_search",
            "_custom_properties",
        }
        return {
            key: value
            for key, value in self.__dict__.items()
            if key not in filtered_fields
        }

    def as_indigo_object(self, session: Indigo):
        if self.cmf:
            return session.deserialize(list(map(int, self.cmf.split(" "))))  # type: ignore
        raise ValueError("Unexpected cmf value")


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
