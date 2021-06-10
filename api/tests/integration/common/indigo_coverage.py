import sys
import os
from inspect import getmembers
from types import BuiltinFunctionType, BuiltinMethodType, MethodType, FunctionType
import zipfile

from util import isIronPython, isJython, getPlatform

cur_path = os.path.abspath(os.path.dirname(__file__))
distPaths = [os.path.join(cur_path, '../../../indigo/dist'), os.path.join(cur_path, '../../dist/')]
success = False

if isIronPython():
    raise RuntimeError("Indigo coverage is not supported in .NET")
elif isJython():
    raise RuntimeError("Indigo coverage is not supported in Java")
else:
    dll_full_path = os.path.normpath(os.path.join(cur_path, "../../../indigo/api/python"))
    rdll_full_path = os.path.normpath(os.path.join(cur_path, "../../../indigo/api/plugins/renderer/python"))
    idll_full_path = os.path.normpath(os.path.join(cur_path, "../../../indigo/api/plugins/inchi/python"))
    bdll_full_path = os.path.normpath(os.path.join(cur_path, "../../../indigo/api/plugins/bingo/python"))

    if not os.path.exists(os.path.join(dll_full_path, 'lib')):
        for distPath in distPaths:
            if not os.path.exists(distPath):
                continue
            dll_full_path = '%s/python' % (distPath)
            for item in os.listdir(distPath):
                if item.startswith('indigo-python-') and item.endswith('.zip') and (item.find(getPlatform()) != -1 or item.find('universal') != -1):
                    curdir = os.path.abspath(os.curdir)
                    os.chdir(distPath)
                    if 'INDIGO_TEST_MODE' not in os.environ:
                        with zipfile.ZipFile(item) as zf:
                            zf.extractall()
                        os.environ['INDIGO_TEST_MODE'] = '1'
                    os.chdir(curdir)
                    dll_full_path = os.path.abspath(os.path.join(cur_path, distPath, item.replace('.zip', '')))
                    break
            if not os.path.exists(dll_full_path):
                continue
            break

    sys.path.insert(0, dll_full_path)
    sys.path.insert(0, rdll_full_path)
    sys.path.insert(0, idll_full_path)
    sys.path.insert(0, bdll_full_path)
    from indigo import Indigo, IndigoObject, IndigoException
    from indigo_renderer import IndigoRenderer
    from indigo_inchi import IndigoInchi
    from bingo import Bingo, BingoException, BingoObject
    success = True

if not success:
    raise RuntimeError('Indigo not found at %s' % distPaths)


class IndigoObjectCoverageWrapper(IndigoObject):
    def __init__(self, dispatcher, id, parent=None):
        IndigoObject.__init__(self, dispatcher, id, parent)
        self._type = None
        self._type = int(self.dbgInternalType()[1:3])

    def __getattribute__(self, item):
        dispatcher = object.__getattribute__(self, 'dispatcher')
        type = object.__getattribute__(self, '_type')
        if dispatcher is not None:
            if item in dispatcher._indigoObjectCoverageDict:
                dispatcher._indigoObjectCoverageDict[item] += 1
                if type:
                    if type not in dispatcher._indigoObjectCoverageByTypeDict:
                        dispatcher._indigoObjectCoverageByTypeDict[type] = {}
                        dispatcher._indigoObjectCoverageByTypeDict[type][item] = 1
                    else:
                        if item not in dispatcher._indigoObjectCoverageByTypeDict[type]:
                            dispatcher._indigoObjectCoverageByTypeDict[type][item] = 1
                        else:
                            dispatcher._indigoObjectCoverageByTypeDict[type][item] += 1
        return object.__getattribute__(self, item)


