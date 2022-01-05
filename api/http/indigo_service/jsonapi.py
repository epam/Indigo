#
# Copyright (C) from 2009 to Present EPAM Systems.
#
# This file is part of Indigo toolkit.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

import base64
import functools
from enum import Enum
from typing import Dict, Generic, List, Optional, TypeVar, Union

from fastapi import HTTPException
from pydantic import BaseModel, conlist, validator
from pydantic.generics import GenericModel

# Generic Types

DataT = TypeVar("DataT")
ErrorsT = TypeVar("ErrorsT")
TypeT = TypeVar("TypeT")
AttributesT = TypeVar("AttributesT")
OutputFormatT = TypeVar("OutputFormatT")


# Formats


class CompoundFormat(str, Enum):
    AUTO = "auto"
    MOLFILE = "molfile"
    SMILES = "smiles"
    CML = "cml"
    SMARTS = "smarts"
    INCHI = "inchi"
    KET = "ket"


class PrimitiveFormat(BaseModel):
    __root__: str = "primitive"


# Base request and response models


class Data(GenericModel, Generic[TypeT, AttributesT]):
    type: TypeT
    attributes: AttributesT


class Errors(BaseModel):
    detail: str


class Request(GenericModel, Generic[TypeT, AttributesT]):
    data: Data[TypeT, AttributesT]

    class Config:  # pylint: disable=too-few-public-methods
        anystr_strip_whitespace = True
        extra = "forbid"
        allow_mutation = False


class Response(GenericModel, Generic[TypeT, AttributesT]):
    data: Optional[Data[TypeT, AttributesT]]

    class Config:  # pylint: disable=too-few-public-methods
        anystr_strip_whitespace = True
        extra = "forbid"
        allow_mutation = False


class ErrorResponse(BaseModel):
    errors: Optional[List[Errors]]


def make_error_response(err_: Exception) -> ErrorResponse:
    return ErrorResponse(**{"errors": [{"detail": str(err_)}]})


# Version Model


class VersionModelFormat(str, Enum):
    VERSION = "version"


class VersionModel(BaseModel):
    indigo: str


VersionResponse = Response[VersionModelFormat, VersionModel]


def make_version_response(value: str) -> VersionResponse:
    return VersionResponse(
        **{"data": {"type": "version", "attributes": {"indigo": value}}}
    )


# Compound Models


class CompoundModelType(BaseModel):
    __root__: str = "compound"


class CompoundArrayModelType(BaseModel):
    __root__: str = "compoundArray"


class CompoundPairModelType(BaseModel):
    __root__: str = "compoundPair"


class CompoundConvertModelType(BaseModel):
    __root__: str = "convert"


class CompoundModifiers(str, Enum):
    AROMATIZE = "aromatize"
    DEAROMATIZE = "dearomatize"
    CLEAN2D = "clean2d"
    LAYOUT = "layout"


class CompoundObject(BaseModel):
    structure: str
    format: CompoundFormat = CompoundFormat.AUTO


class CompoundObjectWithModifiers(CompoundObject):
    modifiers: Optional[List[CompoundModifiers]]


class CompoundModel(GenericModel, Generic[OutputFormatT]):
    compound: CompoundObjectWithModifiers
    outputFormat: OutputFormatT


class CompoundArrayModel(GenericModel, Generic[OutputFormatT]):
    array: List[CompoundObject]
    outputFormat: Optional[OutputFormatT]


class Compounds(GenericModel, Generic[OutputFormatT]):
    compounds: List[CompoundObject]
    outputFormat: Optional[OutputFormatT]


class CompoundPair(GenericModel, Generic[OutputFormatT]):
    # fmt: off
    compounds: conlist(CompoundObject, min_items=2, max_items=2)  # type: ignore # pylint: disable=line-too-long
    # fmt: on
    outputFormat: Optional[OutputFormatT]


CompoundRequest = Request[CompoundModelType, CompoundModel[CompoundFormat]]
CompoundConvertRequest = Request[
    CompoundConvertModelType, CompoundModel[CompoundFormat]
]
CompoundPairRequest = Request[
    CompoundPairModelType, CompoundPair[CompoundFormat]
]
CompoundPrimitiveRequest = Request[
    CompoundModelType, CompoundModel[PrimitiveFormat]
]
CompoundPairPrimitiveRequest = Request[
    CompoundPairModelType, CompoundPair[PrimitiveFormat]
]
CompoundResponse = Response[CompoundModelType, CompoundObject]
CompoundListResponse = Response[
    CompoundArrayModelType, CompoundArrayModel[CompoundFormat]
]


