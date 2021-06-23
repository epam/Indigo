from os import error
from typing import Callable, Generator, Tuple, Union

from fastapi import FastAPI, Request, Response
from indigo import IndigoException, IndigoObject

from .indigo_tools import indigo, indigo_new
from .model import (
    Error,
    IndigoAmbiguousHRequest,
    IndigoBaseRequest,
    IndigoMolPairRequest,
    IndigoReactionProductEnumerateRequest,
    IndigoResponse,
    IndigoStructurePropsRequest,
    SupportedTypes,
)

app = FastAPI()


BASE_URL_INDIGO = "/indigo"
BASE_URL_INDIGO_OBJECT = "/indigoObject"
RESP_HEADER_CONTENT_TYPE = "application/vnd.api+json"


# def get_indigo_object(data: DataModel):
#     if isinstance(data.attributes, list):
#         data_type = data.type
#         for attribute in data.attributes:
#             if data_type == SupportedTypes.SMILES:
#                 yield indigo().loadMolecule(attribute.content)
#             elif data_type == SupportedTypes.MOLFILE:
#                 yield indigo().loadMoleculeFromBuffer(bytes(attribute.content, "utf-8"))
#             elif data_type == SupportedTypes.REACTION:
#                 yield indigo().loadReaction(attribute.content)
#             elif data_type == SupportedTypes.QUERY_REACTION:
#                 yield indigo().loadQueryReaction(attribute.content)
#             else:
#                 raise AttributeError(f"Unsupported type {data_type}")
#     else:
#         raise AttributeError(f"Unsupported attributes type: {type(data.attributes)}")


# def parse_indigo_request(
#     request: IndigoRequest, separate: bool = False
# ) -> Generator[Union[IndigoObject, Tuple[IndigoObject]], None, None]:
#     """
#     extract objects from request,
#     is `separate` is set, yields data elements by chunks corresponding
#     to list elements in request, otherwise yields data elements one by one
#     """
#     for data_item in request.data:
#         # TODO: maybe add itertools.groupby and parse request in buckets by type
#         if separate:
#             yield tuple(get_indigo_object(data_item))
#         else:
#             yield from get_indigo_object(data_item)


def error_response(msg: str) -> IndigoResponse:
    return IndigoResponse(errors=[Error(detail=msg)])


def apply(molecule: IndigoObject, function: str) -> IndigoResponse:
    """apply function to molecule and form IndigoResponse"""
    # indigo_response = IndigoResponse()
    try:
        getattr(molecule, function)()
    except IndigoException as e:
        return error_response(str(e))

    return IndigoResponse(
        data={
            "type": SupportedTypes.MOLFILE,
            "attributes": {"content": molecule.molfile()},
        }
    )


def apply_to_result(
    molecule: IndigoObject, method_name: str, res_type: SupportedTypes, func: Callable
) -> IndigoResponse:
    try:
        result = func(getattr(molecule, method_name)())
    except IndigoException as e:
        return error_response(str(e))

    return IndigoResponse(
        data={
            "type": res_type,
            "attributes": {"content": result},
        }
    )


def apply_bool(molecule: IndigoObject, function: str) -> IndigoResponse:
    """apply boolean function to molecule and form bool IndigoResponse"""
    # indigo_response = IndigoResponse()
    # try:
    #     result: bool = getattr(molecule, function)()
    #     indigo_response.data = {
    #         "type": SupportedTypes.BOOL,
    #         "attributes": {"content": bool(result)},
    #     }
    # except IndigoException as e:
    #     indigo_response.errors = [str(e)]
    # return indigo_response
    return apply_to_result(molecule, function, SupportedTypes.BOOL, bool)


def apply_int(molecule: IndigoObject, function: str) -> IndigoResponse:
    # indigo_response = IndigoResponse()
    # try:
    #     result: int = getattr(molecule, function)()
    #     indigo_response.data = {
    #         "type": SupportedTypes.INT,
    #         "attributes": {"content": int(result)},
    #     }
    # except IndigoException as e:
    #     indigo_response.errors = [str(e)]
    # return indigo_response
    return apply_to_result(molecule, function, SupportedTypes.INT, int)


