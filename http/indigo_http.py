import asyncio
from enum import Enum
from typing import Dict, List, Optional, Union, Tuple

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


class AttributesModel(BaseModel):
    content: str


class DataModel(BaseModel):
    type: SupportedTypes
    attributes: List[AttributesModel]


class IndigoRequest(BaseModel):
    data: DataModel


class IndigoResponse(BaseModel):
    data: Optional[Dict] = None
    meta: Optional[Dict] = None
    errors: Optional[List[str]] = None


def get_molecules(request: IndigoRequest) -> List[IndigoObject]:
    res = []
    if request.data.type == SupportedTypes.MOLFILE:
        if isinstance(request.data.attributes, list):
            item: AttributesModel
            for item in request.data.attributes:
                res.append(
                    indigo().loadMoleculeFromBuffer(bytes(item.content, "utf-8"))
                )
    return res


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
        mol1, mol2 = get_molecules(indigo_request)
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
async def exact_match(indigo_response: IndigoResponse, molecule1: str, molecule2: str):
    try:
        mol1 = indigo().loadMolecule(molecule1)
        mol2 = indigo().loadMolecule(molecule2)
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


@app.get(f"{base_url_indigo}/version")
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

    indigo_response = IndigoResponse()
    molecule1 = indigo_request.data.attributes[0].content
    try:
        mol1 = indigo().loadMolecule(molecule1)
        mol1.aromatize()
        indigo_response.data = {
            "type": "molfile",
            "attributes": {"content": mol1.molfile()},
        }

    except IndigoException as err_:
        indigo_response.errors = [
            str(err_),
        ]

    return indigo_response


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
            str(err_.__cause__),
        ]
    return indigo_response


@app.post(f"{base_url_indigo_object}/smarts")
async def smarts(indigo_request: IndigoRequest) -> IndigoResponse:
    indigo_response = IndigoResponse()
    molecule1 = indigo_request.data.attributes[0].content
    try:
        mol1 = indigo().loadMolecule(molecule1)
        indigo_response.data = {
            "type": "smarts",
            "attributes": {"content": mol1.smarts()},
        }
    except IndigoException as err_:
        indigo_response.errors = [
            str(err_.__cause__),
        ]
    return indigo_response
