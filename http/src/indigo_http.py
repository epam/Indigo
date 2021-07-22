import json

from fastapi import FastAPI, Request
from fastapi.params import Depends
from indigo import IndigoException, IndigoObject
from indigo.renderer import IndigoRenderer
from starlette.responses import FileResponse

from .indigo_tools import create_temp_file, indigo, indigo_new
from .model import (
    IndigoAmbiguousHRequest,
    IndigoBaseRequest,
    IndigoExtractCommondScaffoldRequest,
    IndigoMolPairRequest,
    IndigoMolRequest,
    IndigoReactionProductEnumerateRequest,
    IndigoResponse,
    IndigoStructurePropsRequest,
    SupportedTypes,
)
from .responses import error_response, make_response

app = FastAPI()


BASE_URL_INDIGO = "/indigo"
BASE_URL_INDIGO_OBJECT = "/indigoObject"
RESP_HEADER_CONTENT_TYPE = "application/vnd.api+json"


@app.middleware("http")
async def isolate_indigo_session(request: Request, call_next):
    with indigo_new():
        response = await call_next(request)
        if not request.scope["path"].startswith(
            ("/docs", f"{BASE_URL_INDIGO_OBJECT}/render")
        ):
            response.headers["Content-Type"] = RESP_HEADER_CONTENT_TYPE
        return response


@app.post(f"{BASE_URL_INDIGO}/checkStructure", response_model=IndigoResponse)
async def check_structure(indigo_request: IndigoStructurePropsRequest):
    structure = indigo_request.data.attributes.content
    props = indigo_request.data.attributes.props

    json_res: str = indigo().checkStructure(structure, props)
    result = json.loads(json_res)

    return make_response(SupportedTypes.BOOL, result == {})


@app.post(f"{BASE_URL_INDIGO}/commonBits", response_model=IndigoResponse)
async def common_bits(indigo_request: IndigoMolPairRequest) -> IndigoResponse:
    try:
        mol1 = indigo().loadMolecule(indigo_request.data.attributes.mol1)
        mol2 = indigo().loadMolecule(indigo_request.data.attributes.mol2)
        bits = indigo().commonBits(mol1.fingerprint("sim"), mol2.fingerprint("sim"))
    except IndigoException as e:
        return error_response(str(e))

    return make_response(SupportedTypes.COMMON_BITS, bits)


@app.post(f"{BASE_URL_INDIGO}/decomposeMolecules", response_model=IndigoResponse)
async def decompose_molecules(indigo_request: IndigoBaseRequest) -> str:
    # TODO: https://lifescience.opensource.epam.com/indigo/api/index.html
    #  at R-Group Decomposition says indigo.decomposeMolecules(scaf, arr) is deprecated
    return make_response(SupportedTypes.BOOL, True)


@app.post(f"{BASE_URL_INDIGO}/exactMatch", response_model=IndigoResponse)
async def exact_match(indigo_request: IndigoMolPairRequest) -> IndigoResponse:
    # indigo_response = IndigoResponse()
    try:
        mol1 = indigo().loadMolecule(indigo_request.data.attributes.mol1)
        mol2 = indigo().loadMolecule(indigo_request.data.attributes.mol2)
        flags = indigo_request.data.attributes.flags
        match = bool(indigo().exactMatch(mol1, mol2, flags))
    except IndigoException as e:
        return error_response(str(e))

    return make_response(SupportedTypes.BOOL, match)


@app.get(f"{BASE_URL_INDIGO}/version", response_model=IndigoResponse)
async def indigo_version() -> IndigoResponse:
    return make_response(SupportedTypes.VERSION, indigo().version())


@app.post(f"{BASE_URL_INDIGO_OBJECT}/aromatize", response_model=IndigoResponse)
async def aromatize(indigo_request: IndigoMolRequest) -> IndigoResponse:
    molecule_string = indigo_request.data.attributes.content
    molecule = indigo().loadMolecule(molecule_string)
    molecule.aromatize()
    return make_response(indigo_request.data.type, molecule)


