import asyncio
from enum import Enum
from typing import Dict, List, Optional, Union, Tuple, Generator

from fastapi import FastAPI, Response, Request
from indigo import IndigoException, IndigoObject
from pydantic import BaseModel
from indigo_tools import indigo, indigo_new

app = FastAPI()

base_url_indigo = "/indigo"
base_url_indigo_object = "/indigoObject"
resp_header_cont_type = "application/vnd.api+json"


class SupportedTypes(Enum):
    MOLFILE = "molfile"
    SMILES = "smiles"
    BOOL = "bool"


class AttributesModel(BaseModel):
    content: str


class DataModel(BaseModel):
    type: SupportedTypes = None
    attributes: List[AttributesModel] = None


class IndigoRequest(BaseModel):
    data: DataModel = None


class IndigoResponse(BaseModel):
    data: Optional[Dict] = None
    meta: Optional[Dict] = None
    errors: Optional[List[str]] = None


def get_molecules(request: IndigoRequest) -> Generator[IndigoObject, None, None]:
    """extract molecules from request"""
    if isinstance(request.data.attributes, list):
        item: AttributesModel
        type_ = request.data.type
        for molecule in request.data.attributes:
            if type_ == SupportedTypes.SMILES:
                yield indigo().loadMolecule(molecule.content)
            elif type_ == SupportedTypes.MOLFILE:
                yield indigo().loadMoleculeFromBuffer(bytes(molecule.content, "utf-8"))
            else:
                raise AttributeError(f"Unsupported type {type_}")
    else:
        raise AttributeError(f"Unsupported attributes {type(request.data.attributes)}")


def apply(molecule: IndigoObject, function: str) -> IndigoResponse:
    """apply function to molecule and form IndigoResponse"""
    indigo_response = IndigoResponse()
    try:
        getattr(molecule, function)()
        indigo_response.data = {
            "type": SupportedTypes.MOLFILE,
            "attributes": {"content": molecule.molfile()},
        }
    except IndigoException as err_:
        indigo_response.errors = [
            str(err_),
        ]
    return indigo_response


def apply_bool(molecule: IndigoObject, function: str) -> IndigoResponse:
    """apply boolean function to molecule and form bool IndigoResponse"""
    indigo_response = IndigoResponse()
    try:
        result: bool = getattr(molecule, function)()
        indigo_response.data = {
            "type": SupportedTypes.BOOL,
            "attributes": {"content": bool(result)},
        }
    except IndigoException as err_:
        indigo_response.errors = [str(err_)]
    return indigo_response


@app.middleware("http")
async def isolate_indigo_session(request: Request, call_next):
    with indigo_new():
        response = await call_next(request)
        if not request.scope["path"].startswith("/docs"):
            response.headers["Content-Type"] = resp_header_cont_type
        return response


@app.post(f"{base_url_indigo}/checkStructure")
async def check_structure(body: IndigoRequest):
    # todo: find documentation about this function
    pass


@app.post(f"{base_url_indigo}/commonBits", response_model=IndigoResponse)
async def common_bits(indigo_request: IndigoRequest) -> IndigoResponse:
    indigo_response = IndigoResponse()
    try:
        mol1, mol2 = list(get_molecules(indigo_request))
        indigo_response.data = {
            "type": "common_bits",
            "attributes": {
                "common_bits": indigo().commonBits(
                    mol1.fingerprint("sim"), mol2.fingerprint("sim")
                )
            },
        }
    except IndigoException as err_:
        indigo_response.errors = [
            str(err_),
        ]

    return indigo_response


async def decompose_molecules(scaffold: str, structures: str) -> str:
    # todo: find documentation about this function
    pass


@app.post(f"{base_url_indigo}/exactMatch", response_model=IndigoResponse)
async def exact_match(indigo_request: IndigoRequest) -> IndigoResponse:
    indigo_response = IndigoResponse()
    try:
        mol1, mol2 = list(get_molecules(indigo_request))
        match = True if indigo().exactMatch(mol1, mol2) else False
        indigo_response.data = {
            "type": "bool",
            "attributes": {"is_match": match},
        }

    except IndigoException as err_:
        indigo_response.errors = [
            str(err_),
        ]

    return indigo_response


@app.get(f"{base_url_indigo}/version", response_model=IndigoResponse)
async def indigo_version() -> IndigoResponse:
    indigo_response = IndigoResponse()
    mol1 = indigo().loadMolecule("CN1C=NC2=C1C(=O)N(C(=O)N2C)C")
    indigo_response.data = {
        "type": "version_string",
        "attributes": {"content": indigo().version()},
    }
    return indigo_response


@app.post(f"{base_url_indigo_object}/aromatize", response_model=IndigoResponse)
async def aromatize(indigo_request: IndigoRequest) -> IndigoResponse:
    molecule_string = indigo_request.data.attributes[0].content
    molecule = indigo().loadMolecule(molecule_string)
    return apply(molecule, "aromatize")