def make_compound_response(
    value: str, compound_format: CompoundFormat
) -> CompoundResponse:
    if compound_format == CompoundFormat.AUTO:
        raise RuntimeError(
            "Compound format should be specified, auto "
            "is not supported on response"
        )
    return CompoundResponse(
        **{
            "data": {
                "type": "compound",
                "attributes": {"structure": value, "format": compound_format},
            }
        }
    )


def make_list_compound_response(
    values: List[str], compound_format: CompoundFormat
) -> CompoundListResponse:
    return CompoundListResponse(
        **{
            "data": {
                "type": "compoundArray",
                "attributes": {
                    "array": [
                        {"structure": value, "format": compound_format}
                        for value in values
                    ]
                },
            }
        }
    )


# Match models

SourceToTargetT = TypeVar("SourceToTargetT")


class MapAtomModelType(BaseModel):
    __root__: str = "atomMapping"


class MapBondModelType(BaseModel):
    __root__: str = "bondMapping"


class MapModel(GenericModel, Generic[SourceToTargetT]):
    sourceToTarget: List[SourceToTargetT]


class MapAtomModel(BaseModel):
    sourceAtoms: List[int]
    targetAtoms: List[int]


class MapBondModel(BaseModel):
    sourceBonds: List[int]
    targetBonds: List[int]


class MatchOutputFormat(str, Enum):
    MAP_ATOM = "mapAtom"
    MAP_BOND = "mapBond"
    HIGHLIGHTED_TARGET_MOLFILE = "highlightedTargetMolfile"
    HIGHLIGHTED_TARGET_SMILES = "highlightedTargetSmiles"
    HIGHLIGHTED_TARGET_CML = "highlightedTargetCml"


class MatchModelType(BaseModel):
    __root__: str = "match"


class MatchModel(BaseModel):
    source: CompoundObject
    targets: List[CompoundObject]
    outputFormat: MatchOutputFormat
    flag: Optional[str] = "ALL"


MapAtomResponse = Response[MapAtomModelType, MapModel[MapAtomModel]]
MapBondResponse = Response[MapBondModelType, MapModel[MapBondModel]]
MatchRequest = Request[MatchModelType, MatchModel]
MatchResponse = Union[MapAtomResponse, MapBondResponse, CompoundListResponse]


def make_map_atom_response(
    source_to_target: List[MapAtomModel],
) -> MapAtomResponse:
    return MapAtomResponse(
        **{
            "data": {
                "type": "atomMapping",
                "attributes": {"sourceToTarget": source_to_target},
            }
        }
    )


def make_map_bond_response(
    source_to_target: List[MapBondModel],
) -> MapBondResponse:
    return MapBondResponse(
        **{
            "data": {
                "type": "bondMapping",
                "attributes": {"sourceToTarget": source_to_target},
            }
        }
    )


def make_match_response(
    raw_response: Union[List[MapAtomModel], List[MapBondModel], List[str]],
    output_format: MatchOutputFormat,
) -> MatchResponse:
    if output_format == MatchOutputFormat.MAP_ATOM:
        return make_map_atom_response(raw_response)  # type: ignore
    if output_format == MatchOutputFormat.MAP_BOND:
        return make_map_bond_response(raw_response)  # type: ignore

    response_fn = functools.partial(make_list_compound_response, raw_response)
    if output_format == MatchOutputFormat.HIGHLIGHTED_TARGET_MOLFILE:
        return response_fn(CompoundFormat.MOLFILE)
    if output_format == MatchOutputFormat.HIGHLIGHTED_TARGET_SMILES:
        return response_fn(CompoundFormat.SMILES)
    if output_format == MatchOutputFormat.HIGHLIGHTED_TARGET_CML:
        return response_fn(CompoundFormat.CML)
    raise RuntimeError(f"{output_format=} is not supported")


# Common bits


class CommonBitsModelType(BaseModel):
    __root__ = "commonBits"


class CommonBitsModel(BaseModel):
    source: CompoundObject
    targets: List[CompoundObject]


class CommonBitsCountModel(BaseModel):
    count: int


class CommonBitsCountArrayModelType(BaseModel):
    __root__ = "commonBitsCountArray"


class CommonBitsCountArrayModel(BaseModel):
    array: List[CommonBitsCountModel]