def apply_float(molecule: IndigoObject, function: str) -> IndigoResponse:
    # indigo_response = IndigoResponse()
    # try:
    #     result: float = getattr(molecule, function)()
    #     indigo_response.data = {
    #         "type": SupportedTypes.FLOAT,
    #         "attributes": {"content": float(result)},
    #     }
    # except IndigoException as e:
    #     indigo_response.errors = [str(e)]
    # return indigo_response
    return apply_to_result(molecule, function, SupportedTypes.FLOAT, float)


@app.middleware("http")
async def isolate_indigo_session(request: Request, call_next):
    with indigo_new():
        response = await call_next(request)
        if not request.scope["path"].startswith("/docs"):
            response.headers["Content-Type"] = RESP_HEADER_CONTENT_TYPE
        return response


@app.post(f"{BASE_URL_INDIGO}/checkStructure")
async def check_structure(indigo_request: IndigoStructurePropsRequest):
    structure = indigo_request.data.attributes.content
    props = indigo_request.data.attributes.props

    result: str = indigo().checkStructure(structure, props)

    return IndigoResponse(
        data={"type": "check_structure_result", "attributes": {"result": result}}
    )


@app.post(f"{BASE_URL_INDIGO}/commonBits")
async def common_bits(indigo_request: IndigoMolPairRequest) -> IndigoResponse:
    try:
        mol1 = indigo().loadMolecule(indigo_request.data.attributes.mol1)
        mol2 = indigo().loadMolecule(indigo_request.data.attributes.mol2)
        bits = indigo().commonBits(mol1.fingerprint("sim"), mol2.fingerprint("sim"))
    except IndigoException as e:
        return error_response(str(e))

    return IndigoResponse(
        data={
            "type": "common_bits",
            "attributes": {"common_bits": bits},
        }
    )


async def decompose_molecules(scaffold: str, structures: str) -> str:
    # todo: find documentation about this function
    pass


@app.post(f"{BASE_URL_INDIGO}/exactMatch", response_model=IndigoResponse)
async def exact_match(indigo_request: IndigoMolPairRequest) -> IndigoResponse:
    # indigo_response = IndigoResponse()
    try:
        mol1 = indigo().loadMolecule(indigo_request.data.attributes.mol1)
        mol2 = indigo().loadMolecule(indigo_request.data.attributes.mol2)
        match = bool(indigo().exactMatch(mol1, mol2))
    except IndigoException as e:
        return error_response(str(e))

    return IndigoResponse(
        data={
            "type": "bool",
            "attributes": {"is_match": match},
        }
    )


@app.get(f"{BASE_URL_INDIGO}/version", response_model=IndigoResponse)
async def indigo_version() -> IndigoResponse:
    indigo_response = IndigoResponse()
    # mol1 = indigo().loadMolecule("CN1C=NC2=C1C(=O)N(C(=O)N2C)C")
    indigo_response.data = {
        "type": "version_string",
        "attributes": {"content": indigo().version()},
    }
    return indigo_response


@app.post(f"{BASE_URL_INDIGO_OBJECT}/aromatize", response_model=IndigoResponse)
async def aromatize(indigo_request: IndigoBaseRequest) -> IndigoResponse:
    molecule_string = indigo_request.data.attributes.content
    molecule = indigo().loadMolecule(molecule_string)
    return apply(molecule, "aromatize")


@app.post(f"{BASE_URL_INDIGO_OBJECT}/smiles")
async def smiles(indigo_request: IndigoBaseRequest) -> IndigoResponse:
    # indigo_response = IndigoResponse()
    molecule_string = indigo_request.data.attributes.content
    try:
        mol = indigo().loadMolecule(molecule_string)
    except IndigoException as e:
        return error_response(str(e))

    return IndigoResponse(
        data={
            "type": "smiles",
            "attributes": {"content": mol.smiles()},
        }
    )


@app.post(f"{BASE_URL_INDIGO_OBJECT}/smarts")
async def smarts(indigo_request: IndigoBaseRequest) -> IndigoResponse:
    molecule_string = indigo_request.data.attributes.content
    try:
        mol = indigo().loadMolecule(molecule_string)
    except IndigoException as e:
        return error_response(str(e))

    return IndigoResponse(
        data={
            "type": "smarts",
            "attributes": {"content": mol.smarts()},
        }
    )


