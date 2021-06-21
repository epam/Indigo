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
    FLOAT = "float"


class AttributesModel(BaseModel):
    content: str


class DataBaseModel(BaseModel):
    type: SupportedTypes
    # attributes: AttributesModel


class NewDataModel(DataBaseModel):
    attributes: AttributesModel


class DataModel(DataBaseModel):
    attributes: List[AttributesModel]


class ReactionProductEnumerateAttributes(BaseModel):
    reaction: str
    monomers_table: List[List[str]]


class StructurePropsAttributes(AttributesModel):
    props: Optional[str] = None


class StructurePropsDataModel(DataBaseModel):
    attributes: StructurePropsAttributes


class ReactionProductEnumerateDataModel(DataBaseModel):
    attributes: ReactionProductEnumerateAttributes


class IndigoRequest(BaseModel):
    """
    old data model
    """

    data: List[DataModel]


class IndigoBaseRequest(BaseModel):
    """
    new request base model
    """

    data: NewDataModel


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


class Error(BaseModel):
    status: Optional[int] = None
    detail: Optional[str] = None


class IndigoResponse(BaseModel):
    data: Optional[Dict] = None
    meta: Optional[Dict] = None
    errors: Optional[List[Error]] = None