CommonBitsRequest = Request[CommonBitsModelType, CommonBitsModel]
CommonBitsResponse = Response[
    CommonBitsCountArrayModelType, CommonBitsCountArrayModel
]


def make_common_bits_response(common_bits: List[int]) -> CommonBitsResponse:
    return CommonBitsResponse(
        **{
            "data": {
                "type": "commonBitsCountArray",
                "attributes": {
                    "array": [{"count": count for count in common_bits}]
                },
            }
        }
    )


# Similarity models


class Fingerprint(str, Enum):
    SIM = "sim"
    SUB = "sub"
    SUB_RES = "sub-res"
    SUB_TAU = "sub-tau"
    FULL = "full"


class SimilarityMetric(str, Enum):
    TANIMOTO = "tanimoto"
    TVERSKY = "tversky"
    EUCLID_SUB = "euclid"


class SimilaritiesModelType(BaseModel):
    __root__ = "similarities"


class SimilaritiesValuesType(BaseModel):
    __root__ = "similarityValues"


class SimilaritiesValuesModel(BaseModel):
    values: List[float]


class SimilaritiesModel(BaseModel):
    source: CompoundObject
    targets: List[CompoundObject]
    fingerprint: Fingerprint
    metric: SimilarityMetric
    alpha: Optional[float] = 0.5
    beta: Optional[float] = 0.5

    @validator("alpha", "beta")
    def tversky_factor(
        cls, factor: Optional[float]
    ) -> Optional[
        float
    ]:  # pylint: disable=no-self-argument,no-self-use,line-too-long
        if factor is not None:
            if not 0 <= factor <= 1:
                raise ValueError("alpha and beta should be between 0 and 1")
        return factor


SimilaritiesRequest = Request[SimilaritiesModelType, SimilaritiesModel]

SimilaritiesResponse = Response[
    SimilaritiesValuesType, SimilaritiesValuesModel
]


def make_similarities_response(
    similarities: List[float],
) -> SimilaritiesResponse:
    return SimilaritiesResponse(
        **{
            "data": {
                "type": "similarityValues",
                "attributes": {"values": similarities},
            }
        }
    )


# Validation models


class Validations(str, Enum):
    STEREO_3D = "stereo3D"
    AMBIGUOUS_H = "ambiguousH"
    BAD_VALENCE = "badValence"
    CHIRALITY = "chirality"
    QUERY = "query"
    R_GROUPS = "rGroups"
    STEREO = "stereo"
    VALENCE = "valence"


class ValidationModelType(BaseModel):
    __root__: str = "validation"


class ValidationResultsModelType(BaseModel):
    __root__: str = "validationResults"


class ValidationModel(BaseModel):
    compound: CompoundObject
    validations: List[Validations]


class ValidationResultsModel(BaseModel):
    stereo3D: Optional[str]
    ambiguousH: Optional[str]
    badValence: Optional[str]
    chirality: Optional[str]
    query: Optional[str]
    rGroups: Optional[str]
    stereo: Optional[str]
    valence: Optional[str]


ValidationResponse = Response[
    ValidationResultsModelType, ValidationResultsModel
]
ValidationRequest = Request[ValidationModelType, ValidationModel]


def make_validation_response(
    validations: Dict[Validations, str]
) -> ValidationResponse:
    return ValidationResponse(
        **{"data": {"type": "validationResults", "attributes": validations}}
    )


# Descriptor models


class Descriptors(str, Enum):

    COUNT_ALLENE_CENTERS = "countAlleneCenters"
    COUNT_ATOMS = "countAtoms"
    COUNT_ATTACHMENT_POINTS = "countAttachmentPoints"
    COUNT_BONDS = "countBonds"
    COUNT_CATALYSTS = "countCatalysts"
    COUNT_COMPONENTS = "countComponents"
    COUNT_DATA_S_GROUPS = "countDataSGroups"
    COUNT_GENERIC_S_GROUPS = "countGeneridcSGrpoups"
    COUNT_HEAVY_ATOMS = "countHeavyAtoms"
    COUNT_HYDROGENS = "countHydrogens"
    COUNT_IMPLICIT_HYDROGENS = "countImplicitHydrogens"
    COUNT_MOLECULES = "countMolecules"
    COUNT_MULTIPLE_GROUPS = "countMultipleGroups"
    COUNT_PRODUCTS = "countProducts"
    COUNT_PSEUDO_ATOMS = "countPseudoatoms"
    COUNT_R_GROUPS = "countRGroups"
    COUNT_R_SITES = "countRSites"
    COUNT_REACTANTS = "countReactants"
    COUNT_REPEATING_UNITS = "countRepeatingUnits"
    COUNT_SSSR = "countSSSR"
    COUNT_STEREOCENTERS = "countStereocenters"
    COUNT_SUPERATOMS = "countSuperatoms"
    IS_CHIRAL = "isChiral"
    IS_HIGHLIGHTED = "isHighlighted"
    MOLECULAR_WEIGHT = "molecularWeight"
    MONOISOTOPIC_MASS = "monoisotopicMass"
    MOST_ABUNDANT_MASS = "mostAbundantMass"
    NAME = "name"
    GET_ACID_PKA_VALUE = "acidPkaValue"
    GET_BASIC_PKA_VALUE = "basicPkaValue"
    GROSS_FORMULA = "grossFormula"