@app.post(f"{BASE_URL_INDIGO_OBJECT}/smiles", response_model=IndigoResponse)
async def smiles(indigo_request: IndigoBaseRequest) -> IndigoResponse:
    molecule_string = indigo_request.data.attributes.content
    try:
        mol = indigo().loadMolecule(molecule_string)
    except IndigoException as e:
        return error_response(str(e))

    return make_response(SupportedTypes.SMILES, mol.smiles())


@app.post(f"{BASE_URL_INDIGO_OBJECT}/smarts", response_model=IndigoResponse)
async def smarts(indigo_request: IndigoBaseRequest) -> IndigoResponse:
    molecule_string = indigo_request.data.attributes.content
    try:
        mol = indigo().loadMolecule(molecule_string)
    except IndigoException as e:
        return error_response(str(e))

    return make_response(SupportedTypes.SMARTS, mol.smarts())


@app.post(f"{BASE_URL_INDIGO_OBJECT}/standardize", response_model=IndigoResponse)
async def standardize(indigo_request: IndigoMolRequest) -> IndigoResponse:
    molecule = indigo().loadMolecule(indigo_request.data.attributes.content)
    molecule.standardize()
    return make_response(indigo_request.data.type, molecule)


@app.post(f"{BASE_URL_INDIGO_OBJECT}/unfoldHydrogens", response_model=IndigoResponse)
async def unfold_hydrogens(indigo_request: IndigoMolRequest) -> IndigoResponse:
    molecule = indigo().loadMolecule(indigo_request.data.attributes.content)
    molecule.unfoldHydrogens()
    return make_response(indigo_request.data.type, molecule)


@app.post(f"{BASE_URL_INDIGO_OBJECT}/validateChirality", response_model=IndigoResponse)
async def validate_chirality(indigo_request: IndigoBaseRequest) -> IndigoResponse:
    molecule = indigo().loadMolecule(indigo_request.data.attributes.content)
    result = molecule.validateChirality()  # TODO: empty response?
    return make_response(SupportedTypes.BOOL, result == None)


@app.post(f"{BASE_URL_INDIGO_OBJECT}/check3DStereo", response_model=IndigoResponse)
async def check_3d_stereo(indigo_request: IndigoBaseRequest) -> IndigoResponse:
    molecule = indigo().loadMolecule(indigo_request.data.attributes.content)
    result = molecule.check3DStereo()
    return make_response(SupportedTypes.BOOL, result)


@app.post(f"{BASE_URL_INDIGO_OBJECT}/checkAmbiguousH", response_model=IndigoResponse)
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

    result = indigo_object.check3DStereo()
    return make_response(SupportedTypes.BOOL, result)


@app.post(f"{BASE_URL_INDIGO_OBJECT}/checkBadValence", response_model=IndigoResponse)
async def check_bad_valence(indigo_request: IndigoBaseRequest) -> IndigoResponse:
    molecule = indigo().loadMolecule(indigo_request.data.attributes.content)
    result = molecule.checkBadValence()
    return make_response(SupportedTypes.BOOL, result)


@app.post(f"{BASE_URL_INDIGO_OBJECT}/checkChirality", response_model=IndigoResponse)
async def check_chirality(indigo_request: IndigoBaseRequest) -> IndigoResponse:
    molecule = indigo().loadMolecule(indigo_request.data.attributes.content)
    result = molecule.checkChirality()  # TODO: return value?
    return make_response(SupportedTypes.BOOL, result)


@app.post(f"{BASE_URL_INDIGO_OBJECT}/checkQuery", response_model=IndigoResponse)
async def check_query(indigo_request: IndigoBaseRequest) -> IndigoResponse:
    molecule = indigo().loadMolecule(indigo_request.data.attributes.content)
    result = molecule.checkQuery()
    return make_response(SupportedTypes.BOOL, result)


@app.post(f"{BASE_URL_INDIGO_OBJECT}/checkRGroups", response_model=IndigoResponse)
async def check_rgroups(indigo_request: IndigoBaseRequest) -> IndigoResponse:
    molecule = indigo().loadMolecule(indigo_request.data.attributes.content)
    result = molecule.checkRGroups()
    return make_response(SupportedTypes.BOOL, result)


@app.post(f"{BASE_URL_INDIGO_OBJECT}/checkStereo", response_model=IndigoResponse)
async def check_stereo(indigo_request: IndigoBaseRequest) -> IndigoResponse:
    molecule = indigo().loadMolecule(indigo_request.data.attributes.content)
    result = molecule.checkStereo()
    return make_response(SupportedTypes.BOOL, result)


