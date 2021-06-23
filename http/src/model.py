from enum import Enum
from typing import Dict, List, Optional, Union

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
    content: Union[str, Dict]


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


class MolPairAttributes(BaseModel):
    mol1: str
    mol2: str


class StructurePropsDataModel(DataBaseModel):
    attributes: StructurePropsAttributes


class ReactionProductEnumerateDataModel(DataBaseModel):
    attributes: ReactionProductEnumerateAttributes


class MolPairDataModel(DataBaseModel):
    attributes: MolPairAttributes


class IndigoRequest(BaseModel):
    """
    old request model
    """

    data: List[DataModel]


class IndigoBaseRequest(BaseModel):
    """
    new request model
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


class IndigoMolPairRequest(BaseModel):
    """
    request model for:
    - POST /commonBits
    - POST /exactMatch
    """

    data: MolPairDataModel


class Error(BaseModel):
    status: Optional[int] = None
    detail: Optional[str] = None


class IndigoResponse(BaseModel):
    data: Optional[Dict] = None
    meta: Optional[Dict] = None
    errors: Optional[List[Error]] = None