@app.post(f"{base_url_indigo_object}/smiles")
async def smiles(indigo_request: IndigoRequest) -> IndigoResponse:
    indigo_response = IndigoResponse()
    molecule1 = indigo_request.data.attributes[0].content
    try:
        mol1 = indigo().loadMolecule(molecule1)
        indigo_response.data = {
            "type": "smiles",
            "attributes": {"content": mol1.smiles()},
        }
    except IndigoException as err_:
        indigo_response.errors = [
            str(err_),
        ]
    return indigo_response


@app.post(f"{base_url_indigo_object}/smarts")
async def smarts(indigo_request: IndigoRequest) -> IndigoResponse:
    # TODO: query molecule only
    indigo_response = IndigoResponse()
    molecule1 = indigo_request.data.attributes.arg1.content
    try:
        mol1 = indigo().loadMolecule(molecule1)
        indigo_response.data = {
            "type": "smarts",
            "attributes": {"content": mol1.smarts()},
        }
    except IndigoException as err_:
        indigo_response.errors = [
            str(err_),
        ]
    return indigo_response


@app.post(f"{base_url_indigo_object}/standardize")
async def standardize(indigo_request: IndigoRequest) -> IndigoResponse:
    molecule, *_ = list(get_molecules(indigo_request))
    return apply(molecule, "standardize")


@app.post(f"{base_url_indigo_object}/unfoldHydrogens")
async def unfold_hydrogens(indigo_request: IndigoRequest) -> IndigoResponse:
    molecule, *_ = list(get_molecules(indigo_request))
    return apply(molecule, "unfoldHydrogens")


@app.post(f"{base_url_indigo_object}/validateChirality")
async def validate_chirality(indigo_request: IndigoRequest) -> IndigoResponse:
    indigo_response = IndigoResponse()
    molecule, *_ = list(get_molecules(indigo_request))
    molecule.validateChirality()
    return indigo_response


@app.post(f"{base_url_indigo_object}/check3DStereo")
async def check_3d_stereo(indigo_request: IndigoRequest) -> IndigoResponse:
    molecule, *_ = list(get_molecules(indigo_request))
    return apply_bool(molecule, "check3DStereo")


@app.post(f"{base_url_indigo_object}/checkAmbiguousH")
async def check_ambiguous_h(indigo_request: IndigoRequest) -> IndigoResponse:
    # TODO: Accepts a molecule or reaction (but not query molecule or query reaction).
    # Returns a string describing the first encountered mistake with ambiguous H counter.
    # Returns an empty string if the input molecule/reaction is fine.
    molecule, *_ = list(get_molecules(indigo_request))
    return apply_bool(molecule, "checkAmbiguousH")


@app.post(f"{base_url_indigo_object}/checkBadValence")
async def check_bad_valence(indigo_request: IndigoRequest) -> IndigoResponse:
    molecule, *_ = list(get_molecules(indigo_request))
    return apply_bool(molecule, "checkBadValence")


@app.post(f"{base_url_indigo_object}/checkChirality")
async def check_chirality(indigo_request: IndigoRequest) -> IndigoResponse:
    molecule, *_ = list(get_molecules(indigo_request))
    return apply_bool(molecule, "checkChirality")


@app.post(f"{base_url_indigo_object}/checkQuery")
async def check_query(indigo_request: IndigoRequest) -> IndigoResponse:
    molecule, *_ = list(get_molecules(indigo_request))
    return apply_bool(molecule, "checkQuery")


@app.post(f"{base_url_indigo_object}/checkRGroups")
async def check_rgroups(indigo_request: IndigoRequest) -> IndigoResponse:
    molecule, *_ = list(get_molecules(indigo_request))
    return apply_bool(molecule, "checkRGroups")


@app.post(f"{base_url_indigo_object}/checkStereo")
async def check_stereo(indigo_request: IndigoRequest) -> IndigoResponse:
    molecule, *_ = list(get_molecules(indigo_request))
    return apply_bool(molecule, "checkStereo")


@app.post(f"{base_url_indigo_object}/checkValence")
async def check_valence(indigo_request: IndigoRequest) -> IndigoResponse:
    # TODO: iterate all atoms
    molecule, *_ = list(get_molecules(indigo_request))
    return apply_bool(molecule, "checkValence")


@app.post(f"{base_url_indigo_object}/clean2d")
async def clean_2d(indigo_request: IndigoRequest) -> IndigoResponse:
    molecule, *_ = list(get_molecules(indigo_request))
    return apply(molecule, "clean2d")


@app.post(f"{base_url_indigo_object}/clear")
async def clear(indigo_request: IndigoRequest) -> IndigoResponse:
    molecule, *_ = list(get_molecules(indigo_request))
    return apply(molecule, "clear")