@app.post(f"{BASE_URL_INDIGO_OBJECT}/checkValence", response_model=IndigoResponse)
async def check_valence(indigo_request: IndigoBaseRequest) -> IndigoResponse:
    # TODO: iterate all atoms
    molecule = indigo().loadMolecule(indigo_request.data.attributes.content)
    result = molecule.checkValence()
    return make_response(SupportedTypes.BOOL, result)


@app.post(f"{BASE_URL_INDIGO_OBJECT}/clean2d", response_model=IndigoResponse)
async def clean_2d(indigo_request: IndigoMolRequest) -> IndigoResponse:
    molecule = indigo().loadMolecule(indigo_request.data.attributes.content)
    molecule.clean2d()
    return make_response(indigo_request.data.type, molecule)


@app.post(f"{BASE_URL_INDIGO_OBJECT}/clear", response_model=IndigoResponse)
async def clear(indigo_request: IndigoMolRequest) -> IndigoResponse:
    molecule = indigo().loadMolecule(indigo_request.data.attributes.content)
    molecule.clear()
    return make_response(indigo_request.data.type, molecule)


@app.post(f"{BASE_URL_INDIGO_OBJECT}/clearAAM", response_model=IndigoResponse)
async def clear_aam(indigo_request: IndigoMolRequest) -> IndigoResponse:
    molecule = indigo().loadMolecule(indigo_request.data.attributes.content)
    molecule.clearAAM()
    return make_response(indigo_request.data.type, molecule)


@app.post(f"{BASE_URL_INDIGO_OBJECT}/clearAlleneCenters", response_model=IndigoResponse)
async def clear_allene_centers(indigo_request: IndigoMolRequest) -> IndigoResponse:
    molecule = indigo().loadMolecule(indigo_request.data.attributes.content)
    molecule.clearAlleneCenters()
    return make_response(indigo_request.data.type, molecule)


@app.post(
    f"{BASE_URL_INDIGO_OBJECT}/clearAttachmentPoints", response_model=IndigoResponse
)
async def clear_attachment_points(indigo_request: IndigoMolRequest) -> IndigoResponse:
    molecule = indigo().loadMolecule(indigo_request.data.attributes.content)
    molecule.clearAttachmentPoints()
    return make_response(indigo_request.data.type, molecule)


@app.post(f"{BASE_URL_INDIGO_OBJECT}/clearCisTrans", response_model=IndigoResponse)
async def clear_cis_trans(indigo_request: IndigoMolRequest) -> IndigoResponse:
    molecule = indigo().loadMolecule(indigo_request.data.attributes.content)
    molecule.clearCisTrans()
    return make_response(indigo_request.data.type, molecule)


@app.post(f"{BASE_URL_INDIGO_OBJECT}/clearStereocenters", response_model=IndigoResponse)
async def clear_stereocenters(indigo_request: IndigoMolRequest) -> IndigoResponse:
    molecule = indigo().loadMolecule(indigo_request.data.attributes.content)
    molecule.clearStereocenters()
    return make_response(indigo_request.data.type, molecule)


@app.post(f"{BASE_URL_INDIGO_OBJECT}/countAlleneCenters", response_model=IndigoResponse)
async def count_allene_centers(indigo_request: IndigoBaseRequest) -> IndigoResponse:
    molecule = indigo().loadMolecule(indigo_request.data.attributes.content)
    result = molecule.countAlleneCenters()
    return make_response(SupportedTypes.INT, result)


@app.post(f"{BASE_URL_INDIGO_OBJECT}/countAtoms", response_model=IndigoResponse)
async def count_atoms(indigo_request: IndigoBaseRequest) -> IndigoResponse:
    molecule = indigo().loadMolecule(indigo_request.data.attributes.content)
    result = molecule.countAtoms()
    return make_response(SupportedTypes.INT, result)


@app.post(
    f"{BASE_URL_INDIGO_OBJECT}/countAttachmentPoints", response_model=IndigoResponse
)
async def count_attachment_points(indigo_request: IndigoBaseRequest) -> IndigoResponse:
    molecule = indigo().loadMolecule(indigo_request.data.attributes.content)
    result = molecule.countAttachmentPoints()
    return make_response(SupportedTypes.INT, result)