@app.post(f"{BASE_URL_INDIGO_OBJECT}/standardize")
async def standardize(indigo_request: IndigoBaseRequest) -> IndigoResponse:
    molecule = indigo().loadMolecule(indigo_request.data.attributes.content)
    return apply(molecule, "standardize")


@app.post(f"{BASE_URL_INDIGO_OBJECT}/unfoldHydrogens")
async def unfold_hydrogens(indigo_request: IndigoBaseRequest) -> IndigoResponse:
    molecule = indigo().loadMolecule(indigo_request.data.attributes.content)
    return apply(molecule, "unfoldHydrogens")


@app.post(f"{BASE_URL_INDIGO_OBJECT}/validateChirality")
async def validate_chirality(indigo_request: IndigoBaseRequest) -> IndigoResponse:
    molecule = indigo().loadMolecule(indigo_request.data.attributes.content)
    molecule.validateChirality()  # TODO: empty response?
    return IndigoResponse()


@app.post(f"{BASE_URL_INDIGO_OBJECT}/check3DStereo")
async def check_3d_stereo(indigo_request: IndigoBaseRequest) -> IndigoResponse:
    molecule = indigo().loadMolecule(indigo_request.data.attributes.content)
    return apply_bool(molecule, "check3DStereo")


@app.post(f"{BASE_URL_INDIGO_OBJECT}/checkAmbiguousH")
async def check_ambiguous_h(indigo_request: IndigoAmbiguousHRequest) -> IndigoResponse:
    # TODO: Accepts a molecule or reaction (but not query molecule or query reaction).
    # Returns a string describing the first encountered mistake with ambiguous H counter.
    # Returns an empty string if the input molecule/reaction is fine.
    try:
        if indigo_request.data.attributes.molecule is not None:
            indigo_object = indigo().loadMolecule(
                indigo_request.data.attributes.molecule
            )
        else:
            indigo_object = indigo().loadReaction(
                indigo_request.data.attributes.reaction
            )
    except (IndigoException, ValueError) as e:
        return error_response(str(e))

    return apply_bool(indigo_object, "checkAmbiguousH")


@app.post(f"{BASE_URL_INDIGO_OBJECT}/checkBadValence")
async def check_bad_valence(indigo_request: IndigoBaseRequest) -> IndigoResponse:
    molecule = indigo().loadMolecule(indigo_request.data.attributes.content)
    return apply_bool(molecule, "checkBadValence")


@app.post(f"{BASE_URL_INDIGO_OBJECT}/checkChirality")
async def check_chirality(indigo_request: IndigoBaseRequest) -> IndigoResponse:
    molecule = indigo().loadMolecule(indigo_request.data.attributes.content)
    return apply_bool(molecule, "checkChirality")


@app.post(f"{BASE_URL_INDIGO_OBJECT}/checkQuery")
async def check_query(indigo_request: IndigoBaseRequest) -> IndigoResponse:
    molecule = indigo().loadMolecule(indigo_request.data.attributes.content)
    return apply_bool(molecule, "checkQuery")


@app.post(f"{BASE_URL_INDIGO_OBJECT}/checkRGroups")
async def check_rgroups(indigo_request: IndigoBaseRequest) -> IndigoResponse:
    molecule = indigo().loadMolecule(indigo_request.data.attributes.content)
    return apply_bool(molecule, "checkRGroups")


@app.post(f"{BASE_URL_INDIGO_OBJECT}/checkStereo")
async def check_stereo(indigo_request: IndigoBaseRequest) -> IndigoResponse:
    molecule = indigo().loadMolecule(indigo_request.data.attributes.content)
    return apply_bool(molecule, "checkStereo")


@app.post(f"{BASE_URL_INDIGO_OBJECT}/checkValence")
async def check_valence(indigo_request: IndigoBaseRequest) -> IndigoResponse:
    # TODO: iterate all atoms
    molecule = indigo().loadMolecule(indigo_request.data.attributes.content)
    return apply_bool(molecule, "checkValence")


