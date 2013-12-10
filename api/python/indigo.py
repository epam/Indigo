#
#
# Copyright (C) 2010-2013 GGA Software Services LLC
#
# This file is part of Indigo toolkit.
#
# This file may be distributed and/or modified under the terms of the
# GNU General Public License version 3 as published by the Free Software
# Foundation and appearing in the file LICENSE.GPL included in the
# packaging of this file.
#
# This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
# WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.

import sys
import os
import platform
from array import array
from ctypes import c_int, c_char_p, c_float, POINTER, pointer, CDLL, RTLD_GLOBAL, c_ulonglong, c_byte


class IndigoException (Exception):
    def __init__(self, value):
        self.value = value

    def __str__(self):
        return repr(self.value.decode('ascii')) if sys.version_info > (3, 0) else repr(self.value)


class IndigoObject(object):
    def __init__(self, dispatcher, id, parent=None):
        self.id = id
        self.dispatcher = dispatcher
        self.parent = parent

    def __enter__ (self):
        return self

    def __exit__ (self, exc_type, exc_value, traceback):
        self.dispatcher._setSessionId()
        self.dispatcher._lib.indigoClose(self.id)

    def __del__ (self):
        self.dispose()

    def dispose(self):
        if self.id >= 0:
            if getattr(Indigo, "_lib", None) is not None:
                self.dispatcher._setSessionId()
                Indigo._lib.indigoFree(self.id)
                self.id = -1

    def __iter__ (self):
        return self

    def _next(self):
        self.dispatcher._setSessionId()
        newobj = self.dispatcher._checkResult(Indigo._lib.indigoNext(self.id))
        if newobj == 0:
            return None
        else:
            return self.dispatcher.IndigoObject(self.dispatcher, newobj, self)

    def __next__ (self):
        obj = self._next()
        if obj == None:
            raise StopIteration
        return obj

    def next (self):
        return self.__next__()

    def grossFormula(self):
        self.dispatcher._setSessionId()
        gfid = self.dispatcher._checkResult(Indigo._lib.indigoGrossFormula(self.id))
        gf = self.dispatcher.IndigoObject(self.dispatcher, gfid)
        return self.dispatcher._checkResultString(Indigo._lib.indigoToString(gf.id))

    def toString(self):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResultString(Indigo._lib.indigoToString(self.id))

    def mdlct(self):
        self.dispatcher._setSessionId()
        buf = self.dispatcher.writeBuffer()
        self.dispatcher._checkResult(Indigo._lib.indigoSaveMDLCT(self.id, buf.id))
        return buf.toBuffer()

    def xyz (self):
        self.dispatcher._setSessionId()
        xyz = Indigo._lib.indigoXYZ(self.id)
        if xyz is None:
            raise IndigoException(Indigo._lib.indigoGetLastError())
        return [xyz[0], xyz[1], xyz[2]]

    def setXYZ (self, x, y, z):
        self.dispatcher._setSessionId()
        self.dispatcher._checkResult(Indigo._lib.indigoSetXYZ(self.id, x, y, z))

    def alignAtoms (self, atom_ids, desired_xyz):
        self.dispatcher._setSessionId()
        if len(atom_ids) * 3 != len(desired_xyz):
            raise IndigoException("alignAtoms(): desired_xyz[] must be exactly 3 times bigger than atom_ids[]")
        atoms = (c_int * len(atom_ids))()
        for i in range(len(atoms)):
            atoms[i] = atom_ids[i]
        xyz = (c_float * len(desired_xyz))()
        for i in range(len(desired_xyz)):
            xyz[i] = desired_xyz[i]
        return self.dispatcher._checkResultFloat(
            self.dispatcher._lib.indigoAlignAtoms(self.id, len(atoms), atoms, xyz))

    def addStereocenter(self, type, v1, v2, v3, v4=-1):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoAddStereocenter(self.id, type, v1, v2, v3, v4))

    def clone(self):
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(self.dispatcher, self.dispatcher._checkResult(Indigo._lib.indigoClone(self.id)))

    def close(self):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoClose(self.id))

    def _next(self):
        self.dispatcher._setSessionId()
        newobj = self.dispatcher._checkResult(Indigo._lib.indigoNext(self.id))
        if newobj == 0:
            return None
        else:
            return self.dispatcher.IndigoObject(self.dispatcher, newobj, self)

    def hasNext(self):
        self.dispatcher._setSessionId()
        return bool(self.dispatcher._checkResult(Indigo._lib.indigoHasNext(self.id)))

    def index(self):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoIndex(self.id))

    def remove(self):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoRemove(self.id))

    def saveMolfile(self, filename):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoSaveMolfileToFile(self.id, filename.encode('ascii')))

    def molfile(self):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResultString(Indigo._lib.indigoMolfile(self.id))

    def saveCml(self, filename):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoSaveCmlToFile(self.id, filename.encode('ascii')))

    def cml(self):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResultString(Indigo._lib.indigoCml(self.id))

    def saveMDLCT(self, output):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoSaveMDLCT(self.id, output.id))

    def addReactant(self, molecule):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoAddReactant(self.id, molecule.id))

    def addProduct(self, molecule):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoAddProduct(self.id, molecule.id))

    def addCatalyst(self, molecule):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoAddCatalyst(self.id, molecule.id))

    def countReactants(self):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoCountReactants(self.id))

    def countProducts(self):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoCountProducts(self.id))

    def countCatalysts(self):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoCountCatalysts(self.id))

    def countMolecules(self):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoCountMolecules(self.id))

    def getMolecule(self, index):
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(self.dispatcher, self.dispatcher._checkResult(Indigo._lib.indigoGetMolecule(self.id, index)))

    def iterateReactants(self):
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(self.dispatcher, self.dispatcher._checkResult(Indigo._lib.indigoIterateReactants(self.id)))

    def iterateProducts(self):
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(self.dispatcher, self.dispatcher._checkResult(Indigo._lib.indigoIterateProducts(self.id)))

    def iterateCatalysts(self):
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(self.dispatcher, self.dispatcher._checkResult(Indigo._lib.indigoIterateCatalysts(self.id)))

    def iterateMolecules(self):
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(self.dispatcher, self.dispatcher._checkResult(Indigo._lib.indigoIterateMolecules(self.id)))

    def saveRxnfile(self, filename):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoSaveRxnfileToFile(self.id, filename.encode('ascii')))

    def rxnfile(self):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResultString(Indigo._lib.indigoRxnfile(self.id))

    def optimize(self, options=''):
        self.dispatcher._setSessionId()
        options = '' if options is None else options
        return self.dispatcher._checkResult(Indigo._lib.indigoOptimize(self.id, options.encode('ascii')))

    def normalize(self, options=''):
        self.dispatcher._setSessionId()
        options = '' if options is None else options
        return bool(self.dispatcher._checkResult(Indigo._lib.indigoNormalize(self.id, options.encode('ascii'))))

    def automap(self, mode=''):
        self.dispatcher._setSessionId()
        mode = '' if mode is None else mode
        return self.dispatcher._checkResult(Indigo._lib.indigoAutomap(self.id, mode.encode('ascii')))

    def atomMappingNumber(self, reaction_atom):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoGetAtomMappingNumber(self.id, reaction_atom.id))

    def setAtomMappingNumber(self, reaction_atom, number):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoSetAtomMappingNumber(self.id, reaction_atom.id, number))

    def reactingCenter(self, reaction_bond):
        self.dispatcher._setSessionId()
        value = c_int()
        res = self.dispatcher._checkResult(Indigo._lib.indigoGetReactingCenter(self.id, reaction_bond.id, pointer(value)))
        if res == 0:
            return None
        return value.value

    def setReactingCenter(self, reaction_bond, rc):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoSetReactingCenter(self.id, reaction_bond.id, rc))

    def clearAAM(self):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoClearAAM(self.id))

    def correctReactingCenters(self):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoCorrectReactingCenters(self.id))

    def iterateAtoms(self):
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(self.dispatcher, self.dispatcher._checkResult(Indigo._lib.indigoIterateAtoms(self.id)))

    def iteratePseudoatoms(self):
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(self.dispatcher, self.dispatcher._checkResult(Indigo._lib.indigoIteratePseudoatoms(self.id)))

    def iterateRSites(self):
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(self.dispatcher, self.dispatcher._checkResult(Indigo._lib.indigoIterateRSites(self.id)))

    def iterateStereocenters(self):
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(self.dispatcher, self.dispatcher._checkResult(Indigo._lib.indigoIterateStereocenters(self.id)))

    def iterateAlleneCenters(self):
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(self.dispatcher, self.dispatcher._checkResult(Indigo._lib.indigoIterateAlleneCenters(self.id)))

    def iterateRGroups(self):
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(self.dispatcher, self.dispatcher._checkResult(Indigo._lib.indigoIterateRGroups(self.id)))

    def isPseudoatom(self):
        self.dispatcher._setSessionId()
        return bool(self.dispatcher._checkResult(Indigo._lib.indigoIsPseudoatom(self.id)))

    def isRSite(self):
        self.dispatcher._setSessionId()
        return bool(self.dispatcher._checkResult(Indigo._lib.indigoIsRSite(self.id)))

    def stereocenterType(self):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoStereocenterType(self.id))

    def stereocenterGroup(self):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoStereocenterGroup(self.id))

    def setStereocenterGroup(self, group):
        self.dispatcher._setSessionId()
        self.dispatcher._checkResult(Indigo._lib.indigoSetStereocenterGroup(self.id, group))

    def changeStereocenterType(self, type):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoChangeStereocenterType(self.id, type))

    def validateChirality(self):
        self.dispatcher._setSessionId()
        self.dispatcher._checkResult(Indigo._lib.indigoValidateChirality(self.id))

    def singleAllowedRGroup(self):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoSingleAllowedRGroup(self.id))

    def iterateRGroupFragments(self):
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(self.dispatcher, self.dispatcher._checkResult(Indigo._lib.indigoIterateRGroupFragments(self.id)))

    def countAttachmentPoints(self):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoCountAttachmentPoints(self.id))

    def iterateAttachmentPoints(self, order):
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(self.dispatcher, self.dispatcher._checkResult(Indigo._lib.indigoIterateAttachmentPoints(self.id, order)))

    def symbol(self):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResultString(Indigo._lib.indigoSymbol(self.id))

    def degree(self):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoDegree(self.id))

    def charge(self):
        self.dispatcher._setSessionId()
        value = c_int()
        res = self.dispatcher._checkResult(Indigo._lib.indigoGetCharge(self.id, pointer(value)))
        if res == 0:
            return None
        return value.value

    def getExplicitValence(self):
        self.dispatcher._setSessionId()
        value = c_int()
        res = self.dispatcher._checkResult(Indigo._lib.indigoGetExplicitValence(self.id, pointer(value)))
        if res == 0:
            return None
        return value.value

    def setExplicitValence(self, valence):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoSetExplicitValence(self.id, valence))

    def radicalElectrons(self):
        self.dispatcher._setSessionId()
        value = c_int()
        res = self.dispatcher._checkResult(Indigo._lib.indigoGetRadicalElectrons(self.id, pointer(value)))
        if res == 0:
            return None
        return value.value

    def radical(self):
        self.dispatcher._setSessionId()
        value = c_int()
        res = self.dispatcher._checkResult(Indigo._lib.indigoGetRadical(self.id, pointer(value)))
        if res == 0:
            return None
        return value.value

    def setRadical(self, radical):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoSetRadical(self.id, radical))

    def atomicNumber(self):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoAtomicNumber(self.id))

    def isotope(self):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoIsotope(self.id))

    def valence(self):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoValence(self.id))

    def countHydrogens(self):
        self.dispatcher._setSessionId()
        value = c_int()
        res = self.dispatcher._checkResult(Indigo._lib.indigoCountHydrogens(self.id, pointer(value)))
        if res == 0:
            return None
        return value.value

    def countImplicitHydrogens(self):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoCountImplicitHydrogens(self.id))

    def setXYZ(self, x, y, z):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoSetXYZ(self.id, x, y, z))

    def countSuperatoms(self):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoCountSuperatoms(self.id))

    def countDataSGroups(self):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoCountDataSGroups(self.id))

    def countRepeatingUnits(self):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoCountRepeatingUnits(self.id))

    def countMultipleGroups(self):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoCountMultipleGroups(self.id))

    def countGenericSGroups(self):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoCountGenericSGroups(self.id))

    def iterateDataSGroups(self):
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(self.dispatcher, self.dispatcher._checkResult(Indigo._lib.indigoIterateDataSGroups(self.id)))

    def iterateSuperatoms(self):
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(self.dispatcher, self.dispatcher._checkResult(Indigo._lib.indigoIterateSuperatoms(self.id)))

    def iterateGenericSGroups(self):
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(self.dispatcher, self.dispatcher._checkResult(Indigo._lib.indigoIterateGenericSGroups(self.id)))

    def iterateRepeatingUnits(self):
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(self.dispatcher, self.dispatcher._checkResult(Indigo._lib.indigoIterateRepeatingUnits(self.id)))

    def iterateMultipleGroups(self):
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(self.dispatcher, self.dispatcher._checkResult(Indigo._lib.indigoIterateMultipleGroups(self.id)))

    def getSuperatom(self, index):
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(self.dispatcher, self.dispatcher._checkResult(Indigo._lib.indigoGetSuperatom(self.id, index)))

    def getDataSGroup(self, index):
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(self.dispatcher, self.dispatcher._checkResult(Indigo._lib.indigoGetDataSGroup(self.id, index)))

    def getGenericSGroup(self, index):
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(self.dispatcher, self.dispatcher._checkResult(Indigo._lib.indigoGetGenericSGroup(self.id, index)))

    def getMultipleGroup(self, index):
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(self.dispatcher, self.dispatcher._checkResult(Indigo._lib.indigoGetMultipleGroup(self.id, index)))

    def getRepeatingUnit(self, index):
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(self.dispatcher, self.dispatcher._checkResult(Indigo._lib.indigoGetRepeatingUnit(self.id, index)))

    def description(self):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResultString(Indigo._lib.indigoDescription(self.id))

    def data(self):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResultString(Indigo._lib.indigoData(self.id))

    def addDataSGroup(self, atoms, bonds, description, data):
        self.dispatcher._setSessionId()
        arr2 = (c_int * len(atoms))()
        for i in range(len(atoms)):
            arr2[i] = atoms[i]
        arr4 = (c_int * len(bonds))()
        for i in range(len(bonds)):
            arr4[i] = bonds[i]
        return self.dispatcher.IndigoObject(self.dispatcher, self.dispatcher._checkResult(Indigo._lib.indigoAddDataSGroup(self.id, len(arr2), arr2, len(arr4), arr4, description.encode('ascii'), data.encode('ascii'))))

    def addSuperatom(self, atoms, name):
        self.dispatcher._setSessionId()
        arr2 = (c_int * len(atoms))()
        for i in range(len(atoms)):
            arr2[i] = atoms[i]
        return self.dispatcher.IndigoObject(self.dispatcher, self.dispatcher._checkResult(Indigo._lib.indigoAddSuperatom(self.id, len(arr2), arr2, name.encode('ascii'))))

    def setDataSGroupXY(self, x, y, options=''):
        self.dispatcher._setSessionId()
        options = '' if options is None else options
        return self.dispatcher._checkResult(Indigo._lib.indigoSetDataSGroupXY(self.id, x, y, options.encode('ascii')))

    def resetCharge(self):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoResetCharge(self.id))

    def resetExplicitValence(self):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoResetExplicitValence(self.id))

    def resetRadical(self):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoResetRadical(self.id))

    def resetIsotope(self):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoResetIsotope(self.id))

    def setAttachmentPoint(self, order):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoSetAttachmentPoint(self.id, order))

    def clearAttachmentPoints(self):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoClearAttachmentPoints(self.id))

    def removeConstraints(self, type):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoRemoveConstraints(self.id, type.encode('ascii')))

    def addConstraint(self, type, value):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoAddConstraint(self.id, type.encode('ascii'), value.encode('ascii')))

    def addConstraintNot(self, type, value):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoAddConstraintNot(self.id, type.encode('ascii'), value.encode('ascii')))

    def addConstraintOr(self, type, value):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoAddConstraintOr(self.id, type.encode('ascii'), value.encode('ascii')))

    def resetStereo(self):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoResetStereo(self.id))

    def invertStereo(self):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoInvertStereo(self.id))

    def countAtoms(self):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoCountAtoms(self.id))

    def countBonds(self):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoCountBonds(self.id))

    def countPseudoatoms(self):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoCountPseudoatoms(self.id))

    def countRSites(self):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoCountRSites(self.id))

    def iterateBonds(self):
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(self.dispatcher, self.dispatcher._checkResult(Indigo._lib.indigoIterateBonds(self.id)))

    def bondOrder(self):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoBondOrder(self.id))

    def bondStereo(self):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoBondStereo(self.id))

    def topology(self):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoTopology(self.id))

    def iterateNeighbors(self):
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(self.dispatcher, self.dispatcher._checkResult(Indigo._lib.indigoIterateNeighbors(self.id)))

    def bond(self):
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(self.dispatcher, self.dispatcher._checkResult(Indigo._lib.indigoBond(self.id)))

    def getAtom(self, idx):
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(self.dispatcher, self.dispatcher._checkResult(Indigo._lib.indigoGetAtom(self.id, idx)))

    def getBond(self, idx):
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(self.dispatcher, self.dispatcher._checkResult(Indigo._lib.indigoGetBond(self.id, idx)))

    def source(self):
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(self.dispatcher, self.dispatcher._checkResult(Indigo._lib.indigoSource(self.id)))

    def destination(self):
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(self.dispatcher, self.dispatcher._checkResult(Indigo._lib.indigoDestination(self.id)))

    def clearCisTrans(self):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoClearCisTrans(self.id))

    def clearStereocenters(self):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoClearStereocenters(self.id))

    def countStereocenters(self):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoCountStereocenters(self.id))

    def clearAlleneCenters(self):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoClearAlleneCenters(self.id))

    def countAlleneCenters(self):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoCountAlleneCenters(self.id))

    def resetSymmetricCisTrans(self):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoResetSymmetricCisTrans(self.id))

    def resetSymmetricStereocenters(self):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoResetSymmetricStereocenters(self.id))

    def markEitherCisTrans(self):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoMarkEitherCisTrans(self.id))

    def markStereobonds(self):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoMarkStereobonds(self.id))

    def addAtom(self, symbol):
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(self.dispatcher, self.dispatcher._checkResult(Indigo._lib.indigoAddAtom(self.id, symbol.encode('ascii'))))

    def resetAtom(self, symbol):
        self.dispatcher._setSessionId()
        self.dispatcher._checkResult(Indigo._lib.indigoResetAtom(self.id, symbol.encode('ascii')))

    def addRSite(self, name):
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(self.dispatcher, self.dispatcher._checkResult(Indigo._lib.indigoAddRSite(self.id, name.encode('ascii'))))

    def setRSite(self, name):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoSetRSite(self.id, name.encode('ascii')))

    def setCharge(self, charge):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoSetCharge(self.id, charge))

    def setIsotope(self, isotope):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoSetIsotope(self.id, isotope))

    def setImplicitHCount(self, impl_h):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoSetImplicitHCount(self.id, impl_h))

    def addBond(self, destination, order):
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(self.dispatcher, self.dispatcher._checkResult(Indigo._lib.indigoAddBond(self.id, destination.id, order)))

    def setBondOrder(self, order):
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(self.dispatcher, self.dispatcher._checkResult(Indigo._lib.indigoSetBondOrder(self.id, order)))

    def merge(self, what):
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(self.dispatcher, self.dispatcher._checkResult(Indigo._lib.indigoMerge(self.id, what.id)))

    def highlight(self):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoHighlight(self.id))

    def unhighlight(self):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoUnhighlight(self.id))

    def isHighlighted(self):
        self.dispatcher._setSessionId()
        return bool(self.dispatcher._checkResult(Indigo._lib.indigoIsHighlighted(self.id)))

    def countComponents(self):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoCountComponents(self.id))

    def componentIndex(self):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoComponentIndex(self.id))

    def iterateComponents(self):
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(self.dispatcher, self.dispatcher._checkResult(Indigo._lib.indigoIterateComponents(self.id)))

    def component(self, index):
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(self.dispatcher, self.dispatcher._checkResult(Indigo._lib.indigoComponent(self.id, index)))

    def countSSSR(self):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoCountSSSR(self.id))

    def iterateSSSR(self):
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(self.dispatcher, self.dispatcher._checkResult(Indigo._lib.indigoIterateSSSR(self.id)))

    def iterateSubtrees(self, min_atoms, max_atoms):
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(self.dispatcher, self.dispatcher._checkResult(Indigo._lib.indigoIterateSubtrees(self.id, min_atoms, max_atoms)))

    def iterateRings(self, min_atoms, max_atoms):
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(self.dispatcher, self.dispatcher._checkResult(Indigo._lib.indigoIterateRings(self.id, min_atoms, max_atoms)))

    def iterateEdgeSubmolecules(self, min_bonds, max_bonds):
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(self.dispatcher, self.dispatcher._checkResult(Indigo._lib.indigoIterateEdgeSubmolecules(self.id, min_bonds, max_bonds)))

    def countHeavyAtoms(self):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoCountHeavyAtoms(self.id))


    def molecularWeight(self):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResultFloat(Indigo._lib.indigoMolecularWeight(self.id))

    def mostAbundantMass(self):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResultFloat(Indigo._lib.indigoMostAbundantMass(self.id))

    def monoisotopicMass(self):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResultFloat(Indigo._lib.indigoMonoisotopicMass(self.id))

    def canonicalSmiles(self):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResultString(Indigo._lib.indigoCanonicalSmiles(self.id))

    def layeredCode(self):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResultString(Indigo._lib.indigoLayeredCode(self.id))

    def symmetryClasses(self):
        self.dispatcher._setSessionId()
        c_size = c_int()
        c_buf = self.dispatcher._checkResultPtr(Indigo._lib.indigoSymmetryClasses(self.id, pointer(c_size)))
        res = array("i")
        for i in range(c_size.value):
            res.append(c_buf[i])
        return res

    def hasCoord(self):
        self.dispatcher._setSessionId()
        return bool(self.dispatcher._checkResult(Indigo._lib.indigoHasCoord(self.id)))

    def hasZCoord(self):
        self.dispatcher._setSessionId()
        return bool(self.dispatcher._checkResult(Indigo._lib.indigoHasZCoord(self.id)))

    def isChiral(self):
        self.dispatcher._setSessionId()
        return bool(self.dispatcher._checkResult(Indigo._lib.indigoIsChiral(self.id)))

    def createSubmolecule(self, vertices):
        self.dispatcher._setSessionId()
        arr2 = (c_int * len(vertices))()
        for i in range(len(vertices)):
            arr2[i] = vertices[i]
        return self.dispatcher.IndigoObject(self.dispatcher, self.dispatcher._checkResult(Indigo._lib.indigoCreateSubmolecule(self.id, len(arr2), arr2)))

    def createEdgeSubmolecule(self, vertices, edges):
        self.dispatcher._setSessionId()
        arr2 = (c_int * len(vertices))()
        for i in range(len(vertices)):
            arr2[i] = vertices[i]
        arr4 = (c_int * len(edges))()
        for i in range(len(edges)):
            arr4[i] = edges[i]
        return self.dispatcher.IndigoObject(self.dispatcher, self.dispatcher._checkResult(Indigo._lib.indigoCreateEdgeSubmolecule(self.id, len(arr2), arr2, len(arr4), arr4)))

    def getSubmolecule(self, vertices):
        self.dispatcher._setSessionId()
        arr2 = (c_int * len(vertices))()
        for i in range(len(vertices)):
            arr2[i] = vertices[i]
        return self.dispatcher.IndigoObject(self.dispatcher, self.dispatcher._checkResult(Indigo._lib.indigoGetSubmolecule(self.id, len(arr2), arr2)))

    def removeAtoms(self, vertices):
        self.dispatcher._setSessionId()
        arr2 = (c_int * len(vertices))()
        for i in range(len(vertices)):
            arr2[i] = vertices[i]
        return self.dispatcher._checkResult(Indigo._lib.indigoRemoveAtoms(self.id, len(arr2), arr2))

    def removeBonds(self, bonds):
        self.dispatcher._setSessionId()
        arr2 = (c_int * len(bonds))()
        for i in range(len(bonds)):
            arr2[i] = bonds[i]
        return self.dispatcher._checkResult(Indigo._lib.indigoRemoveBonds(self.id, len(arr2), arr2))

    def aromatize(self):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoAromatize(self.id))

    def dearomatize(self):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoDearomatize(self.id))

    def foldHydrogens(self):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoFoldHydrogens(self.id))

    def unfoldHydrogens(self):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoUnfoldHydrogens(self.id))

    def layout(self):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoLayout(self.id))

    def smiles(self):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResultString(Indigo._lib.indigoSmiles(self.id))

    def name(self):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResultString(Indigo._lib.indigoName(self.id))

    def setName(self, name):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoSetName(self.id, name.encode('ascii')))

    def serialize(self):
        self.dispatcher._setSessionId()
        c_size = c_int()
        c_buf = POINTER(c_byte)()
        self.dispatcher._checkResult(Indigo._lib.indigoSerialize(self.id, pointer(c_buf), pointer(c_size)))
        res = array('b')
        for i in range(c_size.value):
            res.append(c_buf[i])
        return res

    def hasProperty(self, prop):
        self.dispatcher._setSessionId()
        return bool(self.dispatcher._checkResult(Indigo._lib.indigoHasProperty(self.id, prop)))

    def getProperty(self, prop):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResultString(Indigo._lib.indigoGetProperty(self.id, prop.encode('ascii')))

    def setProperty(self, prop, value):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoSetProperty(self.id, prop.encode('ascii'), value.encode('ascii')))

    def removeProperty(self, prop):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoRemoveProperty(self.id, prop))

    def iterateProperties(self):
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(self.dispatcher, self.dispatcher._checkResult(Indigo._lib.indigoIterateProperties(self.id)))

    def clearProperties(self):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoClearProperties(self.id))

    def checkBadValence(self):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResultString(Indigo._lib.indigoCheckBadValence(self.id))

    def checkAmbiguousH(self):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResultString(Indigo._lib.indigoCheckAmbiguousH(self.id))

    def fingerprint(self, type):
        self.dispatcher._setSessionId()
        newobj = self.dispatcher._checkResult(Indigo._lib.indigoFingerprint(self.id, type.encode('ascii')))
        if newobj == 0:
            return None
        return self.dispatcher.IndigoObject(self.dispatcher, newobj, self)

    def countBits(self):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoCountBits(self.id))

    def rawData(self):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResultString(Indigo._lib.indigoRawData(self.id))

    def tell(self):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoTell(self.id))

    def sdfAppend(self, item):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoSdfAppend(self.id, item.id))

    def smilesAppend(self, item):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoSmilesAppend(self.id, item.id))

    def rdfHeader(self):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoRdfHeader(self.id))

    def rdfAppend(self, item):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoRdfAppend(self.id, item.id))

    def cmlHeader(self):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoCmlHeader(self.id))

    def cmlAppend(self, item):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoCmlAppend(self.id, item.id))

    def cmlFooter(self):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoCmlFooter(self.id))

    def append(self, object):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoAppend(self.id, object.id))

    def arrayAdd(self, object):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoArrayAdd(self.id, object.id))

    def at(self, index):
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(self.dispatcher, self.dispatcher._checkResult(Indigo._lib.indigoAt(self.id, index)))

    def count(self):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoCount(self.id))

    def clear(self):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoClear(self.id))

    def iterateArray(self):
        self.dispatcher._setSessionId()
        newobj = self.dispatcher._checkResult(Indigo._lib.indigoIterateArray(self.id))
        if newobj == 0:
            return None
        else:
            return self.dispatcher.IndigoObject(self.dispatcher, newobj, self)

    def ignoreAtom(self, atom_object):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoIgnoreAtom(self.id, atom_object.id))

    def unignoreAtom(self, atom_object):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoUnignoreAtom(self.id, atom_object.id))

    def unignoreAllAtoms(self):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoUnignoreAllAtoms(self.id))

    def match(self, query):
        self.dispatcher._setSessionId()
        newobj = self.dispatcher._checkResult(Indigo._lib.indigoMatch(self.id, query.id))
        if newobj == 0:
            return None
        else:
            return self.dispatcher.IndigoObject(self.dispatcher, newobj, self)

    def countMatches(self, query):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoCountMatches(self.id, query.id))

    def countMatchesWithLimit(self, query, embeddings_limit):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoCountMatchesWithLimit(self.id, query.id, embeddings_limit))

    def iterateMatches(self, query):
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(self.dispatcher, self.dispatcher._checkResult(Indigo._lib.indigoIterateMatches(self.id, query.id)))

    def highlightedTarget(self):
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(self.dispatcher, self.dispatcher._checkResult(Indigo._lib.indigoHighlightedTarget(self.id)))

    def mapAtom(self, atom):
        self.dispatcher._setSessionId()
        newobj = self.dispatcher._checkResult(Indigo._lib.indigoMapAtom(self.id, atom.id))
        if newobj == 0:
            return None
        else:
            return self.dispatcher.IndigoObject(self.dispatcher, newobj, self)

    def mapBond(self, bond):
        self.dispatcher._setSessionId()
        newobj = self.dispatcher._checkResult(Indigo._lib.indigoMapBond(self.id, bond.id))
        if newobj == 0:
            return None
        else:
            return self.dispatcher.IndigoObject(self.dispatcher, newobj, self)

    def mapMolecule(self, molecule):
        self.dispatcher._setSessionId()
        newobj = self.dispatcher._checkResult(Indigo._lib.indigoMapMolecule(self.id, molecule.id))
        if newobj == 0:
            return None
        else:
            return self.dispatcher.IndigoObject(self.dispatcher, newobj, self)

    def allScaffolds(self):
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(self.dispatcher, self.dispatcher._checkResult(Indigo._lib.indigoAllScaffolds(self.id)))

    def decomposedMoleculeScaffold(self):
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(self.dispatcher, self.dispatcher._checkResult(Indigo._lib.indigoDecomposedMoleculeScaffold(self.id)))

    def iterateDecomposedMolecules(self):
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(self.dispatcher, self.dispatcher._checkResult(Indigo._lib.indigoIterateDecomposedMolecules(self.id)))

    def decomposedMoleculeHighlighted(self):
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(self.dispatcher, self.dispatcher._checkResult(Indigo._lib.indigoDecomposedMoleculeHighlighted(self.id)))

    def decomposedMoleculeWithRGroups(self):
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(self.dispatcher, self.dispatcher._checkResult(Indigo._lib.indigoDecomposedMoleculeWithRGroups(self.id)))

    def decomposeMolecule(self, mol):
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(self.dispatcher, self.dispatcher._checkResult(Indigo._lib.indigoDecomposeMolecule(self.id, mol.id)))

    def iterateDecompositions(self):
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(self.dispatcher, self.dispatcher._checkResult(Indigo._lib.indigoIterateDecompositions(self.id)))

    def addDecomposition(self, q_match):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoAddDecomposition(self.id, q_match.id))

    def toString(self):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResultString(Indigo._lib.indigoToString(self.id))

    def toBuffer(self):
        self.dispatcher._setSessionId()
        c_size = c_int()
        c_buf = POINTER(c_byte)()
        self.dispatcher._checkResult(Indigo._lib.indigoToBuffer(self.id, pointer(c_buf), pointer(c_size)))
        res = array("b")
        for i in range(c_size.value):
            res.append(c_buf[i])
        return res

    def stereocenterPyramid(self):
        ptr = self.dispatcher._checkResultPtr(Indigo._lib.indigoStereocenterPyramid(self.id))
        res = [0] * 4
        for i in range(4):
            res[i] = ptr[i]
        return res

    def expandAbbreviations(self):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoExpandAbbreviations(self.id))

    def dbgInternalType(self):
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResultString(Indigo._lib.indigoDbgInternalType(self.id))


