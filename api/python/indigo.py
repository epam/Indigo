#
# Copyright (C) 2010 GGA Software Services LLC
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

import os
import platform
import ctypes
import array
import inspect

from array import *
from ctypes import *

class IndigoException (Exception):
  def __init__(self, value):
    self.value = value
  def __str__(self):
    return repr(self.value)

class Indigo:
  UP = 1
  DOWN = 2
  EITHER = 3
  CIS = 4
  TRANS = 5

  class IndigoObject:
    def __init__ (self, dispatcher, id):
      self.id = id
      self._lib = dispatcher._lib
      self._sid = dispatcher._sid
      self.dispatcher = dispatcher

    def grossFormula (self):
      self.dispatcher._setSID()
      gfid = self.dispatcher._checkResult(self._lib.indigoGrossFormula(self.id))
      gf = Indigo.IndigoObject(self.dispatcher, gfid)
      return self.dispatcher._checkResultString(self._lib.indigoToString(gf.id))

    def toString (self):
      self.dispatcher._setSID()
      return self.dispatcher._checkResultString(self._lib.indigoToString(self.id))

    def toBuffer (self):
      self.dispatcher._setSID()
      c_size = c_int()
      c_buf = POINTER(c_char)()
      self.dispatcher._checkResult(self._lib.indigoToBuffer(
          self.id, pointer(c_buf), pointer(c_size)))
      res = array('c')
      for i in xrange(c_size.value):
        res.append(c_buf[i])
      return res

    def mdlct (self):
      self.dispatcher._setSID()
      buf = self.dispatcher.writeBuffer()
      self.dispatcher._checkResult(self._lib.indigoSaveMDLCT(self.id, buf.id))
      return buf.toBuffer()
    
    def xyz (self):
      self.dispatcher._setSID()
      xyz = self._lib.indigoXYZ(self.id)
      if xyz is None:
        raise IndigoException(self._lib.indigoGetLastError())
      return [xyz[0], xyz[1], xyz[2]]

    def __del__ (self):
      self.dispatcher._setSID()
      self._lib.indigoFree(self.id)
    def __iter__ (self):
      return self
    def next (self):
      self.dispatcher._setSID()
      res = self.dispatcher._checkResult(self._lib.indigoNext(self.id))
      if res == 0:
        raise StopIteration
      return Indigo.IndigoObject(self.dispatcher, res)

  def __init__ (self, path = None):
    if not path:
      dirname = os.path.dirname(inspect.getfile(inspect.currentframe()))
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
      self._lib = CDLL(path + "/libindigo.so", mode=RTLD_GLOBAL)
    elif os.name == 'nt':
      arch = platform.architecture()[0]
      path += "/Win"
      if arch == '32bit':
        path += "/x86"
      elif arch == '64bit':
        path += "/x64"
      else:
        raise IndigoException("unknown platform " + arch)
      self._zlib = CDLL(path + "/zlib.dll")
      self._lib = CDLL(path + "/indigo.dll")
    elif platform.mac_ver()[0]:
      path += "/Mac/"
      # append "10.5" or "10.6" to the path
      path += '.'.join(platform.mac_ver()[0].split('.')[:2])
      self._lib = CDLL(path + "/libindigo.dylib", mode=RTLD_GLOBAL)
    else:
      raise IndigoException("unsupported OS: " + os.name)

    self.dllpath = path

    #self._lib.indigoSetErrorHandler.restype = None
    #self._lib.indigoSetErrorHandler.argtypes = [ERRFUNC, c_void_p]
    
    # auto-generated with "generate-python.py indigo.h"
    self._lib.indigoVersion.restype = c_char_p
    self._lib.indigoVersion.argtypes = None
    self._lib.indigoAllocSessionId.restype = c_ulonglong
    self._lib.indigoAllocSessionId.argtypes = None
    self._lib.indigoSetSessionId.restype = None
    self._lib.indigoSetSessionId.argtypes = [c_ulonglong]
    self._lib.indigoReleaseSessionId.restype = None
    self._lib.indigoReleaseSessionId.argtypes = [c_ulonglong]
    self._lib.indigoGetLastError.restype = c_char_p
    self._lib.indigoGetLastError.argtypes = None
    self._lib.indigoSetErrorMessage.restype = None
    self._lib.indigoSetErrorMessage.argtypes = [c_char_p]
    self._lib.indigoFree.restype = c_int
    self._lib.indigoFree.argtypes = [c_int]
    self._lib.indigoClone.restype = c_int
    self._lib.indigoClone.argtypes = [c_int]
    self._lib.indigoCountReferences.restype = c_int
    self._lib.indigoCountReferences.argtypes = None
    self._lib.indigoSetOption.restype = c_int
    self._lib.indigoSetOption.argtypes = [c_char_p, c_char_p]
    self._lib.indigoSetOptionInt.restype = c_int
    self._lib.indigoSetOptionInt.argtypes = [c_char_p, c_int]
    self._lib.indigoSetOptionBool.restype = c_int
    self._lib.indigoSetOptionBool.argtypes = [c_char_p, c_int]
    self._lib.indigoSetOptionFloat.restype = c_int
    self._lib.indigoSetOptionFloat.argtypes = [c_char_p, c_float]
    self._lib.indigoSetOptionColor.restype = c_int
    self._lib.indigoSetOptionColor.argtypes = [c_char_p, c_float, c_float, c_float]
    self._lib.indigoSetOptionXY.restype = c_int
    self._lib.indigoSetOptionXY.argtypes = [c_char_p, c_int, c_int]
    self._lib.indigoReadFile.restype = c_int
    self._lib.indigoReadFile.argtypes = [c_char_p]
    self._lib.indigoReadString.restype = c_int
    self._lib.indigoReadString.argtypes = [c_char_p]
    self._lib.indigoLoadString.restype = c_int
    self._lib.indigoLoadString.argtypes = [c_char_p]
    self._lib.indigoReadBuffer.restype = c_int
    self._lib.indigoReadBuffer.argtypes = [c_char_p, c_int]
    self._lib.indigoLoadBuffer.restype = c_int
    self._lib.indigoLoadBuffer.argtypes = [c_char_p, c_int]
    self._lib.indigoWriteFile.restype = c_int
    self._lib.indigoWriteFile.argtypes = [c_char_p]
    self._lib.indigoWriteBuffer.restype = c_int
    self._lib.indigoWriteBuffer.argtypes = None
    self._lib.indigoLoadMolecule.restype = c_int
    self._lib.indigoLoadMolecule.argtypes = [c_int]
    self._lib.indigoLoadMoleculeFromString.restype = c_int
    self._lib.indigoLoadMoleculeFromString.argtypes = [c_char_p]
    self._lib.indigoLoadMoleculeFromFile.restype = c_int
    self._lib.indigoLoadMoleculeFromFile.argtypes = [c_char_p]
    self._lib.indigoLoadMoleculeFromBuffer.restype = c_int
    self._lib.indigoLoadMoleculeFromBuffer.argtypes = [c_char_p, c_int]
    self._lib.indigoLoadQueryMolecule.restype = c_int
    self._lib.indigoLoadQueryMolecule.argtypes = [c_int]
    self._lib.indigoLoadQueryMoleculeFromString.restype = c_int
    self._lib.indigoLoadQueryMoleculeFromString.argtypes = [c_char_p]
    self._lib.indigoLoadQueryMoleculeFromFile.restype = c_int
    self._lib.indigoLoadQueryMoleculeFromFile.argtypes = [c_char_p]
    self._lib.indigoLoadQueryMoleculeFromBuffer.restype = c_int
    self._lib.indigoLoadQueryMoleculeFromBuffer.argtypes = [c_char_p, c_int]
    self._lib.indigoLoadSmarts.restype = c_int
    self._lib.indigoLoadSmarts.argtypes = [c_int]
    self._lib.indigoLoadSmartsFromString.restype = c_int
    self._lib.indigoLoadSmartsFromString.argtypes = [c_char_p]
    self._lib.indigoLoadSmartsFromFile.restype = c_int
    self._lib.indigoLoadSmartsFromFile.argtypes = [c_char_p]
    self._lib.indigoLoadSmartsFromBuffer.restype = c_int
    self._lib.indigoLoadSmartsFromBuffer.argtypes = [c_char_p, c_int]
    self._lib.indigoSaveMolfile.restype = c_int
    self._lib.indigoSaveMolfile.argtypes = [c_int, c_int]
    self._lib.indigoSaveMolfileToFile.restype = c_int
    self._lib.indigoSaveMolfileToFile.argtypes = [c_int, c_char_p]
    self._lib.indigoMolfile.restype = c_char_p
    self._lib.indigoMolfile.argtypes = [c_int]
    self._lib.indigoSaveCml.restype = c_int
    self._lib.indigoSaveCml.argtypes = [c_int, c_int]
    self._lib.indigoSaveCmlToFile.restype = c_int
    self._lib.indigoSaveCmlToFile.argtypes = [c_int, c_char_p]
    self._lib.indigoCml.restype = c_char_p
    self._lib.indigoCml.argtypes = [c_int]
    self._lib.indigoSaveMDLCT.restype = c_int
    self._lib.indigoSaveMDLCT.argtypes = [c_int, c_int]
    self._lib.indigoLoadReaction.restype = c_int
    self._lib.indigoLoadReaction.argtypes = [c_int]
    self._lib.indigoLoadReactionFromString.restype = c_int
    self._lib.indigoLoadReactionFromString.argtypes = [c_char_p]
    self._lib.indigoLoadReactionFromFile.restype = c_int
    self._lib.indigoLoadReactionFromFile.argtypes = [c_char_p]
    self._lib.indigoLoadReactionFromBuffer.restype = c_int
    self._lib.indigoLoadReactionFromBuffer.argtypes = [c_char_p, c_int]
    self._lib.indigoLoadQueryReaction.restype = c_int
    self._lib.indigoLoadQueryReaction.argtypes = [c_int]
    self._lib.indigoLoadQueryReactionFromString.restype = c_int
    self._lib.indigoLoadQueryReactionFromString.argtypes = [c_char_p]
    self._lib.indigoLoadQueryReactionFromFile.restype = c_int
    self._lib.indigoLoadQueryReactionFromFile.argtypes = [c_char_p]
    self._lib.indigoLoadQueryReactionFromBuffer.restype = c_int
    self._lib.indigoLoadQueryReactionFromBuffer.argtypes = [c_char_p, c_int]
    self._lib.indigoCreateReaction.restype = c_int
    self._lib.indigoCreateReaction.argtypes = None
    self._lib.indigoCreateQueryReaction.restype = c_int
    self._lib.indigoCreateQueryReaction.argtypes = None
    self._lib.indigoAddReactant.restype = c_int
    self._lib.indigoAddReactant.argtypes = [c_int, c_int]
    self._lib.indigoAddProduct.restype = c_int
    self._lib.indigoAddProduct.argtypes = [c_int, c_int]
    self._lib.indigoCountReactants.restype = c_int
    self._lib.indigoCountReactants.argtypes = [c_int]
    self._lib.indigoCountProducts.restype = c_int
    self._lib.indigoCountProducts.argtypes = [c_int]
    self._lib.indigoCountMolecules.restype = c_int
    self._lib.indigoCountMolecules.argtypes = [c_int]
    self._lib.indigoIterateReactants.restype = c_int
    self._lib.indigoIterateReactants.argtypes = [c_int]
    self._lib.indigoIterateProducts.restype = c_int
    self._lib.indigoIterateProducts.argtypes = [c_int]
    self._lib.indigoIterateMolecules.restype = c_int
    self._lib.indigoIterateMolecules.argtypes = [c_int]
    self._lib.indigoSaveRxnfile.restype = c_int
    self._lib.indigoSaveRxnfile.argtypes = [c_int, c_int]
    self._lib.indigoSaveRxnfileToFile.restype = c_int
    self._lib.indigoSaveRxnfileToFile.argtypes = [c_int, c_char_p]
    self._lib.indigoRxnfile.restype = c_char_p
    self._lib.indigoRxnfile.argtypes = [c_int]
    self._lib.indigoAutomap.restype = c_int
    self._lib.indigoAutomap.argtypes = [c_int, c_char_p]
    self._lib.indigoIterateAtoms.restype = c_int
    self._lib.indigoIterateAtoms.argtypes = [c_int]
    self._lib.indigoIteratePseudoatoms.restype = c_int
    self._lib.indigoIteratePseudoatoms.argtypes = [c_int]
    self._lib.indigoIterateRSites.restype = c_int
    self._lib.indigoIterateRSites.argtypes = [c_int]
    self._lib.indigoIterateRGroups.restype = c_int
    self._lib.indigoIterateRGroups.argtypes = [c_int]
    self._lib.indigoIterateRGroupFragments.restype = c_int
    self._lib.indigoIterateRGroupFragments.argtypes = [c_int]
    self._lib.indigoCountAttachmentPoints.restype = c_int
    self._lib.indigoCountAttachmentPoints.argtypes = [c_int]
    self._lib.indigoIsPseudoatom.restype = c_int
    self._lib.indigoIsPseudoatom.argtypes = [c_int]
    self._lib.indigoIsRSite.restype = c_int
    self._lib.indigoIsRSite.argtypes = [c_int]
    self._lib.indigoSingleAllowedRGroup.restype = c_int
    self._lib.indigoSingleAllowedRGroup.argtypes = [c_int]
    self._lib.indigoPseudoatomLabel.restype = c_char_p
    self._lib.indigoPseudoatomLabel.argtypes = [c_int]
    self._lib.indigoDegree.restype = c_int
    self._lib.indigoDegree.argtypes = [c_int]
    self._lib.indigoGetCharge.restype = c_int
    self._lib.indigoGetCharge.argtypes = [c_int, POINTER(c_int)]
    self._lib.indigoGetExplicitValence.restype = c_int
    self._lib.indigoGetExplicitValence.argtypes = [c_int, POINTER(c_int)]
    self._lib.indigoGetRadicalElectrons.restype = c_int
    self._lib.indigoGetRadicalElectrons.argtypes = [c_int, POINTER(c_int)]
    self._lib.indigoAtomNumber.restype = c_int
    self._lib.indigoAtomNumber.argtypes = [c_int]
    self._lib.indigoAtomIsotope.restype = c_int
    self._lib.indigoAtomIsotope.argtypes = [c_int]
    self._lib.indigoResetCharge.restype = c_int
    self._lib.indigoResetCharge.argtypes = [c_int]
    self._lib.indigoResetExplicitValence.restype = c_int
    self._lib.indigoResetExplicitValence.argtypes = [c_int]
    self._lib.indigoResetRadical.restype = c_int
    self._lib.indigoResetRadical.argtypes = [c_int]
    self._lib.indigoResetIsotope.restype = c_int
    self._lib.indigoResetIsotope.argtypes = [c_int]
    self._lib.indigoCountAtoms.restype = c_int
    self._lib.indigoCountAtoms.argtypes = [c_int]
    self._lib.indigoCountBonds.restype = c_int
    self._lib.indigoCountBonds.argtypes = [c_int]
    self._lib.indigoCountPseudoatoms.restype = c_int
    self._lib.indigoCountPseudoatoms.argtypes = [c_int]
    self._lib.indigoCountRSites.restype = c_int
    self._lib.indigoCountRSites.argtypes = [c_int]
    self._lib.indigoIterateBonds.restype = c_int
    self._lib.indigoIterateBonds.argtypes = [c_int]
    self._lib.indigoBondOrder.restype = c_int
    self._lib.indigoBondOrder.argtypes = [c_int]
    self._lib.indigoBondStereo.restype = c_int
    self._lib.indigoBondStereo.argtypes = [c_int]
    self._lib.indigoIterateNeighbors.restype = c_int
    self._lib.indigoIterateNeighbors.argtypes = [c_int]
    self._lib.indigoBond.restype = c_int
    self._lib.indigoBond.argtypes = [c_int]
    self._lib.indigoGetAtom.restype = c_int
    self._lib.indigoGetAtom.argtypes = [c_int, c_int]
    self._lib.indigoGetBond.restype = c_int
    self._lib.indigoGetBond.argtypes = [c_int, c_int]
    self._lib.indigoCisTransClear.restype = c_int
    self._lib.indigoCisTransClear.argtypes = [c_int]
    self._lib.indigoStereocentersClear.restype = c_int
    self._lib.indigoStereocentersClear.argtypes = [c_int]
    self._lib.indigoCountStereocenters.restype = c_int
    self._lib.indigoCountStereocenters.argtypes = [c_int]
    self._lib.indigoGrossFormula.restype = c_int
    self._lib.indigoGrossFormula.argtypes = [c_int]
    self._lib.indigoMolecularWeight.restype = c_float
    self._lib.indigoMolecularWeight.argtypes = [c_int]
    self._lib.indigoMostAbundantMass.restype = c_float
    self._lib.indigoMostAbundantMass.argtypes = [c_int]
    self._lib.indigoMonoisotopicMass.restype = c_float
    self._lib.indigoMonoisotopicMass.argtypes = [c_int]
    self._lib.indigoCanonicalSmiles.restype = c_char_p
    self._lib.indigoCanonicalSmiles.argtypes = [c_int]
    self._lib.indigoLayeredCode.restype = c_char_p
    self._lib.indigoLayeredCode.argtypes = [c_int]
    self._lib.indigoCountComponents.restype = c_int
    self._lib.indigoCountComponents.argtypes = [c_int]
    self._lib.indigoXYZ.restype = POINTER(c_float)
    self._lib.indigoXYZ.argtypes = [c_int]
    self._lib.indigoCreateSubmolecule.restype = c_int
    self._lib.indigoCreateSubmolecule.argtypes = [c_int, c_int, POINTER(c_int)]
    self._lib.indigoCreateEdgeSubmolecule.restype = c_int
    self._lib.indigoCreateEdgeSubmolecule.argtypes = [c_int, c_int, POINTER(c_int), c_int, POINTER(c_int)]
    self._lib.indigoAlignAtoms.restype = c_float
    self._lib.indigoAlignAtoms.argtypes = [c_int, c_int, POINTER(c_int), POINTER(c_float)]
    self._lib.indigoAromatize.restype = c_int
    self._lib.indigoAromatize.argtypes = [c_int]
    self._lib.indigoDearomatize.restype = c_int
    self._lib.indigoDearomatize.argtypes = [c_int]
    self._lib.indigoFoldHydrogens.restype = c_int
    self._lib.indigoFoldHydrogens.argtypes = [c_int]
    self._lib.indigoUnfoldHydrogens.restype = c_int
    self._lib.indigoUnfoldHydrogens.argtypes = [c_int]
    self._lib.indigoLayout.restype = c_int
    self._lib.indigoLayout.argtypes = [c_int]
    self._lib.indigoSmiles.restype = c_char_p
    self._lib.indigoSmiles.argtypes = [c_int]
    self._lib.indigoExactMatch.restype = c_int
    self._lib.indigoExactMatch.argtypes = [c_int, c_int]
    self._lib.indigoName.restype = c_char_p
    self._lib.indigoName.argtypes = [c_int]
    self._lib.indigoSetName.restype = c_int
    self._lib.indigoSetName.argtypes = [c_int, c_char_p]
    self._lib.indigoHasProperty.restype = c_int
    self._lib.indigoHasProperty.argtypes = [c_int, c_char_p]
    self._lib.indigoGetProperty.restype = c_char_p
    self._lib.indigoGetProperty.argtypes = [c_int, c_char_p]
    self._lib.indigoSetProperty.restype = c_int
    self._lib.indigoSetProperty.argtypes = [c_int, c_char_p, c_char_p]
    self._lib.indigoRemoveProperty.restype = c_int
    self._lib.indigoRemoveProperty.argtypes = [c_int, c_char_p]
    self._lib.indigoIterateProperties.restype = c_int
    self._lib.indigoIterateProperties.argtypes = [c_int]
    self._lib.indigoCheckBadValence.restype = c_char_p
    self._lib.indigoCheckBadValence.argtypes = [c_int]
    self._lib.indigoCheckAmbiguousH.restype = c_char_p
    self._lib.indigoCheckAmbiguousH.argtypes = [c_int]
    self._lib.indigoFingerprint.restype = c_int
    self._lib.indigoFingerprint.argtypes = [c_int, c_char_p]
    self._lib.indigoCountBits.restype = c_int
    self._lib.indigoCountBits.argtypes = [c_int]
    self._lib.indigoCommonBits.restype = c_int
    self._lib.indigoCommonBits.argtypes = [c_int, c_int]
    self._lib.indigoSimilarity.restype = c_float
    self._lib.indigoSimilarity.argtypes = [c_int, c_int, c_char_p]
    self._lib.indigoIterateSDF.restype = c_int
    self._lib.indigoIterateSDF.argtypes = [c_int]
    self._lib.indigoIterateRDF.restype = c_int
    self._lib.indigoIterateRDF.argtypes = [c_int]
    self._lib.indigoIterateSmiles.restype = c_int
    self._lib.indigoIterateSmiles.argtypes = [c_int]
    self._lib.indigoIterateSDFile.restype = c_int
    self._lib.indigoIterateSDFile.argtypes = [c_char_p]
    self._lib.indigoIterateRDFile.restype = c_int
    self._lib.indigoIterateRDFile.argtypes = [c_char_p]
    self._lib.indigoIterateSmilesFile.restype = c_int
    self._lib.indigoIterateSmilesFile.argtypes = [c_char_p]
    self._lib.indigoRawData.restype = c_char_p
    self._lib.indigoRawData.argtypes = [c_int]
    self._lib.indigoTell.restype = c_int
    self._lib.indigoTell.argtypes = [c_int]
    self._lib.indigoSdfAppend.restype = c_int
    self._lib.indigoSdfAppend.argtypes = [c_int, c_int]
    self._lib.indigoSmilesAppend.restype = c_int
    self._lib.indigoSmilesAppend.argtypes = [c_int, c_int]
    self._lib.indigoCreateArray.restype = c_int
    self._lib.indigoCreateArray.argtypes = None
    self._lib.indigoArrayAdd.restype = c_int
    self._lib.indigoArrayAdd.argtypes = [c_int, c_int]
    self._lib.indigoArrayAt.restype = c_int
    self._lib.indigoArrayAt.argtypes = [c_int, c_int]
    self._lib.indigoArrayCount.restype = c_int
    self._lib.indigoArrayCount.argtypes = [c_int]
    self._lib.indigoArrayClear.restype = c_int
    self._lib.indigoArrayClear.argtypes = [c_int]
    self._lib.indigoIterateArray.restype = c_int
    self._lib.indigoIterateArray.argtypes = [c_int]
    self._lib.indigoMatchSubstructure.restype = c_int
    self._lib.indigoMatchSubstructure.argtypes = [c_int, c_int]
    self._lib.indigoMatchHighlight.restype = c_int
    self._lib.indigoMatchHighlight.argtypes = [c_int]
    self._lib.indigoMapAtom.restype = c_int
    self._lib.indigoMapAtom.argtypes = [c_int, c_int]
    self._lib.indigoCountSubstructureMatches.restype = c_int
    self._lib.indigoCountSubstructureMatches.argtypes = [c_int, c_int]
    self._lib.indigoExtractCommonScaffold.restype = c_int
    self._lib.indigoExtractCommonScaffold.argtypes = [c_int, c_char_p]
    self._lib.indigoAllScaffolds.restype = c_int
    self._lib.indigoAllScaffolds.argtypes = [c_int]
    self._lib.indigoDecomposeMolecules.restype = c_int
    self._lib.indigoDecomposeMolecules.argtypes = [c_int, c_int]
    self._lib.indigoDecomposedMoleculeScaffold.restype = c_int
    self._lib.indigoDecomposedMoleculeScaffold.argtypes = [c_int]
    self._lib.indigoIterateDecomposedMolecules.restype = c_int
    self._lib.indigoIterateDecomposedMolecules.argtypes = [c_int]
    self._lib.indigoDecomposedMoleculeHighlighted.restype = c_int
    self._lib.indigoDecomposedMoleculeHighlighted.argtypes = [c_int]
    self._lib.indigoDecomposedMoleculeWithRGroups.restype = c_int
    self._lib.indigoDecomposedMoleculeWithRGroups.argtypes = [c_int]
    self._lib.indigoNext.restype = c_int
    self._lib.indigoNext.argtypes = [c_int]
    self._lib.indigoHasNext.restype = c_int
    self._lib.indigoHasNext.argtypes = [c_int]
    self._lib.indigoIndex.restype = c_int
    self._lib.indigoIndex.argtypes = [c_int]
    self._lib.indigoToString.restype = c_char_p
    self._lib.indigoToString.argtypes = [c_int]
    self._lib.indigoReactionProductEnumerate.restype = c_int
    self._lib.indigoReactionProductEnumerate.argtypes = [c_int, c_int]

    self._lib.indigoToBuffer.restype = c_int
    self._lib.indigoToBuffer.argtypes = [c_int, POINTER(POINTER(c_char)), POINTER(c_int)]
 
    self._sid = self._lib.indigoAllocSessionId()
    self._lib.indigoSetSessionId(self._sid)

    self.countReferences = self._static_int(self._lib.indigoCountReferences)

    self.loadMolecule = self._static_obj_string(self._lib.indigoLoadMoleculeFromString)
    self.loadMoleculeFromFile = self._static_obj_string(self._lib.indigoLoadMoleculeFromFile)
    self.loadQueryMolecule = self._static_obj_string(self._lib.indigoLoadQueryMoleculeFromString)
    self.loadQueryMoleculeFromFile = self._static_obj_string(self._lib.indigoLoadQueryMoleculeFromFile)
    self.loadSmarts = self._static_obj_string(self._lib.indigoLoadSmartsFromString)
    self.loadSmartsFromFile = self._static_obj_string(self._lib.indigoLoadSmartsFromFile)
    self.loadReaction = self._static_obj_string(self._lib.indigoLoadReactionFromString)
    self.loadReactionFromFile = self._static_obj_string(self._lib.indigoLoadReactionFromFile)
    self.loadQueryReaction = self._static_obj_string(self._lib.indigoLoadQueryReactionFromString)
    self.loadQueryReactionFromFile = self._static_obj_string(self._lib.indigoLoadQueryReactionFromFile)
    self.createReaction = self._static_obj(self._lib.indigoCreateReaction)
    self.createQueryReaction = self._static_obj(self._lib.indigoCreateQueryReaction)
    self.createArray = self._static_obj(self._lib.indigoCreateArray)

    self.commonBits = self._static_int_obj_obj(self._lib.indigoCommonBits)

    self.iterateSDFile = self._static_obj_string(self._lib.indigoIterateSDFile)
    self.iterateRDFile = self._static_obj_string(self._lib.indigoIterateRDFile)
    self.iterateSmilesFile = self._static_obj_string(self._lib.indigoIterateSmilesFile)

    self.matchSubstructure = self._static_obj_obj_obj(self._lib.indigoMatchSubstructure)
    self.countSubstructureMatches = self._static_int_obj_obj(self._lib.indigoCountSubstructureMatches)
    self.extractCommonScaffold = self._static_obj_obj_string(self._lib.indigoExtractCommonScaffold)
    self.decomposeMolecules = self._static_obj_obj_obj(self._lib.indigoDecomposeMolecules)
    self.reactionProductEnumerate = self._static_obj_obj_obj(self._lib.indigoReactionProductEnumerate)

    self.IndigoObject.clone = self._member_obj(self._lib.indigoClone)

    self.IndigoObject.molfile = self._member_string(self._lib.indigoMolfile)
    self.IndigoObject.saveMolfile = self._member_void_string(self._lib.indigoSaveMolfileToFile)
    self.IndigoObject.cml = self._member_string(self._lib.indigoCml)
    self.IndigoObject.saveCml = self._member_void_string(self._lib.indigoSaveCmlToFile)
    self.IndigoObject.addReactant = self._member_void_obj(self._lib.indigoAddReactant)
    self.IndigoObject.addProduct  = self._member_void_obj(self._lib.indigoAddProduct)
    self.IndigoObject.countReactants = self._member_int(self._lib.indigoCountReactants)
    self.IndigoObject.countProducts = self._member_int(self._lib.indigoCountProducts)
    self.IndigoObject.countMolecules = self._member_int(self._lib.indigoCountMolecules)
    self.IndigoObject.iterateReactants = self._member_obj(self._lib.indigoIterateReactants)
    self.IndigoObject.iterateProducts  = self._member_obj(self._lib.indigoIterateProducts)
    self.IndigoObject.iterateMolecules  = self._member_obj(self._lib.indigoIterateMolecules)

    self.IndigoObject.rxnfile = self._member_string(self._lib.indigoRxnfile)
    self.IndigoObject.saveRxnfile = self._member_void_string(self._lib.indigoSaveRxnfileToFile)
    self.IndigoObject.automap = self._member_void_string(self._lib.indigoAutomap)

    self.IndigoObject.iterateAtoms = self._member_obj(self._lib.indigoIterateAtoms)
    self.IndigoObject.iteratePseudoatoms = self._member_obj(self._lib.indigoIteratePseudoatoms)
    self.IndigoObject.iterateRSites = self._member_obj(self._lib.indigoIterateRSites)
    self.IndigoObject.iterateRGroups = self._member_obj(self._lib.indigoIterateRGroups)
    self.IndigoObject.iterateRGroupFragments = self._member_obj(self._lib.indigoIterateRGroupFragments)
    self.IndigoObject.countAttachmentPoints = self._member_int(self._lib.indigoCountAttachmentPoints)
    self.IndigoObject.isPseudoatom = self._member_bool(self._lib.indigoIsPseudoatom)
    self.IndigoObject.isRSite = self._member_bool(self._lib.indigoIsRSite)
    self.IndigoObject.singleAllowedRGroup = self._member_int(self._lib.indigoSingleAllowedRGroup)
    self.IndigoObject.pseudoatomLabel = self._member_string(self._lib.indigoPseudoatomLabel)

    self.IndigoObject.degree = self._member_int(self._lib.indigoDegree)
    self.IndigoObject.charge = self._member_intptr(self._lib.indigoGetCharge)
    self.IndigoObject.explicitValence = self._member_intptr(self._lib.indigoGetExplicitValence)
    self.IndigoObject.radicalElectrons = self._member_intptr(self._lib.indigoGetRadicalElectrons)
    self.IndigoObject.atomNumber = self._member_int(self._lib.indigoAtomNumber)
    self.IndigoObject.atomIsotope = self._member_int(self._lib.indigoAtomIsotope)

    self.IndigoObject.resetCharge = self._member_void(self._lib.indigoResetCharge)
    self.IndigoObject.resetExplicitValence = self._member_void(self._lib.indigoResetExplicitValence)
    self.IndigoObject.resetRadical = self._member_void(self._lib.indigoResetRadical)
    self.IndigoObject.resetIsotope = self._member_void(self._lib.indigoResetIsotope)

    self.IndigoObject.countAtoms = self._member_int(self._lib.indigoCountAtoms)
    self.IndigoObject.countBonds = self._member_int(self._lib.indigoCountBonds)
    self.IndigoObject.countPseudoatoms = self._member_int(self._lib.indigoCountPseudoatoms)
    self.IndigoObject.countRSites = self._member_int(self._lib.indigoCountRSites)

    self.IndigoObject.iterateBonds = self._member_obj(self._lib.indigoIterateBonds)
    self.IndigoObject.bondOrder = self._member_int(self._lib.indigoBondOrder)
    self.IndigoObject.bondStereo = self._member_int(self._lib.indigoBondStereo)

    self.IndigoObject.iterateNeighbors = self._member_obj(self._lib.indigoIterateNeighbors)
    self.IndigoObject.bond = self._member_obj(self._lib.indigoBond)
    self.IndigoObject.getAtom = self._member_obj_int(self._lib.indigoGetAtom)
    self.IndigoObject.getBond = self._member_obj_int(self._lib.indigoGetBond)

    self.IndigoObject.cisTransClear = self._member_void(self._lib.indigoCisTransClear)
    self.IndigoObject.stereocentersClear = self._member_void(self._lib.indigoStereocentersClear)
    self.IndigoObject.countStereocenters = self._member_int(self._lib.indigoCountStereocenters)

    self.IndigoObject.molecularWeight = self._member_float(self._lib.indigoMolecularWeight)
    self.IndigoObject.monoisotopicMass = self._member_float(self._lib.indigoMonoisotopicMass)
    self.IndigoObject.mostAbundantMass = self._member_float(self._lib.indigoMostAbundantMass)

    self.IndigoObject.canonicalSmiles = self._member_string(self._lib.indigoCanonicalSmiles)
    self.IndigoObject.layeredCode = self._member_string(self._lib.indigoLayeredCode)
    self.IndigoObject.countComponents = self._member_int(self._lib.indigoCountComponents)
    self.IndigoObject.hasZCoord = self._member_bool(self._lib.indigoHasZCoord)
    
    self.IndigoObject.aromatize = self._member_void(self._lib.indigoAromatize)
    self.IndigoObject.dearomatize = self._member_void(self._lib.indigoDearomatize)
    self.IndigoObject.foldHydrogens = self._member_void(self._lib.indigoFoldHydrogens)
    self.IndigoObject.unfoldHydrogens = self._member_void(self._lib.indigoUnfoldHydrogens)
    self.IndigoObject.layout = self._member_void(self._lib.indigoLayout)

    self.IndigoObject.smiles = self._member_string(self._lib.indigoSmiles)
    self.IndigoObject.name = self._member_string(self._lib.indigoName)
    self.IndigoObject.setName = self._member_void_string(self._lib.indigoSetName)
    self.IndigoObject.hasProperty = self._member_bool_string(self._lib.indigoHasProperty)
    self.IndigoObject.getProperty = self._member_string_string(self._lib.indigoGetProperty)
    self.IndigoObject.setProperty = self._member_void_string_string(self._lib.indigoSetProperty)
    self.IndigoObject.removeProperty = self._member_void_string(self._lib.indigoRemoveProperty)
    self.IndigoObject.iterateProperties = self._member_obj(self._lib.indigoIterateProperties)

    self.IndigoObject.checkBadValence = self._member_string(self._lib.indigoCheckBadValence)
    self.IndigoObject.checkAmbiguousH = self._member_string(self._lib.indigoCheckAmbiguousH)
    self.IndigoObject.rawData = self._member_string(self._lib.indigoRawData)

    self.IndigoObject.fingerprint = self._member_obj_string(self._lib.indigoFingerprint)
    self.IndigoObject.countBits = self._member_int(self._lib.indigoCountBits)
    self.IndigoObject.tell = self._member_int(self._lib.indigoTell)
    self.IndigoObject.sdfAppend = self._member_void_obj(self._lib.indigoSdfAppend)
    self.IndigoObject.smilesAppend = self._member_void_obj(self._lib.indigoSmilesAppend)

    self.IndigoObject.iterateArray = self._member_obj(self._lib.indigoIterateArray)
    self.IndigoObject.arrayCount = self._member_int(self._lib.indigoArrayCount)
    self.IndigoObject.arrayClear = self._member_void(self._lib.indigoArrayClear)
    self.IndigoObject.arrayAdd = self._member_void_obj(self._lib.indigoArrayAdd)
    self.IndigoObject.arrayAt = self._member_obj_int(self._lib.indigoArrayAt)
    
    self.IndigoObject.matchHighlight = self._member_obj(self._lib.indigoMatchHighlight);
    self.IndigoObject.mapAtom = self._member_obj_obj(self._lib.indigoMapAtom);
    self.IndigoObject.allScaffolds = self._member_obj(self._lib.indigoAllScaffolds);
    self.IndigoObject.decomposedMoleculeScaffold = self._member_obj(self._lib.indigoDecomposedMoleculeScaffold)
    self.IndigoObject.iterateDecomposedMolecules = self._member_obj(self._lib.indigoIterateDecomposedMolecules)
    self.IndigoObject.iterateDecomposedMolecules = self._member_obj(self._lib.indigoIterateDecomposedMolecules)
    self.IndigoObject.decomposedMoleculeHighlighted = self._member_obj(self._lib.indigoDecomposedMoleculeHighlighted)
    self.IndigoObject.decomposedMoleculeWithRGroups = self._member_obj(self._lib.indigoDecomposedMoleculeWithRGroups)
    
    self.IndigoObject.index = self._member_int(self._lib.indigoIndex)

    self.IndigoObject.createSubmolecule = self._member_obj_iarr(self._lib.indigoCreateSubmolecule)
    self.IndigoObject.createEdgeSubmolecule = self._member_obj_iarr_iarr(self._lib.indigoCreateEdgeSubmolecule)

  def _static_obj (self, func):
    def newfunc ():
      self._setSID()
      res = self._checkResult(func())
      return self.IndigoObject(self, res)
    return newfunc

  def _static_int (self, func):
    def newfunc ():
      self._setSID()
      return self._checkResult(func())
    return newfunc

  def _static_int_obj_obj (self, func):
    def newfunc (item1, item2):
      self._setSID()
      return self._checkResult(func(item1.id, item2.id))
    return newfunc
   
  def _static_obj_string (self, func):
    def newfunc (str):
      self._setSID()
      res = self._checkResult(func(str))
      return self.IndigoObject(self, res)
    return newfunc

  def _static_obj_obj_string (self, func):
    def newfunc (obj1, string):
      self._setSID()
      res = func(obj1.id, string)
      if res == 0:
        return None
      return self.IndigoObject(self, self._checkResult(res))
    return newfunc

  def _static_obj_obj_obj (self, func):
    def newfunc (obj1, obj2):
      self._setSID()
      res = func(obj1.id, obj2.id)
      if res == 0:
        return None
      return self.IndigoObject(self, self._checkResult(res))
    return newfunc

  def _setSID (self):
    self._lib.indigoSetSessionId(self._sid)

  def _member_string (self, func):
    dispatcher = self
    def newfunc (self):
      dispatcher._setSID()
      return dispatcher._checkResultString(func(self.id))
    return newfunc

  def _member_bool (self, func):
    dispatcher = self
    def newfunc (self):
      dispatcher._setSID()
      res = dispatcher._checkResult(func(self.id))
      return res == 1                                                 
    return newfunc

  def _member_bool_string (self, func):
    dispatcher = self
    def newfunc (self, str):
      dispatcher._setSID()
      res = dispatcher._checkResult(func(self.id, str))
      return res == 1                                                 
    return newfunc

  def _member_string_string (self, func):
    dispatcher = self
    def newfunc (self, str):
      dispatcher._setSID()
      return dispatcher._checkResultString(func(self.id, str))
    return newfunc

  def _member_string_buf (self, func):
    dispatcher = self
    def newfunc (self):
      dispatcher._setSID()
      wb = dispatcher.writeBuffer()
      dispatcher._checkResult(func(self.id, wb.id))
      res = dispatcher._checkResultString(dispatcher._lib.indigoToString(wb.id))
      return res
    return newfunc

  def _member_string_file (self, func):
    dispatcher = self
    def newfunc (self, filename):
      dispatcher._setSID()
      wf = dispatcher.writeFile(filename)
      res = dispatcher._checkResult(func(self.id, wf.id))
      return res
    return newfunc
    
  def _member_float (self, func):
    dispatcher = self
    def newfunc (self):
      dispatcher._setSID()
      return dispatcher._checkResultFloat(func(self.id))
    return newfunc

  def _member_void (self, func):
    dispatcher = self
    def newfunc (self):
      dispatcher._setSID()
      dispatcher._checkResult(func(self.id))
      return
    return newfunc

  def _member_void_obj (self, func):
    dispatcher = self
    def newfunc (self, other):
      dispatcher._setSID()
      dispatcher._checkResult(func(self.id, other.id))
      return
    return newfunc

  def _member_void_string (self, func):
    dispatcher = self
    def newfunc (self, str):
      dispatcher._setSID()
      dispatcher._checkResult(func(self.id, str))
      return
    return newfunc

  def _member_void_string_string (self, func):
    dispatcher = self
    def newfunc (self, str1, str2):
      dispatcher._setSID()
      dispatcher._checkResult(func(self.id, str1, str2))
      return
    return newfunc

  def _member_int (self, func):
    dispatcher = self
    def newfunc (self):
      dispatcher._setSID()
      return dispatcher._checkResult(func(self.id))
    return newfunc

  def _member_intptr (self, func):
    dispatcher = self
    def newfunc (self):
      dispatcher._setSID()
      value = c_int()
      res = dispatcher._checkResult(func(self.id, pointer(value)))
      if res == 0:
        return None
      return value.value
    return newfunc

  def _member_intz (self, func):
    dispatcher = self
    def newfunc (self):
      dispatcher._setSID()
      res = dispatcher._checkResult(func(self.id))
      if res == 0:
        return None
      return res
    return newfunc

  def _member_obj (self, func):
    dispatcher = self
    def newfunc (self):
      dispatcher._setSID()
      newobj = dispatcher._checkResult(func(self.id))
      return dispatcher.IndigoObject(dispatcher, newobj)
    return newfunc

  def _member_obj_int (self, func):
    dispatcher = self
    def newfunc (self, param):
      dispatcher._setSID()
      newobj = dispatcher._checkResult(func(self.id, param))
      return dispatcher.IndigoObject(dispatcher, newobj)
    return newfunc

  def _member_obj_obj (self, func):
    dispatcher = self
    def newfunc (self, param):
      dispatcher._setSID()
      newobj = dispatcher._checkResult(func(self.id, param.id))
      return dispatcher.IndigoObject(dispatcher, newobj)
    return newfunc

  def _member_obj_string (self, func):
    dispatcher = self
    def newfunc (self, param):
      dispatcher._setSID()
      newobj = dispatcher._checkResult(func(self.id, param))
      return dispatcher.IndigoObject(dispatcher, newobj)
    return newfunc

  def _member_obj_iarr (self, func):
    dispatcher = self
    def newfunc (self, intarr):
      arr = (c_int * len(intarr))()
      for i in xrange(len(intarr)):
        arr[i] = intarr[i]
      dispatcher._setSID()
      newobj = dispatcher._checkResult(func(self.id, len(intarr), arr))
      return dispatcher.IndigoObject(dispatcher, newobj)
    return newfunc

  def _member_obj_iarr_iarr (self, func):
    dispatcher = self
    def newfunc (self, intarr1, intarr2):
      arr1 = (c_int * len(intarr1))()
      for i in xrange(len(intarr1)):
        arr1[i] = intarr1[i]
      arr2 = (c_int * len(intarr2))()
      for i in xrange(len(intarr2)):
        arr2[i] = intarr2[i]
      dispatcher._setSID()
      newobj = dispatcher._checkResult(func(self.id, len(intarr1), arr1, len(intarr2), arr2))
      return dispatcher.IndigoObject(dispatcher, newobj)
    return newfunc

  def version (self):
    return self._lib.indigoVersion()

  def alignAtoms (self, mol, atom_ids, desired_xyz):
    self._lib.indigoSetSessionId(self._sid)
    self._checkResultFloat
    if len(atom_ids) * 3 != len(desired_xyz):
      raise IndigoException("alignAtoms(): desired_xyz[] must be exactly 3 times bigger than atom_ids[]")
    atoms = (c_int * len(atom_ids))()
    for i in xrange(len(atoms)):
      atoms[i] = atom_ids[i]
    xyz = (c_float * len(desired_xyz))()
    for i in xrange(len(desired_xyz)):
      xyz[i] = desired_xyz[i]
    return self._checkResultFloat(
      self._lib.indigoAlignAtoms(mol.id, len(atoms), atoms, xyz))

  def similarity (self, item1, item2, metrics = None):
    self._lib.indigoSetSessionId(self._sid)
    if metrics is None:
      metrics = ''
    return self._checkResultFloat(self._lib.indigoSimilarity(item1.id, item2.id, metrics))

  def exactMatch (self, item1, item2):
    self._lib.indigoSetSessionId(self._sid)
    res = self._checkResult(self._lib.indigoExactMatch(item1.id, item2.id))
    return res == 1

  def writeBuffer (self):
    id = self._checkResult(self._lib.indigoWriteBuffer())
    return self.IndigoObject(self, id)

  def writeFile (self, filename):
    id = self._checkResult(self._lib.indigoWriteFile(filename))
    return self.IndigoObject(self, id)

  def setOption (self, option, value1, value2 = None, value3 = None):
    if type(value1).__name__ == 'str' and value2 is None and value3 is None:
      self._checkResult(self._lib.indigoSetOption(option, value1))
    elif type(value1).__name__ == 'int' and value2 is None and value3 is None:
      self._checkResult(self._lib.indigoSetOptionInt(option, value1))
    elif type(value1).__name__ == 'float' and value2 is None and value3 is None:
      self._checkResult(self._lib.indigoSetOptionFloat(option, value1))
    elif type(value1).__name__ == 'bool' and value2 is None and value3 is None:
      self._checkResult(self._lib.indigoSetOptionBool(option, 1 if value1 else 0))
    elif type(value1).__name__ == 'int' and value2 and \
         type(value2).__name__ == 'int' and value3 is None:
      self._checkResult(self._lib.indigoSetOptionXY(option, value1, value2))
    elif type(value1).__name__ == 'float' and value2 and \
         type(value2).__name__ == 'float' and value3 and \
         type(value3).__name__ == 'float':
      self._checkResult(self._lib.indigoSetOptionColor(option, value1, value2, value3))
    else:
      raise IndigoException("bad options")

  def _checkResult (self, result):
    if result < 0:
      raise IndigoException(self._lib.indigoGetLastError())
    return result

  def _checkResultFloat (self, result):
    if result < -0.5:
      raise IndigoException(self._lib.indigoGetLastError())
    return result

  def _checkResultString (self, result):
    if result is None:
      raise IndigoException(self._lib.indigoGetLastError())
    return result

  def __del__ (self):
    if hasattr(self, '_lib'):
      self._lib.indigoReleaseSessionId(self._sid)
