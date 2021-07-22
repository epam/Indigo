from typing import Any, Iterable, List, Union

from fastapi import status
from fastapi.responses import JSONResponse
from indigo import IndigoObject

from .model import (
    Error,
    IndigoResponse,
    IndigoType,
    ResponseAttributesModel,
    ResponseDataModel,
    SupportedTypes,
)


def get_ingigo_repr(indigo_object: IndigoObject, repr_type: SupportedTypes):
    if not isinstance(indigo_object, IndigoObject):
        raise TypeError(f"Passed value is not IndigoObject, got {indigo_object}")

    if repr_type in (SupportedTypes.MOLFILE, SupportedTypes.MOLFILE_LIST):
        return indigo_object.molfile()
    elif repr_type in (SupportedTypes.SMARTS, SupportedTypes.SMARTS_LIST):
        return indigo_object.smiles()
    elif repr_type in (SupportedTypes.SMILES, SupportedTypes.SMILES_LIST):
        return indigo_object.smarts()
    else:
        raise AttributeError(f"Passed repr type {repr_type} is not supported")


def _make_response(
    result_type: SupportedTypes,
    result_attributes: Union[ResponseAttributesModel, List[ResponseAttributesModel]],
):
    return IndigoResponse(
        data=ResponseDataModel(type=result_type, attributes=result_attributes)
    )


def _make_array_response(result_type: SupportedTypes, result: Iterable[IndigoObject]):
    if not hasattr(result, "__iter__"):
        raise ValueError("Passed result it not iterable")

    result_attributes = [
        ResponseAttributesModel(result=get_ingigo_repr(r, result_type)) for r in result
    ]

    return _make_response(result_type, result_attributes)


def make_response(result_type: IndigoType, result: Any):
    supported_type: SupportedTypes = result_type.convert_to_supp_type()

    if supported_type.value.lower().endswith("list"):
        return _make_array_response(supported_type, result)

    if isinstance(result, IndigoObject):
        result_attributes = ResponseAttributesModel(
            result=get_ingigo_repr(result, supported_type)
        )
    else:
        result_attributes = ResponseAttributesModel(result=result)

    return _make_response(supported_type, result_attributes)


def error_response(
    msg: str, status_code: int = status.HTTP_500_INTERNAL_SERVER_ERROR
) -> IndigoResponse:
    return JSONResponse(
        status_code=status_code,
        content=IndigoResponse(errors=[Error(detail=msg)]).dict(),
    )