class Indigo(object):
    ABS = 1
    OR = 2
    AND = 3
    EITHER = 4
    UP = 5
    DOWN = 6
    CIS = 7
    TRANS = 8
    CHAIN = 9
    RING = 10
    ALLENE = 11

    SINGLET = 101
    DOUBLET = 102
    TRIPLET = 103
    RC_NOT_CENTER = -1
    RC_UNMARKED = 0
    RC_CENTER = 1
    RC_UNCHANGED = 2
    RC_MADE_OR_BROKEN = 4
    RC_ORDER_CHANGED = 8

    _crt = None
    _crtp = None
    _lib = None

    # Python embeds path into .pyc code if method is marked with @staticmethod
    # This causes an error when Indigo is loaded from different places by relative path
    def _initStatic (self, path = None):
        if not path:
            cur_file = os.path.abspath(__file__)
            dirname = os.path.dirname(cur_file)
            if not dirname:
                dirname = '.'
            path = dirname + '/lib'
        if os.name == 'posix' and not platform.mac_ver()[0]:
            arch = platform.architecture()[0]
            path += "/Linux"
            if arch == '32bit':
                path += "/x86"
            elif arch == '64bit':
                path += "/x64"
            else:
                raise IndigoException("unknown platform " + arch)
            Indigo._lib = CDLL(path + "/libindigo.so", mode=RTLD_GLOBAL)
        elif os.name == 'nt':
            arch = platform.architecture()[0]
            path += "/Win"
            if arch == '32bit':
                path += "/x86"
            elif arch == '64bit':
                path += "/x64"
            else:
                raise IndigoException("unknown platform " + arch)
            if os.path.exists(path + "/msvcr100.dll"):
                Indigo._crt = CDLL(path + "/msvcr100.dll")
                Indigo._crtp = CDLL(path + "/msvcp100.dll")
            if os.path.exists(path + "/msvcr110.dll"):
                Indigo._crt = CDLL(path + "/msvcr110.dll")
                Indigo._crtp = CDLL(path + "/msvcp110.dll")
            if os.path.exists(path + "/msvcr120.dll"):
                Indigo._crt = CDLL(path + "/msvcr120.dll")
                Indigo._crtp = CDLL(path + "/msvcp120.dll")
            Indigo._lib = CDLL(path + "/indigo.dll")
        elif platform.mac_ver()[0]:
            path += "/Mac/"
            mac_ver = '.'.join(platform.mac_ver()[0].split('.')[:2])
            current_mac_ver = int(mac_ver.split('.')[1])
            using_mac_ver = None
            for version in reversed(range(5, current_mac_ver + 1)):
                if os.path.exists(path + '/10.' + str(version)):
                    using_mac_ver = str(version)
                    break
            if (using_mac_ver is None):
                raise IndigoException("no binaries for this version of Mac OS X: " + mac_ver)
            path += '/10.' + using_mac_ver
            Indigo._lib = CDLL(path + "/libindigo.dylib", mode=RTLD_GLOBAL)
        else:
            raise IndigoException("unsupported OS: " + os.name)
        Indigo.dllpath = path

    def _setSessionId (self):
        Indigo._lib.indigoSetSessionId(self._sid)

    def __init__ (self, path=None):
        if Indigo._lib is None:
            self._initStatic(path)
        self._sid = Indigo._lib.indigoAllocSessionId()
        # Capture a reference to the _lib to access it in the __del__ method because
        # at interpreter shutdown, the module's global variables are set to None
        self._lib = Indigo._lib
        self._setSessionId()
        self.IndigoObject = IndigoObject
        Indigo._lib.indigoVersion.restype = c_char_p
        Indigo._lib.indigoVersion.argtypes = None
        Indigo._lib.indigoAllocSessionId.restype = c_ulonglong
        Indigo._lib.indigoAllocSessionId.argtypes = None
        Indigo._lib.indigoSetSessionId.restype = None
        Indigo._lib.indigoSetSessionId.argtypes = [c_ulonglong]
        Indigo._lib.indigoReleaseSessionId.restype = None
        Indigo._lib.indigoReleaseSessionId.argtypes = [c_ulonglong]
        Indigo._lib.indigoGetLastError.restype = c_char_p
        Indigo._lib.indigoGetLastError.argtypes = None
        Indigo._lib.indigoFree.restype = c_int
        Indigo._lib.indigoFree.argtypes = [c_int]
        Indigo._lib.indigoCountReferences.restype = c_int
        Indigo._lib.indigoCountReferences.argtypes = None
        Indigo._lib.indigoFreeAllObjects.restype = c_int
        Indigo._lib.indigoFreeAllObjects.argtypes = None
        Indigo._lib.indigoSetOption.restype = c_int
        Indigo._lib.indigoSetOption.argtypes = [c_char_p, c_char_p]
        Indigo._lib.indigoSetOptionInt.restype = c_int
        Indigo._lib.indigoSetOptionInt.argtypes = [c_char_p, c_int]
        Indigo._lib.indigoSetOptionBool.restype = c_int
        Indigo._lib.indigoSetOptionBool.argtypes = [c_char_p, c_int]
        Indigo._lib.indigoSetOptionFloat.restype = c_int
        Indigo._lib.indigoSetOptionFloat.argtypes = [c_char_p, c_float]
        Indigo._lib.indigoSetOptionColor.restype = c_int
        Indigo._lib.indigoSetOptionColor.argtypes = [c_char_p, c_float, c_float, c_float]
        Indigo._lib.indigoSetOptionXY.restype = c_int
        Indigo._lib.indigoSetOptionXY.argtypes = [c_char_p, c_int, c_int]
        Indigo._lib.indigoReadFile.restype = c_int
        Indigo._lib.indigoReadFile.argtypes = [c_char_p]
        Indigo._lib.indigoLoadString.restype = c_int
        Indigo._lib.indigoLoadString.argtypes = [c_char_p]
        Indigo._lib.indigoLoadBuffer.restype = c_int
        Indigo._lib.indigoLoadBuffer.argtypes = [POINTER(c_byte), c_int]
        Indigo._lib.indigoWriteFile.restype = c_int
        Indigo._lib.indigoWriteFile.argtypes = [c_char_p]
        Indigo._lib.indigoWriteBuffer.restype = c_int
        Indigo._lib.indigoWriteBuffer.argtypes = None
        Indigo._lib.indigoCreateMolecule.restype = c_int
        Indigo._lib.indigoCreateMolecule.argtypes = None
        Indigo._lib.indigoCreateQueryMolecule.restype = c_int
        Indigo._lib.indigoCreateQueryMolecule.argtypes = None
        Indigo._lib.indigoLoadMoleculeFromString.restype = c_int
        Indigo._lib.indigoLoadMoleculeFromString.argtypes = [c_char_p]
        Indigo._lib.indigoLoadMoleculeFromFile.restype = c_int
        Indigo._lib.indigoLoadMoleculeFromFile.argtypes = [c_char_p]
        Indigo._lib.indigoLoadQueryMoleculeFromString.restype = c_int
        Indigo._lib.indigoLoadQueryMoleculeFromString.argtypes = [c_char_p]
        Indigo._lib.indigoLoadQueryMoleculeFromFile.restype = c_int
        Indigo._lib.indigoLoadQueryMoleculeFromFile.argtypes = [c_char_p]
        Indigo._lib.indigoLoadSmartsFromString.restype = c_int
        Indigo._lib.indigoLoadSmartsFromString.argtypes = [c_char_p]
        Indigo._lib.indigoLoadSmartsFromFile.restype = c_int
        Indigo._lib.indigoLoadSmartsFromFile.argtypes = [c_char_p]
        Indigo._lib.indigoLoadReactionFromString.restype = c_int
        Indigo._lib.indigoLoadReactionFromString.argtypes = [c_char_p]
        Indigo._lib.indigoLoadReactionFromFile.restype = c_int
        Indigo._lib.indigoLoadReactionFromFile.argtypes = [c_char_p]
        Indigo._lib.indigoLoadQueryReactionFromString.restype = c_int
        Indigo._lib.indigoLoadQueryReactionFromString.argtypes = [c_char_p]
        Indigo._lib.indigoLoadQueryReactionFromFile.restype = c_int
        Indigo._lib.indigoLoadQueryReactionFromFile.argtypes = [c_char_p]
        Indigo._lib.indigoLoadReactionSmartsFromString.restype = c_int
        Indigo._lib.indigoLoadReactionSmartsFromString.argtypes = [c_char_p]
        Indigo._lib.indigoLoadReactionSmartsFromFile.restype = c_int
        Indigo._lib.indigoLoadReactionSmartsFromFile.argtypes = [c_char_p]
        Indigo._lib.indigoCreateReaction.restype = c_int
        Indigo._lib.indigoCreateReaction.argtypes = None
        Indigo._lib.indigoCreateQueryReaction.restype = c_int
        Indigo._lib.indigoCreateQueryReaction.argtypes = None
        Indigo._lib.indigoExactMatch.restype = c_int
        Indigo._lib.indigoExactMatch.argtypes = [c_int, c_int, c_char_p]
        Indigo._lib.indigoSetTautomerRule.restype = c_int
        Indigo._lib.indigoSetTautomerRule.argtypes = [c_int, c_char_p, c_char_p]
        Indigo._lib.indigoRemoveTautomerRule.restype = c_int
        Indigo._lib.indigoRemoveTautomerRule.argtypes = [c_int]
        Indigo._lib.indigoClearTautomerRules.restype = c_int
        Indigo._lib.indigoClearTautomerRules.argtypes = None
        Indigo._lib.indigoUnserialize.restype = c_int
        Indigo._lib.indigoUnserialize.argtypes = [POINTER(c_byte), c_int]
        Indigo._lib.indigoCommonBits.restype = c_int
        Indigo._lib.indigoCommonBits.argtypes = [c_int, c_int]
        Indigo._lib.indigoSimilarity.restype = c_float
        Indigo._lib.indigoSimilarity.argtypes = [c_int, c_int, c_char_p]
        Indigo._lib.indigoIterateSDF.restype = c_int
        Indigo._lib.indigoIterateSDF.argtypes = [c_int]
        Indigo._lib.indigoIterateRDF.restype = c_int
        Indigo._lib.indigoIterateRDF.argtypes = [c_int]
        Indigo._lib.indigoIterateSmiles.restype = c_int
        Indigo._lib.indigoIterateSmiles.argtypes = [c_int]
        Indigo._lib.indigoIterateCML.restype = c_int
        Indigo._lib.indigoIterateCML.argtypes = [c_int]
        Indigo._lib.indigoIterateSDFile.restype = c_int
        Indigo._lib.indigoIterateSDFile.argtypes = [c_char_p]
        Indigo._lib.indigoIterateRDFile.restype = c_int
        Indigo._lib.indigoIterateRDFile.argtypes = [c_char_p]
        Indigo._lib.indigoIterateSmilesFile.restype = c_int
        Indigo._lib.indigoIterateSmilesFile.argtypes = [c_char_p]
        Indigo._lib.indigoIterateCMLFile.restype = c_int
        Indigo._lib.indigoIterateCMLFile.argtypes = [c_char_p]
        Indigo._lib.indigoCreateSaver.restype = c_int
        Indigo._lib.indigoCreateSaver.argtypes = [c_int, c_char_p]
        Indigo._lib.indigoCreateFileSaver.restype = c_int
        Indigo._lib.indigoCreateFileSaver.argtypes = [c_char_p, c_char_p]
        Indigo._lib.indigoCreateArray.restype = c_int
        Indigo._lib.indigoCreateArray.argtypes = None
        Indigo._lib.indigoSubstructureMatcher.restype = c_int
        Indigo._lib.indigoSubstructureMatcher.argtypes = [c_int, c_char_p]
        Indigo._lib.indigoExtractCommonScaffold.restype = c_int
        Indigo._lib.indigoExtractCommonScaffold.argtypes = [c_int, c_char_p]
        Indigo._lib.indigoDecomposeMolecules.restype = c_int
        Indigo._lib.indigoDecomposeMolecules.argtypes = [c_int, c_int]
        Indigo._lib.indigoCreateDecomposer.restype = c_int
        Indigo._lib.indigoCreateDecomposer.argtypes = [c_int]
        Indigo._lib.indigoReactionProductEnumerate.restype = c_int
        Indigo._lib.indigoReactionProductEnumerate.argtypes = [c_int, c_int]
        Indigo._lib.indigoTransform.restype = c_int
        Indigo._lib.indigoTransform.argtypes = [c_int, c_int]
        Indigo._lib.indigoDbgBreakpoint.restype = None
        Indigo._lib.indigoDbgBreakpoint.argtypes = None
        Indigo._lib.indigoClone.restype = c_int
        Indigo._lib.indigoClone.argtypes = [c_int]
        Indigo._lib.indigoClose.restype = c_int
        Indigo._lib.indigoClose.argtypes = [c_int]
        Indigo._lib.indigoNext.restype = c_int
        Indigo._lib.indigoNext.argtypes = [c_int]
        Indigo._lib.indigoHasNext.restype = c_int
        Indigo._lib.indigoHasNext.argtypes = [c_int]
        Indigo._lib.indigoIndex.restype = c_int
        Indigo._lib.indigoIndex.argtypes = [c_int]
        Indigo._lib.indigoRemove.restype = c_int
        Indigo._lib.indigoRemove.argtypes = [c_int]
        Indigo._lib.indigoSaveMolfileToFile.restype = c_int
        Indigo._lib.indigoSaveMolfileToFile.argtypes = [c_int, c_char_p]
        Indigo._lib.indigoMolfile.restype = c_char_p
        Indigo._lib.indigoMolfile.argtypes = [c_int]
        Indigo._lib.indigoSaveCmlToFile.restype = c_int
        Indigo._lib.indigoSaveCmlToFile.argtypes = [c_int, c_char_p]
        Indigo._lib.indigoCml.restype = c_char_p
        Indigo._lib.indigoCml.argtypes = [c_int]
        Indigo._lib.indigoSaveMDLCT.restype = c_int
        Indigo._lib.indigoSaveMDLCT.argtypes = [c_int, c_int]
        Indigo._lib.indigoAddReactant.restype = c_int
        Indigo._lib.indigoAddReactant.argtypes = [c_int, c_int]
        Indigo._lib.indigoAddProduct.restype = c_int
        Indigo._lib.indigoAddProduct.argtypes = [c_int, c_int]
        Indigo._lib.indigoAddCatalyst.restype = c_int
        Indigo._lib.indigoAddCatalyst.argtypes = [c_int, c_int]
        Indigo._lib.indigoCountReactants.restype = c_int
        Indigo._lib.indigoCountReactants.argtypes = [c_int]
        Indigo._lib.indigoCountProducts.restype = c_int
        Indigo._lib.indigoCountProducts.argtypes = [c_int]
        Indigo._lib.indigoCountCatalysts.restype = c_int
        Indigo._lib.indigoCountCatalysts.argtypes = [c_int]
        Indigo._lib.indigoCountMolecules.restype = c_int
        Indigo._lib.indigoCountMolecules.argtypes = [c_int]
        Indigo._lib.indigoGetMolecule.restype = c_int
        Indigo._lib.indigoGetMolecule.argtypes = [c_int, c_int]
        Indigo._lib.indigoIterateReactants.restype = c_int
        Indigo._lib.indigoIterateReactants.argtypes = [c_int]
        Indigo._lib.indigoIterateProducts.restype = c_int
        Indigo._lib.indigoIterateProducts.argtypes = [c_int]
        Indigo._lib.indigoIterateCatalysts.restype = c_int
        Indigo._lib.indigoIterateCatalysts.argtypes = [c_int]
        Indigo._lib.indigoIterateMolecules.restype = c_int
        Indigo._lib.indigoIterateMolecules.argtypes = [c_int]
        Indigo._lib.indigoSaveRxnfileToFile.restype = c_int
        Indigo._lib.indigoSaveRxnfileToFile.argtypes = [c_int, c_char_p]
        Indigo._lib.indigoRxnfile.restype = c_char_p
        Indigo._lib.indigoRxnfile.argtypes = [c_int]
        Indigo._lib.indigoOptimize.restype = c_int
        Indigo._lib.indigoOptimize.argtypes = [c_int, c_char_p]
        Indigo._lib.indigoNormalize.restype = c_int
        Indigo._lib.indigoNormalize.argtypes = [c_int, c_char_p]
        Indigo._lib.indigoAutomap.restype = c_int
        Indigo._lib.indigoAutomap.argtypes = [c_int, c_char_p]
        Indigo._lib.indigoGetAtomMappingNumber.restype = c_int
        Indigo._lib.indigoGetAtomMappingNumber.argtypes = [c_int, c_int]
        Indigo._lib.indigoSetAtomMappingNumber.restype = c_int
        Indigo._lib.indigoSetAtomMappingNumber.argtypes = [c_int, c_int, c_int]
        Indigo._lib.indigoGetReactingCenter.restype = c_int
        Indigo._lib.indigoGetReactingCenter.argtypes = [c_int, c_int, POINTER(c_int)]
        Indigo._lib.indigoSetReactingCenter.restype = c_int
        Indigo._lib.indigoSetReactingCenter.argtypes = [c_int, c_int, c_int]
        Indigo._lib.indigoClearAAM.restype = c_int
        Indigo._lib.indigoClearAAM.argtypes = [c_int]
        Indigo._lib.indigoCorrectReactingCenters.restype = c_int
        Indigo._lib.indigoCorrectReactingCenters.argtypes = [c_int]
        Indigo._lib.indigoIterateAtoms.restype = c_int
        Indigo._lib.indigoIterateAtoms.argtypes = [c_int]
        Indigo._lib.indigoIteratePseudoatoms.restype = c_int
        Indigo._lib.indigoIteratePseudoatoms.argtypes = [c_int]
        Indigo._lib.indigoIterateRSites.restype = c_int
        Indigo._lib.indigoIterateRSites.argtypes = [c_int]
        Indigo._lib.indigoIterateStereocenters.restype = c_int
        Indigo._lib.indigoIterateStereocenters.argtypes = [c_int]
        Indigo._lib.indigoIterateAlleneCenters.restype = c_int
        Indigo._lib.indigoIterateAlleneCenters.argtypes = [c_int]
        Indigo._lib.indigoIterateRGroups.restype = c_int
        Indigo._lib.indigoIterateRGroups.argtypes = [c_int]
        Indigo._lib.indigoIsPseudoatom.restype = c_int
        Indigo._lib.indigoIsPseudoatom.argtypes = [c_int]
        Indigo._lib.indigoIsRSite.restype = c_int
        Indigo._lib.indigoIsRSite.argtypes = [c_int]
        Indigo._lib.indigoStereocenterType.restype = c_int
        Indigo._lib.indigoStereocenterType.argtypes = [c_int]
        Indigo._lib.indigoStereocenterGroup.restype = c_int
        Indigo._lib.indigoStereocenterGroup.argtypes = [c_int]
        Indigo._lib.indigoSetStereocenterGroup.restype = c_int
        Indigo._lib.indigoSetStereocenterGroup.argtypes = [c_int]
        Indigo._lib.indigoChangeStereocenterType.restype = c_int
        Indigo._lib.indigoChangeStereocenterType.argtypes = [c_int, c_int]
        Indigo._lib.indigoSingleAllowedRGroup.restype = c_int
        Indigo._lib.indigoSingleAllowedRGroup.argtypes = [c_int]
        Indigo._lib.indigoAddStereocenter.restype = c_int
        Indigo._lib.indigoAddStereocenter.argtypes = [c_int, c_int, c_int, c_int, c_int, c_int]
        Indigo._lib.indigoIterateRGroupFragments.restype = c_int
        Indigo._lib.indigoIterateRGroupFragments.argtypes = [c_int]
        Indigo._lib.indigoCountAttachmentPoints.restype = c_int
        Indigo._lib.indigoCountAttachmentPoints.argtypes = [c_int]
        Indigo._lib.indigoIterateAttachmentPoints.restype = c_int
        Indigo._lib.indigoIterateAttachmentPoints.argtypes = [c_int, c_int]
        Indigo._lib.indigoSymbol.restype = c_char_p
        Indigo._lib.indigoSymbol.argtypes = [c_int]
        Indigo._lib.indigoDegree.restype = c_int
        Indigo._lib.indigoDegree.argtypes = [c_int]
        Indigo._lib.indigoGetCharge.restype = c_int
        Indigo._lib.indigoGetCharge.argtypes = [c_int, POINTER(c_int)]
        Indigo._lib.indigoGetExplicitValence.restype = c_int
        Indigo._lib.indigoGetExplicitValence.argtypes = [c_int, POINTER(c_int)]
        Indigo._lib.indigoSetExplicitValence.restype = c_int
        Indigo._lib.indigoSetExplicitValence.argtypes = [c_int, c_int]
        Indigo._lib.indigoGetRadicalElectrons.restype = c_int
        Indigo._lib.indigoGetRadicalElectrons.argtypes = [c_int, POINTER(c_int)]
        Indigo._lib.indigoGetRadical.restype = c_int
        Indigo._lib.indigoGetRadical.argtypes = [c_int, POINTER(c_int)]
        Indigo._lib.indigoSetRadical.restype = c_int
        Indigo._lib.indigoSetRadical.argtypes = [c_int, c_int]
        Indigo._lib.indigoAtomicNumber.restype = c_int
        Indigo._lib.indigoAtomicNumber.argtypes = [c_int]
        Indigo._lib.indigoIsotope.restype = c_int
        Indigo._lib.indigoIsotope.argtypes = [c_int]
        Indigo._lib.indigoValence.restype = c_int
        Indigo._lib.indigoValence.argtypes = [c_int]
        Indigo._lib.indigoCountHydrogens.restype = c_int
        Indigo._lib.indigoCountHydrogens.argtypes = [c_int, POINTER(c_int)]
        Indigo._lib.indigoCountImplicitHydrogens.restype = c_int
        Indigo._lib.indigoCountImplicitHydrogens.argtypes = [c_int]
        Indigo._lib.indigoXYZ.restype = POINTER(c_float)
        Indigo._lib.indigoXYZ.argtypes = [c_int]
        Indigo._lib.indigoSetXYZ.restype = c_int
        Indigo._lib.indigoSetXYZ.argtypes = [c_int, c_float, c_float, c_float]
        Indigo._lib.indigoCountSuperatoms.restype = c_int
        Indigo._lib.indigoCountSuperatoms.argtypes = [c_int]
        Indigo._lib.indigoCountDataSGroups.restype = c_int
        Indigo._lib.indigoCountDataSGroups.argtypes = [c_int]
        Indigo._lib.indigoCountRepeatingUnits.restype = c_int
        Indigo._lib.indigoCountRepeatingUnits.argtypes = [c_int]
        Indigo._lib.indigoCountMultipleGroups.restype = c_int
        Indigo._lib.indigoCountMultipleGroups.argtypes = [c_int]
        Indigo._lib.indigoCountGenericSGroups.restype = c_int
        Indigo._lib.indigoCountGenericSGroups.argtypes = [c_int]
        Indigo._lib.indigoIterateDataSGroups.restype = c_int
        Indigo._lib.indigoIterateDataSGroups.argtypes = [c_int]
        Indigo._lib.indigoIterateSuperatoms.restype = c_int
        Indigo._lib.indigoIterateSuperatoms.argtypes = [c_int]
        Indigo._lib.indigoIterateGenericSGroups.restype = c_int
        Indigo._lib.indigoIterateGenericSGroups.argtypes = [c_int]
        Indigo._lib.indigoIterateRepeatingUnits.restype = c_int
        Indigo._lib.indigoIterateRepeatingUnits.argtypes = [c_int]
        Indigo._lib.indigoIterateMultipleGroups.restype = c_int
        Indigo._lib.indigoIterateMultipleGroups.argtypes = [c_int]
        Indigo._lib.indigoGetSuperatom.restype = c_int
        Indigo._lib.indigoGetSuperatom.argtypes = [c_int, c_int]
        Indigo._lib.indigoGetDataSGroup.restype = c_int
        Indigo._lib.indigoGetDataSGroup.argtypes = [c_int, c_int]
        Indigo._lib.indigoGetGenericSGroup.restype = c_int
        Indigo._lib.indigoGetGenericSGroup.argtypes = [c_int, c_int]
        Indigo._lib.indigoGetMultipleGroup.restype = c_int
        Indigo._lib.indigoGetMultipleGroup.argtypes = [c_int, c_int]
        Indigo._lib.indigoGetRepeatingUnit.restype = c_int
        Indigo._lib.indigoGetRepeatingUnit.argtypes = [c_int, c_int]
        Indigo._lib.indigoDescription.restype = c_char_p
        Indigo._lib.indigoDescription.argtypes = [c_int]
        Indigo._lib.indigoData.restype = c_char_p
        Indigo._lib.indigoData.argtypes = [c_int]
        Indigo._lib.indigoAddDataSGroup.restype = c_int
        Indigo._lib.indigoAddDataSGroup.argtypes = [c_int, c_int, POINTER(c_int), c_int, POINTER(c_int), c_char_p, c_char_p]
        Indigo._lib.indigoAddSuperatom.restype = c_int
        Indigo._lib.indigoAddSuperatom.argtypes = [c_int, c_int, POINTER(c_int), c_char_p]
        Indigo._lib.indigoSetDataSGroupXY.restype = c_int
        Indigo._lib.indigoSetDataSGroupXY.argtypes = [c_int, c_float, c_float, c_char_p]
        Indigo._lib.indigoResetCharge.restype = c_int
        Indigo._lib.indigoResetCharge.argtypes = [c_int]
        Indigo._lib.indigoResetExplicitValence.restype = c_int
        Indigo._lib.indigoResetExplicitValence.argtypes = [c_int]
        Indigo._lib.indigoResetRadical.restype = c_int
        Indigo._lib.indigoResetRadical.argtypes = [c_int]
        Indigo._lib.indigoResetIsotope.restype = c_int
        Indigo._lib.indigoResetIsotope.argtypes = [c_int]
        Indigo._lib.indigoSetAttachmentPoint.restype = c_int
        Indigo._lib.indigoSetAttachmentPoint.argtypes = [c_int, c_int]
        Indigo._lib.indigoClearAttachmentPoints.restype = c_int
        Indigo._lib.indigoClearAttachmentPoints.argtypes = [c_int]
        Indigo._lib.indigoRemoveConstraints.restype = c_int
        Indigo._lib.indigoRemoveConstraints.argtypes = [c_int, c_char_p]
        Indigo._lib.indigoAddConstraint.restype = c_int
        Indigo._lib.indigoAddConstraint.argtypes = [c_int, c_char_p, c_char_p]
        Indigo._lib.indigoAddConstraintNot.restype = c_int
        Indigo._lib.indigoAddConstraintNot.argtypes = [c_int, c_char_p, c_char_p]
        Indigo._lib.indigoAddConstraintOr.restype = c_int
        Indigo._lib.indigoAddConstraintOr.argtypes = [c_int, c_char_p, c_char_p]
        Indigo._lib.indigoResetStereo.restype = c_int
        Indigo._lib.indigoResetStereo.argtypes = [c_int]
        Indigo._lib.indigoInvertStereo.restype = c_int
        Indigo._lib.indigoInvertStereo.argtypes = [c_int]
        Indigo._lib.indigoCountAtoms.restype = c_int
        Indigo._lib.indigoCountAtoms.argtypes = [c_int]
        Indigo._lib.indigoCountBonds.restype = c_int
        Indigo._lib.indigoCountBonds.argtypes = [c_int]
        Indigo._lib.indigoCountPseudoatoms.restype = c_int
        Indigo._lib.indigoCountPseudoatoms.argtypes = [c_int]
        Indigo._lib.indigoCountRSites.restype = c_int
        Indigo._lib.indigoCountRSites.argtypes = [c_int]
        Indigo._lib.indigoIterateBonds.restype = c_int
        Indigo._lib.indigoIterateBonds.argtypes = [c_int]
        Indigo._lib.indigoBondOrder.restype = c_int
        Indigo._lib.indigoBondOrder.argtypes = [c_int]
        Indigo._lib.indigoBondStereo.restype = c_int
        Indigo._lib.indigoBondStereo.argtypes = [c_int]
        Indigo._lib.indigoTopology.restype = c_int
        Indigo._lib.indigoTopology.argtypes = [c_int]
        Indigo._lib.indigoIterateNeighbors.restype = c_int
        Indigo._lib.indigoIterateNeighbors.argtypes = [c_int]
        Indigo._lib.indigoBond.restype = c_int
        Indigo._lib.indigoBond.argtypes = [c_int]
        Indigo._lib.indigoGetAtom.restype = c_int
        Indigo._lib.indigoGetAtom.argtypes = [c_int, c_int]
        Indigo._lib.indigoGetBond.restype = c_int
        Indigo._lib.indigoGetBond.argtypes = [c_int, c_int]
        Indigo._lib.indigoSource.restype = c_int
        Indigo._lib.indigoSource.argtypes = [c_int]
        Indigo._lib.indigoDestination.restype = c_int
        Indigo._lib.indigoDestination.argtypes = [c_int]
        Indigo._lib.indigoClearCisTrans.restype = c_int
        Indigo._lib.indigoClearCisTrans.argtypes = [c_int]
        Indigo._lib.indigoClearStereocenters.restype = c_int
        Indigo._lib.indigoClearStereocenters.argtypes = [c_int]
        Indigo._lib.indigoCountStereocenters.restype = c_int
        Indigo._lib.indigoCountStereocenters.argtypes = [c_int]
        Indigo._lib.indigoClearAlleneCenters.restype = c_int
        Indigo._lib.indigoClearAlleneCenters.argtypes = [c_int]
        Indigo._lib.indigoCountAlleneCenters.restype = c_int
        Indigo._lib.indigoCountAlleneCenters.argtypes = [c_int]
        Indigo._lib.indigoResetSymmetricCisTrans.restype = c_int
        Indigo._lib.indigoResetSymmetricCisTrans.argtypes = [c_int]
        Indigo._lib.indigoResetSymmetricStereocenters.restype = c_int
        Indigo._lib.indigoResetSymmetricStereocenters.argtypes = [c_int]
        Indigo._lib.indigoMarkEitherCisTrans.restype = c_int
        Indigo._lib.indigoMarkEitherCisTrans.argtypes = [c_int]
        Indigo._lib.indigoMarkStereobonds.restype = c_int
        Indigo._lib.indigoMarkStereobonds.argtypes = [c_int]
        Indigo._lib.indigoAddAtom.restype = c_int
        Indigo._lib.indigoAddAtom.argtypes = [c_int, c_char_p]
        Indigo._lib.indigoResetAtom.restype = c_int
        Indigo._lib.indigoResetAtom.argtypes = [c_int, c_char_p]
        Indigo._lib.indigoAddRSite.restype = c_int
        Indigo._lib.indigoAddRSite.argtypes = [c_int, c_char_p]
        Indigo._lib.indigoSetRSite.restype = c_int
        Indigo._lib.indigoSetRSite.argtypes = [c_int, c_char_p]
        Indigo._lib.indigoSetCharge.restype = c_int
        Indigo._lib.indigoSetCharge.argtypes = [c_int, c_int]
        Indigo._lib.indigoSetIsotope.restype = c_int
        Indigo._lib.indigoSetIsotope.argtypes = [c_int, c_int]
        Indigo._lib.indigoSetImplicitHCount.restype = c_int
        Indigo._lib.indigoSetImplicitHCount.argtypes = [c_int, c_int]
        Indigo._lib.indigoAddBond.restype = c_int
        Indigo._lib.indigoAddBond.argtypes = [c_int, c_int, c_int]
        Indigo._lib.indigoSetBondOrder.restype = c_int
        Indigo._lib.indigoSetBondOrder.argtypes = [c_int, c_int]
        Indigo._lib.indigoMerge.restype = c_int
        Indigo._lib.indigoMerge.argtypes = [c_int, c_int]
        Indigo._lib.indigoHighlight.restype = c_int
        Indigo._lib.indigoHighlight.argtypes = [c_int]
        Indigo._lib.indigoUnhighlight.restype = c_int
        Indigo._lib.indigoUnhighlight.argtypes = [c_int]
        Indigo._lib.indigoIsHighlighted.restype = c_int
        Indigo._lib.indigoIsHighlighted.argtypes = [c_int]
        Indigo._lib.indigoCountComponents.restype = c_int
        Indigo._lib.indigoCountComponents.argtypes = [c_int]
        Indigo._lib.indigoComponentIndex.restype = c_int
        Indigo._lib.indigoComponentIndex.argtypes = [c_int]
        Indigo._lib.indigoIterateComponents.restype = c_int
        Indigo._lib.indigoIterateComponents.argtypes = [c_int]
        Indigo._lib.indigoComponent.restype = c_int
        Indigo._lib.indigoComponent.argtypes = [c_int, c_int]
        Indigo._lib.indigoCountSSSR.restype = c_int
        Indigo._lib.indigoCountSSSR.argtypes = [c_int]
        Indigo._lib.indigoIterateSSSR.restype = c_int
        Indigo._lib.indigoIterateSSSR.argtypes = [c_int]
        Indigo._lib.indigoIterateSubtrees.restype = c_int
        Indigo._lib.indigoIterateSubtrees.argtypes = [c_int, c_int, c_int]
        Indigo._lib.indigoIterateRings.restype = c_int
        Indigo._lib.indigoIterateRings.argtypes = [c_int, c_int, c_int]
        Indigo._lib.indigoIterateEdgeSubmolecules.restype = c_int
        Indigo._lib.indigoIterateEdgeSubmolecules.argtypes = [c_int, c_int, c_int]
        Indigo._lib.indigoCountHeavyAtoms.restype = c_int
        Indigo._lib.indigoCountHeavyAtoms.argtypes = [c_int]
        Indigo._lib.indigoGrossFormula.restype = c_int
        Indigo._lib.indigoGrossFormula.argtypes = [c_int]
        Indigo._lib.indigoMolecularWeight.restype = c_float
        Indigo._lib.indigoMolecularWeight.argtypes = [c_int]
        Indigo._lib.indigoMostAbundantMass.restype = c_float
        Indigo._lib.indigoMostAbundantMass.argtypes = [c_int]
        Indigo._lib.indigoMonoisotopicMass.restype = c_float
        Indigo._lib.indigoMonoisotopicMass.argtypes = [c_int]
        Indigo._lib.indigoCanonicalSmiles.restype = c_char_p
        Indigo._lib.indigoCanonicalSmiles.argtypes = [c_int]
        Indigo._lib.indigoLayeredCode.restype = c_char_p
        Indigo._lib.indigoLayeredCode.argtypes = [c_int]
        Indigo._lib.indigoSymmetryClasses.restype = POINTER(c_int)
        Indigo._lib.indigoSymmetryClasses.argtypes = [c_int, POINTER(c_int)]
        Indigo._lib.indigoHasCoord.restype = c_int
        Indigo._lib.indigoHasCoord.argtypes = [c_int]
        Indigo._lib.indigoHasZCoord.restype = c_int
        Indigo._lib.indigoHasZCoord.argtypes = [c_int]
        Indigo._lib.indigoIsChiral.restype = c_int
        Indigo._lib.indigoIsChiral.argtypes = [c_int]
        Indigo._lib.indigoCreateSubmolecule.restype = c_int
        Indigo._lib.indigoCreateSubmolecule.argtypes = [c_int, c_int, POINTER(c_int)]
        Indigo._lib.indigoCreateEdgeSubmolecule.restype = c_int
        Indigo._lib.indigoCreateEdgeSubmolecule.argtypes = [c_int, c_int, POINTER(c_int), c_int, POINTER(c_int)]
        Indigo._lib.indigoGetSubmolecule.restype = c_int
        Indigo._lib.indigoGetSubmolecule.argtypes = [c_int, c_int, POINTER(c_int)]
        Indigo._lib.indigoRemoveAtoms.restype = c_int
        Indigo._lib.indigoRemoveAtoms.argtypes = [c_int, c_int, POINTER(c_int)]
        Indigo._lib.indigoRemoveBonds.restype = c_int
        Indigo._lib.indigoRemoveBonds.argtypes = [c_int, c_int, POINTER(c_int)]
        Indigo._lib.indigoAlignAtoms.restype = c_float
        Indigo._lib.indigoAlignAtoms.argtypes = [c_int, c_int, POINTER(c_int), POINTER(c_float)]
        Indigo._lib.indigoAromatize.restype = c_int
        Indigo._lib.indigoAromatize.argtypes = [c_int]
        Indigo._lib.indigoDearomatize.restype = c_int
        Indigo._lib.indigoDearomatize.argtypes = [c_int]
        Indigo._lib.indigoFoldHydrogens.restype = c_int
        Indigo._lib.indigoFoldHydrogens.argtypes = [c_int]
        Indigo._lib.indigoUnfoldHydrogens.restype = c_int
        Indigo._lib.indigoUnfoldHydrogens.argtypes = [c_int]
        Indigo._lib.indigoLayout.restype = c_int
        Indigo._lib.indigoLayout.argtypes = [c_int]
        Indigo._lib.indigoSmiles.restype = c_char_p
        Indigo._lib.indigoSmiles.argtypes = [c_int]
        Indigo._lib.indigoName.restype = c_char_p
        Indigo._lib.indigoName.argtypes = [c_int]
        Indigo._lib.indigoSetName.restype = c_int
        Indigo._lib.indigoSetName.argtypes = [c_int, c_char_p]
        Indigo._lib.indigoSerialize.restype = c_int
        Indigo._lib.indigoSerialize.argtypes = [c_int, POINTER(POINTER(c_byte)), POINTER(c_int)]
        Indigo._lib.indigoHasProperty.restype = c_int
        Indigo._lib.indigoHasProperty.argtypes = [c_int, c_char_p]
        Indigo._lib.indigoGetProperty.restype = c_char_p
        Indigo._lib.indigoGetProperty.argtypes = [c_int, c_char_p]
        Indigo._lib.indigoSetProperty.restype = c_int
        Indigo._lib.indigoSetProperty.argtypes = [c_int, c_char_p, c_char_p]
        Indigo._lib.indigoRemoveProperty.restype = c_int
        Indigo._lib.indigoRemoveProperty.argtypes = [c_int, c_char_p]
        Indigo._lib.indigoIterateProperties.restype = c_int
        Indigo._lib.indigoIterateProperties.argtypes = [c_int]
        Indigo._lib.indigoClearProperties.restype = c_int
        Indigo._lib.indigoClearProperties.argtypes = [c_int]
        Indigo._lib.indigoCheckBadValence.restype = c_char_p
        Indigo._lib.indigoCheckBadValence.argtypes = [c_int]
        Indigo._lib.indigoCheckAmbiguousH.restype = c_char_p
        Indigo._lib.indigoCheckAmbiguousH.argtypes = [c_int]
        Indigo._lib.indigoFingerprint.restype = c_int
        Indigo._lib.indigoFingerprint.argtypes = [c_int, c_char_p]
        Indigo._lib.indigoCountBits.restype = c_int
        Indigo._lib.indigoCountBits.argtypes = [c_int]
        Indigo._lib.indigoRawData.restype = c_char_p
        Indigo._lib.indigoRawData.argtypes = [c_int]
        Indigo._lib.indigoTell.restype = c_int
        Indigo._lib.indigoTell.argtypes = [c_int]
        Indigo._lib.indigoSdfAppend.restype = c_int
        Indigo._lib.indigoSdfAppend.argtypes = [c_int, c_int]
        Indigo._lib.indigoSmilesAppend.restype = c_int
        Indigo._lib.indigoSmilesAppend.argtypes = [c_int, c_int]
        Indigo._lib.indigoRdfHeader.restype = c_int
        Indigo._lib.indigoRdfHeader.argtypes = [c_int]
        Indigo._lib.indigoRdfAppend.restype = c_int
        Indigo._lib.indigoRdfAppend.argtypes = [c_int, c_int]
        Indigo._lib.indigoCmlHeader.restype = c_int
        Indigo._lib.indigoCmlHeader.argtypes = [c_int]
        Indigo._lib.indigoCmlAppend.restype = c_int
        Indigo._lib.indigoCmlAppend.argtypes = [c_int, c_int]
        Indigo._lib.indigoCmlFooter.restype = c_int
        Indigo._lib.indigoCmlFooter.argtypes = [c_int]
        Indigo._lib.indigoAppend.restype = c_int
        Indigo._lib.indigoAppend.argtypes = [c_int, c_int]
        Indigo._lib.indigoArrayAdd.restype = c_int
        Indigo._lib.indigoArrayAdd.argtypes = [c_int, c_int]
        Indigo._lib.indigoAt.restype = c_int
        Indigo._lib.indigoAt.argtypes = [c_int, c_int]
        Indigo._lib.indigoCount.restype = c_int
        Indigo._lib.indigoCount.argtypes = [c_int]
        Indigo._lib.indigoClear.restype = c_int
        Indigo._lib.indigoClear.argtypes = [c_int]
        Indigo._lib.indigoIterateArray.restype = c_int
        Indigo._lib.indigoIterateArray.argtypes = [c_int]
        Indigo._lib.indigoIgnoreAtom.restype = c_int
        Indigo._lib.indigoIgnoreAtom.argtypes = [c_int, c_int]
        Indigo._lib.indigoUnignoreAtom.restype = c_int
        Indigo._lib.indigoUnignoreAtom.argtypes = [c_int, c_int]
        Indigo._lib.indigoUnignoreAllAtoms.restype = c_int
        Indigo._lib.indigoUnignoreAllAtoms.argtypes = [c_int]
        Indigo._lib.indigoMatch.restype = c_int
        Indigo._lib.indigoMatch.argtypes = [c_int, c_int]
        Indigo._lib.indigoCountMatches.restype = c_int
        Indigo._lib.indigoCountMatches.argtypes = [c_int, c_int]
        Indigo._lib.indigoCountMatchesWithLimit.restype = c_int
        Indigo._lib.indigoCountMatchesWithLimit.argtypes = [c_int, c_int, c_int]
        Indigo._lib.indigoIterateMatches.restype = c_int
        Indigo._lib.indigoIterateMatches.argtypes = [c_int, c_int]
        Indigo._lib.indigoHighlightedTarget.restype = c_int
        Indigo._lib.indigoHighlightedTarget.argtypes = [c_int]
        Indigo._lib.indigoMapAtom.restype = c_int
        Indigo._lib.indigoMapAtom.argtypes = [c_int, c_int]
        Indigo._lib.indigoMapBond.restype = c_int
        Indigo._lib.indigoMapBond.argtypes = [c_int, c_int]
        Indigo._lib.indigoMapMolecule.restype = c_int
        Indigo._lib.indigoMapMolecule.argtypes = [c_int, c_int]
        Indigo._lib.indigoAllScaffolds.restype = c_int
        Indigo._lib.indigoAllScaffolds.argtypes = [c_int]
        Indigo._lib.indigoDecomposedMoleculeScaffold.restype = c_int
        Indigo._lib.indigoDecomposedMoleculeScaffold.argtypes = [c_int]
        Indigo._lib.indigoIterateDecomposedMolecules.restype = c_int
        Indigo._lib.indigoIterateDecomposedMolecules.argtypes = [c_int]
        Indigo._lib.indigoDecomposedMoleculeHighlighted.restype = c_int
        Indigo._lib.indigoDecomposedMoleculeHighlighted.argtypes = [c_int]
        Indigo._lib.indigoDecomposedMoleculeWithRGroups.restype = c_int
        Indigo._lib.indigoDecomposedMoleculeWithRGroups.argtypes = [c_int]
        Indigo._lib.indigoDecomposeMolecule.restype = c_int
        Indigo._lib.indigoDecomposeMolecule.argtypes = [c_int, c_int]
        Indigo._lib.indigoIterateDecompositions.restype = c_int
        Indigo._lib.indigoIterateDecompositions.argtypes = [c_int]
        Indigo._lib.indigoAddDecomposition.restype = c_int
        Indigo._lib.indigoAddDecomposition.argtypes = [c_int, c_int]
        Indigo._lib.indigoToString.restype = c_char_p
        Indigo._lib.indigoToString.argtypes = [c_int]
        Indigo._lib.indigoToBuffer.restype = c_int
        Indigo._lib.indigoToBuffer.argtypes = [c_int, POINTER(POINTER(c_byte)), POINTER(c_int)]
        Indigo._lib.indigoStereocenterPyramid.restype = POINTER(c_int)
        Indigo._lib.indigoStereocenterPyramid.argtypes = [c_int]
        Indigo._lib.indigoExpandAbbreviations.restype = c_int
        Indigo._lib.indigoExpandAbbreviations.argtypes = [c_int]
        Indigo._lib.indigoValidateChirality.restype = c_int
        Indigo._lib.indigoValidateChirality.argtypes = [c_int]
        Indigo._lib.indigoDbgInternalType.restype = c_char_p
        Indigo._lib.indigoDbgInternalType.argtypes = [c_int]

    def __del__ (self):
        if hasattr(self, '_lib'):
            self._lib.indigoReleaseSessionId(self._sid)

    def writeBuffer (self):
        self._setSessionId()
        id = self._checkResult(Indigo._lib.indigoWriteBuffer())
        return self.IndigoObject(self, id)

    def writeFile (self, filename):
        self._setSessionId()
        id = self._checkResult(Indigo._lib.indigoWriteFile(filename.encode('ascii')))
        return self.IndigoObject(self, id)

    def unserialize(self, arr):
        self._setSessionId()
        values = (c_byte * len(arr))()
        for i in range(len(arr)):
            values[i] = arr[i]
        res = Indigo._lib.indigoUnserialize(values, len(arr))
        return self.IndigoObject(self, self._checkResult(res))

    def setOption (self, option, value1, value2=None, value3=None):
        self._setSessionId()
        if (type(value1).__name__ == 'str' or type(value1).__name__ == 'unicode') and value2 is None and value3 is None:
            self._checkResult(Indigo._lib.indigoSetOption(option.encode('ascii'), value1.encode('ascii')))
        elif type(value1).__name__ == 'int' and value2 is None and value3 is None:
            self._checkResult(Indigo._lib.indigoSetOptionInt(option.encode('ascii'), value1))
        elif type(value1).__name__ == 'float' and value2 is None and value3 is None:
            self._checkResult(Indigo._lib.indigoSetOptionFloat(option.encode('ascii'), value1))
        elif type(value1).__name__ == 'bool' and value2 is None and value3 is None:
            value1_b = 0
            if value1:
                value1_b = 1
            self._checkResult(Indigo._lib.indigoSetOptionBool(option.encode('ascii'), value1_b))
        elif type(value1).__name__ == 'int' and value2 and \
                 type(value2).__name__ == 'int' and value3 is None:
            self._checkResult(Indigo._lib.indigoSetOptionXY(option.encode('ascii'), value1, value2))
        elif type(value1).__name__ == 'float' and value2 and \
                 type(value2).__name__ == 'float' and value3 and \
                 type(value3).__name__ == 'float':
            self._checkResult(Indigo._lib.indigoSetOptionColor(option.encode('ascii'), value1, value2, value3))
        else:
            raise IndigoException("bad option")

    def _checkResult (self, result):
        if result < 0:
            raise IndigoException(Indigo._lib.indigoGetLastError())
        return result

    def _checkResultFloat (self, result):
        if result < -0.5:
            raise IndigoException(Indigo._lib.indigoGetLastError())
        return result

    def _checkResultPtr (self, result):
        if result is None:
            raise IndigoException(Indigo._lib.indigoGetLastError())
        return result

    def _checkResultString (self, result):
        return self._checkResultPtr(result).decode('ascii') if sys.version_info >= (3, 0) else self._checkResultPtr(result).encode('ascii')

    def convertToArray (self, iteratable):
        if isinstance(iteratable, IndigoObject):
            return iteratable
        try:
            some_object_iterator = iter(iteratable)
            res = self.createArray()
            for obj in some_object_iterator:
                 res.arrayAdd(self.convertToArray(obj))
            return res
        except TypeError:
            raise IndigoException("Cannot convert object %s to an array" % (iteratable))

    def dbgBreakpoint(self):
        self._setSessionId()
        return Indigo._lib.indigoDbgBreakpoint()

    def version(self):
        self._setSessionId()
        return self._checkResultString(Indigo._lib.indigoVersion())

    def countReferences(self):
        self._setSessionId()
        return self._checkResult(Indigo._lib.indigoCountReferences())

    def writeFile(self, filename):
        self._setSessionId()
        return self.IndigoObject(self, self._checkResult(Indigo._lib.indigoWriteFile(filename.encode('ascii'))))

    def writeBuffer(self):
        self._setSessionId()
        return self.IndigoObject(self, self._checkResult(Indigo._lib.indigoWriteBuffer()))

    def createMolecule(self):
        self._setSessionId()
        return self.IndigoObject(self, self._checkResult(Indigo._lib.indigoCreateMolecule()))

    def createQueryMolecule(self):
        self._setSessionId()
        return self.IndigoObject(self, self._checkResult(Indigo._lib.indigoCreateQueryMolecule()))

    def loadMolecule(self, string):
        self._setSessionId()
        #sys.__stdout__.write(string)
        return self.IndigoObject(self, self._checkResult(Indigo._lib.indigoLoadMoleculeFromString(string.encode('ascii'))))

    def loadMoleculeFromFile(self, filename):
        self._setSessionId()
        return self.IndigoObject(self, self._checkResult(Indigo._lib.indigoLoadMoleculeFromFile(filename.encode('ascii'))))

    def loadQueryMolecule(self, string):
        self._setSessionId()
        return self.IndigoObject(self, self._checkResult(Indigo._lib.indigoLoadQueryMoleculeFromString(string.encode('ascii'))))

    def loadQueryMoleculeFromFile(self, filename):
        self._setSessionId()
        return self.IndigoObject(self, self._checkResult(Indigo._lib.indigoLoadQueryMoleculeFromFile(filename.encode('ascii'))))

    def loadSmarts(self, string):
        self._setSessionId()
        return self.IndigoObject(self, self._checkResult(Indigo._lib.indigoLoadSmartsFromString(string.encode('ascii'))))

    def loadSmartsFromFile(self, filename):
        self._setSessionId()
        return self.IndigoObject(self, self._checkResult(Indigo._lib.indigoLoadSmartsFromFile(filename.encode('ascii'))))

    def loadReaction(self, string):
        self._setSessionId()
        return self.IndigoObject(self, self._checkResult(Indigo._lib.indigoLoadReactionFromString(string.encode('ascii'))))

    def loadReactionFromFile(self, filename):
        self._setSessionId()
        return self.IndigoObject(self, self._checkResult(Indigo._lib.indigoLoadReactionFromFile(filename.encode('ascii'))))

    def loadQueryReaction(self, string):
        self._setSessionId()
        return self.IndigoObject(self, self._checkResult(Indigo._lib.indigoLoadQueryReactionFromString(string.encode('ascii'))))

    def loadQueryReactionFromFile(self, filename):
        self._setSessionId()
        return self.IndigoObject(self, self._checkResult(Indigo._lib.indigoLoadQueryReactionFromFile(filename.encode('ascii'))))

    def loadReactionSmarts(self, string):
        self._setSessionId()
        return self.IndigoObject(self, self._checkResult(Indigo._lib.indigoLoadReactionSmartsFromString(string.encode('ascii'))))

    def loadReactionSmartsFromFile(self, filename):
        self._setSessionId()
        return self.IndigoObject(self, self._checkResult(Indigo._lib.indigoLoadReactionSmartsFromFile(filename.encode('ascii'))))

    def createReaction(self):
        self._setSessionId()
        return self.IndigoObject(self, self._checkResult(Indigo._lib.indigoCreateReaction()))

    def createQueryReaction(self):
        self._setSessionId()
        return self.IndigoObject(self, self._checkResult(Indigo._lib.indigoCreateQueryReaction()))

    def exactMatch(self, item1, item2, flags=''):
        self._setSessionId()
        flags = '' if flags is None else flags
        newobj = self._checkResult(Indigo._lib.indigoExactMatch(item1.id, item2.id, flags.encode('ascii')))
        if newobj == 0:
            return None
        else:
            return self.IndigoObject(self, newobj, [item1, item2, self])

    def setTautomerRule(self, id, beg, end):
        self._setSessionId()
        return self._checkResult(Indigo._lib.indigoSetTautomerRule(id, beg.encode('ascii'), end.encode('ascii')))

    def removeTautomerRule(self, id):
        self._setSessionId()
        return self._checkResult(Indigo._lib.indigoRemoveTautomerRule(id))

    def clearTautomerRules(self):
        self._setSessionId()
        return self._checkResult(Indigo._lib.indigoClearTautomerRules())

    def commonBits(self, fingerprint1, fingerprint2):
        self._setSessionId()
        return self._checkResult(Indigo._lib.indigoCommonBits(fingerprint1.id, fingerprint2.id))

    def similarity(self, item1, item2, metrics=''):
        self._setSessionId()
        metrics = '' if metrics is None else metrics
        return self._checkResultFloat(Indigo._lib.indigoSimilarity(item1.id, item2.id, metrics.encode('ascii')))

    def iterateSDFile(self, filename):
        self._setSessionId()
        return self.IndigoObject(self, self._checkResult(Indigo._lib.indigoIterateSDFile(filename.encode('ascii'))))

    def iterateRDFile(self, filename):
        self._setSessionId()
        return self.IndigoObject(self, self._checkResult(Indigo._lib.indigoIterateRDFile(filename.encode('ascii'))))

    def iterateSmilesFile(self, filename):
        self._setSessionId()
        return self.IndigoObject(self, self._checkResult(Indigo._lib.indigoIterateSmilesFile(filename.encode('ascii'))))

    def iterateCMLFile(self, filename):
        self._setSessionId()
        return self.IndigoObject(self, self._checkResult(Indigo._lib.indigoIterateCMLFile(filename.encode('ascii'))))

    def createFileSaver(self, filename, format):
        self._setSessionId()
        return self.IndigoObject(self, self._checkResult(Indigo._lib.indigoCreateFileSaver(filename.encode('ascii'), format.encode('ascii'))))

    def createSaver(self, obj, format):
        self._setSessionId()
        return self.IndigoObject(self, self._checkResult(Indigo._lib.indigoCreateSaver(obj.id, format.encode('ascii'))))

    def createArray(self):
        self._setSessionId()
        return self.IndigoObject(self, self._checkResult(Indigo._lib.indigoCreateArray()))

    def substructureMatcher(self, target, mode=''):
        self._setSessionId()
        mode = '' if mode is None else mode
        return self.IndigoObject(self, self._checkResult(Indigo._lib.indigoSubstructureMatcher(target.id, mode.encode('ascii'))), target)

    def extractCommonScaffold(self, structures, options=''):
        self._setSessionId()
        structures = self.convertToArray(structures)
        options = '' if options is None else options
        newobj = self._checkResult(Indigo._lib.indigoExtractCommonScaffold(structures.id, options.encode('ascii')))
        if newobj == 0:
            return None
        else:
            return self.IndigoObject(self, newobj, self)

    def decomposeMolecules(self, scaffold, structures):
        self._setSessionId()
        structures = self.convertToArray(structures)
        return self.IndigoObject(self, self._checkResult(Indigo._lib.indigoDecomposeMolecules(scaffold.id, structures.id)), scaffold)

    def createDecomposer(self, scaffold):
        self._setSessionId()
        return self.IndigoObject(self, self._checkResult(Indigo._lib.indigoCreateDecomposer(scaffold.id)), scaffold)

    def reactionProductEnumerate(self, replacedaction, monomers):
        self._setSessionId()
        monomers = self.convertToArray(monomers)
        return self.IndigoObject(self, self._checkResult(Indigo._lib.indigoReactionProductEnumerate(replacedaction.id, monomers.id)), replacedaction)

    def transform(self, reaction, monomers):
        self._setSessionId()
        return self._checkResult(Indigo._lib.indigoTransform(reaction.id, monomers.id))

    def loadBuffer(self, buf):
        self._setSessionId()
        values = (c_byte * len(buf))()
        for i in range(len(buf)):
            values[i] = buf[i]
        return self.IndigoObject(self, self._checkResult(Indigo._lib.indigoLoadBuffer(values, len(buf))))

    def loadString(self, string):
        self._setSessionId()
        return self.IndigoObject(self, self._checkResult(Indigo._lib.indigoLoadString(string)))

    def iterateSDF(self, reader):
        self._setSessionId()
        result = self._checkResult(Indigo._lib.indigoIterateSDF(reader.id))
        if not result:
            return None
        return self.IndigoObject(self, result, reader)

    def iterateSmiles(self, reader):
        self._setSessionId()
        result = self._checkResult(Indigo._lib.indigoIterateSmiles(reader.id))
        if not result:
            return None
        return self.IndigoObject(self, result, reader)

    def iterateCML(self, reader):
        self._setSessionId()
        result = self._checkResult(Indigo._lib.indigoIterateCML(reader.id))
        if not result:
            return None
        return self.IndigoObject(self, result, reader)

    def iterateRDF(self, reader):
        self._setSessionId()
        result = self._checkResult(Indigo._lib.indigoIterateRDF(reader.id))
        if not result:
            return None
        return self.IndigoObject(self, result, reader)