@app.post(f"{BASE_URL_INDIGO_OBJECT}/clean2d")
async def clean_2d(indigo_request: IndigoBaseRequest) -> IndigoResponse:
    molecule = indigo().loadMolecule(indigo_request.data.attributes.content)
    return apply(molecule, "clean2d")


@app.post(f"{BASE_URL_INDIGO_OBJECT}/clear")
async def clear(indigo_request: IndigoBaseRequest) -> IndigoResponse:
    molecule = indigo().loadMolecule(indigo_request.data.attributes.content)
    return apply(molecule, "clear")


@app.post(f"{BASE_URL_INDIGO_OBJECT}/clearAAM")
async def clear_aam(indigo_request: IndigoBaseRequest) -> IndigoResponse:
    molecule = indigo().loadMolecule(indigo_request.data.attributes.content)
    return apply(molecule, "clearAAM")


@app.post(f"{BASE_URL_INDIGO_OBJECT}/clearAlleneCenters")
async def clear_allene_centers(indigo_request: IndigoBaseRequest) -> IndigoResponse:
    molecule = indigo().loadMolecule(indigo_request.data.attributes.content)
    return apply(molecule, "clearAlleneCenters")


@app.post(f"{BASE_URL_INDIGO_OBJECT}/clearAttachmentPoints")
async def clear_attachment_points(indigo_request: IndigoBaseRequest) -> IndigoResponse:
    molecule = indigo().loadMolecule(indigo_request.data.attributes.content)
    return apply(molecule, "clearAttachmentPoints")


@app.post(f"{BASE_URL_INDIGO_OBJECT}/clearCisTrans")
async def clear_cis_trans(indigo_request: IndigoBaseRequest) -> IndigoResponse:
    molecule = indigo().loadMolecule(indigo_request.data.attributes.content)
    return apply(molecule, "clearCisTrans")


@app.post(f"{BASE_URL_INDIGO_OBJECT}/clearStereocenters")
async def clear_stereocenters(indigo_request: IndigoBaseRequest) -> IndigoResponse:
    molecule = indigo().loadMolecule(indigo_request.data.attributes.content)
    return apply(molecule, "clearStereocenters")


@app.post(f"{BASE_URL_INDIGO_OBJECT}/countAlleneCenters")
async def count_allene_centers(indigo_request: IndigoBaseRequest) -> IndigoResponse:
    molecule = indigo().loadMolecule(indigo_request.data.attributes.content)
    return apply_int(molecule, "countAlleneCenters")


@app.post(f"{BASE_URL_INDIGO_OBJECT}/countAtoms")
async def count_atoms(indigo_request: IndigoBaseRequest) -> IndigoResponse:
    molecule = indigo().loadMolecule(indigo_request.data.attributes.content)
    return apply_int(molecule, "countAtoms")


@app.post(f"{BASE_URL_INDIGO_OBJECT}/countAttachmentPoints")
async def count_attachment_points(indigo_request: IndigoBaseRequest) -> IndigoResponse:
    molecule = indigo().loadMolecule(indigo_request.data.attributes.content)
    return apply_int(molecule, "countAttachmentPoints")


@app.post(f"{BASE_URL_INDIGO_OBJECT}/countBonds")
async def count_bonds(indigo_request: IndigoBaseRequest) -> IndigoResponse:
    molecule = indigo().loadMolecule(indigo_request.data.attributes.content)
    return apply_int(molecule, "countBonds")


@app.post(f"{BASE_URL_INDIGO_OBJECT}/countCatalysts")
async def count_catalysts(indigo_request: IndigoBaseRequest) -> IndigoResponse:
    reaction = indigo().loadReaction(indigo_request.data.attributes.content)
    return apply_int(reaction, "countCatalysts")


@app.post(f"{BASE_URL_INDIGO_OBJECT}/countComponents")
async def count_components(indigo_request: IndigoBaseRequest) -> IndigoResponse:
    molecule = indigo().loadMolecule(indigo_request.data.attributes.content)
    return apply_int(molecule, "countComponents")