class DescriptorModelType(BaseModel):
    __root__ = "descriptor"


class DescriptorModel(BaseModel):
    compound: CompoundObject
    descriptors: List[Descriptors]


class DescriptorResultModelType(BaseModel):
    __root__ = "descriptorResult"


class DescriptorResultModel(BaseModel):
    isPossibleFischerProjection: Optional[str]
    countAlleneCenters: Optional[str]
    countAtoms: Optional[str]
    countAttachmentPoints: Optional[str]
    countBonds: Optional[str]
    countCatalysts: Optional[str]
    countComponents: Optional[str]
    countDataSGroups: Optional[str]
    countGenericSGroups: Optional[str]
    countHeavyAtoms: Optional[str]
    countHydrogens: Optional[str]
    countImplicitHydrogens: Optional[str]
    countMolecules: Optional[str]
    countMultipleGroups: Optional[str]
    countProducts: Optional[str]
    countPseudoatoms: Optional[str]
    countRGroups: Optional[str]
    countRSites: Optional[str]
    countReactants: Optional[str]
    countRepeatingUnits: Optional[str]
    countSSSR: Optional[str]
    countStereocenters: Optional[str]
    countSuperatoms: Optional[str]
    isChiral: Optional[str]
    isHighlighted: Optional[str]
    molecularWeight: Optional[str]
    monoisotopicMass: Optional[str]
    mostAbundantMass: Optional[str]
    name: Optional[str]
    getAcidPkaValue: Optional[str]
    getBasicPkaValue: Optional[str]
    grossFormula: Optional[str]


DescriptorRequest = Request[DescriptorModelType, DescriptorModel]
DescriptorResponse = Response[DescriptorResultModelType, DescriptorResultModel]


def make_descriptor_response(
    descriptors: Dict[Descriptors, str]
) -> DescriptorResponse:
    return DescriptorResponse(
        **{"data": {"type": "descriptorResult", "attributes": descriptors}}
    )


SourceTargetsRequest = Union[
    CommonBitsRequest, SimilaritiesRequest, MatchRequest
]


# Render


rendering_formats = {
    "image/png": "png",
    "image/svg+xml": "svg",
    "application/pdf": "pdf",
}


class RenderModel(BaseModel):
    compound: CompoundObject
    outputFormat: str
    options: Optional[Dict[str, Union[int, float, bool, str]]] = None


class RenderModelType(BaseModel):
    __root__ = "render"


class RenderResultModelType(BaseModel):
    __root__ = "renderResult"


class RenderResultModel(BaseModel):
    image: str


RenderRequest = Request[RenderModelType, RenderModel]
RenderResponse = Response[RenderResultModelType, RenderResultModel]


def make_render_response(
    raw_image: bytes,
    output_format: str,
) -> RenderResponse:
    if output_format == "image/svg+xml":
        str_image = raw_image.decode("utf-8")
        decoded_image = base64.b64encode(str_image.encode("utf-8")).decode(
            "utf-8"
        )
        base64_image = f"data:{output_format};base64,{decoded_image}"
    elif output_format in ["image/png", "application/pdf"]:
        decoded_image = base64.b64encode(raw_image).decode("utf-8")
        base64_image = f"data:{output_format};base64,{decoded_image}"
    else:
        raise HTTPException(
            status_code=400, detail=f"Incorrect output format {output_format}"
        )
    return RenderResponse(
        **{
            "data": {
                "type": "renderResult",
                "attributes": {"image": base64_image},
            }
        }
    )