class IndigoCoverageWrapper(Indigo):
    def __init__(self, path=None):
        Indigo.__init__(self, path)
        if isJython() or isIronPython():
            IndigoObject = IndigoObjectCoverageWrapper
            # TODO: Change standard IndigoObject to IndigoObjectCoverageWrapper
        else:
            self.IndigoObject = IndigoObjectCoverageWrapper
        self._indigoObjectCoverageDict = dict()
        self._indigoObjectCoverageByTypeDict = dict()
        m = self.createMolecule()
        for item in getmembers(m):
            if type(item[1]) in (BuiltinFunctionType, BuiltinMethodType, MethodType, FunctionType) and not item[0].startswith('_'):
                self._indigoObjectCoverageDict[item[0]] = 0
        self._indigoCoverageDict = dict()
        for item in getmembers(self):
            if type(item[1]) in (BuiltinFunctionType, BuiltinMethodType, MethodType, FunctionType) and not item[0].startswith('_'):
                self._indigoCoverageDict[item[0]] = 0

    def __getattribute__(self, item):
        try:
            indigoCoverageDict = object.__getattribute__(self, '_indigoCoverageDict')
            if indigoCoverageDict:
                if item in indigoCoverageDict:
                    indigoCoverageDict[item] += 1
        except AttributeError:
            pass
        return object.__getattribute__(self, item)

    def version(self):
        return super(IndigoCoverageWrapper, self).version() + '-coverage'


class IndigoObjectTypeEnum:
    SCANNER = 1
    MOLECULE = 2
    QUERY_MOLECULE = 3
    REACTION = 4
    QUERY_REACTION = 5
    OUTPUT = 6
    REACTION_ITER = 7
    REACTION_MOLECULE = 8
    GROSS = 9
    SDF_LOADER = 10
    SDF_SAVER = 11
    RDF_MOLECULE = 12
    RDF_REACTION = 13
    RDF_LOADER = 14
    SMILES_MOLECULE = 15
    SMILES_REACTION = 16
    MULTILINE_SMILES_LOADER = 17
    ATOM = 18
    ATOMS_ITER = 19
    RGROUP = 20
    RGROUPS_ITER = 21
    RGROUP_FRAGMENT = 22
    RGROUP_FRAGMENTS_ITER = 23
    ARRAY = 24
    ARRAY_ITER = 25
    ARRAY_ELEMENT = 26
    MOLECULE_SUBSTRUCTURE_MATCH_ITER = 27
    MOLECULE_SUBSTRUCTURE_MATCHER = 28
    REACTION_SUBSTRUCTURE_MATCHER = 29
    SCAFFOLD = 30
    DECONVOLUTION = 31
    DECONVOLUTION_ELEM = 32
    DECONVOLUTION_ITER = 33
    PROPERTIES_ITER = 34
    PROPERTY = 35
    FINGERPRINT = 36
    BOND = 37
    BONDS_ITER = 38
    ATOM_NEIGHBOR = 39
    ATOM_NEIGHBORS_ITER = 40
    SUPERATOM = 41
    SUPERATOMS_ITER = 42
    DATA_SGROUP = 43
    DATA_SGROUPS_ITER = 44
    REPEATING_UNIT = 45
    REPEATING_UNITS_ITER = 46
    MULTIPLE_GROUP = 47
    MULTIPLE_GROUPS_ITER = 48
    GENERIC_SGROUP = 49
    GENERIC_SGROUPS_ITER = 50
    SGROUP_ATOMS_ITER = 51
    SGROUP_BONDS_ITER = 52
    DECOMPOSITION = 53
    COMPONENT = 54
    COMPONENTS_ITER = 55
    COMPONENT_ATOMS_ITER = 56
    COMPONENT_BONDS_ITER = 57
    SUBMOLECULE = 58
    SUBMOLECULE_ATOMS_ITER = 59
    SUBMOLECULE_BONDS_ITER = 60
    MAPPING = 61
    REACTION_MAPPING = 62
    SSSR_ITER = 63
    SUBTREES_ITER = 64
    RINGS_ITER = 65
    EDGE_SUBMOLECULE_ITER = 66
    CML_MOLECULE = 67
    CML_REACTION = 68
    MULTIPLE_CML_LOADER = 69
    SAVER = 70
    ATTACHMENT_POINTS_ITER = 71
    DECOMPOSITION_MATCH = 72
    DECOMPOSITION_MATCH_ITER = 73
    TAUTOMER_ITER = 74
    TAUTOMER_MOLECULE = 75