@app.post(f"{BASE_URL_INDIGO_OBJECT}/countDataSGroups")
async def count_data_sgroups(indigo_request: IndigoBaseRequest) -> IndigoResponse:
    molecule = indigo().loadMolecule(indigo_request.data.attributes.content)
    return apply_int(molecule, "countDataSGroups")


@app.post(f"{BASE_URL_INDIGO_OBJECT}/countGenericSGroups")
async def count_generic_sgroups(indigo_request: IndigoBaseRequest) -> IndigoResponse:
    molecule = indigo().loadMolecule(indigo_request.data.attributes.content)
    return apply_int(molecule, "countGenericSGroups")


@app.post(f"{BASE_URL_INDIGO_OBJECT}/countHeavyAtoms")
async def count_heavy_atoms(indigo_request: IndigoBaseRequest) -> IndigoResponse:
    molecule = indigo().loadMolecule(indigo_request.data.attributes.content)
    return apply_int(molecule, "countHeavyAtoms")


@app.post(f"{BASE_URL_INDIGO_OBJECT}/countHydrogens")
async def count_hydrogens(indigo_request: IndigoBaseRequest) -> IndigoResponse:
    molecule = indigo().loadMolecule(indigo_request.data.attributes.content)
    return apply_int(molecule, "countHydrogens")


@app.post(f"{BASE_URL_INDIGO_OBJECT}/countImplicitHydrogens")
async def count_implicit_hydrogens(indigo_request: IndigoBaseRequest) -> IndigoResponse:
    molecule = indigo().loadMolecule(indigo_request.data.attributes.content)
    return apply_int(molecule, "countImplicitHydrogens")


@app.post(f"{BASE_URL_INDIGO_OBJECT}/countMolecules")
async def count_molecules(indigo_request: IndigoBaseRequest) -> IndigoResponse:
    molecule = indigo().loadMolecule(indigo_request.data.attributes.content)
    return apply_int(molecule, "countMolecules")


@app.post(f"{BASE_URL_INDIGO_OBJECT}/countMultipleGroups")
async def count_multiple_groups(indigo_request: IndigoBaseRequest) -> IndigoResponse:
    molecule = indigo().loadMolecule(indigo_request.data.attributes.content)
    return apply_int(molecule, "countMultipleGroups")


@app.post(f"{BASE_URL_INDIGO_OBJECT}/countProducts")
async def count_products(indigo_request: IndigoBaseRequest) -> IndigoResponse:
    reaction = indigo().loadReaction(indigo_request.data.attributes.content)
    return apply_int(reaction, "countProducts")


@app.post(f"{BASE_URL_INDIGO_OBJECT}/countPseudoatoms")
async def count_pseudoatoms(indigo_request: IndigoBaseRequest) -> IndigoResponse:
    molecule = indigo().loadMolecule(indigo_request.data.attributes.content)
    return apply_int(molecule, "countPseudoatoms")


@app.post(f"{BASE_URL_INDIGO_OBJECT}/countRGroups")
async def count_rgroups(indigo_request: IndigoBaseRequest) -> IndigoResponse:
    molecule = indigo().loadMolecule(indigo_request.data.attributes.content)
    return apply_int(molecule, "countRGroups")


@app.post(f"{BASE_URL_INDIGO_OBJECT}/countRSites")
async def count_rsites(indigo_request: IndigoBaseRequest) -> IndigoResponse:
    molecule = indigo().loadMolecule(indigo_request.data.attributes.content)
    return apply_int(molecule, "countRSites")


@app.post(f"{BASE_URL_INDIGO_OBJECT}/countReactants")
async def count_reactants(indigo_request: IndigoBaseRequest) -> IndigoResponse:
    reaction = indigo().loadReaction(indigo_request.data.attributes.content)
    return apply_int(reaction, "countReactants")


@app.post(f"{BASE_URL_INDIGO_OBJECT}/countRepeatingUnits")
async def count_repeating_units(indigo_request: IndigoBaseRequest) -> IndigoResponse:
    molecule = indigo().loadMolecule(indigo_request.data.attributes.content)
    return apply_int(molecule, "countRepeatingUnits")


