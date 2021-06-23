from enum import Enum
from typing import Any, Dict, List, Optional

from pydantic import BaseModel


class SupportedTypes(Enum):
    MOLFILE = "molfile"
    SMILES = "smiles"
    BOOL = "bool"
    INT = "int"
    REACTION = "reaction"
    QUERY_REACTION = "query_reaction"
    GROSSFORMULA = "grossformula"
    COMMON_BITS = "common_bits"
    FLOAT = "float"


class AttributesModel(BaseModel):
    content: str


class DataBaseModel(BaseModel):
    type: SupportedTypes
    # attributes: AttributesModel


class DataModel(DataBaseModel):
    attributes: AttributesModel


class ReactionProductEnumerateAttributes(BaseModel):
    reaction: str
    monomers_table: List[List[str]]


class StructurePropsAttributes(AttributesModel):
    props: Optional[str] = None


class MolPairAttributes(BaseModel):
    mol1: str
    mol2: str


class AmbiguousHAttributes(BaseModel):
    molecule: Optional[str] = None
    reaction: Optional[str] = None


class StructurePropsDataModel(DataBaseModel):
    attributes: StructurePropsAttributes


class ReactionProductEnumerateDataModel(DataBaseModel):
    attributes: ReactionProductEnumerateAttributes


class MolPairDataModel(DataBaseModel):
    attributes: MolPairAttributes


class AmbiguousHDataModel(DataBaseModel):
    attributes: AmbiguousHAttributes


class IndigoBaseRequest(BaseModel):
    """
    new request model
    """

    data: DataModel


class IndigoReactionProductEnumerateRequest(BaseModel):
    """
    request model for:
    - POST /reactionProductEnumerate
    """

    data: ReactionProductEnumerateDataModel


class IndigoStructurePropsRequest(BaseModel):
    """
    request model for:
    - POST /nameToStructure
    - POST /checkStructure
    """

    data: StructurePropsDataModel


class IndigoMolPairRequest(BaseModel):
    """
    request model for:
    - POST /commonBits
    - POST /exactMatch
    """

    data: MolPairDataModel


class IndigoAmbiguousHRequest(BaseModel):
    """
    request model for:
    - POST /checkAmbiguousH
    """

    data: AmbiguousHDataModel


class Error(BaseModel):
    status: Optional[int] = None
    detail: Optional[str] = None


class ResponseAttributesModel(BaseModel):
    result: Any


class ResponseDataModel(DataBaseModel):
    type: Optional[SupportedTypes] = None
    attributes: ResponseAttributesModel


class IndigoBaseResponse(BaseModel):
    meta: Optional[Dict] = None
    errors: Optional[List[Error]] = None


class IndigoResponse(IndigoBaseResponse):
    data: Optional[ResponseDataModel] = None