@app.post(f"{BASE_URL_INDIGO_OBJECT}/countBonds", response_model=IndigoResponse)
async def count_bonds(indigo_request: IndigoBaseRequest) -> IndigoResponse:
    molecule = indigo().loadMolecule(indigo_request.data.attributes.content)
    result = molecule.countBonds()
    return make_response(SupportedTypes.INT, result)


@app.post(f"{BASE_URL_INDIGO_OBJECT}/countCatalysts", response_model=IndigoResponse)
async def count_catalysts(indigo_request: IndigoBaseRequest) -> IndigoResponse:
    reaction = indigo().loadReaction(indigo_request.data.attributes.content)
    result = reaction.countCatalysts()
    return make_response(SupportedTypes.INT, result)


@app.post(f"{BASE_URL_INDIGO_OBJECT}/countComponents", response_model=IndigoResponse)
async def count_components(indigo_request: IndigoBaseRequest) -> IndigoResponse:
    molecule = indigo().loadMolecule(indigo_request.data.attributes.content)
    result = molecule.countComponents()
    return make_response(SupportedTypes.INT, result)


@app.post(f"{BASE_URL_INDIGO_OBJECT}/countDataSGroups", response_model=IndigoResponse)
async def count_data_sgroups(indigo_request: IndigoBaseRequest) -> IndigoResponse:
    molecule = indigo().loadMolecule(indigo_request.data.attributes.content)
    result = molecule.countDataSGroups()
    return make_response(SupportedTypes.INT, result)


@app.post(
    f"{BASE_URL_INDIGO_OBJECT}/countGenericSGroups", response_model=IndigoResponse
)
async def count_generic_sgroups(indigo_request: IndigoBaseRequest) -> IndigoResponse:
    molecule = indigo().loadMolecule(indigo_request.data.attributes.content)
    result = molecule.countGenericSGroups()
    return make_response(SupportedTypes.INT, result)


@app.post(f"{BASE_URL_INDIGO_OBJECT}/countHeavyAtoms", response_model=IndigoResponse)
async def count_heavy_atoms(indigo_request: IndigoBaseRequest) -> IndigoResponse:
    molecule = indigo().loadMolecule(indigo_request.data.attributes.content)
    result = molecule.countHeavyAtoms()
    return make_response(SupportedTypes.INT, result)


@app.post(f"{BASE_URL_INDIGO_OBJECT}/countHydrogens", response_model=IndigoResponse)
async def count_hydrogens(indigo_request: IndigoBaseRequest) -> IndigoResponse:
    molecule = indigo().loadMolecule(indigo_request.data.attributes.content)
    result = molecule.countHydrogens()
    return make_response(SupportedTypes.INT, result)


@app.post(
    f"{BASE_URL_INDIGO_OBJECT}/countImplicitHydrogens", response_model=IndigoResponse
)
async def count_implicit_hydrogens(indigo_request: IndigoBaseRequest) -> IndigoResponse:
    molecule = indigo().loadMolecule(indigo_request.data.attributes.content)
    result = molecule.countImplicitHydrogens()
    return make_response(SupportedTypes.INT, result)


@app.post(f"{BASE_URL_INDIGO_OBJECT}/countMolecules", response_model=IndigoResponse)
async def count_molecules(indigo_request: IndigoBaseRequest) -> IndigoResponse:
    molecule = indigo().loadMolecule(indigo_request.data.attributes.content)
    result = molecule.countMolecules()
    return make_response(SupportedTypes.INT, result)


@app.post(
    f"{BASE_URL_INDIGO_OBJECT}/countMultipleGroups", response_model=IndigoResponse
)
async def count_multiple_groups(indigo_request: IndigoBaseRequest) -> IndigoResponse:
    molecule = indigo().loadMolecule(indigo_request.data.attributes.content)
    result = molecule.countMultipleGroups()
    return make_response(SupportedTypes.INT, result)