IndigoObjectTypeDict = {
    1: 'SCANNER',
    2: 'MOLECULE',
    3: 'QUERY_MOLECULE',
    4: 'REACTION',
    5: 'QUERY_REACTION',
    6: 'OUTPUT',
    7: 'REACTION_ITER',
    8: 'REACTION_MOLECULE',
    9: 'GROSS',
    10: 'SDF_LOADER',
    11: 'SDF_SAVER',
    12: 'RDF_MOLECULE',
    13: 'RDF_REACTION',
    14: 'RDF_LOADER',
    15: 'SMILES_MOLECULE',
    16: 'SMILES_REACTION',
    17: 'MULTILINE_SMILES_LOADER',
    18: 'ATOM',
    19: 'ATOMS_ITER',
    20: 'RGROUP',
    21: 'RGROUPS_ITER',
    22: 'RGROUP_FRAGMENT',
    23: 'RGROUP_FRAGMENTS_ITER',
    24: 'ARRAY',
    25: 'ARRAY_ITER',
    26: 'ARRAY_ELEMENT',
    27: 'MOLECULE_SUBSTRUCTURE_MATCH_ITER',
    28: 'MOLECULE_SUBSTRUCTURE_MATCHER',
    29: 'REACTION_SUBSTRUCTURE_MATCHER',
    30: 'SCAFFOLD',
    31: 'DECONVOLUTION',
    32: 'DECONVOLUTION_ELEM',
    33: 'DECONVOLUTION_ITER',
    34: 'PROPERTIES_ITER',
    35: 'PROPERTY',
    36: 'FINGERPRINT',
    37: 'BOND',
    38: 'BONDS_ITER',
    39: 'ATOM_NEIGHBOR',
    40: 'ATOM_NEIGHBORS_ITER',
    41: 'SUPERATOM',
    42: 'SUPERATOMS_ITER',
    43: 'DATA_SGROUP',
    44: 'DATA_SGROUPS_ITER',
    45: 'REPEATING_UNIT',
    46: 'REPEATING_UNITS_ITER',
    47: 'MULTIPLE_GROUP',
    48: 'MULTIPLE_GROUPS_ITER',
    49: 'GENERIC_SGROUP',
    50: 'GENERIC_SGROUPS_ITER',
    51: 'SGROUP_ATOMS_ITER',
    52: 'SGROUP_BONDS_ITER',
    53: 'DECOMPOSITION',
    54: 'COMPONENT',
    55: 'COMPONENTS_ITER',
    56: 'COMPONENT_ATOMS_ITER',
    57: 'COMPONENT_BONDS_ITER',
    58: 'SUBMOLECULE',
    59: 'SUBMOLECULE_ATOMS_ITER',
    60: 'SUBMOLECULE_BONDS_ITER',
    61: 'MAPPING',
    62: 'REACTION_MAPPING',
    63: 'SSSR_ITER',
    64: 'SUBTREES_ITER',
    65: 'RINGS_ITER',
    66: 'EDGE_SUBMOLECULE_ITER',
    67: 'CML_MOLECULE',
    68: 'CML_REACTION',
    69: 'MULTIPLE_CML_LOADER',
    70: 'SAVER',
    71: 'ATTACHMENT_POINTS_ITER',
    72: 'DECOMPOSITION_MATCH',
    73: 'DECOMPOSITION_MATCH_ITER',
    74: 'TAUTOMER_ITER',
    75: 'TAUTOMER_MOLECULE',
}
