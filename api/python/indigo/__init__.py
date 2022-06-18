#
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
__version__ = "1.7.3"

import os
import platform
import sys
import warnings
from array import array
from ctypes import (
    CDLL,
    POINTER,
    RTLD_GLOBAL,
    c_byte,
    c_char_p,
    c_double,
    c_float,
    c_int,
    c_ulonglong,
    c_void_p,
    pointer,
    sizeof,
)
from enum import Enum

from indigo.exceptions import IndigoException
from indigo.salts import SALTS

DECODE_ENCODING = "utf-8"
ENCODE_ENCODING = "utf-8"


class Hybridization(Enum):
    S = 1
    SP = 2
    SP2 = 3
    SP3 = 4
    SP3D = 5
    SP3D2 = 6
    SP3D3 = 7
    SP3D4 = 8
    SP2D = 9


class IndigoObject(object):
    """Wraps all Indigo model objects"""

    def __init__(self, dispatcher, id, parent=None):
        self.id = id
        self.dispatcher = dispatcher
        self.parent = parent

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        self.dispatcher._setSessionId()
        self.dispatcher._lib.indigoClose(self.id)

    def __del__(self):
        self.dispose()

    def __str__(self):
        internal_type = self.dbgInternalType()
        if internal_type == "#02: <molecule>":
            return "molecule ({})".format(self.smiles())
        elif internal_type == "#03: <query molecule>":
            return "query_molecule ({})".format(self.smarts())
        return object.__str__(self)

    def dispose(self):
        if self.id >= 0:
            if getattr(Indigo, "_lib", None) is not None:
                self.dispatcher._setSessionId()
                Indigo._lib.indigoFree(self.id)
                self.id = -1

    def __iter__(self):
        return self

    def _next(self):
        self.dispatcher._setSessionId()
        newobj = self.dispatcher._checkResult(Indigo._lib.indigoNext(self.id))
        if newobj == 0:
            return None
        else:
            return self.dispatcher.IndigoObject(self.dispatcher, newobj, self)

    def __next__(self):
        obj = self._next()
        if obj is None:
            raise StopIteration
        return obj

    def next(self):
        """Generic iterator method

        Returns:
            IndigoObject: next item in the collection
        """
        return self.__next__()

    def oneBitsList(self):
        """Returns string representation of fingerprint

        Returns:
            str: one bits string for the fingerprint
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResultString(
            Indigo._lib.indigoOneBitsList(self.id)
        )

    def mdlct(self):
        """Gets MDL CT as a buffer

        Returns:
            IndigoObject: buffer containing the MDLCT
        """
        buf = self.dispatcher.writeBuffer()
        self.dispatcher._setSessionId()
        self.dispatcher._checkResult(
            Indigo._lib.indigoSaveMDLCT(self.id, buf.id)
        )
        return buf.toBuffer()

    def xyz(self):
        """Atom method gets coordinates of atom

        Raises:
            IndigoException: if no XYZ coordinates for the object

        Returns:
            list: 3-element array with coordinates
        """
        self.dispatcher._setSessionId()
        xyz = Indigo._lib.indigoXYZ(self.id)
        if xyz is None:
            raise IndigoException(Indigo._lib.indigoGetLastError())
        return [xyz[0], xyz[1], xyz[2]]

    def alignAtoms(self, atom_ids, desired_xyz):
        """Atom method determines and applies the best transformation for the given molecule
        so that the specified atoms move as close as possible to the desired positions

        Args:
            atom_ids (list): atom indexes
            desired_xyz (list): desired coordinates for atoms (size atom_ids * 3)

        Raises:
            IndigoException: if input array size does not match

        Returns:
            float: root-mean-square measure of the difference between the desired and obtained positions
        """
        if len(atom_ids) * 3 != len(desired_xyz):
            raise IndigoException(
                "alignAtoms(): desired_xyz[] must be exactly 3 times bigger than atom_ids[]"
            )
        atoms = (c_int * len(atom_ids))()
        for i in range(len(atoms)):
            atoms[i] = atom_ids[i]
        xyz = (c_float * len(desired_xyz))()
        for i in range(len(desired_xyz)):
            xyz[i] = desired_xyz[i]
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResultFloat(
            self.dispatcher._lib.indigoAlignAtoms(
                self.id, len(atoms), atoms, xyz
            )
        )

    def addStereocenter(self, type, v1, v2, v3, v4=-1):
        """Atom method adds stereo information for atom

        Args:
            type (int): Stereocenter type. Use Indigo constants ABS, OR, AND, EITHER
            v1 (int): pyramid info
            v2 (int): pyramid info
            v3 (int): pyramid info
            v4 (int): pyramid info. Optional, defaults to -1.

        Returns:
            int: 1 if stereocenter is added successfully
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoAddStereocenter(self.id, type, v1, v2, v3, v4)
        )

    def clone(self):
        """Clones IndigoObject

        Returns:
            IndigoObject: cloned object
        """
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(
            self.dispatcher,
            self.dispatcher._checkResult(Indigo._lib.indigoClone(self.id)),
        )

    def check(self, checkflags=""):
        """Molecule method verifies the structure

        Args:
            checkflags (str): Flags to verify. Optional, defaults to "".

        Returns:
            str: verification result as a JSON string
        """
        if checkflags is None:
            checkflags = ""
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResultString(
            Indigo._lib.indigoCheckObj(
                self.id, checkflags.encode(ENCODE_ENCODING)
            )
        )

    def close(self):
        """FileOutput method closes file descriptor

        Returns:
            int: 1 if file is closed successfully. -1 otherwise
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoClose(self.id))

    def hasNext(self):
        """Iterator method checks presence of a next element

        Returns:
            bool: true if collection has a next element, false otherwise
        """
        self.dispatcher._setSessionId()
        return bool(
            self.dispatcher._checkResult(Indigo._lib.indigoHasNext(self.id))
        )

    def index(self):
        """Atom method returns index of the element

        Returns:
            int: element index
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoIndex(self.id))

    def remove(self):
        """Container method removes the element from its container

        Returns:
            int: 1 if element was removed
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoRemove(self.id))

    def saveMolfile(self, filename):
        """Molecule method saves the structure into a Molfile

        Args:
            filename (str): full file path to the output file

        Returns:
            int: 1 if file is saved successfully
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoSaveMolfileToFile(
                self.id, filename.encode(ENCODE_ENCODING)
            )
        )

    def molfile(self):
        """Molecule method returns the structure as a string in Molfile format

        Returns:
            str: Molfile string
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResultString(
            Indigo._lib.indigoMolfile(self.id)
        )

    def saveCml(self, filename):
        """Molecule method saves the structure into a CML file

        Args:
            filename (str): full path to the output file

        Returns:
            int: 1 if the file is saved successfully
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoSaveCmlToFile(
                self.id, filename.encode(ENCODE_ENCODING)
            )
        )

    def cml(self):
        """Molecule method returns the structure as a string in CML format

        Returns:
            str: CML string
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResultString(
            Indigo._lib.indigoCml(self.id)
        )

    def saveCdxml(self, filename):
        """Molecule method saves the structure into a CDXML file

        Args:
            filename (str): full path to the output file

        Returns:
            int: 1 if the file is saved successfully
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoSaveCdxmlToFile(
                self.id, filename.encode(ENCODE_ENCODING)
            )
        )

    def cdxml(self):
        """Molecule method returns the structure as a string in CDXML format

        Returns:
            str: CDXML string
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResultString(
            Indigo._lib.indigoCdxml(self.id)
        )

    def json(self):
        """Structure method returns the structure as a string in KET format

        Returns:
            str: KET format for the structure
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResultString(
            Indigo._lib.indigoJson(self.id)
        )

    def saveMDLCT(self, output):
        """Structure method saves the structure in MDLCT format into a buffer

        Args:
            output (IndigoObject): buffer to be updated

        Returns:
            int: 1 if the structure is saved
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoSaveMDLCT(self.id, output.id)
        )

    def addReactant(self, molecule):
        """Reaction method adds the given molecule copy to reactants

        Args:
            molecule (IndigoObject): molecule to be added

        Returns:
            int: 1 if the molecule was added correctly
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoAddReactant(self.id, molecule.id)
        )

    def addProduct(self, molecule):
        """Reaction method adds the given molecule copy to products

        Args:
            molecule (IndigoObject): molecule to be added

        Returns:
            int: 1 if the molecule was added correctly
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoAddProduct(self.id, molecule.id)
        )

    def addCatalyst(self, molecule):
        """Reaction method adds the given molecule copy to catalysts

        Args:
            molecule (IndigoObject): molecule to be added

        Returns:
            int: 1 if the molecule was added correctly
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoAddCatalyst(self.id, molecule.id)
        )

    def countReactants(self):
        """Reaction method returns the number of reactants

        Returns:
            int: number of reactants
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoCountReactants(self.id)
        )

    def countProducts(self):
        """Reaction method returns rge number of products

        Returns:
            int: number of products
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoCountProducts(self.id)
        )

    def countCatalysts(self):
        """Reaction method returns the number of catalysts

        Returns:
            int: number of catalysts
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoCountCatalysts(self.id)
        )

    def countMolecules(self):
        """Reaction method returns the number of reactants, products, and catalysts

        Returns:
            int: number of reactants, products, and catalysts
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoCountMolecules(self.id)
        )

    def getMolecule(self, index):
        """Reaction method returns a molecule by index

        Args:
            index (int): molecule index

        Returns:
            IndigoObject: molecule object
        """
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(
            self.dispatcher,
            self.dispatcher._checkResult(
                Indigo._lib.indigoGetMolecule(self.id, index)
            ),
        )

    def iterateReactants(self):
        """Reaction method iterates reactants

        Returns:
            IndigoObject: reactant iterator
        """
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(
            self.dispatcher,
            self.dispatcher._checkResult(
                Indigo._lib.indigoIterateReactants(self.id)
            ),
        )

    def iterateProducts(self):
        """Reaction method iterates products

        Returns:
            IndigoObject: product iterator
        """
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(
            self.dispatcher,
            self.dispatcher._checkResult(
                Indigo._lib.indigoIterateProducts(self.id)
            ),
        )

    def iterateCatalysts(self):
        """Reaction method iterates catalysts

        Returns:
            IndigoObject: catalyst iterator
        """
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(
            self.dispatcher,
            self.dispatcher._checkResult(
                Indigo._lib.indigoIterateCatalysts(self.id)
            ),
        )

    def iterateMolecules(self):
        """Reaction method iterates molecules

        Returns:
            IndigoObject: reactant, products, and catalysts iterator
        """
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(
            self.dispatcher,
            self.dispatcher._checkResult(
                Indigo._lib.indigoIterateMolecules(self.id)
            ),
        )

    def saveRxnfile(self, filename):
        """Reaction method saves the reaction into an RXN file

        Args:
            filename (str): output file path for the reaction

        Returns:
            int: 1 if everything is saved without issues
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoSaveRxnfileToFile(
                self.id, filename.encode(ENCODE_ENCODING)
            )
        )

    def rxnfile(self):
        """Reaction method returns the reaction as a string in RXN format

        Returns:
            str: RXN string
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResultString(
            Indigo._lib.indigoRxnfile(self.id)
        )

    def optimize(self, options=""):
        """QueryReaction or QueryMolecule method for query optimizations for faster substructure search

        Args:
            options (str): Options for optimization. Optional, defaults to "".

        Returns:
            int: 1 if optimization is performed without issues
        """
        if options is None:
            options = ""
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoOptimize(
                self.id, options.encode(ENCODE_ENCODING)
            )
        )

    def normalize(self, options=""):
        """Molecule method for structure normalization.
        It neutralizes charges, resolves 5-valence Nitrogen, removes hydrogens, etc.

        Args:
            options (str): Normalization options. Optional, defaults to "".

        Returns:
            int: 1 if normalization is performed without issues
        """
        if options is None:
            options = ""
        self.dispatcher._setSessionId()
        return bool(
            self.dispatcher._checkResult(
                Indigo._lib.indigoNormalize(
                    self.id, options.encode(ENCODE_ENCODING)
                )
            )
        )

    def standardize(self):
        """Molecule method for structure standardization.
        It standardizes charges, stereo, etc.

        Returns:
            int: 1 if standardization is performed without issues
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoStandardize(self.id)
        )

    def ionize(self, pH, pH_toll):
        """Method for structure ionization at specified pH and pH tolerance

        Args:
            pH (float): pH value
            pH_toll (float): pH tolerance

        Returns:
            int: 1 if ionization is performed without issues
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoIonize(self.id, pH, pH_toll)
        )

    def getAcidPkaValue(self, atom, level, min_level):
        """Molecule method calculates acid pKa value

        Args:
            atom (int): input atom index
            level (int): pka level
            min_level (int): pka min level

        Returns:
            float: pka result
        """
        self.dispatcher._setSessionId()
        result = self.dispatcher._checkResultPtr(
            Indigo._lib.indigoGetAcidPkaValue(
                self.id, atom.id, level, min_level
            )
        )
        return result[0]

    def getBasicPkaValue(self, atom, level, min_level):
        """Molecule method calculates basic pKa value

        Args:
            atom (int): input atom index
            level (int): pka level
            min_level (int): pka min level

        Returns:
            float: pka result
        """
        self.dispatcher._setSessionId()
        result = self.dispatcher._checkResultPtr(
            Indigo._lib.indigoGetBasicPkaValue(
                self.id, atom.id, level, min_level
            )
        )
        return result[0]

    def automap(self, mode=""):
        """Automatic reaction atom-to-atom mapping

        Args:
            mode (str): mode is one of the following (separated by a space):
            "discard" : discards the existing mapping entirely and considers only
                        the existing reaction centers (the default)
            "keep"    : keeps the existing mapping and maps unmapped atoms
            "alter"   : alters the existing mapping, and maps the rest of the
                        reaction but may change the existing mapping
            "clear"   : removes the mapping from the reaction.

            "ignore_charges" : do not consider atom charges while searching
            "ignore_isotopes" : do not consider atom isotopes while searching
            "ignore_valence" : do not consider atom valence while searching
            "ignore_radicals" : do not consider atom radicals while searching

        Returns:
            int: 1 if atom mapping is done without errors
        """
        if mode is None:
            mode = ""
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoAutomap(self.id, mode.encode(ENCODE_ENCODING))
        )

    def atomMappingNumber(self, reaction_atom):
        """Reaction atom method returns assigned mapping

        Args:
            reaction_atom (IndigoObject): reaction molecule atom

        Returns:
            int: atom mapping value
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoGetAtomMappingNumber(self.id, reaction_atom.id)
        )

    def setAtomMappingNumber(self, reaction_atom, number):
        """Reaction atom method sets atom mapping

        Args:
            reaction_atom (IndigoObject): reaction molecule atom
            number (int): atom mapping

        Returns:
            int: 1 if atom mapping is set
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoSetAtomMappingNumber(
                self.id, reaction_atom.id, number
            )
        )

    def reactingCenter(self, reaction_bond):
        """Reaction bond method returns reacting center

        Args:
            reaction_bond (IndigoObject): reaction molecule bond

        Returns:
            int: reacting center enum. One of values
                * RC_NOT_CENTER = -1
                * RC_UNMARKED = 0
                * RC_CENTER = 1
                * RC_UNCHANGED = 2
                * RC_MADE_OR_BROKEN = 4
                * RC_ORDER_CHANGED = 8
        """
        value = c_int()
        self.dispatcher._setSessionId()
        res = self.dispatcher._checkResult(
            Indigo._lib.indigoGetReactingCenter(
                self.id, reaction_bond.id, pointer(value)
            )
        )
        if res == 0:
            return None
        return value.value

    def setReactingCenter(self, reaction_bond, rc):
        """Reaction bond method sets reacting center

        Args:
            reaction_bond (IndigoObject): reaction molecule bond
            rc (int): reacting center, one of the following
                * RC_NOT_CENTER = -1
                * RC_UNMARKED = 0
                * RC_CENTER = 1
                * RC_UNCHANGED = 2
                * RC_MADE_OR_BROKEN = 4
                * RC_ORDER_CHANGED = 8

        Returns:
            int: 1 if there are no errors
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoSetReactingCenter(self.id, reaction_bond.id, rc)
        )

    def clearAAM(self):
        """Reaction method clears atom mapping for atoms

        Returns:
            int: 1 if there are no errors
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoClearAAM(self.id)
        )

    def correctReactingCenters(self):
        """Reaction method corrects reacting centers according to AAM

        Returns:
            int: 1 if there are no errors
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoCorrectReactingCenters(self.id)
        )

    def iterateAtoms(self):
        """Molecule method returns an iterator for all atoms
        including r-sites and pseudoatoms

        Returns:
            IndigoObject: atom iterator
        """
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(
            self.dispatcher,
            self.dispatcher._checkResult(
                Indigo._lib.indigoIterateAtoms(self.id)
            ),
        )

    def iteratePseudoatoms(self):
        """Molecule method returns an iterator for all pseudoatoms

        Returns:
            IndigoObject: atom iterator
        """
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(
            self.dispatcher,
            self.dispatcher._checkResult(
                Indigo._lib.indigoIteratePseudoatoms(self.id)
            ),
        )

    def iterateRSites(self):
        """Molecule method returns an iterator for all r-sites

        Returns:
            IndigoObject: atom iterator
        """
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(
            self.dispatcher,
            self.dispatcher._checkResult(
                Indigo._lib.indigoIterateRSites(self.id)
            ),
        )

    def iterateStereocenters(self):
        """Molecule method returns an iterator for all atoms with stereocenters

        Returns:
            IndigoObject: atom iterator
        """
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(
            self.dispatcher,
            self.dispatcher._checkResult(
                Indigo._lib.indigoIterateStereocenters(self.id)
            ),
        )

    def iterateAlleneCenters(self):
        """Molecule method returns an iterator for all atoms with allene centers

        Returns:
            IndigoObject: atom iterator
        """
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(
            self.dispatcher,
            self.dispatcher._checkResult(
                Indigo._lib.indigoIterateAlleneCenters(self.id)
            ),
        )

    def iterateRGroups(self):
        """Molecule method returns an iterator for all r-group

        Returns:
            IndigoObject: r-group iterator
        """
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(
            self.dispatcher,
            self.dispatcher._checkResult(
                Indigo._lib.indigoIterateRGroups(self.id)
            ),
        )

    def countRGroups(self):
        """Molecule method returns the number of r-groups

        Returns:
            int: number of r-groups
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoCountRGroups(self.id)
        )

    def isPseudoatom(self):
        """Atom method returns true if atom is pseudoatom

        Returns:
            bool: True if pseudoatom
        """
        self.dispatcher._setSessionId()
        return bool(
            self.dispatcher._checkResult(
                Indigo._lib.indigoIsPseudoatom(self.id)
            )
        )

    def isRSite(self):
        """Atom method returns true if atom is R-site

        Returns:
            bool: True if R-site
        """
        self.dispatcher._setSessionId()
        return bool(
            self.dispatcher._checkResult(Indigo._lib.indigoIsRSite(self.id))
        )

    def isTemplateAtom(self):
        """Atom method returns true if atom is a template atom

        Returns:
            bool: True if template
        """
        self.dispatcher._setSessionId()
        return bool(
            self.dispatcher._checkResult(
                Indigo._lib.indigoIsTemplateAtom(self.id)
            )
        )

    def stereocenterType(self):
        """Atom method returns stereo center type

        Returns:
            int: type of stereocenter
                * ABS = 1
                * OR = 2
                * AND = 3
                * EITHER = 4
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoStereocenterType(self.id)
        )

    def stereocenterGroup(self):
        """Atom method returns stereocenter group

        Returns:
            int: group index
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoStereocenterGroup(self.id)
        )

    def setStereocenterGroup(self, group):
        """Atom method sets stereocenter group

        Args:
            group (int): group index
        """
        self.dispatcher._setSessionId()
        self.dispatcher._checkResult(
            Indigo._lib.indigoSetStereocenterGroup(self.id, group)
        )

    def changeStereocenterType(self, type):
        """Atom method changes stereocenter type

        Args:
            type (int): stereo type.
                * ABS = 1
                * OR = 2
                * AND = 3
                * EITHER = 4

        Returns:
            int: 1 if there are no errors
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoChangeStereocenterType(self.id, type)
        )

    def validateChirality(self):
        """Molecule or reaction method validates chirality"""
        self.dispatcher._setSessionId()
        self.dispatcher._checkResult(
            Indigo._lib.indigoValidateChirality(self.id)
        )

    def singleAllowedRGroup(self):
        """Atom method returns single allowed r-group

        Returns:
            int: single allowed r-group
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoSingleAllowedRGroup(self.id)
        )

    def iterateRGroupFragments(self):
        """RGroup method iterates r-group fragments

        Returns:
            IndigoObject: r-group fragment iterator
        """
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(
            self.dispatcher,
            self.dispatcher._checkResult(
                Indigo._lib.indigoIterateRGroupFragments(self.id)
            ),
        )

    def countAttachmentPoints(self):
        """Molecule or RGroup method returns the number of attachment points

        Returns:
            int: number of attachment points
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoCountAttachmentPoints(self.id)
        )

    def iterateAttachmentPoints(self, order):
        """Molecule method iterates attachment points

        Args:
            order (int): attachment points order

        Returns:
            IndigoObject: attachment points iterator
        """
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(
            self.dispatcher,
            self.dispatcher._checkResult(
                Indigo._lib.indigoIterateAttachmentPoints(self.id, order)
            ),
        )

    def symbol(self):
        """Atom method returns string symbol

        Returns:
            str: atom symbol
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResultString(
            Indigo._lib.indigoSymbol(self.id)
        )

    def degree(self):
        """Atom method returns the atom number of neighbors

        Returns:
            int: number of atom neighbors
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoDegree(self.id))

    def charge(self):
        """Atom method returns the charge of the atom

        Returns:
            int: charge
        """
        value = c_int()
        self.dispatcher._setSessionId()
        res = self.dispatcher._checkResult(
            Indigo._lib.indigoGetCharge(self.id, pointer(value))
        )
        if res == 0:
            return None
        return value.value

    def getHybridization(self):
        """Atom method returns HybridizationType

        Returns:
            Hybridization: atom hybridization
        """
        self.dispatcher._setSessionId()
        return Hybridization(
            self.dispatcher._checkResult(
                Indigo._lib.indigoGetHybridization(self.id)
            )
        )

    def getHybridizationStr(self):
        """Atom method returns hybridization type string

        Returns:
            str: atom hybridization
        """
        return self.getHybridization().name

    def getExplicitValence(self):
        """Atom method returns the explicit valence

        Returns:
            int: valence
        """
        value = c_int()
        self.dispatcher._setSessionId()
        res = self.dispatcher._checkResult(
            Indigo._lib.indigoGetExplicitValence(self.id, pointer(value))
        )
        if res == 0:
            return None
        return value.value

    def setExplicitValence(self, valence):
        """Atom method sets the explicit valence

        Args:
            valence (int): valence

        Returns:
            int: 1 if there are no errors
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoSetExplicitValence(self.id, valence)
        )

    def radicalElectrons(self):
        """Atom method returns the number of radical electrons

        Returns:
            int: radical electrons number
        """
        value = c_int()
        self.dispatcher._setSessionId()
        res = self.dispatcher._checkResult(
            Indigo._lib.indigoGetRadicalElectrons(self.id, pointer(value))
        )
        if res == 0:
            return None
        return value.value

    def radical(self):
        """Atom method returns the radical value

        Returns:
            int: radical value
        """
        value = c_int()
        self.dispatcher._setSessionId()
        res = self.dispatcher._checkResult(
            Indigo._lib.indigoGetRadical(self.id, pointer(value))
        )
        if res == 0:
            return None
        return value.value

    def setRadical(self, radical):
        """Atom method sets the radical value

        Args:
            radical (int): radical value

        Returns:
            int: 1 if there are no errors
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoSetRadical(self.id, radical)
        )

    def atomicNumber(self):
        """Atom method returns the atomic number

        Returns:
            int: atomic number
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoAtomicNumber(self.id)
        )

    def isotope(self):
        """Atom method returns the isotope number

        Returns:
            int: isotope number
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoIsotope(self.id))

    def valence(self):
        """Atom method returns the valence

        Returns:
            int: atom valence
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoValence(self.id))

    def checkValence(self):
        """Atom method validates the valence

        Returns:
            int: 1 if valence has no errors, 0 otherwise
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoCheckValence(self.id)
        )

    def checkQuery(self):
        """Atom, Bond, Molecule, Reaction method verifies if object is query

        Returns:
            int: 1 if object is query, 0 otherwise
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoCheckQuery(self.id)
        )

    def checkRGroups(self):
        """Molecule method verifies if the structure contains r-groups

        Returns:
            int: 1 if molecule contains r-groups, 0 otherwise
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoCheckRGroups(self.id)
        )

    def checkChirality(self):
        """Molecule method verifies if the structure has a chiral flag

        Returns:
            int: 1 if there is chiral flag, 0 otherwise
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoCheckChirality(self.id)
        )

    def check3DStereo(self):
        """Molecule method verifies if the structure contains 3d stereo

        Returns:
            int: 1 if structure contains 3d stereo, 0 otherwise
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoCheck3DStereo(self.id)
        )

    def checkStereo(self):
        """Molecule method verifies if the structure contains stereocenters

        Returns:
            int: 1 if molecule contains stereo, 0 otherwise
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoCheckStereo(self.id)
        )

    def checkSalt(self):
        """Molecule method verifies if the structure contains salt.

        Returns:
            bool: True if structure contains salt
        """
        self.dispatcher._setSessionId()
        for target_fragment in self.iterateComponents():
            target_fragment = target_fragment.clone()
            for salt in SALTS:
                query_salt = self.dispatcher.loadQueryMolecule(salt)
                matcher = self.dispatcher.substructureMatcher(target_fragment)
                if matcher.match(query_salt):
                    return True
        return False

    def stripSalt(self, inplace=False):
        """Molecule method strips all inorganic components.

        Args:
            inplace(bool): if False - returns the copy of the molecule,
                           without inorganic components, if True - strips
                           inorganic components from the molecule itself.

        Returns:
            IndigoObject: if inplace=False - new molecule without inorganic
                          components,
                          if inplace=True - initial molecule without inorganic
                          components.
        """
        self.dispatcher._setSessionId()
        salts_atoms = []
        idx = 0
        for target_fragment in self.iterateComponents():
            target_fragment = target_fragment.clone()
            n_atoms = target_fragment.countAtoms()

            for salt in SALTS:
                query_salt = self.dispatcher.loadQueryMolecule(salt)
                matcher = self.dispatcher.substructureMatcher(target_fragment)
                if matcher.match(query_salt):
                    salt_position = [i for i in range(idx, idx + n_atoms)]
                    salts_atoms.extend(salt_position)
            idx += n_atoms

        if not inplace:
            saltless_fragment = self.clone()
            saltless_fragment.removeAtoms(salts_atoms)
            return saltless_fragment
        else:
            self.removeAtoms(salts_atoms)
            return self

    def countHydrogens(self):
        """Atom or Molecule method returns the number of hydrogens

        Returns:
            int: number of hydrogens
        """
        value = c_int()
        self.dispatcher._setSessionId()
        res = self.dispatcher._checkResult(
            Indigo._lib.indigoCountHydrogens(self.id, pointer(value))
        )
        if res == 0:
            return None
        return value.value

    def countImplicitHydrogens(self):
        """Atom or Molecule method returns the number of implicit hydrogens

        Returns:
            int: number of hydrogens
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoCountImplicitHydrogens(self.id)
        )

    def setXYZ(self, x, y, z):
        """Atom methods sets the given coordinates

        Args:
            x (float): X coordinate
            y (float): Y coordinate
            z (float): Z coordinate

        Returns:
            int: 1 if there are no errors
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoSetXYZ(self.id, x, y, z)
        )

    def countSuperatoms(self):
        """Molecule method calculates the number of super atoms

        Returns:
            int: number of super atoms
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoCountSuperatoms(self.id)
        )

    def countDataSGroups(self):
        """Molecule method returns the number of data s-groups

        Returns:
            int: number of s-groups
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoCountDataSGroups(self.id)
        )

    def countRepeatingUnits(self):
        """Molecule method returns the number of repeating units

        Returns:
            int: number of repeating units
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoCountRepeatingUnits(self.id)
        )

    def countMultipleGroups(self):
        """Molecule method returns the number of multiple s-groups

        Returns:
            int: number of multiple s-groups
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoCountMultipleGroups(self.id)
        )

    def countGenericSGroups(self):
        """Molecule method returns the number of generic s-groups

        Returns:
            int: number of generic s-groups
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoCountGenericSGroups(self.id)
        )

    def iterateDataSGroups(self):
        """Molecule method iterates data s-groups

        Returns:
            IndigoObject: s-groups iterator
        """
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(
            self.dispatcher,
            self.dispatcher._checkResult(
                Indigo._lib.indigoIterateDataSGroups(self.id)
            ),
        )

    def iterateSuperatoms(self):
        """Molecule method iterates superatoms

        Returns:
            IndigoObject: superatoms iterator
        """
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(
            self.dispatcher,
            self.dispatcher._checkResult(
                Indigo._lib.indigoIterateSuperatoms(self.id)
            ),
        )

    def iterateGenericSGroups(self):
        """Molecule method iterates generic s-groups

        Returns:
            IndigoObject: generic s-groups iterator
        """
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(
            self.dispatcher,
            self.dispatcher._checkResult(
                Indigo._lib.indigoIterateGenericSGroups(self.id)
            ),
        )

    def iterateRepeatingUnits(self):
        """Molecule method iterates repeating units

        Returns:
            IndigoObject: repeating units iterator
        """
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(
            self.dispatcher,
            self.dispatcher._checkResult(
                Indigo._lib.indigoIterateRepeatingUnits(self.id)
            ),
        )

    def iterateMultipleGroups(self):
        """Molecule method iterates Multiple s-groups

        Returns:
            IndigoObject: Mul s-groups iterator
        """
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(
            self.dispatcher,
            self.dispatcher._checkResult(
                Indigo._lib.indigoIterateMultipleGroups(self.id)
            ),
        )

    def iterateSGroups(self):
        """Molecule method iterates s-groups

        Returns:
            IndigoObject: s-groups iterator
        """
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(
            self.dispatcher,
            self.dispatcher._checkResult(
                Indigo._lib.indigoIterateSGroups(self.id)
            ),
        )

    def iterateTGroups(self):
        """Molecule method iterates t-groups

        Returns:
            IndigoObject: t-groups iterator
        """
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(
            self.dispatcher,
            self.dispatcher._checkResult(
                Indigo._lib.indigoIterateTGroups(self.id)
            ),
        )

    def getSuperatom(self, index):
        """Molecule method returns a superatom by index

        Args:
            index (int): super atom index

        Returns:
            IndigoObject: super atom
        """
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(
            self.dispatcher,
            self.dispatcher._checkResult(
                Indigo._lib.indigoGetSuperatom(self.id, index)
            ),
        )

    def getDataSGroup(self, index):
        """Molecule method returns a data s-group by index

        Args:
            index (int): sgroup index

        Returns:
            IndigoObject: data sgroup
        """
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(
            self.dispatcher,
            self.dispatcher._checkResult(
                Indigo._lib.indigoGetDataSGroup(self.id, index)
            ),
        )

    def getGenericSGroup(self, index):
        """Molecule method returns a generic s-group by index

        Args:
            index (int): s-group index

        Returns:
            IndigoObject: generic s-group
        """
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(
            self.dispatcher,
            self.dispatcher._checkResult(
                Indigo._lib.indigoGetGenericSGroup(self.id, index)
            ),
        )

    def getMultipleGroup(self, index):
        """Molecule method returns a Multiple s-group by index

        Args:
            index (int): mul s-group index

        Returns:
            IndigoObject: mul s-group
        """
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(
            self.dispatcher,
            self.dispatcher._checkResult(
                Indigo._lib.indigoGetMultipleGroup(self.id, index)
            ),
        )

    def getRepeatingUnit(self, index):
        """Molecule method returns a repeating unit by index

        Args:
            index (int): repeating unit index

        Returns:
            IndigoObject: repeating unit
        """
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(
            self.dispatcher,
            self.dispatcher._checkResult(
                Indigo._lib.indigoGetRepeatingUnit(self.id, index)
            ),
        )

    def description(self):
        """Data s-group method returns description

        Returns:
            str: s-group description
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResultString(
            Indigo._lib.indigoDescription(self.id)
        )

    def data(self):
        """Data s-group method returns data

        Returns:
            str: s-group data
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResultString(
            Indigo._lib.indigoData(self.id)
        )

    def addDataSGroup(self, atoms, bonds, description, data):
        """Molecule method adds a data s-group

        Args:
            atoms (list): atom indexes list
            bonds (list): bond indexes list
            description (str): data s-group description
            data (str): data s-group data

        Returns:
            IndigoObject: SGroup object
        """
        arr2 = (c_int * len(atoms))()
        for i in range(len(atoms)):
            arr2[i] = atoms[i]
        arr4 = (c_int * len(bonds))()
        for i in range(len(bonds)):
            arr4[i] = bonds[i]
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(
            self.dispatcher,
            self.dispatcher._checkResult(
                Indigo._lib.indigoAddDataSGroup(
                    self.id,
                    len(arr2),
                    arr2,
                    len(arr4),
                    arr4,
                    description.encode(ENCODE_ENCODING),
                    data.encode(ENCODE_ENCODING),
                )
            ),
        )

    def addSuperatom(self, atoms, name):
        """Molecule method adds superatom

        Args:
            atoms (list): atom indexes list
            name (str): superatom name

        Returns:
            IndigoObject: superatom object
        """
        arr2 = (c_int * len(atoms))()
        for i in range(len(atoms)):
            arr2[i] = atoms[i]
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(
            self.dispatcher,
            self.dispatcher._checkResult(
                Indigo._lib.indigoAddSuperatom(
                    self.id, len(arr2), arr2, name.encode(ENCODE_ENCODING)
                )
            ),
        )

    def setDataSGroupXY(self, x, y, options=""):
        """SGroup method sets coordinates

        Args:
            x (float): X coordinate
            y (float): Y coordinate
            options (str): options. Optional, defaults to "".

        Returns:
            int: 1 if there are no errors
        """
        self.dispatcher._setSessionId()
        if options is None:
            options = ""
        return self.dispatcher._checkResult(
            Indigo._lib.indigoSetDataSGroupXY(
                self.id, x, y, options.encode(ENCODE_ENCODING)
            )
        )

    def setSGroupData(self, data):
        """SGroup method adds data

        Args:
            data (str): data string

        Returns:
            int: 1 if there are no errors
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoSetSGroupData(
                self.id, data.encode(ENCODE_ENCODING)
            )
        )

    def setSGroupCoords(self, x, y):
        """SGroup method sets coordinates

        Args:
            x (float): X coordinate
            y (float): Y coordinate

        Returns:
            int: 1 if there are no errors
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoSetSGroupCoords(self.id, x, y)
        )

    def setSGroupDescription(self, description):
        """SGroup method sets description

        Args:
            description (str): description string

        Returns:
            int: 1 if there are no errors
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoSetSGroupDescription(
                self.id, description.encode(ENCODE_ENCODING)
            )
        )

    def setSGroupFieldName(self, name):
        """SGroup method sets field name

        Args:
            name (str): name string

        Returns:
            int: 1 if there are no errors
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoSetSGroupFieldName(
                self.id, name.encode(ENCODE_ENCODING)
            )
        )

    def setSGroupQueryCode(self, code):
        """SGroup methods sets query code

        Args:
            code (str): code string

        Returns:
            int: 1 if there are no errors
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoSetSGroupQueryCode(
                self.id, code.encode(ENCODE_ENCODING)
            )
        )

    def setSGroupQueryOper(self, oper):
        """SGroup method sets query oper

        Args:
            oper (str): query oper string

        Returns:
            int: 1 if there are no errors
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoSetSGroupQueryOper(
                self.id, oper.encode(ENCODE_ENCODING)
            )
        )

    def setSGroupDisplay(self, option):
        """SGroup method sets display

        Args:
            option (str): display string

        Returns:
            int: 1 if there are no errors
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoSetSGroupDisplay(
                self.id, option.encode(ENCODE_ENCODING)
            )
        )

    def setSGroupLocation(self, option):
        """SGroup method sets location

        Args:
            option (str): location string

        Returns:
            int: 1 if there are no errors
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoSetSGroupLocation(
                self.id, option.encode(ENCODE_ENCODING)
            )
        )

    def setSGroupTag(self, tag):
        """SGroup method sets tag

        Args:
            tag (str): tag string

        Returns:
            int: 1 if there are no errors
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoSetSGroupTag(
                self.id, tag.encode(ENCODE_ENCODING)
            )
        )

    def setSGroupTagAlign(self, tag_align):
        """SGroup method sets tag align

        Args:
            tag_align (int): tag align value

        Returns:
            int: 1 if there are no errors
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoSetSGroupTagAlign(self.id, tag_align)
        )

    def setSGroupDataType(self, data_type):
        """SGroup method sets data type

        Args:
            data_type (str): data type string

        Returns:
            int: 1 if there are no errors
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoSetSGroupDataType(
                self.id, data_type.encode(ENCODE_ENCODING)
            )
        )

    def setSGroupXCoord(self, x):
        """Sgroup method sets X coordinate

        Args:
            x (float): X coordinate

        Returns:
            int: 1 if there are no errors
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoSetSGroupXCoord(self.id, x)
        )

    def setSGroupYCoord(self, y):
        """Sgroup method sets Y coordinate

        Args:
            y (float): Y coordinate

        Returns:
            int: 1 if there are no errors
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoSetSGroupYCoord(self.id, y)
        )

    def createSGroup(self, sgtype, mapping, name):
        """Molecule method creates an SGroup

        Args:
            sgtype (str): sgroup type
            mapping (IndigoObject): mapping object
            name (str): sgroup name

        Returns:
            IndigoObject: sgroup object
        """
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(
            self.dispatcher,
            self.dispatcher._checkResult(
                Indigo._lib.indigoCreateSGroup(
                    sgtype.encode(ENCODE_ENCODING),
                    mapping.id,
                    name.encode(ENCODE_ENCODING),
                )
            ),
        )

    def setSGroupClass(self, sgclass):
        """SGroup method sets class

        Args:
            sgclass (str): sgroup class

        Returns:
            int: 1 if there are no errors
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoSetSGroupClass(
                self.id, sgclass.encode(ENCODE_ENCODING)
            )
        )

    def setSGroupName(self, sgname):
        """SGroup method sets group name

        Args:
            sgname (str): sgroup name string

        Returns:
            int: 1 if there are no errors
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoSetSGroupName(
                self.id, sgname.encode(ENCODE_ENCODING)
            )
        )

    def getSGroupClass(self):
        """SGroup method returns sgroup class

        Returns:
            str: sgroup class string
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResultString(
            Indigo._lib.indigoGetSGroupClass(self.id)
        )

    def getSGroupName(self):
        """SGroup method returns sgroup name

        Returns:
            str: sgroup name string
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResultString(
            Indigo._lib.indigoGetSGroupName(self.id)
        )

    def getSGroupNumCrossBonds(self):
        """SGroup method returns the number of cross bonds

        Returns:
            int: number of cross bonds
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoGetSGroupNumCrossBonds(self.id)
        )

    def addSGroupAttachmentPoint(self, aidx, lvidx, apid):
        """SGroup method sets attachment point info

        Args:
            aidx (int): index
            lvidx (int): index
            apid (str): id string

        Returns:
            int: attachment point index
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoAddSGroupAttachmentPoint(
                self.id, aidx, lvidx, apid.encode(ENCODE_ENCODING)
            )
        )

    def deleteSGroupAttachmentPoint(self, apidx):
        """SGroup method removes the attachment point

        Args:
            apidx (int): attachment point index

        Returns:
            int: 1 if there are no errors
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoDeleteSGroupAttachmentPoint(self.id, apidx)
        )

    def getSGroupDisplayOption(self):
        """SGroup method returns display option

        Returns:
            int: display option
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoGetSGroupDisplayOption(self.id)
        )

    def setSGroupDisplayOption(self, option):
        """SGroup method sets display option

        Args:
            option (int): display option

        Returns:
            int: 1 if there are no errors
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoSetSGroupDisplayOption(self.id, option)
        )

    def getSGroupSeqId(self):
        """SGroup method returns SEQID

        Returns:
            int: SEQID value
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoGetSGroupSeqId(self.id)
        )

    def getSGroupCoords(self):
        """Sgroup method returns coordinates

        Raises:
            IndigoException: if no coordinates exist for the sgroup

        Returns:
            list: [x, y] coordinates
        """
        self.dispatcher._setSessionId()
        xyz = Indigo._lib.indigoGetSGroupCoords(self.id)
        if xyz is None:
            raise IndigoException(Indigo._lib.indigoGetLastError())
        return [xyz[0], xyz[1]]

    def getRepeatingUnitSubscript(self):
        """Repeating unit method returns subscript

        Returns:
            str: subscript value
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResultString(
            Indigo._lib.indigoGetRepeatingUnitSubscript(self.id)
        )

    def getRepeatingUnitConnectivity(self):
        """Repeating unit method returns connectivity

        Returns:
            int: connectivity value
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoGetRepeatingUnitConnectivity(self.id)
        )

    def getSGroupMultiplier(self):
        """Multiple group method returns multiplier

        Returns:
            int: multiplier value
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoGetSGroupMultiplier(self.id)
        )

    def setSGroupMultiplier(self, mult):
        """Multiple group sets multiplier value

        Args:
            mult (int): multiplier value

        Returns:
            int: 1 if there are no errors
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoSetSGroupMultiplier(self.id, mult)
        )

    def setSGroupBrackets(self, style, x1, y1, x2, y2, x3, y3, x4, y4):
        """SGroup method sets brackets

        Args:
            style (int): bracket style
            x1 (float): X1 coordinate
            y1 (float): Y1 coordinate
            x2 (float): X2 coordinate
            y2 (float): Y2 coordinate
            x3 (float): X3 coordinate
            y3 (float): Y3 coordinate
            x4 (float): X4 coordinate
            y4 (float): Y4 coordinate

        Returns:
            int: 1 if there are no errors
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoSetSGroupBrackets(
                self.id, style, x1, y1, x2, y2, x3, y3, x4, y4
            )
        )

    def findSGroups(self, prop, val):
        """Molecule method finds SGroup by property and value

        Args:
            prop (str): property string
            val (str): value string

        Returns:
            IndigoObject: SGroup iterator
        """
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(
            self.dispatcher,
            self.dispatcher._checkResult(
                Indigo._lib.indigoFindSGroups(
                    self.id,
                    prop.encode(ENCODE_ENCODING),
                    val.encode(ENCODE_ENCODING),
                )
            ),
        )

    def getSGroupType(self):
        """SGroup method returns type

        Returns:
            int: sgroup type
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoGetSGroupType(self.id)
        )

    def getSGroupIndex(self):
        """SGroup method returns index

        Returns:
            int: sgroup index
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoGetSGroupIndex(self.id)
        )

    def getSGroupOriginalId(self):
        """SGroup method returns original id

        Returns:
            int: original id
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoGetSGroupOriginalId(self.id)
        )

    def setSGroupOriginalId(self, original):
        """SGroup method sets original id

        Args:
            original (int): original id value

        Returns:
            int: 1 if there are no errors
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoSetSGroupOriginalId(self.id, original)
        )

    def getSGroupParentId(self):
        """SGroup method returns parent id

        Returns:
            int: parent id
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoGetSGroupParentId(self.id)
        )

    def setSGroupParentId(self, parent):
        """SGroup method sets parent id

        Args:
            parent (int): parent id

        Returns:
            int: 1 if there are no errors
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoSetSGroupParentId(self.id, parent)
        )

    def addTemplate(self, templates, name):
        """Molecule method adds template TGroup

        Args:
            templates (IndigoObject): molecule template
            name (str): template name

        Returns:
            int: template index
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoAddTemplate(
                self.id, templates.id, name.encode(ENCODE_ENCODING)
            )
        )

    def removeTemplate(self, name):
        """Molecule method removes template TGroup by name

        Args:
            name (str): template name

        Returns:
            int: 1 if there are no errors
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoRemoveTemplate(
                self.id, name.encode(ENCODE_ENCODING)
            )
        )

    def findTemplate(self, name):
        """Molecule method finds template by name

        Args:
            name (str): template name

        Returns:
            int: template index
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoFindTemplate(
                self.id, name.encode(ENCODE_ENCODING)
            )
        )

    def getTGroupClass(self):
        """TGroup method returns class

        Returns:
            str: class value
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResultString(
            Indigo._lib.indigoGetTGroupClass(self.id)
        )

    def getTGroupName(self):
        """TGroup method returns name

        Returns:
            str: name value
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResultString(
            Indigo._lib.indigoGetTGroupName(self.id)
        )

    def getTGroupAlias(self):
        """TGroup method returns alias

        Returns:
            str: alias value
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResultString(
            Indigo._lib.indigoGetTGroupAlias(self.id)
        )

    def transformSCSRtoCTAB(self):
        """Molecule method transforms SCSR to full CTAB

        Returns:
            int: 1 if there are no errors
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoTransformSCSRtoCTAB(self.id)
        )

    def transformCTABtoSCSR(self, templates):
        """Molecule method transforms CTAB to SCSR using templates

        Args:
            templates (IndigoObject): Molecule object with templates

        Returns:
            int: 1 if there are no errors
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoTransformCTABtoSCSR(self.id, templates.id)
        )

    def getTemplateAtomClass(self):
        """Atom method returns template class

        Returns:
            str: template class
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoGetTemplateAtomClass(self.id)
        )

    def setTemplateAtomClass(self, name):
        """Atom method sets template class

        Args:
            name (str): class name

        Returns:
            int: 1 if there are no errors
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoSetTemplateAtomClass(
                self.id, name.encode(ENCODE_ENCODING)
            )
        )

    def clean2d(self):
        """Molecule or reaction method recalculates coordinates

        Returns:
            int: 1 if there are no errors
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoClean2d(self.id))

    def resetCharge(self):
        """Atom method resets charge

        Returns:
            int: 1 if there are no errors
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoResetCharge(self.id)
        )

    def resetExplicitValence(self):
        """Atom method resets explicit valence

        Returns:
            int: 1 if there are no errors
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoResetExplicitValence(self.id)
        )

    def resetRadical(self):
        """Atom method resets radical

        Returns:
            int: 1 if there are no errors
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoResetRadical(self.id)
        )

    def resetIsotope(self):
        """Atom method resets isotope

        Returns:
            int: 1 if there are no errors
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoResetIsotope(self.id)
        )

    def setAttachmentPoint(self, order):
        """Atom method sets attachment point

        Args:
            order (int): attachment point order

        Returns:
            int: 1 if there are no errors
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoSetAttachmentPoint(self.id, order)
        )

    def clearAttachmentPoints(self):
        """Atom method clears attachment points

        Returns:
            int: 1 if there are no errors
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoClearAttachmentPoints(self.id)
        )

    def removeConstraints(self, type):
        """Atom method removes constraints

        Args:
            type (str): constraint type

        Returns:
            int: 1 if there are no errors
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoRemoveConstraints(
                self.id, type.encode(ENCODE_ENCODING)
            )
        )

    def addConstraint(self, type, value):
        """Atom method adds a constraint

        Args:
            type (str): constraint type
            value (str): constraint value

        Returns:
            int: 1 if there are no errors
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoAddConstraint(
                self.id,
                type.encode(ENCODE_ENCODING),
                value.encode(ENCODE_ENCODING),
            )
        )

    def addConstraintNot(self, type, value):
        """Atom method adds a NOT constraint

        Args:
            type (str): constraint type
            value (str): constraint value

        Returns:
            int: 1 if there are no errors
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoAddConstraintNot(
                self.id,
                type.encode(ENCODE_ENCODING),
                value.encode(ENCODE_ENCODING),
            )
        )

    def addConstraintOr(self, type, value):
        """Atom method adds an OR constraint

        Args:
            type (str): constraint type
            value (str): constraint value

        Returns:
            int: 1 if there are no errors
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoAddConstraintOr(
                self.id,
                type.encode(ENCODE_ENCODING),
                value.encode(ENCODE_ENCODING),
            )
        )

    def resetStereo(self):
        """Atom or bond method resets stereo

        Returns:
            int: 1 if there are no errors
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoResetStereo(self.id)
        )

    def invertStereo(self):
        """Atom or bond method inverts stereo

        Returns:
            int: 1 if there are no errors
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoInvertStereo(self.id)
        )

    def countAtoms(self):
        """Molecule or SGroup method returns the number of atoms

        Returns:
            int: number of atoms
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoCountAtoms(self.id)
        )

    def countBonds(self):
        """Molecule or SGroup method returns the number of bonds

        Returns:
            int: number of bonds
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoCountBonds(self.id)
        )

    def countPseudoatoms(self):
        """Molecule method returns the number of pseudoatoms

        Returns:
            int: number of pseudoatoms
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoCountPseudoatoms(self.id)
        )

    def countRSites(self):
        """Molecule method returns the number of r-sites

        Returns:
            int: number of r-sites
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoCountRSites(self.id)
        )

    def iterateBonds(self):
        """Molecule or SGroup method returns bonds iterator

        Returns:
            IndigoObject: bonds iterator
        """
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(
            self.dispatcher,
            self.dispatcher._checkResult(
                Indigo._lib.indigoIterateBonds(self.id)
            ),
        )

    def logP(self):
        """Molecule method returns calculated Crippen logP value

        Returns:
            float: calculated logP value of the molecule
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResultFloat(
            Indigo._lib.indigoLogP(self.id)
        )

    def molarRefractivity(self):
        """Molecule method returns calculated Crippen molar refractivity

        Returns:
            float: calculated value of molar refractivity
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResultFloat(
            Indigo._lib.indigoMolarRefractivity(self.id)
        )

    def bondOrder(self):
        """Bond method returns bond order

        Returns:
            int: bond order
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoBondOrder(self.id)
        )

    def bondStereo(self):
        """Bond method returns bond stereo

        Returns:
            int: bond stereo
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoBondStereo(self.id)
        )

    def topology(self):
        """Bond method returns bond topology

        Returns:
            int: bond topology
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoTopology(self.id)
        )

    def iterateNeighbors(self):
        """Atom method returns neighbors iterator

        Returns:
            IndigoObject: atom neighbor iterator
        """
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(
            self.dispatcher,
            self.dispatcher._checkResult(
                Indigo._lib.indigoIterateNeighbors(self.id)
            ),
        )

    def bond(self):
        """Atom neighbor method returns bond

        Returns:
            IndigoObject: bond object
        """
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(
            self.dispatcher,
            self.dispatcher._checkResult(Indigo._lib.indigoBond(self.id)),
        )

    def getAtom(self, idx):
        """Molecule method returns atom by index

        Args:
            idx (int): atom index

        Returns:
            IndigoObject: atom object
        """
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(
            self.dispatcher,
            self.dispatcher._checkResult(
                Indigo._lib.indigoGetAtom(self.id, idx)
            ),
        )

    def getBond(self, idx):
        """Molecule method returns bond by index

        Args:
            idx (int): bond index

        Returns:
            IndigoObject: bond object
        """
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(
            self.dispatcher,
            self.dispatcher._checkResult(
                Indigo._lib.indigoGetBond(self.id, idx)
            ),
        )

    def source(self):
        """Bond method returns source atom

        Returns:
            IndigoObject: atom object
        """
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(
            self.dispatcher,
            self.dispatcher._checkResult(Indigo._lib.indigoSource(self.id)),
        )

    def destination(self):
        """Bond method returns destination atom

        Returns:
            IndigoObject: atom object
        """
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(
            self.dispatcher,
            self.dispatcher._checkResult(
                Indigo._lib.indigoDestination(self.id)
            ),
        )

    def clearCisTrans(self):
        """Molecule or reaction method clears cis-trans stereo

        Returns:
            int: 1 if there are no errors
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoClearCisTrans(self.id)
        )

    def clearStereocenters(self):
        """Molecule or reaction method clears stereo centers

        Returns:
            int: 1 if there are no errors
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoClearStereocenters(self.id)
        )

    def countStereocenters(self):
        """Molecule method returns the number of stereocenters

        Returns:
            int: number of stereocenters
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoCountStereocenters(self.id)
        )

    def clearAlleneCenters(self):
        """Molecule method clears allene centers

        Returns:
            int: 1 if there are no errors
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoClearAlleneCenters(self.id)
        )

    def countAlleneCenters(self):
        """Molecule method returns the number of allene centers

        Returns:
            int: number of allene centers
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoCountAlleneCenters(self.id)
        )

    def resetSymmetricCisTrans(self):
        """Molecule or reaction method clears symmetric stereo cis-trans

        Returns:
            int: number of reset centers
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoResetSymmetricCisTrans(self.id)
        )

    def resetSymmetricStereocenters(self):
        """Molecule or reaction method clears symmetric stereocenters

        Returns:
            int: number of reset centers
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoResetSymmetricStereocenters(self.id)
        )

    def markEitherCisTrans(self):
        """Molecule or reaction method marks cis-trans stereo

        Returns:
            int: number of marked stereo
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoMarkEitherCisTrans(self.id)
        )

    def markStereobonds(self):
        """Molecule or reaction method marks stereo bonds

        Returns:
            int: 0 if there are no errors
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoMarkStereobonds(self.id)
        )

    def addAtom(self, symbol):
        """Molecule method adds an atom

        Args:
            symbol (str): atom symbol

        Returns:
            IndigoObject: atom object
        """
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(
            self.dispatcher,
            self.dispatcher._checkResult(
                Indigo._lib.indigoAddAtom(
                    self.id, symbol.encode(ENCODE_ENCODING)
                )
            ),
        )

    def resetAtom(self, symbol):
        """Atom method resets atom to the new symbol

        Args:
            symbol (str): atom symbol
        """
        self.dispatcher._setSessionId()
        self.dispatcher._checkResult(
            Indigo._lib.indigoResetAtom(
                self.id, symbol.encode(ENCODE_ENCODING)
            )
        )

    def addRSite(self, name):
        """Molecule method adds r-site

        Args:
            name (str): r-site name

        Returns:
            IndigoObject: atom object
        """
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(
            self.dispatcher,
            self.dispatcher._checkResult(
                Indigo._lib.indigoAddRSite(
                    self.id, name.encode(ENCODE_ENCODING)
                )
            ),
        )

    def setRSite(self, name):
        """Atom method sets r-site

        Args:
            name (str): r-site name

        Returns:
            int: 1 if there are no errors
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoSetRSite(self.id, name.encode(ENCODE_ENCODING))
        )

    def setCharge(self, charge):
        """Atom method sets charge

        Args:
            name (int): charge value

        Returns:
            int: 1 if there are no errors
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoSetCharge(self.id, charge)
        )

    def setIsotope(self, isotope):
        """Atom method sets isotope

        Args:
            name (int): isotope value

        Returns:
            int: 1 if there are no errors
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoSetIsotope(self.id, isotope)
        )

    def setImplicitHCount(self, impl_h):
        """Atom method sets implicit hydrogen count

        Args:
            name (int): implicit hydrogen count

        Returns:
            int: 1 if there are no errors
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoSetImplicitHCount(self.id, impl_h)
        )

    def addBond(self, destination, order):
        """Atom method adds bond

        Args:
            destination (IndigoObject): atom object destination
            order (int): bond order

        Returns:
            IndigoObject: bond object
        """
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(
            self.dispatcher,
            self.dispatcher._checkResult(
                Indigo._lib.indigoAddBond(self.id, destination.id, order)
            ),
        )

    def setBondOrder(self, order):
        """Bond method sets order

        Args:
            order (int): order value

        Returns:
            int: 1 if there are no errors
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoSetBondOrder(self.id, order)
        )

    def merge(self, what):
        """Molecule method merges molecule with the given structure

        Args:
            what (IndigoObject): molecule object to merge with

        Returns:
            IndigoObject: mapping object for merged structure
        """
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(
            self.dispatcher,
            self.dispatcher._checkResult(
                Indigo._lib.indigoMerge(self.id, what.id)
            ),
        )

    def highlight(self):
        """Atom or bond method to add highlighting

        Returns:
            int: 1 if there are no errors
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoHighlight(self.id)
        )

    def unhighlight(self):
        """Atom or bond method to remove highlighting

        Returns:
            int: 1 if there are no errors
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoUnhighlight(self.id)
        )

    def isHighlighted(self):
        """Atom or bond method returns True if highlighted

        Returns:
            bool: True if highlighted, False otherwise
        """
        self.dispatcher._setSessionId()
        return bool(
            self.dispatcher._checkResult(
                Indigo._lib.indigoIsHighlighted(self.id)
            )
        )

    def countComponents(self):
        """Molecule method returns the number of components

        Returns:
            int: number of components
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoCountComponents(self.id)
        )

    def componentIndex(self):
        """Atom method returns component index

        Returns:
            int: component index
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoComponentIndex(self.id)
        )

    def iterateComponents(self):
        """Molecule method returns components iterator

        Returns:
            IndigoObject: molecule components iterator
        """
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(
            self.dispatcher,
            self.dispatcher._checkResult(
                Indigo._lib.indigoIterateComponents(self.id)
            ),
        )

    def component(self, index):
        """Molecule method returns component by index

        Args:
            index (int): component index

        Returns:
            IndigoObject: molecule component object
        """
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(
            self.dispatcher,
            self.dispatcher._checkResult(
                Indigo._lib.indigoComponent(self.id, index)
            ),
        )

    def countSSSR(self):
        """Molecule method returns the size of the smallest set of smallest rings

        Returns:
            int: SSSR rings count
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoCountSSSR(self.id)
        )

    def iterateSSSR(self):
        """Molecule method returns smallest set of smallest rings iterator

        Returns:
            IndigoObject: SSSR iterator
        """
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(
            self.dispatcher,
            self.dispatcher._checkResult(
                Indigo._lib.indigoIterateSSSR(self.id)
            ),
        )

    def iterateSubtrees(self, min_atoms, max_atoms):
        """Molecule method returns subtrees iterator

        Args:
            min_atoms (int): min atoms neighbors limit
            max_atoms (int): max atoms neighbors limit

        Returns:
            IndigoObject: subtrees iterator
        """
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(
            self.dispatcher,
            self.dispatcher._checkResult(
                Indigo._lib.indigoIterateSubtrees(
                    self.id, min_atoms, max_atoms
                )
            ),
        )

    def iterateRings(self, min_atoms, max_atoms):
        """Molecule method returns rings iterator

        Args:
            min_atoms (int): min atoms neighbors limit
            max_atoms (int): max atoms neighbors limit

        Returns:
            IndigoObject: rings iterator
        """
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(
            self.dispatcher,
            self.dispatcher._checkResult(
                Indigo._lib.indigoIterateRings(self.id, min_atoms, max_atoms)
            ),
        )

    def iterateEdgeSubmolecules(self, min_bonds, max_bonds):
        """Molecule method returns edge submolecules iterator

        Args:
            min_bonds (int): min bonds neighbors limit
            max_bonds (int): max bonds neighbors limit

        Returns:
            IndigoObject: submolecules iterator
        """
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(
            self.dispatcher,
            self.dispatcher._checkResult(
                Indigo._lib.indigoIterateEdgeSubmolecules(
                    self.id, min_bonds, max_bonds
                )
            ),
        )

    def countHeavyAtoms(self):
        """Molecule method returns the number of heavy atoms

        Returns:
            int: heavy atom count
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoCountHeavyAtoms(self.id)
        )

    def grossFormula(self):
        """Molecule method returns gross formula

        Returns:
            str: gross formula
        """
        self.dispatcher._setSessionId()
        gfid = self.dispatcher._checkResult(
            Indigo._lib.indigoGrossFormula(self.id)
        )
        gf = self.dispatcher.IndigoObject(self.dispatcher, gfid)
        return self.dispatcher._checkResultString(
            Indigo._lib.indigoToString(gf.id)
        )

    def molecularWeight(self):
        """Molecule method returns molecular weight

        Returns:
            float: molecular weight value
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResultFloat(
            Indigo._lib.indigoMolecularWeight(self.id)
        )

    def mostAbundantMass(self):
        """Molecule method returns the most abundant mass

        Returns:
            float: most abundant mass
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResultFloat(
            Indigo._lib.indigoMostAbundantMass(self.id)
        )

    def monoisotopicMass(self):
        """Molecule method returns the monoisotopic mass

        Returns:
            float: monoisotopic mass
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResultFloat(
            Indigo._lib.indigoMonoisotopicMass(self.id)
        )

    def massComposition(self):
        """Molecule method returns mass composition

        Returns:
            str: mass composition string
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResultString(
            Indigo._lib.indigoMassComposition(self.id)
        )

    def tpsa(self, includeSP=False):
        """Molecule method returns the TPSA value

        Args:
            includeSP (bool): include S and P atoms to TPSA calculation,
                              false by default

        Returns:
            float: TPSA value
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResultFloat(
            Indigo._lib.indigoTPSA(self.id, includeSP)
        )

    def numRotatableBonds(self):
        """Molecule method returns the number of rotatable bonds

        Returns:
            int: number of rotatable bonds
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoNumRotatableBonds(self.id)
        )

    def numHydrogenBondAcceptors(self):
        """Molecule method returns the number of hydrogen bond acceptors

        Returns:
            float: number of hydrogen bond acceptors
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoNumHydrogenBondAcceptors(self.id)
        )

    def numHydrogenBondDonors(self):
        """Molecule method returns the number of hydrogen bond donors

        Returns:
            float: number of hydrogen bond donors
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoNumHydrogenBondDonors(self.id)
        )

    def canonicalSmiles(self):
        """Molecule or reaction method returns canonical smiles

        Returns:
            str: canonical smiles string
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResultString(
            Indigo._lib.indigoCanonicalSmiles(self.id)
        )

    def canonicalSmarts(self):
        """Molecule method returns canonical smarts

        Returns:
            str: canonical smarts
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResultString(
            Indigo._lib.indigoCanonicalSmarts(self.id)
        )

    def layeredCode(self):
        """Molecule method returns layered code

        Returns:
            str: layered code string
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResultString(
            Indigo._lib.indigoLayeredCode(self.id)
        )

    def symmetryClasses(self):
        """Molecule method returns symmetry classes

        Returns:
            str: symmetry classes string
        """
        c_size = c_int()
        self.dispatcher._setSessionId()
        c_buf = self.dispatcher._checkResultPtr(
            Indigo._lib.indigoSymmetryClasses(self.id, pointer(c_size))
        )
        res = array("i")
        for i in range(c_size.value):
            res.append(c_buf[i])
        return res

    def hasCoord(self):
        """Molecule method returns True if the structure contains coordinates

        Returns:
            bool: True if contains coordinates, False otherwise
        """
        self.dispatcher._setSessionId()
        return bool(
            self.dispatcher._checkResult(Indigo._lib.indigoHasCoord(self.id))
        )

    def hasZCoord(self):
        """Molecule method returns True if the structure contains Z coordinate

        Returns:
            bool: True if contains Z coordinate, False otherwise
        """
        self.dispatcher._setSessionId()
        return bool(
            self.dispatcher._checkResult(Indigo._lib.indigoHasZCoord(self.id))
        )

    def isChiral(self):
        """Molecule method returns True if the structure contains chiral flag

        Returns:
            bool: True if contains chiral flag, False otherwise
        """
        self.dispatcher._setSessionId()
        return bool(
            self.dispatcher._checkResult(Indigo._lib.indigoIsChiral(self.id))
        )

    def isPossibleFischerProjection(self, options):
        """Molecule method returns True if the structure contains possible Fischer projection

        Args:
            options (str): projection options

        Returns:
            bool: True if structure contains possible Fischer projection
        """
        self.dispatcher._setSessionId()
        return bool(
            self.dispatcher._checkResult(
                Indigo._lib.indigoIsPossibleFischerProjection(
                    self.id, options.encode(ENCODE_ENCODING)
                )
            )
        )

    def createSubmolecule(self, vertices):
        """Molecule method creates a submolecule from the given atom list

        Args:
            vertices (list): list of atom indexes

        Returns:
            IndigoObject: molecule object as submolecule
        """
        arr2 = (c_int * len(vertices))()
        for i in range(len(vertices)):
            arr2[i] = vertices[i]
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(
            self.dispatcher,
            self.dispatcher._checkResult(
                Indigo._lib.indigoCreateSubmolecule(self.id, len(arr2), arr2)
            ),
        )

    def createEdgeSubmolecule(self, vertices, edges):
        """Molecule method creates a submolecule from the given vertex atom and bond list

        Args:
            vertices (list): list of atom indexes
            edges (list): list of bond indexes

        Returns:
            IndigoObject: molecule object as submolecule
        """
        arr2 = (c_int * len(vertices))()
        for i in range(len(vertices)):
            arr2[i] = vertices[i]
        arr4 = (c_int * len(edges))()
        for i in range(len(edges)):
            arr4[i] = edges[i]
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(
            self.dispatcher,
            self.dispatcher._checkResult(
                Indigo._lib.indigoCreateEdgeSubmolecule(
                    self.id, len(arr2), arr2, len(arr4), arr4
                )
            ),
        )

    def getSubmolecule(self, vertices):
        """Molecule method returns submolecule by the given atom list

        Args:
            vertices (list): list of atom indexes

        Returns:
            IndigoObject: submolecule object
        """
        arr2 = (c_int * len(vertices))()
        for i in range(len(vertices)):
            arr2[i] = vertices[i]
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(
            self.dispatcher,
            self.dispatcher._checkResult(
                Indigo._lib.indigoGetSubmolecule(self.id, len(arr2), arr2)
            ),
            self,
        )

    def removeAtoms(self, vertices):
        """Molecule method removes atoms

        Args:
            vertices (list): atom indexes list

        Returns:
            int: 1 if there are no errors
        """
        arr2 = (c_int * len(vertices))()
        for i in range(len(vertices)):
            arr2[i] = vertices[i]
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoRemoveAtoms(self.id, len(arr2), arr2)
        )

    def removeBonds(self, bonds):
        """Molecule method removes bonds

        Args:
            bonds (list): bond indexes list

        Returns:
            int: 1 if there are no errors
        """
        arr2 = (c_int * len(bonds))()
        for i in range(len(bonds)):
            arr2[i] = bonds[i]
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoRemoveBonds(self.id, len(arr2), arr2)
        )

    def aromatize(self):
        """Molecule or reaction method aromatizes the structure

        Returns:
            int: 1 if there are no errors
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoAromatize(self.id)
        )

    def dearomatize(self):
        """Molecule or reaction method de-aromatizes the structure

        Returns:
            int: 1 if there are no errors
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoDearomatize(self.id)
        )

    def foldHydrogens(self):
        """Molecule or reaction method folds hydrogens

        Returns:
            int: 1 if there are no errors
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoFoldHydrogens(self.id)
        )

    def unfoldHydrogens(self):
        """Molecule or reaction method unfolds hydrogens

        Returns:
            int: 1 if there are no errors
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoUnfoldHydrogens(self.id)
        )

    def layout(self):
        """Molecule or reaction method calculates layout for the structure

        Returns:
            int: 1 if there are no errors
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoLayout(self.id))

    def smiles(self):
        """Molecule or reaction method calculates SMILES for the structure

        Returns:
            str: smiles string
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResultString(
            Indigo._lib.indigoSmiles(self.id)
        )

    def smarts(self):
        """Molecule or reaction method calculates SMARTS for the structure

        Returns:
            str: smarts string
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResultString(
            Indigo._lib.indigoSmarts(self.id)
        )

    def name(self):
        """IndigoObject method returns name

        Returns:
            str: name string
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResultString(
            Indigo._lib.indigoName(self.id)
        )

    def setName(self, name):
        """IndigoObject method sets name

        Args:
            name (str): name string

        Returns:
            int: 1 if there are no errors
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoSetName(self.id, name.encode(ENCODE_ENCODING))
        )

    def serialize(self):
        """IndigoObject method serializes the object into byte array

        Returns:
            list: array of bytes
        """
        c_size = c_int()
        c_buf = POINTER(c_byte)()
        self.dispatcher._setSessionId()
        self.dispatcher._checkResult(
            Indigo._lib.indigoSerialize(
                self.id, pointer(c_buf), pointer(c_size)
            )
        )
        res = array("b")
        for i in range(c_size.value):
            res.append(c_buf[i])
        return res

    def hasProperty(self, prop):
        """Object method returns True if the given property exists

        Args:
            prop (str): property name

        Returns:
            bool: flag True if property exists
        """
        self.dispatcher._setSessionId()
        return bool(
            self.dispatcher._checkResult(
                Indigo._lib.indigoHasProperty(self.id, prop)
            )
        )

    def getProperty(self, prop):
        """Object method returns property by the given name

        Args:
            prop (str): property name

        Returns:
            str: property value
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResultString(
            Indigo._lib.indigoGetProperty(
                self.id, prop.encode(ENCODE_ENCODING)
            )
        )

    def setProperty(self, prop, value):
        """Object method sets property

        Args:
            prop (str): property name
            value (str): property value

        Returns:
            int: 1 if there are no errors
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoSetProperty(
                self.id,
                prop.encode(ENCODE_ENCODING),
                value.encode(ENCODE_ENCODING),
            )
        )

    def removeProperty(self, prop):
        """Object method removes property

        Args:
            prop (str): property name

        Returns:
            int: 1 if there are no errors
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoRemoveProperty(
                self.id, prop.encode(ENCODE_ENCODING)
            )
        )

    def iterateProperties(self):
        """Object method returns properties iterator

        Returns:
            IndigoObject: properties iterator
        """
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(
            self.dispatcher,
            self.dispatcher._checkResult(
                Indigo._lib.indigoIterateProperties(self.id)
            ),
        )

    def clearProperties(self):
        """Object method clears all properties

        Returns:
            int: 1 if there are no errors
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoClearProperties(self.id)
        )

    def checkBadValence(self):
        """Molecule, atom or reaction method validates bad valence

        Returns:
            str: string containing valence validation errors
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResultString(
            Indigo._lib.indigoCheckBadValence(self.id)
        )

    def checkAmbiguousH(self):
        """Molecule or reaction method validates ambiguous hydrogens

        Returns:
            str: string containing hydrogens validation errors
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResultString(
            Indigo._lib.indigoCheckAmbiguousH(self.id)
        )

    def fingerprint(self, type):
        """Molecule or reaction method returns fingerprint representation

        Args:
            type (str): fingerprint type. One of the following: "sim", "sub", "sub-res", "sub-tau", "full"

        Returns:
            IndigoObject: fingerprint object
        """
        self.dispatcher._setSessionId()
        newobj = self.dispatcher._checkResult(
            Indigo._lib.indigoFingerprint(
                self.id, type.encode(ENCODE_ENCODING)
            )
        )
        if newobj == 0:
            return None
        return self.dispatcher.IndigoObject(self.dispatcher, newobj, self)

    def countBits(self):
        """Fingerprint method returns the count of 1 bits

        Returns:
            int: number of 1 bits
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoCountBits(self.id)
        )

    def rawData(self):
        """Object method returns string representation

        Returns:
            str: string for the object
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResultString(
            Indigo._lib.indigoRawData(self.id)
        )

    def tell(self):
        """Object method returns the size of the content, e.g. SDF number of structures

        Returns:
            int: size of the content
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoTell(self.id))

    def sdfAppend(self, item):
        """SDF method adds a new structure

        Args:
            item (IndigoObject): structure to be added

        Returns:
            int: 1 if there are errors
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoSdfAppend(self.id, item.id)
        )

    def smilesAppend(self, item):
        """Smiles builder methods adds a new structure

        Args:
            item (IndigoObject): structure to be added

        Returns:
            int: 1 if there are no errors
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoSmilesAppend(self.id, item.id)
        )

    def rdfHeader(self):
        """RDF builder adds header

        Returns:
            int: 1 if there are no errors
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoRdfHeader(self.id)
        )

    def rdfAppend(self, item):
        """RDF builder method adds a new structure

        Args:
            item (IndigoObject): new structure to be added

        Returns:
            int: 1 if there are no errors
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoRdfAppend(self.id, item.id)
        )

    def cmlHeader(self):
        """CML builder adds header

        Returns:
            int: 1 if there are no errors
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoCmlHeader(self.id)
        )

    def cmlAppend(self, item):
        """CML builder adds a new structure

        Args:
            item (IndigoObject): new structure to be added

        Returns:
            int: 1 if there are no errors
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoCmlAppend(self.id, item.id)
        )

    def cmlFooter(self):
        """CML builder adds footer information

        Returns:
            int: 1 if there are no errors
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoCmlFooter(self.id)
        )

    def append(self, object):
        """Saver method adds a new object

        Args:
            object (IndigoObject): object to be added

        Returns:
            int: 1 if there are no errors
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoAppend(self.id, object.id)
        )

    def arrayAdd(self, object):
        """Array method adds a new object

        Args:
            object (IndigoObject): object to be added

        Returns:
            int: 1 if there are no errors
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoArrayAdd(self.id, object.id)
        )

    def at(self, index):
        """Loader method returns element by index

        Args:
            index (int): element index

        Returns:
            IndigoObject: element object
        """
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(
            self.dispatcher,
            self.dispatcher._checkResult(Indigo._lib.indigoAt(self.id, index)),
        )

    def count(self):
        """Loader method returns the number of elements

        Returns:
            int: number of elements
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoCount(self.id))

    def clear(self):
        """Array, molecule or reaction method clears the object

        Returns:
            int: 1 if there are no errors
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(Indigo._lib.indigoClear(self.id))

    def iterateArray(self):
        """Array method returns iterator for elements

        Returns:
            IndigoObject: elements iterator
        """
        self.dispatcher._setSessionId()
        newobj = self.dispatcher._checkResult(
            Indigo._lib.indigoIterateArray(self.id)
        )
        if newobj == 0:
            return None
        else:
            return self.dispatcher.IndigoObject(self.dispatcher, newobj, self)

    def ignoreAtom(self, atom_object):
        """Matcher method adds atom to the ignore list

        Args:
            atom_object (IndigoObject): atom to ignore

        Returns:
            int: 1 if there are no errors
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoIgnoreAtom(self.id, atom_object.id)
        )

    def unignoreAtom(self, atom_object):
        """Matcher method removes atom from the ignore list

        Args:
            atom_object (IndigoObject): atom to remove from ignore

        Returns:
            int: 1 if there are no errors
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoUnignoreAtom(self.id, atom_object.id)
        )

    def unignoreAllAtoms(self):
        """Matcher method clears the ignore list

        Returns:
            int: 1 if there are no errors
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoUnignoreAllAtoms(self.id)
        )

    def match(self, query):
        """Matcher method executes matching

        Args:
            query (IndigoObject): query structure

        Returns:
            IndigoObject: mapping object
        """
        self.dispatcher._setSessionId()
        newobj = self.dispatcher._checkResult(
            Indigo._lib.indigoMatch(self.id, query.id)
        )
        if newobj == 0:
            return None
        else:
            return self.dispatcher.IndigoObject(self.dispatcher, newobj, self)

    def countMatches(self, query):
        """Matcher method returns the number of matches

        Args:
            query (IndigoObject): query structure

        Returns:
            int: number of matches
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoCountMatches(self.id, query.id)
        )

    def countMatchesWithLimit(self, query, embeddings_limit):
        """Matcher method returns the number of matches with max limit

        Args:
            query (IndigoObject): query structure
            embeddings_limit (int): max number of matches to search

        Returns:
            int: number of matches
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoCountMatchesWithLimit(
                self.id, query.id, embeddings_limit
            )
        )

    def iterateMatches(self, query):
        """Matcher method returns matches iterator

        Args:
            query (IndigoObject): query structure

        Returns:
            IndigoObject: matches iterator
        """
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(
            self.dispatcher,
            self.dispatcher._checkResult(
                Indigo._lib.indigoIterateMatches(self.id, query.id)
            ),
        )

    def highlightedTarget(self):
        """Mapping method returns highlighted target

        Returns:
            IndigoObject: highlighted molecule structure
        """
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(
            self.dispatcher,
            self.dispatcher._checkResult(
                Indigo._lib.indigoHighlightedTarget(self.id)
            ),
        )

    def mapAtom(self, atom):
        """Mapping method returns mapped atom for the given atom

        Args:
            atom (IndigoObject): query atom to map

        Returns:
            IndigoObject: mapped atom
        """
        self.dispatcher._setSessionId()
        newobj = self.dispatcher._checkResult(
            Indigo._lib.indigoMapAtom(self.id, atom.id)
        )
        if newobj == 0:
            return None
        else:
            return self.dispatcher.IndigoObject(self.dispatcher, newobj, self)

    def mapBond(self, bond):
        """Mapping method returns mapped bond for the given bond

        Args:
            bond (IndigoObject): query bond to map

        Returns:
            IndigoObject: mapped bond
        """
        self.dispatcher._setSessionId()
        newobj = self.dispatcher._checkResult(
            Indigo._lib.indigoMapBond(self.id, bond.id)
        )
        if newobj == 0:
            return None
        else:
            return self.dispatcher.IndigoObject(self.dispatcher, newobj, self)

    def mapMolecule(self, molecule):
        """Reaction mapping method returns mapped molecule for the given query molecule

        Args:
            molecule (IndigoObject): query molecule to map

        Returns:
            IndigoObject: mapped molecule
        """
        self.dispatcher._setSessionId()
        newobj = self.dispatcher._checkResult(
            Indigo._lib.indigoMapMolecule(self.id, molecule.id)
        )
        if newobj == 0:
            return None
        else:
            return self.dispatcher.IndigoObject(self.dispatcher, newobj, self)

    def allScaffolds(self):
        """Scaffold method returns all scaffolds

        Returns:
            IndigoObject: array of all scaffolds
        """
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(
            self.dispatcher,
            self.dispatcher._checkResult(
                Indigo._lib.indigoAllScaffolds(self.id)
            ),
        )

    def decomposedMoleculeScaffold(self):
        """Deconvolution method starts molecule decomposition

        Returns:
            IndigoObject: decomposed molecule
        """
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(
            self.dispatcher,
            self.dispatcher._checkResult(
                Indigo._lib.indigoDecomposedMoleculeScaffold(self.id)
            ),
        )

    def iterateDecomposedMolecules(self):
        """Deconvolution method returns decomposed molecules iterator

        Returns:
            IndigoObject: decomposed molecules iterator
        """
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(
            self.dispatcher,
            self.dispatcher._checkResult(
                Indigo._lib.indigoIterateDecomposedMolecules(self.id)
            ),
        )

    def decomposedMoleculeHighlighted(self):
        """Deconvolution method returns decomposed highlighted molecule

        Returns:
            IndigoObject: decomposed highlighted molecule
        """
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(
            self.dispatcher,
            self.dispatcher._checkResult(
                Indigo._lib.indigoDecomposedMoleculeHighlighted(self.id)
            ),
        )

    def decomposedMoleculeWithRGroups(self):
        """Deconvolution method returns decomposed molecule with R-groups

        Returns:
            IndigoObject: decomposed molecule with R-groups
        """
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(
            self.dispatcher,
            self.dispatcher._checkResult(
                Indigo._lib.indigoDecomposedMoleculeWithRGroups(self.id)
            ),
        )

    def decomposeMolecule(self, mol):
        """Deconvolution method makes decomposition for the given molecule

        Args:
            mol (IndigoObject): molecule to decompose

        Returns:
            IndigoObject: deconvolution element object
        """
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(
            self.dispatcher,
            self.dispatcher._checkResult(
                Indigo._lib.indigoDecomposeMolecule(self.id, mol.id)
            ),
        )

    def iterateDecompositions(self):
        """Deconvolution element method returns decompositions iterator

        Returns:
            IndigoObject: decompositions iterator
        """
        self.dispatcher._setSessionId()
        return self.dispatcher.IndigoObject(
            self.dispatcher,
            self.dispatcher._checkResult(
                Indigo._lib.indigoIterateDecompositions(self.id)
            ),
        )

    def addDecomposition(self, q_match):
        """Deconvolution method adds query match

        Args:
            q_match (IndigoObject): decomposition match object

        Returns:
            int: 0 if there are no errors
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoAddDecomposition(self.id, q_match.id)
        )

    def toString(self):
        """Object method returns string representation

        Returns:
            str: string representation for the object
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResultString(
            Indigo._lib.indigoToString(self.id)
        )

    def toBuffer(self):
        """Object method returns binary representation

        Returns:
            list: array of bytes
        """
        c_size = c_int()
        c_buf = POINTER(c_byte)()
        self.dispatcher._setSessionId()
        self.dispatcher._checkResult(
            Indigo._lib.indigoToBuffer(
                self.id, pointer(c_buf), pointer(c_size)
            )
        )
        res = array("b")
        for i in range(c_size.value):
            res.append(c_buf[i])
        return res

    def stereocenterPyramid(self):
        """Atom method returns stereopyramid information

        Returns:
            str: stereopyramid information string
        """
        self.dispatcher._setSessionId()
        ptr = self.dispatcher._checkResultPtr(
            Indigo._lib.indigoStereocenterPyramid(self.id)
        )
        res = [0] * 4
        for i in range(4):
            res[i] = ptr[i]
        return res

    def expandAbbreviations(self):
        """Molecule method expands abbreviations

        Returns:
            int: count of expanded abbreviations
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResult(
            Indigo._lib.indigoExpandAbbreviations(self.id)
        )

    def dbgInternalType(self):
        """Object method returns type

        Returns:
            str: object type string
        """
        self.dispatcher._setSessionId()
        return self.dispatcher._checkResultString(
            Indigo._lib.indigoDbgInternalType(self.id)
        )


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

    SG_TYPE_GEN = 0
    SG_TYPE_DAT = 1
    SG_TYPE_SUP = 2
    SG_TYPE_SRU = 3
    SG_TYPE_MUL = 4
    SG_TYPE_MON = 5
    SG_TYPE_MER = 6
    SG_TYPE_COP = 7
    SG_TYPE_CRO = 8
    SG_TYPE_MOD = 9
    SG_TYPE_GRA = 10
    SG_TYPE_COM = 11
    SG_TYPE_MIX = 12
    SG_TYPE_FOR = 13
    SG_TYPE_ANY = 14

    _lib = None
    _dll_path = None
    _dll_dir = None

    # Python embeds path into .pyc code if method is marked with @staticmethod
    # This causes an error when Indigo is loaded from different places by relative path
    def _initStatic(self, _=None):
        indigo_found = False
        system_name = platform.system().lower()
        machine_name = (
            platform.machine()
            .lower()
            .replace("amd64", "x86_64")
            .replace("arm64", "aarch64")
        )
        if system_name == "linux":
            library_prefix = "lib"
            library_suffix = ".so"
        elif system_name == "darwin":
            library_prefix = "lib"
            library_suffix = ".dylib"
        elif system_name == "windows":
            library_prefix = ""
            library_suffix = ".dll"
        elif system_name.startswith("msys_nt"):
            library_prefix = ""
            library_suffix = ".dll"
            system_name = "windows"
        else:
            raise ValueError("Unsupported OS: {}".format(system_name))

        if machine_name == "x86_64":
            if sizeof(c_void_p) == 4:
                machine_name = "i386"

        library_base_path = os.path.join(
            os.path.dirname(os.path.abspath(__file__)), "lib"
        )
        libraries_directory = "{}-{}".format(system_name, machine_name)
        library_name = "{}indigo{}".format(library_prefix, library_suffix)

        library_path = os.path.join(
            library_base_path, libraries_directory, library_name
        )
        if os.path.exists(library_path):
            Indigo._lib = CDLL(library_path, mode=RTLD_GLOBAL)
            Indigo._dll_path = library_path
            Indigo._dll_dir = os.path.dirname(library_path)
            indigo_found = True
        if not indigo_found:
            raise IndigoException(
                "Could not find native libraries for target OS in search directories: {}".format(
                    library_path
                )
            )

    def _setSessionId(self):
        Indigo._lib.indigoSetSessionId(self._sid)

    def __init__(self, path=None):
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
        Indigo._lib.indigoSetOptionColor.argtypes = [
            c_char_p,
            c_float,
            c_float,
            c_float,
        ]
        Indigo._lib.indigoSetOptionXY.restype = c_int
        Indigo._lib.indigoSetOptionXY.argtypes = [c_char_p, c_int, c_int]
        Indigo._lib.indigoGetOption.restype = c_char_p
        Indigo._lib.indigoGetOption.argtypes = [c_char_p]
        Indigo._lib.indigoGetOptionInt.restype = c_int
        Indigo._lib.indigoGetOptionInt.argtypes = [c_char_p, POINTER(c_int)]
        Indigo._lib.indigoGetOptionBool.argtypes = [c_char_p, POINTER(c_int)]
        Indigo._lib.indigoGetOptionBool.restype = c_int
        Indigo._lib.indigoGetOptionFloat.argtypes = [
            c_char_p,
            POINTER(c_float),
        ]
        Indigo._lib.indigoGetOptionFloat.restype = c_int
        Indigo._lib.indigoGetOptionColor.argtypes = [
            c_char_p,
            POINTER(c_float),
            POINTER(c_float),
            POINTER(c_float),
        ]
        Indigo._lib.indigoGetOptionColor.restype = c_int
        Indigo._lib.indigoGetOptionXY.argtypes = [
            c_char_p,
            POINTER(c_int),
            POINTER(c_int),
        ]
        Indigo._lib.indigoGetOptionXY.restype = c_int
        Indigo._lib.indigoGetOptionType.restype = c_char_p
        Indigo._lib.indigoGetOptionType.argtypes = [c_char_p]
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
        Indigo._lib.indigoLoadMoleculeFromBuffer.restype = c_int
        Indigo._lib.indigoLoadMoleculeFromBuffer.argtypes = [
            POINTER(c_byte),
            c_int,
        ]
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
        Indigo._lib.indigoLoadStructureFromString.restype = c_int
        Indigo._lib.indigoLoadStructureFromString.argtypes = [
            c_char_p,
            c_char_p,
        ]
        Indigo._lib.indigoLoadStructureFromBuffer.restype = c_int
        Indigo._lib.indigoLoadStructureFromBuffer.argtypes = [
            POINTER(c_byte),
            c_int,
            c_char_p,
        ]
        Indigo._lib.indigoLoadStructureFromFile.restype = c_int
        Indigo._lib.indigoLoadStructureFromFile.argtypes = [c_char_p, c_char_p]
        Indigo._lib.indigoCreateReaction.restype = c_int
        Indigo._lib.indigoCreateReaction.argtypes = None
        Indigo._lib.indigoCreateQueryReaction.restype = c_int
        Indigo._lib.indigoCreateQueryReaction.argtypes = None
        Indigo._lib.indigoExactMatch.restype = c_int
        Indigo._lib.indigoExactMatch.argtypes = [c_int, c_int, c_char_p]
        Indigo._lib.indigoSetTautomerRule.restype = c_int
        Indigo._lib.indigoSetTautomerRule.argtypes = [
            c_int,
            c_char_p,
            c_char_p,
        ]
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
        Indigo._lib.indigoIterateCDX.restype = c_int
        Indigo._lib.indigoIterateCDX.argtypes = [c_int]
        Indigo._lib.indigoIterateSDFile.restype = c_int
        Indigo._lib.indigoIterateSDFile.argtypes = [c_char_p]
        Indigo._lib.indigoIterateRDFile.restype = c_int
        Indigo._lib.indigoIterateRDFile.argtypes = [c_char_p]
        Indigo._lib.indigoIterateSmilesFile.restype = c_int
        Indigo._lib.indigoIterateSmilesFile.argtypes = [c_char_p]
        Indigo._lib.indigoIterateCMLFile.restype = c_int
        Indigo._lib.indigoIterateCMLFile.argtypes = [c_char_p]
        Indigo._lib.indigoIterateCDXFile.restype = c_int
        Indigo._lib.indigoIterateCDXFile.argtypes = [c_char_p]
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
        Indigo._lib.indigoRGroupComposition.restype = c_int
        Indigo._lib.indigoRGroupComposition.argtypes = [c_int, c_char_p]
        Indigo._lib.indigoGetFragmentedMolecule.restype = c_int
        Indigo._lib.indigoGetFragmentedMolecule.argtypes = [c_int, c_char_p]
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

        Indigo._lib.indigoCheck.restype = c_char_p
        Indigo._lib.indigoCheck.argtypes = [c_char_p, c_char_p, c_char_p]
        Indigo._lib.indigoCheckObj.restype = c_char_p
        Indigo._lib.indigoCheckObj.argtypes = [c_int, c_char_p]

        Indigo._lib.indigoCheckStructure.restype = c_char_p
        Indigo._lib.indigoCheckStructure.argtypes = [c_char_p, c_char_p]
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
        Indigo._lib.indigoSaveCdxmlToFile.restype = c_int
        Indigo._lib.indigoSaveCdxmlToFile.argtypes = [c_int, c_char_p]
        Indigo._lib.indigoCdxml.restype = c_char_p
        Indigo._lib.indigoCdxml.argtypes = [c_int]
        Indigo._lib.indigoJson.restype = c_char_p
        Indigo._lib.indigoJson.argtypes = [c_int]
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
        Indigo._lib.indigoStandardize.restype = c_int
        Indigo._lib.indigoStandardize.argtypes = [c_int]
        Indigo._lib.indigoIonize.restype = c_int
        Indigo._lib.indigoIonize.argtypes = [c_int, c_float, c_float]
        Indigo._lib.indigoBuildPkaModel.restype = c_int
        Indigo._lib.indigoBuildPkaModel.argtypes = [c_int, c_float, c_char_p]
        Indigo._lib.indigoGetAcidPkaValue.restype = POINTER(c_float)
        Indigo._lib.indigoGetAcidPkaValue.argtypes = [
            c_int,
            c_int,
            c_int,
            c_int,
        ]
        Indigo._lib.indigoGetBasicPkaValue.restype = POINTER(c_float)
        Indigo._lib.indigoGetBasicPkaValue.argtypes = [
            c_int,
            c_int,
            c_int,
            c_int,
        ]
        Indigo._lib.indigoAutomap.restype = c_int
        Indigo._lib.indigoAutomap.argtypes = [c_int, c_char_p]
        Indigo._lib.indigoGetAtomMappingNumber.restype = c_int
        Indigo._lib.indigoGetAtomMappingNumber.argtypes = [c_int, c_int]
        Indigo._lib.indigoSetAtomMappingNumber.restype = c_int
        Indigo._lib.indigoSetAtomMappingNumber.argtypes = [c_int, c_int, c_int]
        Indigo._lib.indigoGetReactingCenter.restype = c_int
        Indigo._lib.indigoGetReactingCenter.argtypes = [
            c_int,
            c_int,
            POINTER(c_int),
        ]
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
        Indigo._lib.indigoCountRGroups.restype = c_int
        Indigo._lib.indigoCountRGroups.argtypes = [c_int]
        Indigo._lib.indigoIsPseudoatom.restype = c_int
        Indigo._lib.indigoIsPseudoatom.argtypes = [c_int]
        Indigo._lib.indigoIsRSite.restype = c_int
        Indigo._lib.indigoIsRSite.argtypes = [c_int]
        Indigo._lib.indigoIsTemplateAtom.restype = c_int
        Indigo._lib.indigoIsTemplateAtom.argtypes = [c_int]
        Indigo._lib.indigoStereocenterType.restype = c_int
        Indigo._lib.indigoStereocenterType.argtypes = [c_int]
        Indigo._lib.indigoStereocenterGroup.restype = c_int
        Indigo._lib.indigoStereocenterGroup.argtypes = [c_int]
        Indigo._lib.indigoSetStereocenterGroup.restype = c_int
        Indigo._lib.indigoSetStereocenterGroup.argtypes = [c_int, c_int]
        Indigo._lib.indigoChangeStereocenterType.restype = c_int
        Indigo._lib.indigoChangeStereocenterType.argtypes = [c_int, c_int]
        Indigo._lib.indigoValidateChirality.restype = c_int
        Indigo._lib.indigoValidateChirality.argtypes = [c_int]
        Indigo._lib.indigoSingleAllowedRGroup.restype = c_int
        Indigo._lib.indigoSingleAllowedRGroup.argtypes = [c_int]
        Indigo._lib.indigoAddStereocenter.restype = c_int
        Indigo._lib.indigoAddStereocenter.argtypes = [
            c_int,
            c_int,
            c_int,
            c_int,
            c_int,
            c_int,
        ]
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
        Indigo._lib.indigoGetRadicalElectrons.argtypes = [
            c_int,
            POINTER(c_int),
        ]
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
        Indigo._lib.indigoGetHybridization.restype = c_int
        Indigo._lib.indigoGetHybridization.argtypes = [c_int]
        Indigo._lib.indigoCheckValence.restype = c_int
        Indigo._lib.indigoCheckValence.argtypes = [c_int]
        Indigo._lib.indigoCheckQuery.restype = c_int
        Indigo._lib.indigoCheckQuery.argtypes = [c_int]
        Indigo._lib.indigoCheckRGroups.restype = c_int
        Indigo._lib.indigoCheckRGroups.argtypes = [c_int]
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
        Indigo._lib.indigoIterateSGroups.restype = c_int
        Indigo._lib.indigoIterateSGroups.argtypes = [c_int]
        Indigo._lib.indigoIterateTGroups.restype = c_int
        Indigo._lib.indigoIterateTGroups.argtypes = [c_int]
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
        Indigo._lib.indigoAddDataSGroup.argtypes = [
            c_int,
            c_int,
            POINTER(c_int),
            c_int,
            POINTER(c_int),
            c_char_p,
            c_char_p,
        ]
        Indigo._lib.indigoAddSuperatom.restype = c_int
        Indigo._lib.indigoAddSuperatom.argtypes = [
            c_int,
            c_int,
            POINTER(c_int),
            c_char_p,
        ]
        Indigo._lib.indigoSetDataSGroupXY.restype = c_int
        Indigo._lib.indigoSetDataSGroupXY.argtypes = [
            c_int,
            c_float,
            c_float,
            c_char_p,
        ]
        Indigo._lib.indigoSetSGroupData.restype = c_int
        Indigo._lib.indigoSetSGroupData.argtypes = [c_int, c_char_p]
        Indigo._lib.indigoSetSGroupCoords.restype = c_int
        Indigo._lib.indigoSetSGroupCoords.argtypes = [c_int, c_float, c_float]
        Indigo._lib.indigoSetSGroupDescription.restype = c_int
        Indigo._lib.indigoSetSGroupDescription.argtypes = [c_int, c_char_p]
        Indigo._lib.indigoSetSGroupFieldName.restype = c_int
        Indigo._lib.indigoSetSGroupFieldName.argtypes = [c_int, c_char_p]
        Indigo._lib.indigoSetSGroupQueryCode.restype = c_int
        Indigo._lib.indigoSetSGroupQueryCode.argtypes = [c_int, c_char_p]
        Indigo._lib.indigoSetSGroupQueryOper.restype = c_int
        Indigo._lib.indigoSetSGroupQueryOper.argtypes = [c_int, c_char_p]
        Indigo._lib.indigoSetSGroupDisplay.restype = c_int
        Indigo._lib.indigoSetSGroupDisplay.argtypes = [c_int, c_char_p]
        Indigo._lib.indigoSetSGroupLocation.restype = c_int
        Indigo._lib.indigoSetSGroupLocation.argtypes = [c_int, c_char_p]
        Indigo._lib.indigoSetSGroupTag.restype = c_int
        Indigo._lib.indigoSetSGroupTag.argtypes = [c_int, c_char_p]
        Indigo._lib.indigoSetSGroupTagAlign.restype = c_int
        Indigo._lib.indigoSetSGroupTagAlign.argtypes = [c_int, c_int]
        Indigo._lib.indigoSetSGroupDataType.restype = c_int
        Indigo._lib.indigoSetSGroupDataType.argtypes = [c_int, c_char_p]
        Indigo._lib.indigoSetSGroupXCoord.restype = c_int
        Indigo._lib.indigoSetSGroupXCoord.argtypes = [c_int, c_float]
        Indigo._lib.indigoSetSGroupYCoord.restype = c_int
        Indigo._lib.indigoSetSGroupYCoord.argtypes = [c_int, c_float]
        Indigo._lib.indigoCreateSGroup.restype = c_int
        Indigo._lib.indigoCreateSGroup.argtypes = [c_char_p, c_int, c_char_p]
        Indigo._lib.indigoSetSGroupClass.restype = c_int
        Indigo._lib.indigoSetSGroupClass.argtypes = [c_int, c_char_p]
        Indigo._lib.indigoSetSGroupName.restype = c_int
        Indigo._lib.indigoSetSGroupName.argtypes = [c_int, c_char_p]
        Indigo._lib.indigoGetSGroupClass.restype = c_char_p
        Indigo._lib.indigoGetSGroupClass.argtypes = [c_int]
        Indigo._lib.indigoGetSGroupName.restype = c_char_p
        Indigo._lib.indigoGetSGroupName.argtypes = [c_int]
        Indigo._lib.indigoGetSGroupNumCrossBonds.restype = c_int
        Indigo._lib.indigoGetSGroupNumCrossBonds.argtypes = [c_int]
        Indigo._lib.indigoAddSGroupAttachmentPoint.restype = c_int
        Indigo._lib.indigoAddSGroupAttachmentPoint.argtypes = [
            c_int,
            c_int,
            c_int,
            c_char_p,
        ]
        Indigo._lib.indigoDeleteSGroupAttachmentPoint.restype = c_int
        Indigo._lib.indigoDeleteSGroupAttachmentPoint.argtypes = [c_int, c_int]
        Indigo._lib.indigoGetSGroupDisplayOption.restype = c_int
        Indigo._lib.indigoGetSGroupDisplayOption.argtypes = [c_int]
        Indigo._lib.indigoSetSGroupDisplayOption.restype = c_int
        Indigo._lib.indigoSetSGroupDisplayOption.argtypes = [c_int, c_int]
        Indigo._lib.indigoGetSGroupSeqId.restype = c_int
        Indigo._lib.indigoGetSGroupSeqId.argtypes = [c_int]
        Indigo._lib.indigoGetSGroupCoords.restype = POINTER(c_float)
        Indigo._lib.indigoGetSGroupCoords.argtypes = [c_int]
        Indigo._lib.indigoGetRepeatingUnitSubscript.restype = c_char_p
        Indigo._lib.indigoGetRepeatingUnitSubscript.argtypes = [c_int]
        Indigo._lib.indigoGetRepeatingUnitConnectivity.restype = c_int
        Indigo._lib.indigoGetRepeatingUnitConnectivity.argtypes = [c_int]
        Indigo._lib.indigoGetSGroupMultiplier.restype = c_int
        Indigo._lib.indigoGetSGroupMultiplier.argtypes = [c_int]
        Indigo._lib.indigoSetSGroupMultiplier.restype = c_int
        Indigo._lib.indigoSetSGroupMultiplier.argtypes = [c_int, c_int]
        Indigo._lib.indigoSetSGroupBrackets.restype = c_int
        Indigo._lib.indigoSetSGroupBrackets.argtypes = [
            c_int,
            c_int,
            c_float,
            c_float,
            c_float,
            c_float,
            c_float,
            c_float,
            c_float,
            c_float,
        ]
        Indigo._lib.indigoFindSGroups.restype = c_int
        Indigo._lib.indigoFindSGroups.argtypes = [c_int, c_char_p, c_char_p]
        Indigo._lib.indigoGetSGroupType.restype = c_int
        Indigo._lib.indigoGetSGroupType.argtypes = [c_int]
        Indigo._lib.indigoGetSGroupIndex.restype = c_int
        Indigo._lib.indigoGetSGroupIndex.argtypes = [c_int]
        Indigo._lib.indigoGetSGroupOriginalId.restype = c_int
        Indigo._lib.indigoGetSGroupOriginalId.argtypes = [c_int]
        Indigo._lib.indigoSetSGroupOriginalId.restype = c_int
        Indigo._lib.indigoSetSGroupOriginalId.argtypes = [c_int, c_int]
        Indigo._lib.indigoGetSGroupParentId.restype = c_int
        Indigo._lib.indigoGetSGroupParentId.argtypes = [c_int]
        Indigo._lib.indigoSetSGroupParentId.restype = c_int
        Indigo._lib.indigoSetSGroupParentId.argtypes = [c_int, c_int]
        Indigo._lib.indigoAddTemplate.restype = c_int
        Indigo._lib.indigoAddTemplate.argtypes = [c_int, c_int, c_char_p]
        Indigo._lib.indigoRemoveTemplate.restype = c_int
        Indigo._lib.indigoRemoveTemplate.argtypes = [c_int, c_char_p]
        Indigo._lib.indigoFindTemplate.restype = c_int
        Indigo._lib.indigoFindTemplate.argtypes = [c_int, c_char_p]
        Indigo._lib.indigoGetTGroupClass.restype = c_char_p
        Indigo._lib.indigoGetTGroupClass.argtypes = [c_int]
        Indigo._lib.indigoGetTGroupName.restype = c_char_p
        Indigo._lib.indigoGetTGroupName.argtypes = [c_int]
        Indigo._lib.indigoGetTGroupAlias.restype = c_char_p
        Indigo._lib.indigoGetTGroupAlias.argtypes = [c_int]
        Indigo._lib.indigoTransformSCSRtoCTAB.restype = c_int
        Indigo._lib.indigoTransformSCSRtoCTAB.argtypes = [c_int]
        Indigo._lib.indigoTransformCTABtoSCSR.restype = c_int
        Indigo._lib.indigoTransformCTABtoSCSR.argtypes = [c_int, c_int]
        Indigo._lib.indigoTransformHELMtoSCSR.restype = c_int
        Indigo._lib.indigoTransformHELMtoSCSR.argtypes = [c_int]
        Indigo._lib.indigoGetTemplateAtomClass.restype = c_char_p
        Indigo._lib.indigoGetTemplateAtomClass.argtypes = [c_int]
        Indigo._lib.indigoSetTemplateAtomClass.restype = c_int
        Indigo._lib.indigoSetTemplateAtomClass.argtypes = [c_int, c_char_p]
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
        Indigo._lib.indigoAddConstraintNot.argtypes = [
            c_int,
            c_char_p,
            c_char_p,
        ]
        Indigo._lib.indigoAddConstraintOr.restype = c_int
        Indigo._lib.indigoAddConstraintOr.argtypes = [
            c_int,
            c_char_p,
            c_char_p,
        ]
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
        Indigo._lib.indigoIterateEdgeSubmolecules.argtypes = [
            c_int,
            c_int,
            c_int,
        ]
        Indigo._lib.indigoCountHeavyAtoms.restype = c_int
        Indigo._lib.indigoCountHeavyAtoms.argtypes = [c_int]
        Indigo._lib.indigoGrossFormula.restype = c_int
        Indigo._lib.indigoGrossFormula.argtypes = [c_int]
        Indigo._lib.indigoMolecularWeight.restype = c_double
        Indigo._lib.indigoMolecularWeight.argtypes = [c_int]
        Indigo._lib.indigoMostAbundantMass.restype = c_double
        Indigo._lib.indigoMostAbundantMass.argtypes = [c_int]
        Indigo._lib.indigoMonoisotopicMass.restype = c_double
        Indigo._lib.indigoMonoisotopicMass.argtypes = [c_int]
        Indigo._lib.indigoMassComposition.restype = c_char_p
        Indigo._lib.indigoMassComposition.argtypes = [c_int]
        Indigo._lib.indigoTPSA.restype = c_double
        Indigo._lib.indigoTPSA.argtypes = [c_int, c_int]
        Indigo._lib.indigoNumRotatableBonds.restype = c_int
        Indigo._lib.indigoNumRotatableBonds.argtypes = [c_int]
        Indigo._lib.indigoNumHydrogenBondAcceptors.restype = c_int
        Indigo._lib.indigoNumHydrogenBondAcceptors.argtypes = [c_int]
        Indigo._lib.indigoNumHydrogenBondDonors.restype = c_int
        Indigo._lib.indigoNumHydrogenBondDonors.argtypes = [c_int]
        Indigo._lib.indigoLogP.restype = c_double
        Indigo._lib.indigoLogP.argtypes = [c_int]
        Indigo._lib.indigoMolarRefractivity.restype = c_double
        Indigo._lib.indigoMolarRefractivity.argtypes = [c_int]
        Indigo._lib.indigoCanonicalSmiles.restype = c_char_p
        Indigo._lib.indigoCanonicalSmiles.argtypes = [c_int]
        Indigo._lib.indigoCanonicalSmarts.restype = c_char_p
        Indigo._lib.indigoCanonicalSmarts.argtypes = [c_int]
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
        Indigo._lib.indigoIsPossibleFischerProjection.restype = c_int
        Indigo._lib.indigoIsPossibleFischerProjection.argtypes = [
            c_int,
            c_char_p,
        ]
        Indigo._lib.indigoCreateSubmolecule.restype = c_int
        Indigo._lib.indigoCreateSubmolecule.argtypes = [
            c_int,
            c_int,
            POINTER(c_int),
        ]
        Indigo._lib.indigoCreateEdgeSubmolecule.restype = c_int
        Indigo._lib.indigoCreateEdgeSubmolecule.argtypes = [
            c_int,
            c_int,
            POINTER(c_int),
            c_int,
            POINTER(c_int),
        ]
        Indigo._lib.indigoGetSubmolecule.restype = c_int
        Indigo._lib.indigoGetSubmolecule.argtypes = [
            c_int,
            c_int,
            POINTER(c_int),
        ]
        Indigo._lib.indigoRemoveAtoms.restype = c_int
        Indigo._lib.indigoRemoveAtoms.argtypes = [c_int, c_int, POINTER(c_int)]
        Indigo._lib.indigoRemoveBonds.restype = c_int
        Indigo._lib.indigoRemoveBonds.argtypes = [c_int, c_int, POINTER(c_int)]
        Indigo._lib.indigoAlignAtoms.restype = c_float
        Indigo._lib.indigoAlignAtoms.argtypes = [
            c_int,
            c_int,
            POINTER(c_int),
            POINTER(c_float),
        ]
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
        Indigo._lib.indigoClean2d.restype = c_int
        Indigo._lib.indigoClean2d.argtypes = [c_int]
        Indigo._lib.indigoSmiles.restype = c_char_p
        Indigo._lib.indigoSmiles.argtypes = [c_int]
        Indigo._lib.indigoSmarts.restype = c_char_p
        Indigo._lib.indigoSmarts.argtypes = [c_int]
        Indigo._lib.indigoName.restype = c_char_p
        Indigo._lib.indigoName.argtypes = [c_int]
        Indigo._lib.indigoSetName.restype = c_int
        Indigo._lib.indigoSetName.argtypes = [c_int, c_char_p]
        Indigo._lib.indigoSerialize.restype = c_int
        Indigo._lib.indigoSerialize.argtypes = [
            c_int,
            POINTER(POINTER(c_byte)),
            POINTER(c_int),
        ]
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
        Indigo._lib.indigoCheckChirality.restype = c_int
        Indigo._lib.indigoCheckChirality.argtypes = [c_int]
        Indigo._lib.indigoCheck3DStereo.restype = c_int
        Indigo._lib.indigoCheck3DStereo.argtypes = [c_int]
        Indigo._lib.indigoCheckStereo.restype = c_int
        Indigo._lib.indigoCheckStereo.argtypes = [c_int]
        Indigo._lib.indigoFingerprint.restype = c_int
        Indigo._lib.indigoFingerprint.argtypes = [c_int, c_char_p]
        Indigo._lib.indigoLoadFingerprintFromBuffer.restype = c_int
        Indigo._lib.indigoLoadFingerprintFromBuffer.argtypes = [
            POINTER(c_byte),
            c_int,
        ]
        Indigo._lib.indigoLoadFingerprintFromDescriptors.restype = c_int
        Indigo._lib.indigoLoadFingerprintFromDescriptors.argtypes = [
            POINTER(c_double),
            c_int,
            c_int,
            c_double,
        ]
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
        Indigo._lib.indigoCountMatchesWithLimit.argtypes = [
            c_int,
            c_int,
            c_int,
        ]
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
        Indigo._lib.indigoIterateTautomers.restype = c_int
        Indigo._lib.indigoIterateTautomers.argtypes = [c_int, c_char_p]
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
        Indigo._lib.indigoOneBitsList.restype = c_char_p
        Indigo._lib.indigoOneBitsList.argtypes = [c_int]
        Indigo._lib.indigoToBuffer.restype = c_int
        Indigo._lib.indigoToBuffer.argtypes = [
            c_int,
            POINTER(POINTER(c_byte)),
            POINTER(c_int),
        ]
        Indigo._lib.indigoStereocenterPyramid.restype = POINTER(c_int)
        Indigo._lib.indigoStereocenterPyramid.argtypes = [c_int]
        Indigo._lib.indigoExpandAbbreviations.restype = c_int
        Indigo._lib.indigoExpandAbbreviations.argtypes = [c_int]
        Indigo._lib.indigoDbgInternalType.restype = c_char_p
        Indigo._lib.indigoDbgInternalType.argtypes = [c_int]
        Indigo._lib.indigoNameToStructure.restype = c_int
        Indigo._lib.indigoNameToStructure.argtypes = [c_char_p, c_char_p]
        Indigo._lib.indigoResetOptions.restype = c_int
        Indigo._lib.indigoResetOptions.argtypes = None

    def __del__(self):
        if hasattr(self, "_lib"):
            self._lib.indigoReleaseSessionId(self._sid)

    def deserialize(self, arr):
        """Creates molecule or reaction object from binary serialized CMF format

        Args:
            arr (list): array of bytes

        Returns:
            IndigoObject: molecule or reaction object
        """
        values = (c_byte * len(arr))()
        for i in range(len(arr)):
            values[i] = arr[i]
        self._setSessionId()
        res = Indigo._lib.indigoUnserialize(values, len(arr))
        return self.IndigoObject(self, self._checkResult(res))

    def unserialize(self, arr):
        """[DEPRECATED] Creates molecule or reaction object from binary serialized CMF format

        Args:
            arr (list): array of bytes

        Returns:
            IndigoObject: molecule or reaction object
        """
        warnings.warn(
            "unserialize() is deprecated, use deserialize() instead",
            DeprecationWarning,
        )
        return self.deserialize(arr)

    def setOption(self, option, value1, value2=None, value3=None):
        """Sets option value

        Args:
            option (str): option name
            value1 (int, str, bool, float): option value
            value2 (int, float): option value for tuples. Optional, defaults to None.
            value3 (float): option value for triple. Optional, defaults to None.

        Raises:
            IndigoException: if option does not exist
        """
        self._setSessionId()
        if (
            (
                type(value1).__name__ == "str"
                or type(value1).__name__ == "unicode"
            )
            and value2 is None
            and value3 is None
        ):
            self._checkResult(
                Indigo._lib.indigoSetOption(
                    option.encode(ENCODE_ENCODING),
                    value1.encode(ENCODE_ENCODING),
                )
            )
        elif (
            type(value1).__name__ == "int"
            and value2 is None
            and value3 is None
        ):
            self._checkResult(
                Indigo._lib.indigoSetOptionInt(
                    option.encode(ENCODE_ENCODING), value1
                )
            )
        elif (
            type(value1).__name__ == "float"
            and value2 is None
            and value3 is None
        ):
            self._checkResult(
                Indigo._lib.indigoSetOptionFloat(
                    option.encode(ENCODE_ENCODING), value1
                )
            )
        elif (
            type(value1).__name__ == "bool"
            and value2 is None
            and value3 is None
        ):
            value1_b = 0
            if value1:
                value1_b = 1
            self._checkResult(
                Indigo._lib.indigoSetOptionBool(
                    option.encode(ENCODE_ENCODING), value1_b
                )
            )
        elif (
            type(value1).__name__ == "int"
            and value2
            and type(value2).__name__ == "int"
            and value3 is None
        ):
            self._checkResult(
                Indigo._lib.indigoSetOptionXY(
                    option.encode(ENCODE_ENCODING), value1, value2
                )
            )
        elif (
            type(value1).__name__ == "float"
            and value2
            and type(value2).__name__ == "float"
            and value3
            and type(value3).__name__ == "float"
        ):
            self._checkResult(
                Indigo._lib.indigoSetOptionColor(
                    option.encode(ENCODE_ENCODING), value1, value2, value3
                )
            )
        else:
            raise IndigoException("bad option")

    def getOption(self, option):
        """Returns option value by name

        Args:
            option (str): option name

        Returns:
            str: option value
        """
        self._setSessionId()
        return self._checkResultString(
            Indigo._lib.indigoGetOption(option.encode(ENCODE_ENCODING))
        )

    def getOptionInt(self, option):
        """Returns option integer value by name

        Args:
            option (str): option name

        Returns:
            int: option value
        """
        self._setSessionId()
        value = c_int()
        self._checkResult(
            Indigo._lib.indigoGetOptionInt(
                option.encode(ENCODE_ENCODING), pointer(value)
            )
        )
        return value.value

    def getOptionBool(self, option):
        """Returns option boolean value by name

        Args:
            option (str): option name

        Returns:
            bool: option value
        """
        self._setSessionId()
        value = c_int()
        self._checkResult(
            Indigo._lib.indigoGetOptionBool(
                option.encode(ENCODE_ENCODING), pointer(value)
            )
        )
        if value.value == 1:
            return True
        return False

    def getOptionFloat(self, option):
        """Returns option float value by name

        Args:
            option (str): option name

        Returns:
            float: option value
        """
        self._setSessionId()
        value = c_float()
        self._checkResult(
            Indigo._lib.indigoGetOptionFloat(
                option.encode(ENCODE_ENCODING), pointer(value)
            )
        )
        return value.value

    def getOptionType(self, option):
        """Returns option value type by name

        Args:
            option (str): option name

        Returns:
            str: option type string
        """
        self._setSessionId()
        return self._checkResultString(
            Indigo._lib.indigoGetOptionType(option.encode(ENCODE_ENCODING))
        )

    def resetOptions(self):
        """Resets options to default state"""
        self._setSessionId()
        self._checkResult(Indigo._lib.indigoResetOptions())

    def _checkResult(self, result):
        if result < 0:
            raise IndigoException(Indigo._lib.indigoGetLastError())
        return result

    def _checkResultFloat(self, result):
        if result < -0.5:
            raise IndigoException(Indigo._lib.indigoGetLastError())
        return result

    def _checkResultPtr(self, result):
        if result is None:
            raise IndigoException(Indigo._lib.indigoGetLastError())
        return result

    def _checkResultString(self, result):
        return self._checkResultPtr(result).decode(DECODE_ENCODING)

    def convertToArray(self, iteratable):
        """Converts iterable object to array

        Args:
            iteratable (IndigoObject): iterable object

        Raises:
            IndigoException: if object is not iterable

        Returns:
            IndigoObject: array of objects
        """
        if isinstance(iteratable, IndigoObject):
            return iteratable
        try:
            some_object_iterator = iter(iteratable)
            res = self.createArray()
            for obj in some_object_iterator:
                res.arrayAdd(self.convertToArray(obj))
            return res
        except TypeError:
            raise IndigoException(
                "Cannot convert object %s to an array" % (iteratable)
            )

    def dbgBreakpoint(self):
        self._setSessionId()
        return Indigo._lib.indigoDbgBreakpoint()

    def version(self):
        """Returns Indigo version

        Returns:
            str: version string
        """
        self._setSessionId()
        return self._checkResultString(Indigo._lib.indigoVersion())

    def countReferences(self):
        """Returns the number of objects in pool

        Returns:
            int: number of objects
        """
        self._setSessionId()
        return self._checkResult(Indigo._lib.indigoCountReferences())

    def writeFile(self, filename):
        """Creates file writer object

        Args:
            filename (str): full path to the file

        Returns:
            IndigoObject: file writer object
        """
        self._setSessionId()
        return self.IndigoObject(
            self,
            self._checkResult(
                Indigo._lib.indigoWriteFile(filename.encode(ENCODE_ENCODING))
            ),
        )

    def writeBuffer(self):
        """Creates buffer to write an object

        Returns:
            IndigoObject: buffer object
        """
        self._setSessionId()
        return self.IndigoObject(
            self, self._checkResult(Indigo._lib.indigoWriteBuffer())
        )

    def createMolecule(self):
        """Creates molecule object

        Returns:
            IndigoObject: molecule object
        """
        self._setSessionId()
        return self.IndigoObject(
            self, self._checkResult(Indigo._lib.indigoCreateMolecule())
        )

    def createQueryMolecule(self):
        """Creates query molecule object

        Returns:
            IndigoObject: query molecule
        """
        self._setSessionId()
        return self.IndigoObject(
            self, self._checkResult(Indigo._lib.indigoCreateQueryMolecule())
        )

    def loadMolecule(self, string):
        """Loads molecule from string. Format is automatically recognized.

        Args:
            string (str): molecule format

        Returns:
            IndigoObject: molecule object

        Raises:
            IndigoException: Exception if structure format is incorrect
        """
        self._setSessionId()
        return self.IndigoObject(
            self,
            self._checkResult(
                Indigo._lib.indigoLoadMoleculeFromString(
                    string.encode(ENCODE_ENCODING)
                )
            ),
        )

    def loadMoleculeFromFile(self, filename):
        """Loads molecule from file. Automatically detects input format.

        Args:
            filename (str): full path to a file

        Returns:
            IndigoObject: loaded molecular structure

        Raises:
            IndigoException: Exception if structure format is incorrect
        """
        self._setSessionId()
        return self.IndigoObject(
            self,
            self._checkResult(
                Indigo._lib.indigoLoadMoleculeFromFile(
                    filename.encode(ENCODE_ENCODING)
                )
            ),
        )

    def loadMoleculeFromBuffer(self, data):
        """Loads molecule from buffer. Automatically detects input format.

        Args:
            data (bytes): input byte array

        Returns:
            IndigoObject: loaded molecular structure

        Raises:
            IndigoException: Exception if structure format is incorrect

        Examples:
            .. code-block:: python

                with open (..), 'rb') as f:
                    m = indigo.loadMoleculeFromBuffer(f.read())
        """
        if sys.version_info[0] < 3:
            buf = map(ord, data)
        else:
            buf = data
        values = (c_byte * len(buf))()
        for i in range(len(buf)):
            values[i] = buf[i]
        self._setSessionId()
        return self.IndigoObject(
            self,
            self._checkResult(
                Indigo._lib.indigoLoadMoleculeFromBuffer(values, len(buf))
            ),
        )

    def loadQueryMolecule(self, string):
        """Loads query molecule from string. Format will be automatically recognized.

        Args:
            string (str): molecule format

        Returns:
            IndigoObject: query molecule object

        Raises:
            IndigoException: Exception if structure format is incorrect
        """
        self._setSessionId()
        return self.IndigoObject(
            self,
            self._checkResult(
                Indigo._lib.indigoLoadQueryMoleculeFromString(
                    string.encode(ENCODE_ENCODING)
                )
            ),
        )

    def loadQueryMoleculeFromFile(self, filename):
        """
        Loads query molecule from file. Automatically detects input format.

        Args:
            filename (str): full path to a file

        Returns:
            IndigoObject: loaded query molecular structure

        Raises:
            IndigoException: Exception if structure format is incorrect
        """
        self._setSessionId()
        return self.IndigoObject(
            self,
            self._checkResult(
                Indigo._lib.indigoLoadQueryMoleculeFromFile(
                    filename.encode(ENCODE_ENCODING)
                )
            ),
        )

    def loadSmarts(self, string):
        """Loads query molecule from string in SMARTS format

        Args:
            string (str): smarts string

        Returns:
            IndigoObject: loaded query molecular structure

        Raises:
            IndigoException: Exception if structure format is incorrect
        """
        self._setSessionId()
        return self.IndigoObject(
            self,
            self._checkResult(
                Indigo._lib.indigoLoadSmartsFromString(
                    string.encode(ENCODE_ENCODING)
                )
            ),
        )

    def loadSmartsFromFile(self, filename):
        """Loads query molecule from file in SMARTS format

        Args:
            filename (str): full path to the file with smarts strings

        Returns:
            IndigoObject: loaded query molecular structure

        Raises:
            IndigoException: Exception if structure format is incorrect
        """
        self._setSessionId()
        return self.IndigoObject(
            self,
            self._checkResult(
                Indigo._lib.indigoLoadSmartsFromFile(
                    filename.encode(ENCODE_ENCODING)
                )
            ),
        )

    def loadReaction(self, string):
        """Loads reaction from string. Format will be automatically recognized.

        Args:
            string (str): reaction format

        Returns:
            IndigoObject: reaction object

        Raises:
            IndigoException: Exception if structure format is incorrect
        """
        self._setSessionId()
        return self.IndigoObject(
            self,
            self._checkResult(
                Indigo._lib.indigoLoadReactionFromString(
                    string.encode(ENCODE_ENCODING)
                )
            ),
        )

    def loadReactionFromFile(self, filename):
        """Loads reaction from file

        Args:
            filename (str): full path to a file

        Returns:
            IndigoObject: loaded reaction

        Raises:
            IndigoException: Exception if structure format is incorrect
        """
        self._setSessionId()
        return self.IndigoObject(
            self,
            self._checkResult(
                Indigo._lib.indigoLoadReactionFromFile(
                    filename.encode(ENCODE_ENCODING)
                )
            ),
        )

    def loadQueryReaction(self, string):
        """Loads query reaction from string. Format will be automatically recognized.

        Args:
            string (str): reaction format

        Returns:
            IndigoObject: query reaction object
        """
        self._setSessionId()
        return self.IndigoObject(
            self,
            self._checkResult(
                Indigo._lib.indigoLoadQueryReactionFromString(
                    string.encode(ENCODE_ENCODING)
                )
            ),
        )

    def loadQueryReactionFromFile(self, filename):
        """Loads query reaction from file. Automatically detects input format.

        Args:
            filename (str): full path to a file

        Returns:
            IndigoObject: loaded query reaction object

        Raises:
            IndigoException: Exception if structure format is incorrect
        """
        self._setSessionId()
        return self.IndigoObject(
            self,
            self._checkResult(
                Indigo._lib.indigoLoadQueryReactionFromFile(
                    filename.encode(ENCODE_ENCODING)
                )
            ),
        )

    def loadReactionSmarts(self, string):
        """Loads query reaction from string in SMARTS format

        Args:
            string (str): smarts string

        Returns:
            IndigoObject: loaded query reaction

        Raises:
            IndigoException: Exception if structure format is incorrect
        """
        self._setSessionId()
        return self.IndigoObject(
            self,
            self._checkResult(
                Indigo._lib.indigoLoadReactionSmartsFromString(
                    string.encode(ENCODE_ENCODING)
                )
            ),
        )

    def loadReactionSmartsFromFile(self, filename):
        """Loads query reaction from file in SMARTS format

        Args:
            filename (str): full path to the file with smarts strings

        Returns:
            IndigoObject: loaded query reaction

        Raises:
            IndigoException: Exception if structure format is incorrect
        """
        self._setSessionId()
        return self.IndigoObject(
            self,
            self._checkResult(
                Indigo._lib.indigoLoadReactionSmartsFromFile(
                    filename.encode(ENCODE_ENCODING)
                )
            ),
        )

    def loadStructure(self, structureStr, parameter=None):
        """Loads structure from string

        Args:
            structureStr (str): string with structure format
            parameter (str): parameters for loading. Optional, defaults to None.

        Returns:
            IndigoObject: loaded object
        """
        self._setSessionId()
        parameter = "" if parameter is None else parameter
        return self.IndigoObject(
            self,
            self._checkResult(
                Indigo._lib.indigoLoadStructureFromString(
                    structureStr.encode(ENCODE_ENCODING),
                    parameter.encode(ENCODE_ENCODING),
                )
            ),
        )

    def loadStructureFromBuffer(self, structureData, parameter=None):
        """Loads structure object from buffer

        Args:
            structureData (list): array of bytes
            parameter (str): parameters for loading. Optional, defaults to None.

        Returns:
            IndigoObject: loaded object
        """
        if sys.version_info[0] < 3:
            buf = map(ord, structureData)
        else:
            buf = structureData
        values = (c_byte * len(buf))()
        for i in range(len(buf)):
            values[i] = buf[i]
        self._setSessionId()
        parameter = "" if parameter is None else parameter
        return self.IndigoObject(
            self,
            self._checkResult(
                Indigo._lib.indigoLoadStructureFromBuffer(
                    values, len(buf), parameter.encode(ENCODE_ENCODING)
                )
            ),
        )

    def loadStructureFromFile(self, filename, parameter=None):
        """Loads structure object from file

        Args:
            filename (str): full path with structure information
            parameter (str): parameters to load. Optional, defaults to None.

        Returns:
            IndigoObject: loaded object
        """
        self._setSessionId()
        parameter = "" if parameter is None else parameter
        return self.IndigoObject(
            self,
            self._checkResult(
                Indigo._lib.indigoLoadStructureFromFile(
                    filename.encode(ENCODE_ENCODING),
                    parameter.encode(ENCODE_ENCODING),
                )
            ),
        )

    def checkStructure(self, structure, props=""):
        """Runs validation for the given structure

        Args:
            structure (IndigoObject):structure object
            props (str): Parameters for validation. Optional, defaults to "".

        Returns:
           str: validation results string
        """
        if props is None:
            props = ""
        self._setSessionId()
        return self._checkResultString(
            Indigo._lib.indigoCheckStructure(
                structure.encode(ENCODE_ENCODING),
                props.encode(ENCODE_ENCODING),
            )
        )

    def loadFingerprintFromBuffer(self, buffer):
        """Creates a fingerprint from the supplied binary data

        Args:
            buffer (list): array of bytes

        Returns:
            IndigoObject: fingerprint object
        """
        self._setSessionId()
        length = len(buffer)

        values = (c_byte * length)()
        for i in range(length):
            values[i] = buffer[i]

        return self.IndigoObject(
            self,
            self._checkResult(
                Indigo._lib.indigoLoadFingerprintFromBuffer(values, length)
            ),
        )

    def loadFingerprintFromDescriptors(self, descriptors, size, density):
        """Packs a list of molecule descriptors into a fingerprint object

        Args:
            descriptors (list): list of normalized numbers (roughly) between 0.0 and 1.0
            size (int): size of the fingerprint in bytes
            density (float): approximate density of '1's vs '0's in the fingerprint

        Returns:
            IndigoObject: fingerprint object
        """
        self._setSessionId()
        length = len(descriptors)

        descr_arr = (c_double * length)()
        for i in range(length):
            descr_arr[i] = descriptors[i]

        result = Indigo._lib.indigoLoadFingerprintFromDescriptors(
            descr_arr, length, size, density
        )
        return self.IndigoObject(self, self._checkResult(result))

    def createReaction(self):
        """Creates reaction object

        Returns:
            IndigoObject: reaction object
        """
        self._setSessionId()
        return self.IndigoObject(
            self, self._checkResult(Indigo._lib.indigoCreateReaction())
        )

    def createQueryReaction(self):
        """Creates query reaction object

        Returns:
            IndigoObject: query reaction object
        """
        self._setSessionId()
        return self.IndigoObject(
            self, self._checkResult(Indigo._lib.indigoCreateQueryReaction())
        )

    def exactMatch(self, item1, item2, flags=""):
        """Creates match object for the given structures

        Args:
            item1 (IndigoObject): first target structure (molecule or reaction)
            item2 (IndigoObject): second target structure (molecule or reaction)
            flags (str): exact match options. Optional, defaults to "".

        Returns:
            IndigoObject: match object
        """
        if flags is None:
            flags = ""
        self._setSessionId()
        newobj = self._checkResult(
            Indigo._lib.indigoExactMatch(
                item1.id, item2.id, flags.encode(ENCODE_ENCODING)
            )
        )
        if newobj == 0:
            return None
        else:
            return self.IndigoObject(self, newobj, [item1, item2, self])

    def setTautomerRule(self, id, beg, end):
        """Sets tautormer rules

        Args:
            id (int): tau rule index
            beg (str): begin value
            end (str): end value

        Returns:
            int: 1 if there are no errors
        """
        self._setSessionId()
        return self._checkResult(
            Indigo._lib.indigoSetTautomerRule(
                id, beg.encode(ENCODE_ENCODING), end.encode(ENCODE_ENCODING)
            )
        )

    def removeTautomerRule(self, id):
        """Removes tautomer rule

        Args:
            id (int): tau rule index

        Returns:
            int: 1 if there are no errors
        """
        self._setSessionId()
        return self._checkResult(Indigo._lib.indigoRemoveTautomerRule(id))

    def clearTautomerRules(self):
        """Clears all tautomer rules

        Returns:
            int: 1 if there are no errors
        """
        self._setSessionId()
        return self._checkResult(Indigo._lib.indigoClearTautomerRules())

    def commonBits(self, fingerprint1, fingerprint2):
        """Returns the number of common 1 bits for the given fingerprints

        Args:
            fingerprint1 (IndigoObject): first fingerprint object
            fingerprint2 (IndigoObject): second fingerprint object

        Returns:
            int: number of common bits
        """
        self._setSessionId()
        return self._checkResult(
            Indigo._lib.indigoCommonBits(fingerprint1.id, fingerprint2.id)
        )

    def similarity(self, item1, item2, metrics=""):
        """Returns the similarity measure between two structures.
        Accepts two molecules, two reactions, or two fingerprints.

        Args:
            item1 (IndigoObject): molecule, reaction or fingerprint object
            item2 (IndigoObject): molecule, reaction or fingerprint object
            metrics (str): "tanimoto", "tversky", "tversky <alpha> <beta>", "euclid-sub" or "normalized-edit". Optional, defaults to "tanimoto".

        Returns:
            float: [description]
        """
        if metrics is None:
            metrics = ""
        self._setSessionId()
        return self._checkResultFloat(
            Indigo._lib.indigoSimilarity(
                item1.id, item2.id, metrics.encode(ENCODE_ENCODING)
            )
        )

    def iterateSDFile(self, filename):
        """Returns iterator for SDF files

        Args:
            filename (str): full file path

        Returns:
            IndigoObject: SD iterator object
        """
        self._setSessionId()
        return self.IndigoObject(
            self,
            self._checkResult(
                Indigo._lib.indigoIterateSDFile(
                    filename.encode(ENCODE_ENCODING)
                )
            ),
        )

    def iterateRDFile(self, filename):
        """Returns iterator for RDF files

        Args:
            filename (str): full file path

        Returns:
            IndigoObject: RD iterator object
        """
        self._setSessionId()
        return self.IndigoObject(
            self,
            self._checkResult(
                Indigo._lib.indigoIterateRDFile(
                    filename.encode(ENCODE_ENCODING)
                )
            ),
        )

    def iterateSmilesFile(self, filename):
        """Returns iterator for smiles files

        Args:
            filename (str): full file path

        Returns:
            IndigoObject: smiles iterator object
        """
        self._setSessionId()
        return self.IndigoObject(
            self,
            self._checkResult(
                Indigo._lib.indigoIterateSmilesFile(
                    filename.encode(ENCODE_ENCODING)
                )
            ),
        )

    def iterateCMLFile(self, filename):
        """Returns iterator for CML files

        Args:
            filename (str): full file path

        Returns:
            IndigoObject: CML iterator object
        """
        self._setSessionId()
        return self.IndigoObject(
            self,
            self._checkResult(
                Indigo._lib.indigoIterateCMLFile(
                    filename.encode(ENCODE_ENCODING)
                )
            ),
        )

    def iterateCDXFile(self, filename):
        """Returns iterator for CDX files

        Args:
            filename (str): full file path

        Returns:
            IndigoObject: CDX iterator object
        """
        self._setSessionId()
        return self.IndigoObject(
            self,
            self._checkResult(
                Indigo._lib.indigoIterateCDXFile(
                    filename.encode(ENCODE_ENCODING)
                )
            ),
        )

    def createFileSaver(self, filename, format):
        """Creates file saver object

        Args:
            filename (str): full file path
            format (str): file format

        Returns:
            IndigoObject: file saver object
        """
        self._setSessionId()
        return self.IndigoObject(
            self,
            self._checkResult(
                Indigo._lib.indigoCreateFileSaver(
                    filename.encode(ENCODE_ENCODING),
                    format.encode(ENCODE_ENCODING),
                )
            ),
        )

    def createSaver(self, obj, format):
        """Creates saver object

        Args:
            obj (IndigoObject): output object
            format (str): format settings

        Returns:
            IndigoObject: saver object
        """
        self._setSessionId()
        return self.IndigoObject(
            self,
            self._checkResult(
                Indigo._lib.indigoCreateSaver(
                    obj.id, format.encode(ENCODE_ENCODING)
                )
            ),
        )

    def createArray(self):
        """Creates array object

        Returns:
            IndigoObject: array object
        """
        self._setSessionId()
        return self.IndigoObject(
            self, self._checkResult(Indigo._lib.indigoCreateArray())
        )

    def substructureMatcher(self, target, mode=""):
        """Creates substructure matcher

        Args:
            target (IndigoObject): target molecule or reaction
            mode (str): substructure mode. Optional, defaults to "".

        Returns:
            IndigoObject: substructure matcher
        """
        if mode is None:
            mode = ""
        self._setSessionId()
        return self.IndigoObject(
            self,
            self._checkResult(
                Indigo._lib.indigoSubstructureMatcher(
                    target.id, mode.encode(ENCODE_ENCODING)
                )
            ),
            target,
        )

    def extractCommonScaffold(self, structures, options=""):
        """Extracts common scaffold for the given structures

        Args:
            structures (IndigoObject): array object of molecule structures
            options (str): extraction options. Optional, defaults to "".

        Returns:
            IndigoObject: scaffold object
        """
        structures = self.convertToArray(structures)
        if options is None:
            options = ""
        self._setSessionId()
        newobj = self._checkResult(
            Indigo._lib.indigoExtractCommonScaffold(
                structures.id, options.encode(ENCODE_ENCODING)
            )
        )
        if newobj == 0:
            return None
        else:
            return self.IndigoObject(self, newobj, self)

    def decomposeMolecules(self, scaffold, structures):
        """Creates deconvolution object for the given structures

        Args:
            scaffold (IndigoObject): query molecule object
            structures (IndigoObject): array of molecule structures

        Returns:
            IndigoObject: deconvolution object
        """
        structures = self.convertToArray(structures)
        self._setSessionId()
        return self.IndigoObject(
            self,
            self._checkResult(
                Indigo._lib.indigoDecomposeMolecules(
                    scaffold.id, structures.id
                )
            ),
            scaffold,
        )

    def rgroupComposition(self, molecule, options=""):
        """Creates composition iterator

        Args:
            molecule (IndigoObject): target molecule object
            options (str): rgroup composition options. Optional, defaults to "".

        Returns:
            IndigoObject: composition iterator
        """
        if options is None:
            options = ""
        self._setSessionId()
        newobj = self._checkResult(
            Indigo._lib.indigoRGroupComposition(
                molecule.id, options.encode(ENCODE_ENCODING)
            )
        )
        if newobj == 0:
            return None
        else:
            return self.IndigoObject(self, newobj, self)

    def getFragmentedMolecule(self, elem, options=""):
        """Returns fragmented molecule for the given composition element

        Args:
            elem (IndigoObject): composition element object
            options (str): Fragmentation options. Optional, defaults to "".

        Returns:
            IndigoObject: fragmented structure object
        """
        if options is None:
            options = ""
        self._setSessionId()
        newobj = self._checkResult(
            Indigo._lib.indigoGetFragmentedMolecule(
                elem.id, options.encode(ENCODE_ENCODING)
            )
        )
        if newobj == 0:
            return None
        else:
            return self.IndigoObject(self, newobj, self)

    def createDecomposer(self, scaffold):
        """Creates deconvolution object for the given scaffold

        Args:
            scaffold (IndigoObject): scaffold molecular structure

        Returns:
            IndigoObject: deconvolution object
        """
        self._setSessionId()
        return self.IndigoObject(
            self,
            self._checkResult(Indigo._lib.indigoCreateDecomposer(scaffold.id)),
            scaffold,
        )

    def reactionProductEnumerate(self, replacedaction, monomers):
        """Creates reaction product enumeration iterator

        Args:
            replacedaction (IndigoObject): query reaction for the enumeration
            monomers (IndigoObject): array of objects to enumerate

        Returns:
            IndigoObject: result products iterator
        """
        self._setSessionId()
        monomers = self.convertToArray(monomers)
        return self.IndigoObject(
            self,
            self._checkResult(
                Indigo._lib.indigoReactionProductEnumerate(
                    replacedaction.id, monomers.id
                )
            ),
            replacedaction,
        )

    def transform(self, reaction, monomers):
        """Transforms the given monomers by reaction

        Args:
            reaction (IndigoObject): query reaction
            monomers (IndigoObject): array of objects to transform

        Returns:
            IndigoObject: mapping object
        """
        self._setSessionId()
        newobj = self._checkResult(
            Indigo._lib.indigoTransform(reaction.id, monomers.id)
        )
        if newobj == 0:
            return None
        else:
            return self.IndigoObject(self, newobj, self)

    def loadBuffer(self, buf):
        """Creates scanner object from buffer

        Args:
            buf(list): array of bytes

        Returns:
            IndigoObject: scanner object
        """
        buf = list(buf)
        values = (c_byte * len(buf))()
        for i in range(len(buf)):
            values[i] = buf[i]
        self._setSessionId()
        return self.IndigoObject(
            self,
            self._checkResult(Indigo._lib.indigoLoadBuffer(values, len(buf))),
        )

    def loadString(self, string):
        """Creates scanner object from string

        Args:
            string (str): string with information

        Returns:
            IndigoObject: scanner object
        """
        self._setSessionId()
        return self.IndigoObject(
            self,
            self._checkResult(
                Indigo._lib.indigoLoadString(string.encode(ENCODE_ENCODING))
            ),
        )

    def iterateSDF(self, reader):
        """Creates SDF iterator from scanner object

        Args:
            reader (IndigoObject): scanner object

        Returns:
            IndigoObject: SD iterator object
        """
        self._setSessionId()
        result = self._checkResult(Indigo._lib.indigoIterateSDF(reader.id))
        if not result:
            return None
        return self.IndigoObject(self, result, reader)

    def iterateSmiles(self, reader):
        """Creates smiles iterator from scanner object

        Args:
            reader (IndigoObject): scanner object

        Returns:
            IndigoObject: smiles iterator object
        """
        self._setSessionId()
        result = self._checkResult(Indigo._lib.indigoIterateSmiles(reader.id))
        if not result:
            return None
        return self.IndigoObject(self, result, reader)

    def iterateCML(self, reader):
        """Creates CML iterator from scanner object

        Args:
            reader (IndigoObject): scanner object

        Returns:
            IndigoObject: CML iterator object
        """
        self._setSessionId()
        result = self._checkResult(Indigo._lib.indigoIterateCML(reader.id))
        if not result:
            return None
        return self.IndigoObject(self, result, reader)

    def iterateCDX(self, reader):
        """Creates CDX iterator from scanner object

        Args:
            reader (IndigoObject): scanner object

        Returns:
            IndigoObject: CDX iterator object
        """
        self._setSessionId()
        result = self._checkResult(Indigo._lib.indigoIterateCDX(reader.id))
        if not result:
            return None
        return self.IndigoObject(self, result, reader)

    def iterateRDF(self, reader):
        """Creates RDF iterator from scanner object

        Args:
            reader (IndigoObject): scanner object

        Returns:
            IndigoObject: RD iterator object
        """
        self._setSessionId()
        result = self._checkResult(Indigo._lib.indigoIterateRDF(reader.id))
        if not result:
            return None
        return self.IndigoObject(self, result, reader)

    def iterateTautomers(self, molecule, params):
        """Iterates tautomers for the given molecule

        Args:
            molecule (IndigoObject): molecule to find tautomers from
            params (str): tau iteration parameters. "INCHI" or "RSMARTS". Defaults to "RSMARTS"

        Returns:
            IndigoObject: molecule iterator object
        """
        self._setSessionId()
        return self.IndigoObject(
            self,
            self._checkResult(
                Indigo._lib.indigoIterateTautomers(
                    molecule.id, params.encode(ENCODE_ENCODING)
                )
            ),
            molecule,
        )

    def nameToStructure(self, name, params=None):
        """
        Converts a chemical name into a corresponding structure

        Args:
            name (str): a name to parse
            params (str): a string (optional) containing parsing options or None if no options are changed

        Raises:
            IndigoException: if parsing fails or no structure is found

        """
        if params is None:
            params = ""
        self._setSessionId()
        return self.IndigoObject(
            self,
            self._checkResult(
                Indigo._lib.indigoNameToStructure(
                    name.encode(ENCODE_ENCODING),
                    params.encode(ENCODE_ENCODING),
                )
            ),
        )

    def buildPkaModel(self, level, threshold, filename):
        """Builds pKa model into file

        Args:
            level (int): max level
            threshold (float): threshold value
            filename (str): full path to the file

        Returns:
            int: 1 if level > 0, 0 otherwise
        """
        self._setSessionId()
        return self._checkResult(
            Indigo._lib.indigoBuildPkaModel(
                level, threshold, filename.encode(ENCODE_ENCODING)
            )
        )

    def transformHELMtoSCSR(self, item):
        """Transforms HELM to SCSR object

        Args:
            item (IndigoObject): object with HELM information

        Returns:
            IndigoObject: molecule with SCSR object
        """
        self._setSessionId()
        return self.IndigoObject(
            self,
            self._checkResult(Indigo._lib.indigoTransformHELMtoSCSR(item.id)),
        )

    def check(self, moltext, checkflags="", props=""):
        """Validates the given structure

        Args:
            moltext (str): input structure string
            checkflags (str): validation flags. Optional, defaults to "".
            props (str): validation properties. Optional, defaults to "".

        Returns:
            str: validation string
        """
        if props is None:
            props = ""
        if checkflags is None:
            checkflags = ""
        self._setSessionId()
        return self._checkResultString(
            Indigo._lib.indigoCheck(
                moltext.encode(ENCODE_ENCODING),
                checkflags.encode(ENCODE_ENCODING),
                props.encode(ENCODE_ENCODING),
            )
        )
