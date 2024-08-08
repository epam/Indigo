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
import os
from ctypes import (
    CDLL,
    POINTER,
    c_byte,
    c_char_p,
    c_double,
    c_float,
    c_int,
    c_int64,
    c_ubyte,
    c_ulonglong,
)
from typing import Optional, Type

from .._common.lib import Lib
from .indigo_exception import IndigoException


class IndigoLib:
    lib: Optional[CDLL] = None

    def __init__(self) -> None:
        if IndigoLib.lib:
            return

        IndigoLib.lib = Lib.load("indigo")
        IndigoLib.lib.indigoVersion.restype = c_char_p
        IndigoLib.lib.indigoVersion.argtypes = []
        IndigoLib.lib.indigoVersionInfo.restype = c_char_p
        IndigoLib.lib.indigoVersionInfo.argtypes = []
        IndigoLib.lib.indigoAllocSessionId.restype = c_ulonglong
        IndigoLib.lib.indigoAllocSessionId.argtypes = []
        IndigoLib.lib.indigoSetSessionId.restype = None
        IndigoLib.lib.indigoSetSessionId.argtypes = [c_ulonglong]
        IndigoLib.lib.indigoReleaseSessionId.restype = None
        IndigoLib.lib.indigoReleaseSessionId.argtypes = [c_ulonglong]
        IndigoLib.lib.indigoGetLastError.restype = c_char_p
        IndigoLib.lib.indigoGetLastError.argtypes = []
        IndigoLib.lib.indigoFree.restype = c_int
        IndigoLib.lib.indigoFree.argtypes = [c_int]
        IndigoLib.lib.indigoCountReferences.restype = c_int
        IndigoLib.lib.indigoCountReferences.argtypes = []
        IndigoLib.lib.indigoFreeAllObjects.restype = c_int
        IndigoLib.lib.indigoFreeAllObjects.argtypes = []
        IndigoLib.lib.indigoSetOption.restype = c_int
        IndigoLib.lib.indigoSetOption.argtypes = [c_char_p, c_char_p]
        IndigoLib.lib.indigoSetOptionInt.restype = c_int
        IndigoLib.lib.indigoSetOptionInt.argtypes = [c_char_p, c_int]
        IndigoLib.lib.indigoSetOptionBool.restype = c_int
        IndigoLib.lib.indigoSetOptionBool.argtypes = [c_char_p, c_int]
        IndigoLib.lib.indigoSetOptionFloat.restype = c_int
        IndigoLib.lib.indigoSetOptionFloat.argtypes = [c_char_p, c_float]
        IndigoLib.lib.indigoSetOptionColor.restype = c_int
        IndigoLib.lib.indigoSetOptionColor.argtypes = [
            c_char_p,
            c_float,
            c_float,
            c_float,
        ]
        IndigoLib.lib.indigoSetOptionXY.restype = c_int
        IndigoLib.lib.indigoSetOptionXY.argtypes = [c_char_p, c_int, c_int]
        IndigoLib.lib.indigoGetOption.restype = c_char_p
        IndigoLib.lib.indigoGetOption.argtypes = [c_char_p]
        IndigoLib.lib.indigoGetOptionInt.restype = c_int
        IndigoLib.lib.indigoGetOptionInt.argtypes = [c_char_p, POINTER(c_int)]
        IndigoLib.lib.indigoGetOptionBool.argtypes = [c_char_p, POINTER(c_int)]
        IndigoLib.lib.indigoGetOptionBool.restype = c_int
        IndigoLib.lib.indigoGetOptionFloat.argtypes = [
            c_char_p,
            POINTER(c_float),
        ]
        IndigoLib.lib.indigoGetOptionFloat.restype = c_int
        IndigoLib.lib.indigoGetOptionColor.argtypes = [
            c_char_p,
            POINTER(c_float),
            POINTER(c_float),
            POINTER(c_float),
        ]
        IndigoLib.lib.indigoGetOptionColor.restype = c_int
        IndigoLib.lib.indigoGetOptionXY.argtypes = [
            c_char_p,
            POINTER(c_int),
            POINTER(c_int),
        ]
        IndigoLib.lib.indigoGetOptionXY.restype = c_int
        IndigoLib.lib.indigoGetOptionType.restype = c_char_p
        IndigoLib.lib.indigoGetOptionType.argtypes = [c_char_p]
        IndigoLib.lib.indigoReadFile.restype = c_int
        IndigoLib.lib.indigoReadFile.argtypes = [c_char_p]
        IndigoLib.lib.indigoLoadString.restype = c_int
        IndigoLib.lib.indigoLoadString.argtypes = [c_char_p]
        IndigoLib.lib.indigoLoadBuffer.restype = c_int
        IndigoLib.lib.indigoLoadBuffer.argtypes = [POINTER(c_byte), c_int]
        IndigoLib.lib.indigoWriteFile.restype = c_int
        IndigoLib.lib.indigoWriteFile.argtypes = [c_char_p]
        IndigoLib.lib.indigoWriteBuffer.restype = c_int
        IndigoLib.lib.indigoWriteBuffer.argtypes = []
        IndigoLib.lib.indigoCreateMolecule.restype = c_int
        IndigoLib.lib.indigoCreateMolecule.argtypes = []
        IndigoLib.lib.indigoCreateQueryMolecule.restype = c_int
        IndigoLib.lib.indigoCreateQueryMolecule.argtypes = []
        IndigoLib.lib.indigoLoadMoleculeFromString.restype = c_int
        IndigoLib.lib.indigoLoadMoleculeFromString.argtypes = [c_char_p]
        IndigoLib.lib.indigoLoadMoleculeFromFile.restype = c_int
        IndigoLib.lib.indigoLoadMoleculeFromFile.argtypes = [c_char_p]
        IndigoLib.lib.indigoLoadMoleculeFromBuffer.restype = c_int
        IndigoLib.lib.indigoLoadMoleculeFromBuffer.argtypes = [
            POINTER(c_byte),
            c_int,
        ]
        IndigoLib.lib.indigoLoadQueryMoleculeFromString.restype = c_int
        IndigoLib.lib.indigoLoadQueryMoleculeFromString.argtypes = [c_char_p]
        IndigoLib.lib.indigoLoadQueryMoleculeFromFile.restype = c_int
        IndigoLib.lib.indigoLoadQueryMoleculeFromFile.argtypes = [c_char_p]
        IndigoLib.lib.indigoLoadSmartsFromString.restype = c_int
        IndigoLib.lib.indigoLoadSmartsFromString.argtypes = [c_char_p]
        IndigoLib.lib.indigoLoadSmartsFromFile.restype = c_int
        IndigoLib.lib.indigoLoadSmartsFromFile.argtypes = [c_char_p]
        IndigoLib.lib.indigoLoadMonomerLibraryFromString.restype = c_int
        IndigoLib.lib.indigoLoadMonomerLibraryFromString.argtypes = [c_char_p]
        IndigoLib.lib.indigoLoadMonomerLibraryFromFile.restype = c_int
        IndigoLib.lib.indigoLoadMonomerLibraryFromFile.argtypes = [c_char_p]
        IndigoLib.lib.indigoLoadKetDocumentFromString.restype = c_int
        IndigoLib.lib.indigoLoadKetDocumentFromString.argtypes = [c_char_p]
        IndigoLib.lib.indigoLoadKetDocumentFromFile.restype = c_int
        IndigoLib.lib.indigoLoadKetDocumentFromFile.argtypes = [c_char_p]
        IndigoLib.lib.indigoLoadSequenceFromString.restype = c_int
        IndigoLib.lib.indigoLoadSequenceFromString.argtypes = [
            c_char_p,
            c_char_p,
            c_int,
        ]
        IndigoLib.lib.indigoLoadSequenceFromFile.restype = c_int
        IndigoLib.lib.indigoLoadSequenceFromFile.argtypes = [
            c_char_p,
            c_char_p,
            c_int,
        ]
        IndigoLib.lib.indigoLoadFastaFromString.restype = c_int
        IndigoLib.lib.indigoLoadFastaFromString.argtypes = [
            c_char_p,
            c_char_p,
            c_int,
        ]
        IndigoLib.lib.indigoLoadFastaFromFile.restype = c_int
        IndigoLib.lib.indigoLoadFastaFromFile.argtypes = [
            c_char_p,
            c_char_p,
            c_int,
        ]
        IndigoLib.lib.indigoLoadIdtFromString.restype = c_int
        IndigoLib.lib.indigoLoadIdtFromString.argtypes = [c_char_p, c_int]
        IndigoLib.lib.indigoLoadIdtFromFile.restype = c_int
        IndigoLib.lib.indigoLoadIdtFromFile.argtypes = [c_char_p, c_int]
        IndigoLib.lib.indigoLoadHelmFromString.restype = c_int
        IndigoLib.lib.indigoLoadHelmFromString.argtypes = [c_char_p, c_int]
        IndigoLib.lib.indigoLoadHelmFromFile.restype = c_int
        IndigoLib.lib.indigoLoadHelmFromFile.argtypes = [c_char_p, c_int]
        IndigoLib.lib.indigoLoadReactionFromString.restype = c_int
        IndigoLib.lib.indigoLoadReactionFromString.argtypes = [c_char_p]
        IndigoLib.lib.indigoLoadReactionFromFile.restype = c_int
        IndigoLib.lib.indigoLoadReactionFromFile.argtypes = [c_char_p]
        IndigoLib.lib.indigoLoadQueryReactionFromString.restype = c_int
        IndigoLib.lib.indigoLoadQueryReactionFromString.argtypes = [c_char_p]
        IndigoLib.lib.indigoLoadQueryReactionFromFile.restype = c_int
        IndigoLib.lib.indigoLoadQueryReactionFromFile.argtypes = [c_char_p]
        IndigoLib.lib.indigoLoadReactionSmartsFromString.restype = c_int
        IndigoLib.lib.indigoLoadReactionSmartsFromString.argtypes = [c_char_p]
        IndigoLib.lib.indigoLoadReactionSmartsFromFile.restype = c_int
        IndigoLib.lib.indigoLoadReactionSmartsFromFile.argtypes = [c_char_p]
        IndigoLib.lib.indigoLoadStructureFromString.restype = c_int
        IndigoLib.lib.indigoLoadStructureFromString.argtypes = [
            c_char_p,
            c_char_p,
        ]
        IndigoLib.lib.indigoLoadStructureFromBuffer.restype = c_int
        IndigoLib.lib.indigoLoadStructureFromBuffer.argtypes = [
            POINTER(c_byte),
            c_int,
            c_char_p,
        ]
        IndigoLib.lib.indigoLoadStructureFromFile.restype = c_int
        IndigoLib.lib.indigoLoadStructureFromFile.argtypes = [
            c_char_p,
            c_char_p,
        ]
        IndigoLib.lib.indigoCreateReaction.restype = c_int
        IndigoLib.lib.indigoCreateReaction.argtypes = []
        IndigoLib.lib.indigoCreateQueryReaction.restype = c_int
        IndigoLib.lib.indigoCreateQueryReaction.argtypes = []
        IndigoLib.lib.indigoExactMatch.restype = c_int
        IndigoLib.lib.indigoExactMatch.argtypes = [c_int, c_int, c_char_p]
        IndigoLib.lib.indigoSetTautomerRule.restype = c_int
        IndigoLib.lib.indigoSetTautomerRule.argtypes = [
            c_int,
            c_char_p,
            c_char_p,
        ]
        IndigoLib.lib.indigoRemoveTautomerRule.restype = c_int
        IndigoLib.lib.indigoRemoveTautomerRule.argtypes = [c_int]
        IndigoLib.lib.indigoClearTautomerRules.restype = c_int
        IndigoLib.lib.indigoClearTautomerRules.argtypes = []
        IndigoLib.lib.indigoUnserialize.restype = c_int
        IndigoLib.lib.indigoUnserialize.argtypes = [POINTER(c_ubyte), c_int]
        IndigoLib.lib.indigoCommonBits.restype = c_int
        IndigoLib.lib.indigoCommonBits.argtypes = [c_int, c_int]
        IndigoLib.lib.indigoSimilarity.restype = c_float
        IndigoLib.lib.indigoSimilarity.argtypes = [c_int, c_int, c_char_p]
        IndigoLib.lib.indigoIterateSDF.restype = c_int
        IndigoLib.lib.indigoIterateSDF.argtypes = [c_int]
        IndigoLib.lib.indigoIterateRDF.restype = c_int
        IndigoLib.lib.indigoIterateRDF.argtypes = [c_int]
        IndigoLib.lib.indigoIterateSmiles.restype = c_int
        IndigoLib.lib.indigoIterateSmiles.argtypes = [c_int]
        IndigoLib.lib.indigoIterateCML.restype = c_int
        IndigoLib.lib.indigoIterateCML.argtypes = [c_int]
        IndigoLib.lib.indigoIterateCDX.restype = c_int
        IndigoLib.lib.indigoIterateCDX.argtypes = [c_int]
        IndigoLib.lib.indigoIterateSDFile.restype = c_int
        IndigoLib.lib.indigoIterateSDFile.argtypes = [c_char_p]
        IndigoLib.lib.indigoIterateRDFile.restype = c_int
        IndigoLib.lib.indigoIterateRDFile.argtypes = [c_char_p]
        IndigoLib.lib.indigoIterateSmilesFile.restype = c_int
        IndigoLib.lib.indigoIterateSmilesFile.argtypes = [c_char_p]
        IndigoLib.lib.indigoIterateCMLFile.restype = c_int
        IndigoLib.lib.indigoIterateCMLFile.argtypes = [c_char_p]
        IndigoLib.lib.indigoIterateCDXFile.restype = c_int
        IndigoLib.lib.indigoIterateCDXFile.argtypes = [c_char_p]
        IndigoLib.lib.indigoCreateSaver.restype = c_int
        IndigoLib.lib.indigoCreateSaver.argtypes = [c_int, c_char_p]
        IndigoLib.lib.indigoCreateFileSaver.restype = c_int
        IndigoLib.lib.indigoCreateFileSaver.argtypes = [c_char_p, c_char_p]
        IndigoLib.lib.indigoCreateArray.restype = c_int
        IndigoLib.lib.indigoCreateArray.argtypes = []
        IndigoLib.lib.indigoSubstructureMatcher.restype = c_int
        IndigoLib.lib.indigoSubstructureMatcher.argtypes = [c_int, c_char_p]
        IndigoLib.lib.indigoExtractCommonScaffold.restype = c_int
        IndigoLib.lib.indigoExtractCommonScaffold.argtypes = [c_int, c_char_p]
        IndigoLib.lib.indigoDecomposeMolecules.restype = c_int
        IndigoLib.lib.indigoDecomposeMolecules.argtypes = [c_int, c_int]
        IndigoLib.lib.indigoRGroupComposition.restype = c_int
        IndigoLib.lib.indigoRGroupComposition.argtypes = [c_int, c_char_p]
        IndigoLib.lib.indigoGetFragmentedMolecule.restype = c_int
        IndigoLib.lib.indigoGetFragmentedMolecule.argtypes = [c_int, c_char_p]
        IndigoLib.lib.indigoCreateDecomposer.restype = c_int
        IndigoLib.lib.indigoCreateDecomposer.argtypes = [c_int]
        IndigoLib.lib.indigoReactionProductEnumerate.restype = c_int
        IndigoLib.lib.indigoReactionProductEnumerate.argtypes = [c_int, c_int]
        IndigoLib.lib.indigoTransform.restype = c_int
        IndigoLib.lib.indigoTransform.argtypes = [c_int, c_int]
        IndigoLib.lib.indigoDbgBreakpoint.restype = None
        IndigoLib.lib.indigoDbgBreakpoint.argtypes = []
        IndigoLib.lib.indigoClone.restype = c_int
        IndigoLib.lib.indigoClone.argtypes = [c_int]

        IndigoLib.lib.indigoCheck.restype = c_char_p
        IndigoLib.lib.indigoCheck.argtypes = [c_char_p, c_char_p, c_char_p]
        IndigoLib.lib.indigoCheckObj.restype = c_char_p
        IndigoLib.lib.indigoCheckObj.argtypes = [c_int, c_char_p]

        IndigoLib.lib.indigoCheckStructure.restype = c_char_p
        IndigoLib.lib.indigoCheckStructure.argtypes = [c_char_p, c_char_p]
        IndigoLib.lib.indigoClose.restype = c_int
        IndigoLib.lib.indigoClose.argtypes = [c_int]
        IndigoLib.lib.indigoNext.restype = c_int
        IndigoLib.lib.indigoNext.argtypes = [c_int]
        IndigoLib.lib.indigoHasNext.restype = c_int
        IndigoLib.lib.indigoHasNext.argtypes = [c_int]
        IndigoLib.lib.indigoIndex.restype = c_int
        IndigoLib.lib.indigoIndex.argtypes = [c_int]
        IndigoLib.lib.indigoRemove.restype = c_int
        IndigoLib.lib.indigoRemove.argtypes = [c_int]
        IndigoLib.lib.indigoGetOriginalFormat.restype = c_char_p
        IndigoLib.lib.indigoGetOriginalFormat.argtypes = [c_int]
        IndigoLib.lib.indigoSaveMolfileToFile.restype = c_int
        IndigoLib.lib.indigoSaveMolfileToFile.argtypes = [c_int, c_char_p]
        IndigoLib.lib.indigoMolfile.restype = c_char_p
        IndigoLib.lib.indigoMolfile.argtypes = [c_int]
        IndigoLib.lib.indigoSaveCmlToFile.restype = c_int
        IndigoLib.lib.indigoSaveCmlToFile.argtypes = [c_int, c_char_p]
        IndigoLib.lib.indigoCml.restype = c_char_p
        IndigoLib.lib.indigoCml.argtypes = [c_int]
        IndigoLib.lib.indigoSaveCdxmlToFile.restype = c_int
        IndigoLib.lib.indigoSaveCdxmlToFile.argtypes = [c_int, c_char_p]
        IndigoLib.lib.indigoSaveCdxToFile.restype = c_int
        IndigoLib.lib.indigoSaveCdxToFile.argtypes = [c_int, c_char_p]
        IndigoLib.lib.indigoCdxml.restype = c_char_p
        IndigoLib.lib.indigoCdxml.argtypes = [c_int]
        IndigoLib.lib.indigoCdxBase64.restype = c_char_p
        IndigoLib.lib.indigoCdxBase64.argtypes = [c_int]
        IndigoLib.lib.indigoJson.restype = c_char_p
        IndigoLib.lib.indigoJson.argtypes = [c_int]
        IndigoLib.lib.indigoSaveMDLCT.restype = c_int
        IndigoLib.lib.indigoSaveMDLCT.argtypes = [c_int, c_int]
        IndigoLib.lib.indigoAddReactant.restype = c_int
        IndigoLib.lib.indigoAddReactant.argtypes = [c_int, c_int]
        IndigoLib.lib.indigoAddProduct.restype = c_int
        IndigoLib.lib.indigoAddProduct.argtypes = [c_int, c_int]
        IndigoLib.lib.indigoAddCatalyst.restype = c_int
        IndigoLib.lib.indigoAddCatalyst.argtypes = [c_int, c_int]
        IndigoLib.lib.indigoCountReactants.restype = c_int
        IndigoLib.lib.indigoCountReactants.argtypes = [c_int]
        IndigoLib.lib.indigoCountProducts.restype = c_int
        IndigoLib.lib.indigoCountProducts.argtypes = [c_int]
        IndigoLib.lib.indigoCountCatalysts.restype = c_int
        IndigoLib.lib.indigoCountCatalysts.argtypes = [c_int]
        IndigoLib.lib.indigoCountMolecules.restype = c_int
        IndigoLib.lib.indigoCountMolecules.argtypes = [c_int]
        IndigoLib.lib.indigoGetMolecule.restype = c_int
        IndigoLib.lib.indigoGetMolecule.argtypes = [c_int, c_int]
        IndigoLib.lib.indigoIterateReactants.restype = c_int
        IndigoLib.lib.indigoIterateReactants.argtypes = [c_int]
        IndigoLib.lib.indigoIterateProducts.restype = c_int
        IndigoLib.lib.indigoIterateProducts.argtypes = [c_int]
        IndigoLib.lib.indigoIterateCatalysts.restype = c_int
        IndigoLib.lib.indigoIterateCatalysts.argtypes = [c_int]
        IndigoLib.lib.indigoIterateMolecules.restype = c_int
        IndigoLib.lib.indigoIterateMolecules.argtypes = [c_int]
        IndigoLib.lib.indigoSaveRxnfileToFile.restype = c_int
        IndigoLib.lib.indigoSaveRxnfileToFile.argtypes = [c_int, c_char_p]
        IndigoLib.lib.indigoRxnfile.restype = c_char_p
        IndigoLib.lib.indigoRxnfile.argtypes = [c_int]
        IndigoLib.lib.indigoOptimize.restype = c_int
        IndigoLib.lib.indigoOptimize.argtypes = [c_int, c_char_p]
        IndigoLib.lib.indigoNormalize.restype = c_int
        IndigoLib.lib.indigoNormalize.argtypes = [c_int, c_char_p]
        IndigoLib.lib.indigoStandardize.restype = c_int
        IndigoLib.lib.indigoStandardize.argtypes = [c_int]
        IndigoLib.lib.indigoIonize.restype = c_int
        IndigoLib.lib.indigoIonize.argtypes = [c_int, c_float, c_float]
        IndigoLib.lib.indigoBuildPkaModel.restype = c_int
        IndigoLib.lib.indigoBuildPkaModel.argtypes = [c_int, c_float, c_char_p]
        IndigoLib.lib.indigoGetAcidPkaValue.restype = POINTER(c_float)
        IndigoLib.lib.indigoGetAcidPkaValue.argtypes = [
            c_int,
            c_int,
            c_int,
            c_int,
        ]
        IndigoLib.lib.indigoGetBasicPkaValue.restype = POINTER(c_float)
        IndigoLib.lib.indigoGetBasicPkaValue.argtypes = [
            c_int,
            c_int,
            c_int,
            c_int,
        ]
        IndigoLib.lib.indigoAutomap.restype = c_int
        IndigoLib.lib.indigoAutomap.argtypes = [c_int, c_char_p]
        IndigoLib.lib.indigoGetAtomMappingNumber.restype = c_int
        IndigoLib.lib.indigoGetAtomMappingNumber.argtypes = [c_int, c_int]
        IndigoLib.lib.indigoSetAtomMappingNumber.restype = c_int
        IndigoLib.lib.indigoSetAtomMappingNumber.argtypes = [
            c_int,
            c_int,
            c_int,
        ]
        IndigoLib.lib.indigoGetReactingCenter.restype = c_int
        IndigoLib.lib.indigoGetReactingCenter.argtypes = [
            c_int,
            c_int,
            POINTER(c_int),
        ]
        IndigoLib.lib.indigoSetReactingCenter.restype = c_int
        IndigoLib.lib.indigoSetReactingCenter.argtypes = [c_int, c_int, c_int]
        IndigoLib.lib.indigoClearAAM.restype = c_int
        IndigoLib.lib.indigoClearAAM.argtypes = [c_int]
        IndigoLib.lib.indigoCorrectReactingCenters.restype = c_int
        IndigoLib.lib.indigoCorrectReactingCenters.argtypes = [c_int]
        IndigoLib.lib.indigoIterateAtoms.restype = c_int
        IndigoLib.lib.indigoIterateAtoms.argtypes = [c_int]
        IndigoLib.lib.indigoIteratePseudoatoms.restype = c_int
        IndigoLib.lib.indigoIteratePseudoatoms.argtypes = [c_int]
        IndigoLib.lib.indigoIterateRSites.restype = c_int
        IndigoLib.lib.indigoIterateRSites.argtypes = [c_int]
        IndigoLib.lib.indigoIterateStereocenters.restype = c_int
        IndigoLib.lib.indigoIterateStereocenters.argtypes = [c_int]
        IndigoLib.lib.indigoIterateAlleneCenters.restype = c_int
        IndigoLib.lib.indigoIterateAlleneCenters.argtypes = [c_int]
        IndigoLib.lib.indigoIterateRGroups.restype = c_int
        IndigoLib.lib.indigoIterateRGroups.argtypes = [c_int]
        IndigoLib.lib.indigoCountRGroups.restype = c_int
        IndigoLib.lib.indigoCountRGroups.argtypes = [c_int]
        IndigoLib.lib.indigoCopyRGroups.restype = c_int
        IndigoLib.lib.indigoCopyRGroups.argtypes = [c_int, c_int]
        IndigoLib.lib.indigoIsPseudoatom.restype = c_int
        IndigoLib.lib.indigoIsPseudoatom.argtypes = [c_int]
        IndigoLib.lib.indigoIsRSite.restype = c_int
        IndigoLib.lib.indigoIsRSite.argtypes = [c_int]
        IndigoLib.lib.indigoIsTemplateAtom.restype = c_int
        IndigoLib.lib.indigoIsTemplateAtom.argtypes = [c_int]
        IndigoLib.lib.indigoStereocenterType.restype = c_int
        IndigoLib.lib.indigoStereocenterType.argtypes = [c_int]
        IndigoLib.lib.indigoStereocenterGroup.restype = c_int
        IndigoLib.lib.indigoStereocenterGroup.argtypes = [c_int]
        IndigoLib.lib.indigoSetStereocenterGroup.restype = c_int
        IndigoLib.lib.indigoSetStereocenterGroup.argtypes = [c_int, c_int]
        IndigoLib.lib.indigoChangeStereocenterType.restype = c_int
        IndigoLib.lib.indigoChangeStereocenterType.argtypes = [c_int, c_int]
        IndigoLib.lib.indigoValidateChirality.restype = c_int
        IndigoLib.lib.indigoValidateChirality.argtypes = [c_int]
        IndigoLib.lib.indigoSingleAllowedRGroup.restype = c_int
        IndigoLib.lib.indigoSingleAllowedRGroup.argtypes = [c_int]
        IndigoLib.lib.indigoAddStereocenter.restype = c_int
        IndigoLib.lib.indigoAddStereocenter.argtypes = [
            c_int,
            c_int,
            c_int,
            c_int,
            c_int,
            c_int,
        ]
        IndigoLib.lib.indigoIterateRGroupFragments.restype = c_int
        IndigoLib.lib.indigoIterateRGroupFragments.argtypes = [c_int]
        IndigoLib.lib.indigoCountAttachmentPoints.restype = c_int
        IndigoLib.lib.indigoCountAttachmentPoints.argtypes = [c_int]
        IndigoLib.lib.indigoIterateAttachmentPoints.restype = c_int
        IndigoLib.lib.indigoIterateAttachmentPoints.argtypes = [c_int, c_int]
        IndigoLib.lib.indigoSymbol.restype = c_char_p
        IndigoLib.lib.indigoSymbol.argtypes = [c_int]
        IndigoLib.lib.indigoDegree.restype = c_int
        IndigoLib.lib.indigoDegree.argtypes = [c_int]
        IndigoLib.lib.indigoGetCharge.restype = c_int
        IndigoLib.lib.indigoGetCharge.argtypes = [c_int, POINTER(c_int)]
        IndigoLib.lib.indigoGetExplicitValence.restype = c_int
        IndigoLib.lib.indigoGetExplicitValence.argtypes = [
            c_int,
            POINTER(c_int),
        ]
        IndigoLib.lib.indigoSetExplicitValence.restype = c_int
        IndigoLib.lib.indigoSetExplicitValence.argtypes = [c_int, c_int]
        IndigoLib.lib.indigoGetRadicalElectrons.restype = c_int
        IndigoLib.lib.indigoGetRadicalElectrons.argtypes = [
            c_int,
            POINTER(c_int),
        ]
        IndigoLib.lib.indigoGetRadical.restype = c_int
        IndigoLib.lib.indigoGetRadical.argtypes = [c_int, POINTER(c_int)]
        IndigoLib.lib.indigoSetRadical.restype = c_int
        IndigoLib.lib.indigoSetRadical.argtypes = [c_int, c_int]
        IndigoLib.lib.indigoAtomicNumber.restype = c_int
        IndigoLib.lib.indigoAtomicNumber.argtypes = [c_int]
        IndigoLib.lib.indigoIsotope.restype = c_int
        IndigoLib.lib.indigoIsotope.argtypes = [c_int]
        IndigoLib.lib.indigoValence.restype = c_int
        IndigoLib.lib.indigoValence.argtypes = [c_int]
        IndigoLib.lib.indigoGetHybridization.restype = c_int
        IndigoLib.lib.indigoGetHybridization.argtypes = [c_int]
        IndigoLib.lib.indigoCheckValence.restype = c_int
        IndigoLib.lib.indigoCheckValence.argtypes = [c_int]
        IndigoLib.lib.indigoCheckQuery.restype = c_int
        IndigoLib.lib.indigoCheckQuery.argtypes = [c_int]
        IndigoLib.lib.indigoCheckRGroups.restype = c_int
        IndigoLib.lib.indigoCheckRGroups.argtypes = [c_int]
        IndigoLib.lib.indigoCountHydrogens.restype = c_int
        IndigoLib.lib.indigoCountHydrogens.argtypes = [c_int, POINTER(c_int)]
        IndigoLib.lib.indigoCountImplicitHydrogens.restype = c_int
        IndigoLib.lib.indigoCountImplicitHydrogens.argtypes = [c_int]
        IndigoLib.lib.indigoXYZ.restype = POINTER(c_float)
        IndigoLib.lib.indigoXYZ.argtypes = [c_int]
        IndigoLib.lib.indigoSetXYZ.restype = c_int
        IndigoLib.lib.indigoSetXYZ.argtypes = [
            c_int,
            c_float,
            c_float,
            c_float,
        ]
        IndigoLib.lib.indigoCountSuperatoms.restype = c_int
        IndigoLib.lib.indigoCountSuperatoms.argtypes = [c_int]
        IndigoLib.lib.indigoCountDataSGroups.restype = c_int
        IndigoLib.lib.indigoCountDataSGroups.argtypes = [c_int]
        IndigoLib.lib.indigoCountRepeatingUnits.restype = c_int
        IndigoLib.lib.indigoCountRepeatingUnits.argtypes = [c_int]
        IndigoLib.lib.indigoCountMultipleGroups.restype = c_int
        IndigoLib.lib.indigoCountMultipleGroups.argtypes = [c_int]
        IndigoLib.lib.indigoCountGenericSGroups.restype = c_int
        IndigoLib.lib.indigoCountGenericSGroups.argtypes = [c_int]
        IndigoLib.lib.indigoIterateDataSGroups.restype = c_int
        IndigoLib.lib.indigoIterateDataSGroups.argtypes = [c_int]
        IndigoLib.lib.indigoIterateSuperatoms.restype = c_int
        IndigoLib.lib.indigoIterateSuperatoms.argtypes = [c_int]
        IndigoLib.lib.indigoIterateGenericSGroups.restype = c_int
        IndigoLib.lib.indigoIterateGenericSGroups.argtypes = [c_int]
        IndigoLib.lib.indigoIterateRepeatingUnits.restype = c_int
        IndigoLib.lib.indigoIterateRepeatingUnits.argtypes = [c_int]
        IndigoLib.lib.indigoIterateMultipleGroups.restype = c_int
        IndigoLib.lib.indigoIterateMultipleGroups.argtypes = [c_int]
        IndigoLib.lib.indigoIterateSGroups.restype = c_int
        IndigoLib.lib.indigoIterateSGroups.argtypes = [c_int]
        IndigoLib.lib.indigoIterateTGroups.restype = c_int
        IndigoLib.lib.indigoIterateTGroups.argtypes = [c_int]
        IndigoLib.lib.indigoGetSuperatom.restype = c_int
        IndigoLib.lib.indigoGetSuperatom.argtypes = [c_int, c_int]
        IndigoLib.lib.indigoGetDataSGroup.restype = c_int
        IndigoLib.lib.indigoGetDataSGroup.argtypes = [c_int, c_int]
        IndigoLib.lib.indigoGetGenericSGroup.restype = c_int
        IndigoLib.lib.indigoGetGenericSGroup.argtypes = [c_int, c_int]
        IndigoLib.lib.indigoGetMultipleGroup.restype = c_int
        IndigoLib.lib.indigoGetMultipleGroup.argtypes = [c_int, c_int]
        IndigoLib.lib.indigoGetRepeatingUnit.restype = c_int
        IndigoLib.lib.indigoGetRepeatingUnit.argtypes = [c_int, c_int]
        IndigoLib.lib.indigoDescription.restype = c_char_p
        IndigoLib.lib.indigoDescription.argtypes = [c_int]
        IndigoLib.lib.indigoData.restype = c_char_p
        IndigoLib.lib.indigoData.argtypes = [c_int]
        IndigoLib.lib.indigoAddDataSGroup.restype = c_int
        IndigoLib.lib.indigoAddDataSGroup.argtypes = [
            c_int,
            c_int,
            POINTER(c_int),
            c_int,
            POINTER(c_int),
            c_char_p,
            c_char_p,
        ]
        IndigoLib.lib.indigoAddSuperatom.restype = c_int
        IndigoLib.lib.indigoAddSuperatom.argtypes = [
            c_int,
            c_int,
            POINTER(c_int),
            c_char_p,
        ]
        IndigoLib.lib.indigoSetDataSGroupXY.restype = c_int
        IndigoLib.lib.indigoSetDataSGroupXY.argtypes = [
            c_int,
            c_float,
            c_float,
            c_char_p,
        ]
        IndigoLib.lib.indigoSetSGroupData.restype = c_int
        IndigoLib.lib.indigoSetSGroupData.argtypes = [c_int, c_char_p]
        IndigoLib.lib.indigoSetSGroupCoords.restype = c_int
        IndigoLib.lib.indigoSetSGroupCoords.argtypes = [
            c_int,
            c_float,
            c_float,
        ]
        IndigoLib.lib.indigoSetSGroupDescription.restype = c_int
        IndigoLib.lib.indigoSetSGroupDescription.argtypes = [c_int, c_char_p]
        IndigoLib.lib.indigoSetSGroupFieldName.restype = c_int
        IndigoLib.lib.indigoSetSGroupFieldName.argtypes = [c_int, c_char_p]
        IndigoLib.lib.indigoSetSGroupQueryCode.restype = c_int
        IndigoLib.lib.indigoSetSGroupQueryCode.argtypes = [c_int, c_char_p]
        IndigoLib.lib.indigoSetSGroupQueryOper.restype = c_int
        IndigoLib.lib.indigoSetSGroupQueryOper.argtypes = [c_int, c_char_p]
        IndigoLib.lib.indigoSetSGroupDisplay.restype = c_int
        IndigoLib.lib.indigoSetSGroupDisplay.argtypes = [c_int, c_char_p]
        IndigoLib.lib.indigoSetSGroupLocation.restype = c_int
        IndigoLib.lib.indigoSetSGroupLocation.argtypes = [c_int, c_char_p]
        IndigoLib.lib.indigoSetSGroupTag.restype = c_int
        IndigoLib.lib.indigoSetSGroupTag.argtypes = [c_int, c_char_p]
        IndigoLib.lib.indigoSetSGroupTagAlign.restype = c_int
        IndigoLib.lib.indigoSetSGroupTagAlign.argtypes = [c_int, c_int]
        IndigoLib.lib.indigoSetSGroupDataType.restype = c_int
        IndigoLib.lib.indigoSetSGroupDataType.argtypes = [c_int, c_char_p]
        IndigoLib.lib.indigoSetSGroupXCoord.restype = c_int
        IndigoLib.lib.indigoSetSGroupXCoord.argtypes = [c_int, c_float]
        IndigoLib.lib.indigoSetSGroupYCoord.restype = c_int
        IndigoLib.lib.indigoSetSGroupYCoord.argtypes = [c_int, c_float]
        IndigoLib.lib.indigoCreateSGroup.restype = c_int
        IndigoLib.lib.indigoCreateSGroup.argtypes = [c_char_p, c_int, c_char_p]
        IndigoLib.lib.indigoSetSGroupClass.restype = c_int
        IndigoLib.lib.indigoSetSGroupClass.argtypes = [c_int, c_char_p]
        IndigoLib.lib.indigoSetSGroupName.restype = c_int
        IndigoLib.lib.indigoSetSGroupName.argtypes = [c_int, c_char_p]
        IndigoLib.lib.indigoGetSGroupClass.restype = c_char_p
        IndigoLib.lib.indigoGetSGroupClass.argtypes = [c_int]
        IndigoLib.lib.indigoGetSGroupName.restype = c_char_p
        IndigoLib.lib.indigoGetSGroupName.argtypes = [c_int]
        IndigoLib.lib.indigoGetSGroupNumCrossBonds.restype = c_int
        IndigoLib.lib.indigoGetSGroupNumCrossBonds.argtypes = [c_int]
        IndigoLib.lib.indigoAddSGroupAttachmentPoint.restype = c_int
        IndigoLib.lib.indigoAddSGroupAttachmentPoint.argtypes = [
            c_int,
            c_int,
            c_int,
            c_char_p,
        ]
        IndigoLib.lib.indigoDeleteSGroupAttachmentPoint.restype = c_int
        IndigoLib.lib.indigoDeleteSGroupAttachmentPoint.argtypes = [
            c_int,
            c_int,
        ]
        IndigoLib.lib.indigoGetSGroupDisplayOption.restype = c_int
        IndigoLib.lib.indigoGetSGroupDisplayOption.argtypes = [c_int]
        IndigoLib.lib.indigoSetSGroupDisplayOption.restype = c_int
        IndigoLib.lib.indigoSetSGroupDisplayOption.argtypes = [c_int, c_int]
        IndigoLib.lib.indigoGetSGroupSeqId.restype = c_int
        IndigoLib.lib.indigoGetSGroupSeqId.argtypes = [c_int]
        IndigoLib.lib.indigoGetSGroupCoords.restype = POINTER(c_float)
        IndigoLib.lib.indigoGetSGroupCoords.argtypes = [c_int]
        IndigoLib.lib.indigoGetRepeatingUnitSubscript.restype = c_char_p
        IndigoLib.lib.indigoGetRepeatingUnitSubscript.argtypes = [c_int]
        IndigoLib.lib.indigoGetRepeatingUnitConnectivity.restype = c_int
        IndigoLib.lib.indigoGetRepeatingUnitConnectivity.argtypes = [c_int]
        IndigoLib.lib.indigoGetSGroupMultiplier.restype = c_int
        IndigoLib.lib.indigoGetSGroupMultiplier.argtypes = [c_int]
        IndigoLib.lib.indigoSetSGroupMultiplier.restype = c_int
        IndigoLib.lib.indigoSetSGroupMultiplier.argtypes = [c_int, c_int]
        IndigoLib.lib.indigoSetSGroupBrackets.restype = c_int
        IndigoLib.lib.indigoSetSGroupBrackets.argtypes = [
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
        IndigoLib.lib.indigoFindSGroups.restype = c_int
        IndigoLib.lib.indigoFindSGroups.argtypes = [c_int, c_char_p, c_char_p]
        IndigoLib.lib.indigoGetSGroupType.restype = c_int
        IndigoLib.lib.indigoGetSGroupType.argtypes = [c_int]
        IndigoLib.lib.indigoGetSGroupIndex.restype = c_int
        IndigoLib.lib.indigoGetSGroupIndex.argtypes = [c_int]
        IndigoLib.lib.indigoGetSGroupOriginalId.restype = c_int
        IndigoLib.lib.indigoGetSGroupOriginalId.argtypes = [c_int]
        IndigoLib.lib.indigoSetSGroupOriginalId.restype = c_int
        IndigoLib.lib.indigoSetSGroupOriginalId.argtypes = [c_int, c_int]
        IndigoLib.lib.indigoGetSGroupParentId.restype = c_int
        IndigoLib.lib.indigoGetSGroupParentId.argtypes = [c_int]
        IndigoLib.lib.indigoSetSGroupParentId.restype = c_int
        IndigoLib.lib.indigoSetSGroupParentId.argtypes = [c_int, c_int]
        IndigoLib.lib.indigoAddTemplate.restype = c_int
        IndigoLib.lib.indigoAddTemplate.argtypes = [c_int, c_int, c_char_p]
        IndigoLib.lib.indigoRemoveTemplate.restype = c_int
        IndigoLib.lib.indigoRemoveTemplate.argtypes = [c_int, c_char_p]
        IndigoLib.lib.indigoFindTemplate.restype = c_int
        IndigoLib.lib.indigoFindTemplate.argtypes = [c_int, c_char_p]
        IndigoLib.lib.indigoGetTGroupClass.restype = c_char_p
        IndigoLib.lib.indigoGetTGroupClass.argtypes = [c_int]
        IndigoLib.lib.indigoGetTGroupName.restype = c_char_p
        IndigoLib.lib.indigoGetTGroupName.argtypes = [c_int]
        IndigoLib.lib.indigoGetTGroupAlias.restype = c_char_p
        IndigoLib.lib.indigoGetTGroupAlias.argtypes = [c_int]
        IndigoLib.lib.indigoTransformSCSRtoCTAB.restype = c_int
        IndigoLib.lib.indigoTransformSCSRtoCTAB.argtypes = [c_int]
        IndigoLib.lib.indigoTransformCTABtoSCSR.restype = c_int
        IndigoLib.lib.indigoTransformCTABtoSCSR.argtypes = [c_int, c_int]
        IndigoLib.lib.indigoTransformHELMtoSCSR.restype = c_int
        IndigoLib.lib.indigoTransformHELMtoSCSR.argtypes = [c_int]
        IndigoLib.lib.indigoGetTemplateAtomClass.restype = c_char_p
        IndigoLib.lib.indigoGetTemplateAtomClass.argtypes = [c_int]
        IndigoLib.lib.indigoSetTemplateAtomClass.restype = c_int
        IndigoLib.lib.indigoSetTemplateAtomClass.argtypes = [c_int, c_char_p]
        IndigoLib.lib.indigoResetCharge.restype = c_int
        IndigoLib.lib.indigoResetCharge.argtypes = [c_int]
        IndigoLib.lib.indigoResetExplicitValence.restype = c_int
        IndigoLib.lib.indigoResetExplicitValence.argtypes = [c_int]
        IndigoLib.lib.indigoResetRadical.restype = c_int
        IndigoLib.lib.indigoResetRadical.argtypes = [c_int]
        IndigoLib.lib.indigoResetIsotope.restype = c_int
        IndigoLib.lib.indigoResetIsotope.argtypes = [c_int]
        IndigoLib.lib.indigoSetAttachmentPoint.restype = c_int
        IndigoLib.lib.indigoSetAttachmentPoint.argtypes = [c_int, c_int]
        IndigoLib.lib.indigoClearAttachmentPoints.restype = c_int
        IndigoLib.lib.indigoClearAttachmentPoints.argtypes = [c_int]
        IndigoLib.lib.indigoRemoveConstraints.restype = c_int
        IndigoLib.lib.indigoRemoveConstraints.argtypes = [c_int, c_char_p]
        IndigoLib.lib.indigoAddConstraint.restype = c_int
        IndigoLib.lib.indigoAddConstraint.argtypes = [
            c_int,
            c_char_p,
            c_char_p,
        ]
        IndigoLib.lib.indigoAddConstraintNot.restype = c_int
        IndigoLib.lib.indigoAddConstraintNot.argtypes = [
            c_int,
            c_char_p,
            c_char_p,
        ]
        IndigoLib.lib.indigoAddConstraintOr.restype = c_int
        IndigoLib.lib.indigoAddConstraintOr.argtypes = [
            c_int,
            c_char_p,
            c_char_p,
        ]
        IndigoLib.lib.indigoResetStereo.restype = c_int
        IndigoLib.lib.indigoResetStereo.argtypes = [c_int]
        IndigoLib.lib.indigoInvertStereo.restype = c_int
        IndigoLib.lib.indigoInvertStereo.argtypes = [c_int]
        IndigoLib.lib.indigoCountAtoms.restype = c_int
        IndigoLib.lib.indigoCountAtoms.argtypes = [c_int]
        IndigoLib.lib.indigoCountBonds.restype = c_int
        IndigoLib.lib.indigoCountBonds.argtypes = [c_int]
        IndigoLib.lib.indigoCountPseudoatoms.restype = c_int
        IndigoLib.lib.indigoCountPseudoatoms.argtypes = [c_int]
        IndigoLib.lib.indigoCountRSites.restype = c_int
        IndigoLib.lib.indigoCountRSites.argtypes = [c_int]
        IndigoLib.lib.indigoIterateBonds.restype = c_int
        IndigoLib.lib.indigoIterateBonds.argtypes = [c_int]
        IndigoLib.lib.indigoBondOrder.restype = c_int
        IndigoLib.lib.indigoBondOrder.argtypes = [c_int]
        IndigoLib.lib.indigoBondStereo.restype = c_int
        IndigoLib.lib.indigoBondStereo.argtypes = [c_int]
        IndigoLib.lib.indigoTopology.restype = c_int
        IndigoLib.lib.indigoTopology.argtypes = [c_int]
        IndigoLib.lib.indigoIterateNeighbors.restype = c_int
        IndigoLib.lib.indigoIterateNeighbors.argtypes = [c_int]
        IndigoLib.lib.indigoBond.restype = c_int
        IndigoLib.lib.indigoBond.argtypes = [c_int]
        IndigoLib.lib.indigoGetAtom.restype = c_int
        IndigoLib.lib.indigoGetAtom.argtypes = [c_int, c_int]
        IndigoLib.lib.indigoGetBond.restype = c_int
        IndigoLib.lib.indigoGetBond.argtypes = [c_int, c_int]
        IndigoLib.lib.indigoSource.restype = c_int
        IndigoLib.lib.indigoSource.argtypes = [c_int]
        IndigoLib.lib.indigoDestination.restype = c_int
        IndigoLib.lib.indigoDestination.argtypes = [c_int]
        IndigoLib.lib.indigoClearCisTrans.restype = c_int
        IndigoLib.lib.indigoClearCisTrans.argtypes = [c_int]
        IndigoLib.lib.indigoClearStereocenters.restype = c_int
        IndigoLib.lib.indigoClearStereocenters.argtypes = [c_int]
        IndigoLib.lib.indigoCountStereocenters.restype = c_int
        IndigoLib.lib.indigoCountStereocenters.argtypes = [c_int]
        IndigoLib.lib.indigoClearAlleneCenters.restype = c_int
        IndigoLib.lib.indigoClearAlleneCenters.argtypes = [c_int]
        IndigoLib.lib.indigoCountAlleneCenters.restype = c_int
        IndigoLib.lib.indigoCountAlleneCenters.argtypes = [c_int]
        IndigoLib.lib.indigoResetSymmetricCisTrans.restype = c_int
        IndigoLib.lib.indigoResetSymmetricCisTrans.argtypes = [c_int]
        IndigoLib.lib.indigoResetSymmetricStereocenters.restype = c_int
        IndigoLib.lib.indigoResetSymmetricStereocenters.argtypes = [c_int]
        IndigoLib.lib.indigoMarkEitherCisTrans.restype = c_int
        IndigoLib.lib.indigoMarkEitherCisTrans.argtypes = [c_int]
        IndigoLib.lib.indigoMarkStereobonds.restype = c_int
        IndigoLib.lib.indigoMarkStereobonds.argtypes = [c_int]
        IndigoLib.lib.indigoAddAtom.restype = c_int
        IndigoLib.lib.indigoAddAtom.argtypes = [c_int, c_char_p]
        IndigoLib.lib.indigoResetAtom.restype = c_int
        IndigoLib.lib.indigoResetAtom.argtypes = [c_int, c_char_p]
        IndigoLib.lib.indigoAddRSite.restype = c_int
        IndigoLib.lib.indigoAddRSite.argtypes = [c_int, c_char_p]
        IndigoLib.lib.indigoSetRSite.restype = c_int
        IndigoLib.lib.indigoSetRSite.argtypes = [c_int, c_char_p]
        IndigoLib.lib.indigoSetCharge.restype = c_int
        IndigoLib.lib.indigoSetCharge.argtypes = [c_int, c_int]
        IndigoLib.lib.indigoSetIsotope.restype = c_int
        IndigoLib.lib.indigoSetIsotope.argtypes = [c_int, c_int]
        IndigoLib.lib.indigoSetImplicitHCount.restype = c_int
        IndigoLib.lib.indigoSetImplicitHCount.argtypes = [c_int, c_int]
        IndigoLib.lib.indigoAddBond.restype = c_int
        IndigoLib.lib.indigoAddBond.argtypes = [c_int, c_int, c_int]
        IndigoLib.lib.indigoSetBondOrder.restype = c_int
        IndigoLib.lib.indigoSetBondOrder.argtypes = [c_int, c_int]
        IndigoLib.lib.indigoMerge.restype = c_int
        IndigoLib.lib.indigoMerge.argtypes = [c_int, c_int]
        IndigoLib.lib.indigoHighlight.restype = c_int
        IndigoLib.lib.indigoHighlight.argtypes = [c_int]
        IndigoLib.lib.indigoUnhighlight.restype = c_int
        IndigoLib.lib.indigoUnhighlight.argtypes = [c_int]
        IndigoLib.lib.indigoIsHighlighted.restype = c_int
        IndigoLib.lib.indigoIsHighlighted.argtypes = [c_int]
        IndigoLib.lib.indigoCountComponents.restype = c_int
        IndigoLib.lib.indigoCountComponents.argtypes = [c_int]
        IndigoLib.lib.indigoComponentIndex.restype = c_int
        IndigoLib.lib.indigoComponentIndex.argtypes = [c_int]
        IndigoLib.lib.indigoIterateComponents.restype = c_int
        IndigoLib.lib.indigoIterateComponents.argtypes = [c_int]
        IndigoLib.lib.indigoComponent.restype = c_int
        IndigoLib.lib.indigoComponent.argtypes = [c_int, c_int]
        IndigoLib.lib.indigoCountSSSR.restype = c_int
        IndigoLib.lib.indigoCountSSSR.argtypes = [c_int]
        IndigoLib.lib.indigoIterateSSSR.restype = c_int
        IndigoLib.lib.indigoIterateSSSR.argtypes = [c_int]
        IndigoLib.lib.indigoIterateSubtrees.restype = c_int
        IndigoLib.lib.indigoIterateSubtrees.argtypes = [c_int, c_int, c_int]
        IndigoLib.lib.indigoIterateRings.restype = c_int
        IndigoLib.lib.indigoIterateRings.argtypes = [c_int, c_int, c_int]
        IndigoLib.lib.indigoIterateEdgeSubmolecules.restype = c_int
        IndigoLib.lib.indigoIterateEdgeSubmolecules.argtypes = [
            c_int,
            c_int,
            c_int,
        ]
        IndigoLib.lib.indigoCountHeavyAtoms.restype = c_int
        IndigoLib.lib.indigoCountHeavyAtoms.argtypes = [c_int]
        IndigoLib.lib.indigoGrossFormula.restype = c_int
        IndigoLib.lib.indigoGrossFormula.argtypes = [c_int]
        IndigoLib.lib.indigoMolecularWeight.restype = c_double
        IndigoLib.lib.indigoMolecularWeight.argtypes = [c_int]
        IndigoLib.lib.indigoMostAbundantMass.restype = c_double
        IndigoLib.lib.indigoMostAbundantMass.argtypes = [c_int]
        IndigoLib.lib.indigoMonoisotopicMass.restype = c_double
        IndigoLib.lib.indigoMonoisotopicMass.argtypes = [c_int]
        IndigoLib.lib.indigoMassComposition.restype = c_char_p
        IndigoLib.lib.indigoMassComposition.argtypes = [c_int]
        IndigoLib.lib.indigoTPSA.restype = c_double
        IndigoLib.lib.indigoTPSA.argtypes = [c_int, c_int]
        IndigoLib.lib.indigoNumRotatableBonds.restype = c_int
        IndigoLib.lib.indigoNumRotatableBonds.argtypes = [c_int]
        IndigoLib.lib.indigoNumHydrogenBondAcceptors.restype = c_int
        IndigoLib.lib.indigoNumHydrogenBondAcceptors.argtypes = [c_int]
        IndigoLib.lib.indigoNumHydrogenBondDonors.restype = c_int
        IndigoLib.lib.indigoNumHydrogenBondDonors.argtypes = [c_int]
        IndigoLib.lib.indigoLogP.restype = c_double
        IndigoLib.lib.indigoLogP.argtypes = [c_int]
        IndigoLib.lib.indigoMolarRefractivity.restype = c_double
        IndigoLib.lib.indigoMolarRefractivity.argtypes = [c_int]
        IndigoLib.lib.indigoPka.restype = c_double
        IndigoLib.lib.indigoPka.argtypes = [c_int]
        IndigoLib.lib.indigoCanonicalSmiles.restype = c_char_p
        IndigoLib.lib.indigoCanonicalSmiles.argtypes = [c_int]
        IndigoLib.lib.indigoCanonicalSmarts.restype = c_char_p
        IndigoLib.lib.indigoCanonicalSmarts.argtypes = [c_int]
        IndigoLib.lib.indigoHash.restype = c_int64
        IndigoLib.lib.indigoHash.argtypes = [c_int]
        IndigoLib.lib.indigoLayeredCode.restype = c_char_p
        IndigoLib.lib.indigoLayeredCode.argtypes = [c_int]
        IndigoLib.lib.indigoSymmetryClasses.restype = POINTER(c_int)
        IndigoLib.lib.indigoSymmetryClasses.argtypes = [c_int, POINTER(c_int)]
        IndigoLib.lib.indigoHasCoord.restype = c_int
        IndigoLib.lib.indigoHasCoord.argtypes = [c_int]
        IndigoLib.lib.indigoHasZCoord.restype = c_int
        IndigoLib.lib.indigoHasZCoord.argtypes = [c_int]
        IndigoLib.lib.indigoIsChiral.restype = c_int
        IndigoLib.lib.indigoIsChiral.argtypes = [c_int]
        IndigoLib.lib.indigoIsPossibleFischerProjection.restype = c_int
        IndigoLib.lib.indigoIsPossibleFischerProjection.argtypes = [
            c_int,
            c_char_p,
        ]
        IndigoLib.lib.indigoCreateSubmolecule.restype = c_int
        IndigoLib.lib.indigoCreateSubmolecule.argtypes = [
            c_int,
            c_int,
            POINTER(c_int),
        ]
        IndigoLib.lib.indigoCreateEdgeSubmolecule.restype = c_int
        IndigoLib.lib.indigoCreateEdgeSubmolecule.argtypes = [
            c_int,
            c_int,
            POINTER(c_int),
            c_int,
            POINTER(c_int),
        ]
        IndigoLib.lib.indigoGetSubmolecule.restype = c_int
        IndigoLib.lib.indigoGetSubmolecule.argtypes = [
            c_int,
            c_int,
            POINTER(c_int),
        ]
        IndigoLib.lib.indigoRemoveAtoms.restype = c_int
        IndigoLib.lib.indigoRemoveAtoms.argtypes = [
            c_int,
            c_int,
            POINTER(c_int),
        ]
        IndigoLib.lib.indigoRemoveBonds.restype = c_int
        IndigoLib.lib.indigoRemoveBonds.argtypes = [
            c_int,
            c_int,
            POINTER(c_int),
        ]
        IndigoLib.lib.indigoAlignAtoms.restype = c_float
        IndigoLib.lib.indigoAlignAtoms.argtypes = [
            c_int,
            c_int,
            POINTER(c_int),
            POINTER(c_float),
        ]
        IndigoLib.lib.indigoAromatize.restype = c_int
        IndigoLib.lib.indigoAromatize.argtypes = [c_int]
        IndigoLib.lib.indigoDearomatize.restype = c_int
        IndigoLib.lib.indigoDearomatize.argtypes = [c_int]
        IndigoLib.lib.indigoFoldHydrogens.restype = c_int
        IndigoLib.lib.indigoFoldHydrogens.argtypes = [c_int]
        IndigoLib.lib.indigoUnfoldHydrogens.restype = c_int
        IndigoLib.lib.indigoUnfoldHydrogens.argtypes = [c_int]
        IndigoLib.lib.indigoFoldUnfoldHydrogens.restype = c_int
        IndigoLib.lib.indigoFoldUnfoldHydrogens.argtypes = [c_int]
        IndigoLib.lib.indigoClearXYZ.restype = c_int
        IndigoLib.lib.indigoClearXYZ.argtypes = [c_int]
        IndigoLib.lib.indigoLayout.restype = c_int
        IndigoLib.lib.indigoLayout.argtypes = [c_int]
        IndigoLib.lib.indigoClean2d.restype = c_int
        IndigoLib.lib.indigoClean2d.argtypes = [c_int]
        IndigoLib.lib.indigoSmiles.restype = c_char_p
        IndigoLib.lib.indigoSmiles.argtypes = [c_int]
        IndigoLib.lib.indigoSequence.restype = c_char_p
        IndigoLib.lib.indigoSequence.argtypes = [c_int, c_int]
        IndigoLib.lib.indigoSaveSequenceToFile.restype = c_int
        IndigoLib.lib.indigoSaveSequenceToFile.argtypes = [
            c_int,
            c_char_p,
            c_int,
        ]
        IndigoLib.lib.indigoFasta.restype = c_char_p
        IndigoLib.lib.indigoFasta.argtypes = [c_int, c_int]
        IndigoLib.lib.indigoSaveFastaToFile.restype = c_int
        IndigoLib.lib.indigoSaveFastaToFile.argtypes = [c_int, c_char_p, c_int]
        IndigoLib.lib.indigoIdt.restype = c_char_p
        IndigoLib.lib.indigoIdt.argtypes = [c_int, c_int]
        IndigoLib.lib.indigoSaveIdtToFile.restype = c_int
        IndigoLib.lib.indigoSaveIdtToFile.argtypes = [c_int, c_char_p, c_int]
        IndigoLib.lib.indigoHelm.restype = c_char_p
        IndigoLib.lib.indigoHelm.argtypes = [c_int, c_int]
        IndigoLib.lib.indigoSaveHelmToFile.restype = c_int
        IndigoLib.lib.indigoSaveHelmToFile.argtypes = [c_int, c_char_p, c_int]
        IndigoLib.lib.indigoSmarts.restype = c_char_p
        IndigoLib.lib.indigoSmarts.argtypes = [c_int]
        IndigoLib.lib.indigoName.restype = c_char_p
        IndigoLib.lib.indigoName.argtypes = [c_int]
        IndigoLib.lib.indigoSetName.restype = c_int
        IndigoLib.lib.indigoSetName.argtypes = [c_int, c_char_p]
        IndigoLib.lib.indigoSerialize.restype = c_int
        IndigoLib.lib.indigoSerialize.argtypes = [
            c_int,
            POINTER(POINTER(c_ubyte)),
            POINTER(c_int),
        ]
        IndigoLib.lib.indigoHasProperty.restype = c_int
        IndigoLib.lib.indigoHasProperty.argtypes = [c_int, c_char_p]
        IndigoLib.lib.indigoGetProperty.restype = c_char_p
        IndigoLib.lib.indigoGetProperty.argtypes = [c_int, c_char_p]
        IndigoLib.lib.indigoSetProperty.restype = c_int
        IndigoLib.lib.indigoSetProperty.argtypes = [c_int, c_char_p, c_char_p]
        IndigoLib.lib.indigoRemoveProperty.restype = c_int
        IndigoLib.lib.indigoRemoveProperty.argtypes = [c_int, c_char_p]
        IndigoLib.lib.indigoIterateProperties.restype = c_int
        IndigoLib.lib.indigoIterateProperties.argtypes = [c_int]
        IndigoLib.lib.indigoClearProperties.restype = c_int
        IndigoLib.lib.indigoClearProperties.argtypes = [c_int]
        IndigoLib.lib.indigoCheckBadValence.restype = c_char_p
        IndigoLib.lib.indigoCheckBadValence.argtypes = [c_int]
        IndigoLib.lib.indigoCheckAmbiguousH.restype = c_char_p
        IndigoLib.lib.indigoCheckAmbiguousH.argtypes = [c_int]
        IndigoLib.lib.indigoCheckChirality.restype = c_int
        IndigoLib.lib.indigoCheckChirality.argtypes = [c_int]
        IndigoLib.lib.indigoCheck3DStereo.restype = c_int
        IndigoLib.lib.indigoCheck3DStereo.argtypes = [c_int]
        IndigoLib.lib.indigoCheckStereo.restype = c_int
        IndigoLib.lib.indigoCheckStereo.argtypes = [c_int]
        IndigoLib.lib.indigoFingerprint.restype = c_int
        IndigoLib.lib.indigoFingerprint.argtypes = [c_int, c_char_p]
        IndigoLib.lib.indigoLoadFingerprintFromBuffer.restype = c_int
        IndigoLib.lib.indigoLoadFingerprintFromBuffer.argtypes = [
            POINTER(c_byte),
            c_int,
        ]
        IndigoLib.lib.indigoLoadFingerprintFromDescriptors.restype = c_int
        IndigoLib.lib.indigoLoadFingerprintFromDescriptors.argtypes = [
            POINTER(c_double),
            c_int,
            c_int,
            c_double,
        ]
        IndigoLib.lib.indigoCountBits.restype = c_int
        IndigoLib.lib.indigoCountBits.argtypes = [c_int]
        IndigoLib.lib.indigoRawData.restype = c_char_p
        IndigoLib.lib.indigoRawData.argtypes = [c_int]
        IndigoLib.lib.indigoTell.restype = c_int
        IndigoLib.lib.indigoTell.argtypes = [c_int]
        IndigoLib.lib.indigoSdfAppend.restype = c_int
        IndigoLib.lib.indigoSdfAppend.argtypes = [c_int, c_int]
        IndigoLib.lib.indigoSmilesAppend.restype = c_int
        IndigoLib.lib.indigoSmilesAppend.argtypes = [c_int, c_int]
        IndigoLib.lib.indigoRdfHeader.restype = c_int
        IndigoLib.lib.indigoRdfHeader.argtypes = [c_int]
        IndigoLib.lib.indigoRdfAppend.restype = c_int
        IndigoLib.lib.indigoRdfAppend.argtypes = [c_int, c_int]
        IndigoLib.lib.indigoCmlHeader.restype = c_int
        IndigoLib.lib.indigoCmlHeader.argtypes = [c_int]
        IndigoLib.lib.indigoCmlAppend.restype = c_int
        IndigoLib.lib.indigoCmlAppend.argtypes = [c_int, c_int]
        IndigoLib.lib.indigoCmlFooter.restype = c_int
        IndigoLib.lib.indigoCmlFooter.argtypes = [c_int]
        IndigoLib.lib.indigoAppend.restype = c_int
        IndigoLib.lib.indigoAppend.argtypes = [c_int, c_int]
        IndigoLib.lib.indigoArrayAdd.restype = c_int
        IndigoLib.lib.indigoArrayAdd.argtypes = [c_int, c_int]
        IndigoLib.lib.indigoAt.restype = c_int
        IndigoLib.lib.indigoAt.argtypes = [c_int, c_int]
        IndigoLib.lib.indigoCount.restype = c_int
        IndigoLib.lib.indigoCount.argtypes = [c_int]
        IndigoLib.lib.indigoClear.restype = c_int
        IndigoLib.lib.indigoClear.argtypes = [c_int]
        IndigoLib.lib.indigoIterateArray.restype = c_int
        IndigoLib.lib.indigoIterateArray.argtypes = [c_int]
        IndigoLib.lib.indigoIgnoreAtom.restype = c_int
        IndigoLib.lib.indigoIgnoreAtom.argtypes = [c_int, c_int]
        IndigoLib.lib.indigoUnignoreAtom.restype = c_int
        IndigoLib.lib.indigoUnignoreAtom.argtypes = [c_int, c_int]
        IndigoLib.lib.indigoUnignoreAllAtoms.restype = c_int
        IndigoLib.lib.indigoUnignoreAllAtoms.argtypes = [c_int]
        IndigoLib.lib.indigoMatch.restype = c_int
        IndigoLib.lib.indigoMatch.argtypes = [c_int, c_int]
        IndigoLib.lib.indigoCountMatches.restype = c_int
        IndigoLib.lib.indigoCountMatches.argtypes = [c_int, c_int]
        IndigoLib.lib.indigoCountMatchesWithLimit.restype = c_int
        IndigoLib.lib.indigoCountMatchesWithLimit.argtypes = [
            c_int,
            c_int,
            c_int,
        ]
        IndigoLib.lib.indigoIterateMatches.restype = c_int
        IndigoLib.lib.indigoIterateMatches.argtypes = [c_int, c_int]
        IndigoLib.lib.indigoHighlightedTarget.restype = c_int
        IndigoLib.lib.indigoHighlightedTarget.argtypes = [c_int]
        IndigoLib.lib.indigoMapAtom.restype = c_int
        IndigoLib.lib.indigoMapAtom.argtypes = [c_int, c_int]
        IndigoLib.lib.indigoMapBond.restype = c_int
        IndigoLib.lib.indigoMapBond.argtypes = [c_int, c_int]
        IndigoLib.lib.indigoMapMolecule.restype = c_int
        IndigoLib.lib.indigoMapMolecule.argtypes = [c_int, c_int]
        IndigoLib.lib.indigoIterateTautomers.restype = c_int
        IndigoLib.lib.indigoIterateTautomers.argtypes = [c_int, c_char_p]
        IndigoLib.lib.indigoAllScaffolds.restype = c_int
        IndigoLib.lib.indigoAllScaffolds.argtypes = [c_int]
        IndigoLib.lib.indigoDecomposedMoleculeScaffold.restype = c_int
        IndigoLib.lib.indigoDecomposedMoleculeScaffold.argtypes = [c_int]
        IndigoLib.lib.indigoIterateDecomposedMolecules.restype = c_int
        IndigoLib.lib.indigoIterateDecomposedMolecules.argtypes = [c_int]
        IndigoLib.lib.indigoDecomposedMoleculeHighlighted.restype = c_int
        IndigoLib.lib.indigoDecomposedMoleculeHighlighted.argtypes = [c_int]
        IndigoLib.lib.indigoDecomposedMoleculeWithRGroups.restype = c_int
        IndigoLib.lib.indigoDecomposedMoleculeWithRGroups.argtypes = [c_int]
        IndigoLib.lib.indigoDecomposeMolecule.restype = c_int
        IndigoLib.lib.indigoDecomposeMolecule.argtypes = [c_int, c_int]
        IndigoLib.lib.indigoIterateDecompositions.restype = c_int
        IndigoLib.lib.indigoIterateDecompositions.argtypes = [c_int]
        IndigoLib.lib.indigoAddDecomposition.restype = c_int
        IndigoLib.lib.indigoAddDecomposition.argtypes = [c_int, c_int]
        IndigoLib.lib.indigoToString.restype = c_char_p
        IndigoLib.lib.indigoToString.argtypes = [c_int]
        IndigoLib.lib.indigoOneBitsList.restype = c_char_p
        IndigoLib.lib.indigoOneBitsList.argtypes = [c_int]
        IndigoLib.lib.indigoToBuffer.restype = c_int
        IndigoLib.lib.indigoToBuffer.argtypes = [
            c_int,
            POINTER(POINTER(c_ubyte)),
            POINTER(c_int),
        ]
        IndigoLib.lib.indigoStereocenterPyramid.restype = POINTER(c_int)
        IndigoLib.lib.indigoStereocenterPyramid.argtypes = [c_int]
        IndigoLib.lib.indigoExpandAbbreviations.restype = c_int
        IndigoLib.lib.indigoExpandAbbreviations.argtypes = [c_int]
        IndigoLib.lib.indigoDbgInternalType.restype = c_char_p
        IndigoLib.lib.indigoDbgInternalType.argtypes = [c_int]
        IndigoLib.lib.indigoNameToStructure.restype = c_int
        IndigoLib.lib.indigoNameToStructure.argtypes = [c_char_p, c_char_p]
        IndigoLib.lib.indigoResetOptions.restype = c_int
        IndigoLib.lib.indigoResetOptions.argtypes = []

    @staticmethod
    def checkResult(
        result: int, exception_class: Type[Exception] = IndigoException
    ):
        if result < 0:
            assert IndigoLib.lib
            raise exception_class(IndigoLib.lib.indigoGetLastError())
        return result

    @staticmethod
    def checkResultFloat(
        result: float, exception_class: Type[Exception] = IndigoException
    ):
        if result < -0.5:
            assert IndigoLib.lib
            raise exception_class(IndigoLib.lib.indigoGetLastError())
        return result

    @staticmethod
    def checkResultPtr(
        result: bytes, exception_class: Type[Exception] = IndigoException
    ):
        if result is None:
            assert IndigoLib.lib
            raise exception_class(IndigoLib.lib.indigoGetLastError())
        return result

    @staticmethod
    def checkResultString(
        result: bytes, exception_class: Type[Exception] = IndigoException
    ):
        return IndigoLib.checkResultPtr(result, exception_class).decode()

    @staticmethod
    def getLibraryDir() -> str:
        assert IndigoLib.lib
        return os.path.dirname(IndigoLib.lib._name)  # noqa