@app.post(f"{BASE_URL_INDIGO_OBJECT}/countSSSR")
async def count_sssr(indigo_request: IndigoBaseRequest) -> IndigoResponse:
    molecule = indigo().loadMolecule(indigo_request.data.attributes.content)
    return apply_int(molecule, "countSSSR")


@app.post(f"{BASE_URL_INDIGO_OBJECT}/countStereocenters")
async def count_stereo_centers(indigo_request: IndigoBaseRequest) -> IndigoResponse:
    molecule = indigo().loadMolecule(indigo_request.data.attributes.content)
    return apply_int(molecule, "countStereocenters")


@app.post(f"{BASE_URL_INDIGO_OBJECT}/countSuperatoms")
async def count_superatoms(indigo_request: IndigoBaseRequest) -> IndigoResponse:
    molecule = indigo().loadMolecule(indigo_request.data.attributes.content)
    return apply_int(molecule, "countSuperatoms")


@app.post(f"{BASE_URL_INDIGO_OBJECT}/dearomatize")
async def clear_dearomatize(indigo_request: IndigoBaseRequest) -> IndigoResponse:
    molecule = indigo().loadMolecule(indigo_request.data.attributes.content)
    return apply(molecule, "dearomatize")


@app.post(f"{BASE_URL_INDIGO_OBJECT}/grossFormula")
async def gross_formula(indigo_request: IndigoBaseRequest) -> IndigoResponse:
    molecule = indigo().loadMolecule(indigo_request.data.attributes.content)
    return IndigoResponse(
        data={
            "type": SupportedTypes.GROSSFORMULA,
            "attributes": {"content": molecule.grossFormula()},
        }
    )


@app.post(f"{BASE_URL_INDIGO_OBJECT}/hasCoord")
async def has_coord(indigo_request: IndigoBaseRequest) -> IndigoResponse:
    molecule = indigo().loadMolecule(indigo_request.data.attributes.content)
    return apply_bool(molecule, "hasCoord")


@app.post(f"{BASE_URL_INDIGO_OBJECT}/isChiral")
async def is_chiral(indigo_request: IndigoBaseRequest) -> IndigoResponse:
    molecule = indigo().loadMolecule(indigo_request.data.attributes.content)
    return apply_bool(molecule, "isChiral")


@app.post(f"{BASE_URL_INDIGO_OBJECT}/molecularWeight")
async def molecular_weight(indigo_request: IndigoBaseRequest) -> IndigoResponse:
    molecule = indigo().loadMolecule(indigo_request.data.attributes.content)
    return apply_float(molecule, "molecularWeight")


@app.post(f"{BASE_URL_INDIGO_OBJECT}/normalize", response_model=IndigoResponse)
async def normalize(indigo_request: IndigoBaseRequest) -> IndigoResponse:
    molecule = indigo().loadMolecule(indigo_request.data.attributes.content)
    return apply(molecule, "normalize")


@app.post(f"{BASE_URL_INDIGO_OBJECT}/nameToStructure")
async def name_to_structure(
    indigo_request: IndigoStructurePropsRequest,
) -> IndigoResponse:
    name = indigo_request.data.attributes.content
    props = indigo_request.data.attributes.props

    structure: IndigoObject = indigo().nameToStructure(name, props)
    # TODO: what to return
    return IndigoResponse()


@app.post(f"{BASE_URL_INDIGO_OBJECT}/reactionProductEnumerate")
async def reaction_product_enumerate(
    indigo_request: IndigoReactionProductEnumerateRequest,
) -> IndigoResponse:
    # data = parse_indigo_request(indigo_request, separate=True)
    try:
        reaction = indigo().loadQueryReaction(indigo_request.data.attributes.reaction)
        monomers_table = indigo_request.data.attributes.monomers_table
        monomers_table = [
            [indigo().loadMolecule(m) for m in row] for row in monomers_table
        ]
        # TODO: check convertToArray implementation to see if it saves nested table structure
        # if not, do conversion manually, as described here:
        # https://lifescience.opensource.epam.com/indigo/api/index.html, see Reaction Products Enumeration
        output_reactions = indigo().reactionProductEnumerate(reaction, monomers_table)
    except IndigoException as e:
        return error_response(str(e))

    # TODO: how to return output reactions
    return IndigoResponse()