@app.post(f"{BASE_URL_INDIGO_OBJECT}/countProducts", response_model=IndigoResponse)
async def count_products(indigo_request: IndigoBaseRequest) -> IndigoResponse:
    reaction = indigo().loadReaction(indigo_request.data.attributes.content)
    result = reaction.countProducts()
    return make_response(SupportedTypes.INT, result)


@app.post(f"{BASE_URL_INDIGO_OBJECT}/countPseudoatoms", response_model=IndigoResponse)
async def count_pseudoatoms(indigo_request: IndigoBaseRequest) -> IndigoResponse:
    molecule = indigo().loadMolecule(indigo_request.data.attributes.content)
    result = molecule.countPseudoatoms()
    return make_response(SupportedTypes.INT, result)


@app.post(f"{BASE_URL_INDIGO_OBJECT}/countRGroups", response_model=IndigoResponse)
async def count_rgroups(indigo_request: IndigoBaseRequest) -> IndigoResponse:
    molecule = indigo().loadMolecule(indigo_request.data.attributes.content)
    result = molecule.countRGroups()
    return make_response(SupportedTypes.INT, result)


@app.post(f"{BASE_URL_INDIGO_OBJECT}/countRSites", response_model=IndigoResponse)
async def count_rsites(indigo_request: IndigoBaseRequest) -> IndigoResponse:
    molecule = indigo().loadMolecule(indigo_request.data.attributes.content)
    result = molecule.countRSites()
    return make_response(SupportedTypes.INT, result)


@app.post(f"{BASE_URL_INDIGO_OBJECT}/countReactants", response_model=IndigoResponse)
async def count_reactants(indigo_request: IndigoBaseRequest) -> IndigoResponse:
    reaction = indigo().loadReaction(indigo_request.data.attributes.content)
    result = reaction.countReactants()
    return make_response(SupportedTypes.INT, result)


@app.post(
    f"{BASE_URL_INDIGO_OBJECT}/countRepeatingUnits", response_model=IndigoResponse
)
async def count_repeating_units(indigo_request: IndigoBaseRequest) -> IndigoResponse:
    molecule = indigo().loadMolecule(indigo_request.data.attributes.content)
    result = molecule.countRepeatingUnits()
    return make_response(SupportedTypes.INT, result)


@app.post(f"{BASE_URL_INDIGO_OBJECT}/countSSSR", response_model=IndigoResponse)
async def count_sssr(indigo_request: IndigoBaseRequest) -> IndigoResponse:
    molecule = indigo().loadMolecule(indigo_request.data.attributes.content)
    result = molecule.countSSSR()
    return make_response(SupportedTypes.INT, result)


@app.post(f"{BASE_URL_INDIGO_OBJECT}/countStereocenters", response_model=IndigoResponse)
async def count_stereo_centers(indigo_request: IndigoBaseRequest) -> IndigoResponse:
    molecule = indigo().loadMolecule(indigo_request.data.attributes.content)
    result = molecule.countStereocenters()
    return make_response(SupportedTypes.INT, result)


@app.post(f"{BASE_URL_INDIGO_OBJECT}/countSuperatoms", response_model=IndigoResponse)
async def count_superatoms(indigo_request: IndigoBaseRequest) -> IndigoResponse:
    molecule = indigo().loadMolecule(indigo_request.data.attributes.content)
    result = molecule.countSuperatoms()
    return make_response(SupportedTypes.INT, result)


@app.post(f"{BASE_URL_INDIGO_OBJECT}/dearomatize", response_model=IndigoResponse)
async def clear_dearomatize(indigo_request: IndigoMolRequest) -> IndigoResponse:
    molecule = indigo().loadMolecule(indigo_request.data.attributes.content)
    molecule.dearomatize()
    return make_response(indigo_request.data.type, molecule)


@app.post(f"{BASE_URL_INDIGO_OBJECT}/grossFormula", response_model=IndigoResponse)
async def gross_formula(indigo_request: IndigoBaseRequest) -> IndigoResponse:
    molecule = indigo().loadMolecule(indigo_request.data.attributes.content)
    return make_response(SupportedTypes.GROSSFORMULA, molecule.grossFormula())


@app.post(f"{BASE_URL_INDIGO_OBJECT}/hasCoord", response_model=IndigoResponse)
async def has_coord(indigo_request: IndigoBaseRequest) -> IndigoResponse:
    molecule = indigo().loadMolecule(indigo_request.data.attributes.content)
    result = molecule.hasCoord()
    return make_response(SupportedTypes.BOOL, result)


@app.post(f"{BASE_URL_INDIGO_OBJECT}/isChiral", response_model=IndigoResponse)
async def is_chiral(indigo_request: IndigoBaseRequest) -> IndigoResponse:
    molecule = indigo().loadMolecule(indigo_request.data.attributes.content)
    result = molecule.isChiral()
    return make_response(SupportedTypes.BOOL, result)


@app.post(f"{BASE_URL_INDIGO_OBJECT}/molecularWeight", response_model=IndigoResponse)
async def molecular_weight(indigo_request: IndigoBaseRequest) -> IndigoResponse:
    molecule = indigo().loadMolecule(indigo_request.data.attributes.content)
    result = molecule.molecularWeight()
    return make_response(SupportedTypes.FLOAT, result)


@app.post(f"{BASE_URL_INDIGO_OBJECT}/normalize", response_model=IndigoResponse)
async def normalize(indigo_request: IndigoMolRequest) -> IndigoResponse:
    molecule = indigo().loadMolecule(indigo_request.data.attributes.content)
    molecule.normalize()
    return make_response(indigo_request.data.type, molecule)


@app.post(f"{BASE_URL_INDIGO_OBJECT}/nameToStructure", response_model=IndigoResponse)
async def name_to_structure(
    indigo_request: IndigoStructurePropsRequest.with_types(
        (SupportedTypes.MOLFILE, SupportedTypes.SMILES_LIST, SupportedTypes.SMARTS)
    ),
) -> IndigoResponse:
    name = indigo_request.data.attributes.content
    props = indigo_request.data.attributes.props

    structure: IndigoObject = indigo().nameToStructure(name, props)
    return make_response(indigo_request.data.type, structure)


@app.post(
    f"{BASE_URL_INDIGO_OBJECT}/reactionProductEnumerate", response_model=IndigoResponse
)
async def reaction_product_enumerate(
    indigo_request: IndigoReactionProductEnumerateRequest,
) -> IndigoResponse:
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

    return make_response(indigo_request.data.type, output_reactions.iterateArray())


@app.post(
    f"{BASE_URL_INDIGO_OBJECT}/extractCommonScaffold", response_model=IndigoResponse
)
async def extract_common_scaffold(
    indigo_request: IndigoExtractCommondScaffoldRequest,
) -> IndigoResponse:
    extract_mode = indigo_request.data.attributes.mode
    structures = [
        indigo().loadMolecule(mol) for mol in indigo_request.data.attributes.structures
    ]
    scaf = indigo().extractCommonScaffold(structures, extract_mode)

    if scaf is None:
        return error_response(msg="No common scaffold")

    return make_response(indigo_request.data.type, scaf.allScaffolds().iterateArray())


@app.post(f"{BASE_URL_INDIGO_OBJECT}/similarity", response_model=IndigoResponse)
async def similarity(ingido_request: IndigoMolPairRequest) -> IndigoResponse:
    """Does not support fingerprints"""

    if ingido_request.data.attributes.is_reactions:
        m1 = indigo().loadReaction(ingido_request.data.attributes.mol1)
        m2 = indigo().loadReaction(ingido_request.data.attributes.mol2)
    else:
        m1 = indigo().loadMolecule(ingido_request.data.attributes.mol1)
        m2 = indigo().loadMolecule(ingido_request.data.attributes.mol2)

    metrics = ingido_request.data.attributes.flags
    result = indigo().similarity(m1, m2, metrics)

    return make_response(SupportedTypes.FLOAT, result)


@app.post(f"{BASE_URL_INDIGO_OBJECT}/renderToFile", response_class=FileResponse)
async def render_to_file(
    indigo_request: IndigoMolRequest, temp_path=Depends(create_temp_file)
) -> FileResponse:
    # TODO: add support for rendering options
    mol = indigo().loadMolecule(indigo_request.data.attributes.content)
    renderer = IndigoRenderer(indigo())
    renderer.renderToFile(mol, temp_path)
    return FileResponse(path=temp_path, filename="mol.png", media_type="image/png")
