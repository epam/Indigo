#
#
# Copyright (C) 2010-2011 GGA Software Services LLC
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
import new

from array import *
from ctypes import *

class IndigoException (Exception):
  def __init__(self, value):
    self.value = value
  def __str__(self):
    return repr(self.value)

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

  RC_NOT_CENTER     = -1
  RC_UNMARKED       =  0
  RC_CENTER         =  1
  RC_UNCHANGED      =  2
  RC_MADE_OR_BROKEN =  4
  RC_ORDER_CHANGED  =  8

  _crt = None
  _lib = None
  
  class IndigoObject(object):
    def __init__ (self, dispatcher, id, parent = None):
      self.id = id
      self.dispatcher = dispatcher
      self.parent = parent
    
    def grossFormula (self):
      self.dispatcher._setSID()
      gfid = self.dispatcher._checkResult(Indigo._lib.indigoGrossFormula(self.id))
      gf = Indigo.IndigoObject(self.dispatcher, gfid)
      return self.dispatcher._checkResultString(Indigo._lib.indigoToString(gf.id))

    def toString (self):
      self.dispatcher._setSID()
      return self.dispatcher._checkResultString(Indigo._lib.indigoToString(self.id))

    def mdlct (self):
      self.dispatcher._setSID()
      buf = self.dispatcher.writeBuffer()
      self.dispatcher._checkResult(Indigo._lib.indigoSaveMDLCT(self.id, buf.id))
      return buf.toBuffer()
    
    def xyz (self):
      self.dispatcher._setSID()
      xyz = Indigo._lib.indigoXYZ(self.id)
      if xyz is None:
        raise IndigoException(Indigo._lib.indigoGetLastError())
      return [xyz[0], xyz[1], xyz[2]]

    def setXYZ (self, x, y, z):
      self.dispatcher._setSID()
      self.dispatcher._checkResult(Indigo._lib.indigoSetXYZ(self.id, x, y, z))

    def alignAtoms (self, atom_ids, desired_xyz):
      self.dispatcher._setSID()
      if len(atom_ids) * 3 != len(desired_xyz):
        raise IndigoException("alignAtoms(): desired_xyz[] must be exactly 3 times bigger than atom_ids[]")
      atoms = (c_int * len(atom_ids))()
      for i in xrange(len(atoms)):
        atoms[i] = atom_ids[i]
      xyz = (c_float * len(desired_xyz))()
      for i in xrange(len(desired_xyz)):
        xyz[i] = desired_xyz[i]
      return self.dispatcher._checkResultFloat(
        Indigo._lib.indigoAlignAtoms(self.id, len(atoms), atoms, xyz))

    def __enter__ (self):
      return self
    def __exit__ (self, exc_type, exc_value, traceback):
      self.dispatcher._setSID()
      Indigo._lib.indigoClose(self.id)
    def __del__ (self):
      if getattr(Indigo, "_lib", None) is not None:
        self.dispatcher._setSID()
        Indigo._lib.indigoFree(self.id)
        
    def __iter__ (self):
      return self
    def __next__ (self):
      obj = self._next()
      if obj == None:
        raise StopIteration
      return obj
    def next (self):
      return self.__next__()

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
      Indigo._lib = CDLL(path + "/indigo.dll")
    elif platform.mac_ver()[0]:
      path += "/Mac/"
      # append "10.5" or "10.6" to the path
      mac_ver = '.'.join(platform.mac_ver()[0].split('.')[:2])
      if mac_ver == "10.7":
         mac_ver = "10.6" # Try to use 10.6 Indigo version for Mac OS X 10.7
      path += mac_ver
      Indigo._lib = CDLL(path + "/libindigo.dylib", mode=RTLD_GLOBAL)
    else:
      raise IndigoException("unsupported OS: " + os.name)

    Indigo.dllpath = path

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
    Indigo._lib.indigoSetErrorMessage.restype = None
    Indigo._lib.indigoSetErrorMessage.argtypes = [c_char_p]
    Indigo._lib.indigoFree.restype = c_int
    Indigo._lib.indigoFree.argtypes = [c_int]
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
    Indigo._lib.indigoWriteFile.restype = c_int
    Indigo._lib.indigoWriteFile.argtypes = [c_char_p]
    Indigo._lib.indigoWriteBuffer.restype = c_int
    Indigo._lib.indigoWriteBuffer.argtypes = None
    Indigo._lib.indigoSaveMDLCT.restype = c_int
    Indigo._lib.indigoSaveMDLCT.argtypes = [c_int, c_int]
    Indigo._lib.indigoUnserialize.restype = c_int
    Indigo._lib.indigoUnserialize.argtypes = [POINTER(c_char), c_int]
    Indigo._lib.indigoXYZ.restype = POINTER(c_float)
    Indigo._lib.indigoXYZ.argtypes = [c_int]
    Indigo._lib.indigoSetXYZ.restype = c_int
    Indigo._lib.indigoSetXYZ.argtypes = [c_int, c_float, c_float, c_float]
    Indigo._lib.indigoAlignAtoms.restype = c_float
    Indigo._lib.indigoAlignAtoms.argtypes = [c_int, c_int, POINTER(c_int), POINTER(c_float)]
    Indigo._lib.indigoToString.restype = c_char_p
    Indigo._lib.indigoToString.argtypes = [c_int]
    Indigo._lib.indigoSimilarity.restype = c_float
    Indigo._lib.indigoSimilarity.argtypes = [c_int, c_int, c_char_p]
    Indigo._lib.indigoDbgBreakpoint.restype = None
    Indigo._lib.indigoDbgBreakpoint.argtypes = None
    
  def __init__ (self, path = None):
    if Indigo._lib == None:
      self._initStatic(path)
      
    self._sid = Indigo._lib.indigoAllocSessionId()
    self._setSID()
    
    self.countReferences = self._static_int(self._lib.indigoCountReferences)
    self.createMolecule = self._static_obj(self._lib.indigoCreateMolecule)
    self.createQueryMolecule = self._static_obj(self._lib.indigoCreateQueryMolecule)
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
    self.loadReactionSmarts = self._static_obj_string(self._lib.indigoLoadReactionSmartsFromString)
    self.loadReactionSmartsFromFile = self._static_obj_string(self._lib.indigoLoadReactionSmartsFromFile)
    self.createReaction = self._static_obj(self._lib.indigoCreateReaction)
    self.createQueryReaction = self._static_obj(self._lib.indigoCreateQueryReaction)
    self.createArray = self._static_obj(self._lib.indigoCreateArray)

    self.commonBits = self._static_int_obj_obj(self._lib.indigoCommonBits)

    self.iterateSDFile = self._static_obj_string(self._lib.indigoIterateSDFile)
    self.iterateRDFile = self._static_obj_string(self._lib.indigoIterateRDFile)
    self.iterateSmilesFile = self._static_obj_string(self._lib.indigoIterateSmilesFile)
    self.iterateCMLFile = self._static_obj_string(self._lib.indigoIterateCMLFile)

    self.substructureMatcher = self._static_obj_obj_string(self._lib.indigoSubstructureMatcher)
    self.exactMatch = self._static_obj_obj_obj_string(self._lib.indigoExactMatch)
    self.setTautomerRule = self._static_void_int_string_string(self._lib.indigoSetTautomerRule)
    self.removeTautomerRule = self._static_void_int(self._lib.indigoRemoveTautomerRule)
    self.clearTautomerRules = self._static_void(self._lib.indigoClearTautomerRules)
    
    self.extractCommonScaffold = self._static_obj_array_string(self._lib.indigoExtractCommonScaffold)
    self.decomposeMolecules = self._static_obj_obj_array(self._lib.indigoDecomposeMolecules)
    self.createDecomposer = self._static_obj_obj(self._lib.indigoCreateDecomposer)
    self.reactionProductEnumerate = self._static_obj_obj_array(self._lib.indigoReactionProductEnumerate)
    self.transform = self._static_void_obj_obj(self._lib.indigoTransform)
    
    self.createFileSaver = self._static_obj_string_string(self._lib.indigoCreateFileSaver)

    self.dbgBreakpoint = Indigo._lib.indigoDbgBreakpoint
    self.IndigoObject.dbgInternalType = Indigo._member_string(Indigo._lib.indigoDbgInternalType)
    
    self.IndigoObject.close = Indigo._member_void(Indigo._lib.indigoClose)
    self.IndigoObject.clone = Indigo._member_obj(Indigo._lib.indigoClone)

    self.IndigoObject.molfile = Indigo._member_string(Indigo._lib.indigoMolfile)
    self.IndigoObject.saveMolfile = Indigo._member_void_string(Indigo._lib.indigoSaveMolfileToFile)
    self.IndigoObject.cml = Indigo._member_string(Indigo._lib.indigoCml)
    self.IndigoObject.saveCml = Indigo._member_void_string(Indigo._lib.indigoSaveCmlToFile)
    self.IndigoObject.addReactant = Indigo._member_void_obj(Indigo._lib.indigoAddReactant)
    self.IndigoObject.addProduct  = Indigo._member_void_obj(Indigo._lib.indigoAddProduct)
    self.IndigoObject.addCatalyst  = Indigo._member_void_obj(Indigo._lib.indigoAddCatalyst)
    self.IndigoObject.countReactants = Indigo._member_int(Indigo._lib.indigoCountReactants)
    self.IndigoObject.countProducts = Indigo._member_int(Indigo._lib.indigoCountProducts)
    self.IndigoObject.countCatalysts = Indigo._member_int(Indigo._lib.indigoCountCatalysts)
    self.IndigoObject.countMolecules = Indigo._member_int(Indigo._lib.indigoCountMolecules)
    self.IndigoObject.iterateReactants = Indigo._member_obj(Indigo._lib.indigoIterateReactants)
    self.IndigoObject.iterateProducts  = Indigo._member_obj(Indigo._lib.indigoIterateProducts)
    self.IndigoObject.iterateCatalysts  = Indigo._member_obj(Indigo._lib.indigoIterateCatalysts)
    self.IndigoObject.iterateMolecules  = Indigo._member_obj(Indigo._lib.indigoIterateMolecules)

    self.IndigoObject.optimize = Indigo._member_void_string(Indigo._lib.indigoOptimize)
    
    self.IndigoObject.rxnfile = Indigo._member_string(Indigo._lib.indigoRxnfile)
    self.IndigoObject.saveRxnfile = Indigo._member_void_string(Indigo._lib.indigoSaveRxnfileToFile)
    self.IndigoObject.automap = Indigo._member_void_string(Indigo._lib.indigoAutomap)

    self.IndigoObject.atomMappingNumber = Indigo._member_int_obj(Indigo._lib.indigoGetAtomMappingNumber)
    self.IndigoObject.setAtomMappingNumber = Indigo._member_void_obj_int(Indigo._lib.indigoSetAtomMappingNumber)
    self.IndigoObject.reactingCenter = Indigo._member_intptr_obj(Indigo._lib.indigoGetReactingCenter)
    self.IndigoObject.setReactingCenter = Indigo._member_void_obj_int(Indigo._lib.indigoSetReactingCenter)
    self.IndigoObject.clearAAM = Indigo._member_void(Indigo._lib.indigoClearAAM)
    self.IndigoObject.correctReactingCenters = Indigo._member_void(Indigo._lib.indigoCorrectReactingCenters)
    
    self.IndigoObject.iterateAtoms = Indigo._member_obj(Indigo._lib.indigoIterateAtoms)
    self.IndigoObject.iteratePseudoatoms = Indigo._member_obj(Indigo._lib.indigoIteratePseudoatoms)
    self.IndigoObject.iterateRSites = Indigo._member_obj(Indigo._lib.indigoIterateRSites)
    self.IndigoObject.iterateStereocenters = Indigo._member_obj(Indigo._lib.indigoIterateStereocenters)
    self.IndigoObject.iterateAlleneCenters = Indigo._member_obj(Indigo._lib.indigoIterateAlleneCenters)
    self.IndigoObject.iterateRGroups = Indigo._member_obj(Indigo._lib.indigoIterateRGroups)
    self.IndigoObject.iterateRGroupFragments = Indigo._member_obj(Indigo._lib.indigoIterateRGroupFragments)
    self.IndigoObject.countAttachmentPoints = Indigo._member_int(Indigo._lib.indigoCountAttachmentPoints)
    self.IndigoObject.iterateAttachmentPoints = Indigo._member_obj_int(Indigo._lib.indigoIterateAttachmentPoints)
    
    self.IndigoObject.isPseudoatom = Indigo._member_bool(Indigo._lib.indigoIsPseudoatom)
    self.IndigoObject.isRSite = Indigo._member_bool(Indigo._lib.indigoIsRSite)
    self.IndigoObject.stereocenterType = Indigo._member_int(Indigo._lib.indigoStereocenterType)
    self.IndigoObject.singleAllowedRGroup = Indigo._member_int(Indigo._lib.indigoSingleAllowedRGroup)
    self.IndigoObject.symbol = Indigo._member_string(Indigo._lib.indigoSymbol)

    self.IndigoObject.changeStereocenterType = Indigo._member_void_int(Indigo._lib.indigoChangeStereocenterType)
    self.IndigoObject.addStereocenter = Indigo._member_int_int_int_int_dint(Indigo._lib.indigoAddStereocenter)

    self.IndigoObject.degree = Indigo._member_int(Indigo._lib.indigoDegree)
    self.IndigoObject.charge = Indigo._member_intptr(Indigo._lib.indigoGetCharge)
    self.IndigoObject.explicitValence = Indigo._member_intptr(Indigo._lib.indigoGetExplicitValence)
    self.IndigoObject.radicalElectrons = Indigo._member_intptr(Indigo._lib.indigoGetRadicalElectrons)
    self.IndigoObject.atomicNumber = Indigo._member_int(Indigo._lib.indigoAtomicNumber)
    self.IndigoObject.isotope = Indigo._member_int(Indigo._lib.indigoIsotope)
    self.IndigoObject.valence = Indigo._member_int(Indigo._lib.indigoValence)
    self.IndigoObject.countHydrogens = Indigo._member_intptr(Indigo._lib.indigoCountHydrogens)
    self.IndigoObject.countImplicitHydrogens = Indigo._member_int(Indigo._lib.indigoCountImplicitHydrogens)

    self.IndigoObject.countSuperatoms = Indigo._member_int(Indigo._lib.indigoCountSuperatoms)
    self.IndigoObject.countDataSGroups = Indigo._member_int(Indigo._lib.indigoCountDataSGroups)
    self.IndigoObject.countRepeatingUnits = Indigo._member_int(Indigo._lib.indigoCountRepeatingUnits)
    self.IndigoObject.countMultipleGroups = Indigo._member_int(Indigo._lib.indigoCountMultipleGroups)
    self.IndigoObject.countGenericSGroups = Indigo._member_int(Indigo._lib.indigoCountGenericSGroups)
    
    self.IndigoObject.iterateSuperatoms = Indigo._member_obj(Indigo._lib.indigoIterateSuperatoms)
    self.IndigoObject.iterateDataSGroups = Indigo._member_obj(Indigo._lib.indigoIterateDataSGroups)
    self.IndigoObject.getSuperatom = Indigo._member_obj_int(Indigo._lib.indigoGetSuperatom)
    self.IndigoObject.getDataSGroup = Indigo._member_obj_int(Indigo._lib.indigoGetDataSGroup)
    self.IndigoObject.description = Indigo._member_string(Indigo._lib.indigoDescription)
    self.IndigoObject.remove = Indigo._member_void(Indigo._lib.indigoRemove)

    self.IndigoObject.resetCharge = Indigo._member_void(Indigo._lib.indigoResetCharge)
    self.IndigoObject.resetExplicitValence = Indigo._member_void(Indigo._lib.indigoResetExplicitValence)
    self.IndigoObject.resetRadical = Indigo._member_void(Indigo._lib.indigoResetRadical)
    self.IndigoObject.resetIsotope = Indigo._member_void(Indigo._lib.indigoResetIsotope)
    self.IndigoObject.resetStereo = Indigo._member_void(Indigo._lib.indigoResetStereo)
    self.IndigoObject.invertStereo = Indigo._member_void(Indigo._lib.indigoInvertStereo)

    self.IndigoObject.setAttachmentPoint = Indigo._member_void_int(Indigo._lib.indigoSetAttachmentPoint)
    self.IndigoObject.clearAttachmentPoints = Indigo._member_void(Indigo._lib.indigoClearAttachmentPoints)

    self.IndigoObject.removeConstraints = Indigo._member_void_string(Indigo._lib.indigoRemoveConstraints)
    self.IndigoObject.addConstraint = Indigo._member_void_string_string(Indigo._lib.indigoAddConstraint)
    self.IndigoObject.addConstraintNot = Indigo._member_void_string_string(Indigo._lib.indigoAddConstraintNot)
    self.IndigoObject.addConstraintOr = Indigo._member_void_string_string(Indigo._lib.indigoAddConstraintOr)

    self.IndigoObject.countAtoms = Indigo._member_int(Indigo._lib.indigoCountAtoms)
    self.IndigoObject.countBonds = Indigo._member_int(Indigo._lib.indigoCountBonds)
    self.IndigoObject.countPseudoatoms = Indigo._member_int(Indigo._lib.indigoCountPseudoatoms)
    self.IndigoObject.countRSites = Indigo._member_int(Indigo._lib.indigoCountRSites)

    self.IndigoObject.iterateBonds = Indigo._member_obj(Indigo._lib.indigoIterateBonds)
    self.IndigoObject.bondOrder = Indigo._member_int(Indigo._lib.indigoBondOrder)
    self.IndigoObject.topology = Indigo._member_int(Indigo._lib.indigoTopology)
    self.IndigoObject.bondStereo = Indigo._member_int(Indigo._lib.indigoBondStereo)

    self.IndigoObject.iterateNeighbors = Indigo._member_obj(Indigo._lib.indigoIterateNeighbors)
    self.IndigoObject.bond = Indigo._member_obj(Indigo._lib.indigoBond)
    self.IndigoObject.getAtom = Indigo._member_obj_int(Indigo._lib.indigoGetAtom)
    self.IndigoObject.getBond = Indigo._member_obj_int(Indigo._lib.indigoGetBond)
    self.IndigoObject.source = Indigo._member_obj(Indigo._lib.indigoSource)
    self.IndigoObject.destination = Indigo._member_obj(Indigo._lib.indigoDestination)

    self.IndigoObject.getMolecule = Indigo._member_obj_int(Indigo._lib.indigoGetMolecule)

    self.IndigoObject.clearCisTrans = Indigo._member_void(Indigo._lib.indigoClearCisTrans)
    self.IndigoObject.clearStereocenters = Indigo._member_void(Indigo._lib.indigoClearStereocenters)
    self.IndigoObject.clearAlleneCenters = Indigo._member_void(Indigo._lib.indigoClearAlleneCenters)
    self.IndigoObject.countStereocenters = Indigo._member_int(Indigo._lib.indigoCountStereocenters)
    self.IndigoObject.countAlleneCenters = Indigo._member_int(Indigo._lib.indigoCountAlleneCenters)
    self.IndigoObject.resetSymmetricCisTrans = Indigo._member_int(Indigo._lib.indigoResetSymmetricCisTrans)
    self.IndigoObject.markEitherCisTrans = Indigo._member_int(Indigo._lib.indigoMarkEitherCisTrans)
    self.IndigoObject.addAtom = Indigo._member_obj_string(Indigo._lib.indigoAddAtom)
    self.IndigoObject.resetAtom = Indigo._member_void_string(Indigo._lib.indigoResetAtom)
    self.IndigoObject.addRSite = Indigo._member_obj_string(Indigo._lib.indigoAddRSite)
    self.IndigoObject.setRSite = Indigo._member_void_string(Indigo._lib.indigoSetRSite)
    self.IndigoObject.setCharge = Indigo._member_void_int(Indigo._lib.indigoSetCharge)
    self.IndigoObject.setIsotope = Indigo._member_void_int(Indigo._lib.indigoSetIsotope)
    self.IndigoObject.setImplicitHCount = Indigo._member_void_int(Indigo._lib.indigoSetImplicitHCount)
    self.IndigoObject.addBond = Indigo._member_obj_obj_int(Indigo._lib.indigoAddBond)
    self.IndigoObject.setBondOrder = Indigo._member_void_int(Indigo._lib.indigoSetBondOrder)
    self.IndigoObject.merge = Indigo._member_obj_obj(Indigo._lib.indigoMerge)
    self.IndigoObject.highlight = Indigo._member_void(Indigo._lib.indigoHighlight)
    self.IndigoObject.unhighlight = Indigo._member_void(Indigo._lib.indigoUnhighlight)
    self.IndigoObject.isHighlighted = Indigo._member_bool(Indigo._lib.indigoIsHighlighted)

    self.IndigoObject.countComponents = Indigo._member_int(Indigo._lib.indigoCountComponents)
    self.IndigoObject.componentIndex = Indigo._member_int(Indigo._lib.indigoComponentIndex)
    self.IndigoObject.component = Indigo._member_obj_int(Indigo._lib.indigoComponent)
    self.IndigoObject.iterateComponents = Indigo._member_obj(Indigo._lib.indigoIterateComponents)
    self.IndigoObject.countSSSR = Indigo._member_int(Indigo._lib.indigoCountSSSR)
    self.IndigoObject.iterateSSSR = Indigo._member_obj(Indigo._lib.indigoIterateSSSR)
    self.IndigoObject.iterateRings = Indigo._member_obj_int_int(Indigo._lib.indigoIterateRings)
    self.IndigoObject.iterateSubtrees = Indigo._member_obj_int_int(Indigo._lib.indigoIterateSubtrees)
    self.IndigoObject.iterateEdgeSubmolecules = Indigo._member_obj_int_int(Indigo._lib.indigoIterateEdgeSubmolecules)

    self.IndigoObject.countHeavyAtoms = Indigo._member_int(Indigo._lib.indigoCountHeavyAtoms)
    self.IndigoObject.molecularWeight = Indigo._member_float(Indigo._lib.indigoMolecularWeight)
    self.IndigoObject.monoisotopicMass = Indigo._member_float(Indigo._lib.indigoMonoisotopicMass)
    self.IndigoObject.mostAbundantMass = Indigo._member_float(Indigo._lib.indigoMostAbundantMass)

    self.IndigoObject.serialize = Indigo._member_buf(Indigo._lib.indigoSerialize)
    self.IndigoObject.toBuffer = Indigo._member_buf(Indigo._lib.indigoToBuffer)

    self.IndigoObject.canonicalSmiles = Indigo._member_string(Indigo._lib.indigoCanonicalSmiles)
    self.IndigoObject.layeredCode = Indigo._member_string(Indigo._lib.indigoLayeredCode)
    self.IndigoObject.symmetryClasses = Indigo._member_intbuf(Indigo._lib.indigoSymmetryClasses)

    self.IndigoObject.hasCoord = Indigo._member_bool(Indigo._lib.indigoHasCoord)
    self.IndigoObject.hasZCoord = Indigo._member_bool(Indigo._lib.indigoHasZCoord)
    self.IndigoObject.isChiral = Indigo._member_bool(Indigo._lib.indigoIsChiral)
    
    self.IndigoObject.aromatize = Indigo._member_bool(Indigo._lib.indigoAromatize)
    self.IndigoObject.dearomatize = Indigo._member_bool(Indigo._lib.indigoDearomatize)
    self.IndigoObject.foldHydrogens = Indigo._member_void(Indigo._lib.indigoFoldHydrogens)
    self.IndigoObject.unfoldHydrogens = Indigo._member_void(Indigo._lib.indigoUnfoldHydrogens)
    self.IndigoObject.layout = Indigo._member_void(Indigo._lib.indigoLayout)    

    self.IndigoObject.smiles = Indigo._member_string(Indigo._lib.indigoSmiles)
    self.IndigoObject.name = Indigo._member_string(Indigo._lib.indigoName)
    self.IndigoObject.setName = Indigo._member_void_string(Indigo._lib.indigoSetName)
    self.IndigoObject.hasProperty = Indigo._member_bool_string(Indigo._lib.indigoHasProperty)
    self.IndigoObject.getProperty = Indigo._member_string_string(Indigo._lib.indigoGetProperty)
    self.IndigoObject.setProperty = Indigo._member_void_string_string(Indigo._lib.indigoSetProperty)
    self.IndigoObject.removeProperty = Indigo._member_void_string(Indigo._lib.indigoRemoveProperty)
    self.IndigoObject.iterateProperties = Indigo._member_obj(Indigo._lib.indigoIterateProperties)
    self.IndigoObject.clearProperties = Indigo._member_obj(Indigo._lib.indigoClearProperties)

    self.IndigoObject.checkBadValence = Indigo._member_string(Indigo._lib.indigoCheckBadValence)
    self.IndigoObject.checkAmbiguousH = Indigo._member_string(Indigo._lib.indigoCheckAmbiguousH)
    self.IndigoObject.rawData = Indigo._member_string(Indigo._lib.indigoRawData)

    self.IndigoObject.fingerprint = Indigo._member_obj_string(Indigo._lib.indigoFingerprint)
    self.IndigoObject.countBits = Indigo._member_int(Indigo._lib.indigoCountBits)
    self.IndigoObject.tell = Indigo._member_int(Indigo._lib.indigoTell)
    self.IndigoObject.sdfAppend = Indigo._member_void_obj(Indigo._lib.indigoSdfAppend)
    self.IndigoObject.smilesAppend = Indigo._member_void_obj(Indigo._lib.indigoSmilesAppend)
    self.IndigoObject.rdfHeader = Indigo._member_void(Indigo._lib.indigoRdfHeader)
    self.IndigoObject.rdfAppend = Indigo._member_void_obj(Indigo._lib.indigoRdfAppend)
    self.IndigoObject.cmlHeader = Indigo._member_void(Indigo._lib.indigoCmlHeader)
    self.IndigoObject.cmlAppend = Indigo._member_void_obj(Indigo._lib.indigoCmlAppend)
    self.IndigoObject.cmlFooter = Indigo._member_void(Indigo._lib.indigoCmlFooter)
    self.IndigoObject.append = Indigo._member_void_obj(Indigo._lib.indigoAppend)

    self.IndigoObject.iterateArray = Indigo._member_obj(Indigo._lib.indigoIterateArray)
    self.IndigoObject.count = Indigo._member_int(Indigo._lib.indigoCount)
    self.IndigoObject.clear = Indigo._member_void(Indigo._lib.indigoClear)
    self.IndigoObject.arrayAdd = Indigo._member_int_obj(Indigo._lib.indigoArrayAdd)
    self.IndigoObject.at = Indigo._member_obj_int(Indigo._lib.indigoAt)
    
    self.IndigoObject.match = Indigo._member_obj_obj(Indigo._lib.indigoMatch)
    self.IndigoObject.ignoreAtom = Indigo._member_void_obj(Indigo._lib.indigoIgnoreAtom)
    self.IndigoObject.unignoreAtom = Indigo._member_void_obj(Indigo._lib.indigoUnignoreAtom)
    self.IndigoObject.unignoreAllAtoms = Indigo._member_void(Indigo._lib.indigoUnignoreAllAtoms)
    self.IndigoObject.countMatches = Indigo._member_int_obj(Indigo._lib.indigoCountMatches)
    self.IndigoObject.countMatchesWithLimit = Indigo._member_int_obj_int(Indigo._lib.indigoCountMatchesWithLimit)
    
    self.IndigoObject.iterateMatches = Indigo._member_obj_obj(Indigo._lib.indigoIterateMatches)
    self.IndigoObject.highlightedTarget = Indigo._member_obj(Indigo._lib.indigoHighlightedTarget);
    self.IndigoObject.mapAtom = Indigo._member_obj_obj(Indigo._lib.indigoMapAtom);
    self.IndigoObject.mapBond = Indigo._member_obj_obj(Indigo._lib.indigoMapBond);	

    self.IndigoObject.mapMolecule = Indigo._member_obj_obj(Indigo._lib.indigoMapMolecule);
	
    self.IndigoObject.allScaffolds = Indigo._member_obj(Indigo._lib.indigoAllScaffolds);
    self.IndigoObject.decomposedMoleculeScaffold = Indigo._member_obj(Indigo._lib.indigoDecomposedMoleculeScaffold)
    self.IndigoObject.iterateDecomposedMolecules = Indigo._member_obj(Indigo._lib.indigoIterateDecomposedMolecules)
    self.IndigoObject.decomposedMoleculeHighlighted = Indigo._member_obj(Indigo._lib.indigoDecomposedMoleculeHighlighted)
    self.IndigoObject.decomposedMoleculeWithRGroups = Indigo._member_obj(Indigo._lib.indigoDecomposedMoleculeWithRGroups)
    self.IndigoObject.decomposeMolecule = Indigo._member_obj_obj(Indigo._lib.indigoDecomposeMolecule)
    self.IndigoObject.iterateDecompositions = Indigo._member_obj(Indigo._lib.indigoIterateDecompositions)
    self.IndigoObject.addDecomposition = Indigo._member_void_obj(Indigo._lib.indigoAddDecomposition)
    
    self.IndigoObject.index = Indigo._member_int(Indigo._lib.indigoIndex)

    self.IndigoObject.createSubmolecule = Indigo._member_obj_iarr(Indigo._lib.indigoCreateSubmolecule)
    self.IndigoObject.createEdgeSubmolecule = Indigo._member_obj_iarr_iarr(Indigo._lib.indigoCreateEdgeSubmolecule)
    self.IndigoObject.getSubmolecule = Indigo._member_obj_iarr(Indigo._lib.indigoGetSubmolecule)
    self.IndigoObject.removeAtoms = Indigo._member_void_iarr(Indigo._lib.indigoRemoveAtoms)
    self.IndigoObject.addDataSGroup = Indigo._member_obj_iarr_iarr_string_string(Indigo._lib.indigoAddDataSGroup)
    self.IndigoObject.addSuperatom = Indigo._member_obj_iarr_string(Indigo._lib.indigoAddSuperatom)
    self.IndigoObject.setDataSGroupXY = Indigo._member_void_float_float_string(Indigo._lib.indigoSetDataSGroupXY)

    self.IndigoObject._next = Indigo._member_obj(Indigo._lib.indigoNext)
    self.IndigoObject.hasNext = Indigo._member_bool(Indigo._lib.indigoHasNext)
    
  @staticmethod
  def _make_wrapper_func (wrapper, func):
    """Return wrapper function with changed name 
    """    
    name = func.__name__ + "_wrapper"
    c = wrapper.func_code
    newcode = new.code( c.co_argcount, c.co_nlocals, c.co_stacksize,
                        c.co_flags, c.co_code, c.co_consts, c.co_names,
                        c.co_varnames, "indigo core", name, 1, c.co_lnotab, c.co_freevars, c.co_cellvars )
               
    new_wrapper = new.function(newcode, globals(), name=name, closure=wrapper.func_closure, argdefs=wrapper.func_defaults)
    return new_wrapper
     
  def _static_obj (self, func):
    func.restype = c_int
    func.argtypes = []
    def newfunc ():
      self._setSID()
      res = self._checkResult(func())
      if res == 0:
        return None
      return Indigo.IndigoObject(self, res)
    return self._make_wrapper_func(newfunc, func)

  def _static_void (self, func):
    func.restype = c_int
    func.argtypes = []
    def newfunc ():
      self._setSID()
      self._checkResult(func())
    return self._make_wrapper_func(newfunc, func)

  def _static_void_int (self, func):
    func.restype = c_int
    func.argtypes = [ c_int ]
    def newfunc (param):
      self._setSID()
      self._checkResult(func(param))
    return self._make_wrapper_func(newfunc, func)

  def _static_void_int_string_string (self, func):
    func.restype = c_int
    func.argtypes = [c_int, c_char_p, c_char_p]
    def newfunc (param, str1 = None, str2 = None):
      self._setSID()
      if str1 is None:
        str1 = ''
      if str2 is None:
        str2 = ''
      res = func(param, str1, str2)
      if res == 0:
        return None
      return Indigo.IndigoObject(self, self._checkResult(res))
    return self._make_wrapper_func(newfunc, func)
  
  def _static_int (self, func):
    func.restype = c_int
    func.argtypes = []
    def newfunc ():
      self._setSID()
      return self._checkResult(func())
    return self._make_wrapper_func(newfunc, func)

  def _static_int_obj_obj (self, func):
    func.restype = c_int
    func.argtypes = [c_int, c_int]
    def newfunc (item1, item2):
      self._setSID()
      return self._checkResult(func(item1.id, item2.id))
    return self._make_wrapper_func(newfunc, func)
    
  def _static_void_obj_obj (self, func):
    func.restype = c_int
    func.argtypes = [c_int, c_int]
    def newfunc (item1, item2):
      self._setSID()
      self._checkResult(func(item1.id, item2.id))
    return self._make_wrapper_func(newfunc, func)
   
  def _static_obj_string (self, func):
    func.restype = c_int
    func.argtypes = [c_char_p]
    def newfunc (str):
      self._setSID()
      res = self._checkResult(func(str))
      if res == 0:
        return None
      return Indigo.IndigoObject(self, res)
    return self._make_wrapper_func(newfunc, func)

  def _static_obj_obj_string (self, func):
    func.restype = c_int
    func.argtypes = [c_int, c_char_p]
    def newfunc (obj, string = None):
      self._setSID()
      if string is None:
        string = ''
      res = func(obj.id, string)
      if res == 0:
        return None
      return Indigo.IndigoObject(self, self._checkResult(res), obj)
    return self._make_wrapper_func(newfunc, func)

  def _static_obj_obj_obj_string (self, func):
    func.restype = c_int
    func.argtypes = [c_int, c_int, c_char_p]
    def newfunc (obj1, obj2, string = None):
      self._setSID()
      if string is None:
        string = ''
      res = func(obj1.id, obj2.id, string)
      if res == 0:
        return None
      return Indigo.IndigoObject(self, self._checkResult(res), [obj1, obj2])
    return self._make_wrapper_func(newfunc, func)

  def _static_obj_string_string (self, func):
    func.restype = c_int
    func.argtypes = [c_char_p, c_char_p]
    def newfunc (string1, string2):
      self._setSID()
      if string1 is None:
        string1 = ''
      if string2 is None:
        string2 = ''
      res = func(string1, string2)
      if res == 0:
        return None
      return Indigo.IndigoObject(self, self._checkResult(res))
    return self._make_wrapper_func(newfunc, func)
    
  def _static_obj_array_string (self, func):
    func.restype = c_int
    func.argtypes = [c_int, c_char_p]
    def newfunc (obj_or_list, string = None):
      self._setSID()
      if string is None:
        string = ''
      obj_indigo_array = self.convertToArray(obj_or_list)
      res = func(obj_indigo_array.id, string)
      if res == 0:
        return None
      return Indigo.IndigoObject(self, self._checkResult(res))
    return self._make_wrapper_func(newfunc, func)

  def _static_obj_obj (self, func):
    func.restype = c_int
    func.argtypes = [c_int]
    def newfunc (obj):
      self._setSID()
      res = func(obj.id)
      if res == 0:
        return None
      return Indigo.IndigoObject(self, self._checkResult(res), obj)
    return self._make_wrapper_func(newfunc, func)
		
  def _static_obj_obj_obj (self, func):
    func.restype = c_int
    func.argtypes = [c_int, c_int]
    def newfunc (obj1, obj2):
      self._setSID()
      res = func(obj1.id, obj2.id)
      if res == 0:
        return None
      return Indigo.IndigoObject(self, self._checkResult(res), [obj1, obj2])
    return self._make_wrapper_func(newfunc, func)

  def _static_obj_obj_array (self, func):
    func.restype = c_int
    func.argtypes = [c_int, c_int]
    def newfunc (obj1, array_obj2):
      self._setSID()
      indigo_array_obj2 = self.convertToArray(array_obj2)
      res = func(obj1.id, indigo_array_obj2.id)
      if res == 0:
        return None
      return Indigo.IndigoObject(self, self._checkResult(res), obj1)
    return self._make_wrapper_func(newfunc, func)

  def _setSID (self):
    Indigo._lib.indigoSetSessionId(self._sid)

  @staticmethod
  def _member_string (func): 
    func.restype = c_char_p
    func.argtypes = [c_int]
    def newfunc (self):
      self.dispatcher._setSID()
      return self.dispatcher._checkResultString(func(self.id))
    return Indigo._make_wrapper_func(newfunc, func)

  @staticmethod
  def _member_buf (func):
    func.restype = c_int
    func.argtypes = [c_int, POINTER(POINTER(c_char)), POINTER(c_int)]
    def newfunc (self):
      self.dispatcher._setSID()
      c_size = c_int()
      c_buf = POINTER(c_char)()
      self.dispatcher._checkResult(func(self.id, pointer(c_buf), pointer(c_size)))
      res = array('c')
      for i in xrange(c_size.value):
        res.append(c_buf[i])
      return res
    return Indigo._make_wrapper_func(newfunc, func)

  @staticmethod
  def _member_intbuf (func):
    func.restype = POINTER(c_int)
    func.argtypes = [c_int, POINTER(c_int)]
    def newfunc (self):
      self.dispatcher._setSID()
      c_size = c_int()
      c_buf = self.dispatcher._checkResultPtr(func(self.id, pointer(c_size)))
      res = array('i')
      for i in xrange(c_size.value):
        res.append(c_buf[i])
      return res
    return Indigo._make_wrapper_func(newfunc, func)


  @staticmethod
  def _member_bool (func):
    func.restype = c_int
    func.argtypes = [c_int]
    def newfunc (self):
      self.dispatcher._setSID()
      res = self.dispatcher._checkResult(func(self.id))
      return res == 1                                                 
    return Indigo._make_wrapper_func(newfunc, func)

  @staticmethod
  def _member_bool_string (func):
    func.restype = c_int
    func.argtypes = [c_int, c_char_p]
    def newfunc (self, str):
      self.dispatcher._setSID()
      res = self.dispatcher._checkResult(func(self.id, str))
      return res == 1                                                 
    return Indigo._make_wrapper_func(newfunc, func)

  @staticmethod
  def _member_string_string (func):
    func.restype = c_char_p
    func.argtypes = [c_int, c_char_p]
    def newfunc (self, str):
      self.dispatcher._setSID()
      return self.dispatcher._checkResultString(func(self.id, str))
    return Indigo._make_wrapper_func(newfunc, func)

  @staticmethod
  def _member_float (func):
    func.restype = c_float
    func.argtypes = [c_int]
    def newfunc (self):
      self.dispatcher._setSID()
      return self.dispatcher._checkResultFloat(func(self.id))
    return Indigo._make_wrapper_func(newfunc, func)

  @staticmethod
  def _member_void (func):
    func.restype = c_int
    func.argtypes = [c_int]
    def newfunc (self):
      self.dispatcher._setSID()
      self.dispatcher._checkResult(func(self.id))
      return
    return Indigo._make_wrapper_func(newfunc, func)

  @staticmethod
  def _member_void_int (func):
    func.restype = c_int
    func.argtypes = [c_int]
    def newfunc (self, param):
      self.dispatcher._setSID()
      self.dispatcher._checkResult(func(self.id, param))
      return
    return Indigo._make_wrapper_func(newfunc, func)

  @staticmethod
  def _member_void_obj (func):
    func.restype = c_int
    func.argtypes = [c_int, c_int]
    def newfunc (self, other):
      self.dispatcher._setSID()
      self.dispatcher._checkResult(func(self.id, other.id))
      return
    return Indigo._make_wrapper_func(newfunc, func)

  @staticmethod
  def _member_void_obj_int (func):
    func.restype = c_int
    func.argtypes = [c_int, c_int, c_int]
    def newfunc (self, other, param):
      self.dispatcher._setSID()
      self.dispatcher._checkResult(func(self.id, other.id, param))
      return
    return Indigo._make_wrapper_func(newfunc, func)

  @staticmethod
  def _member_void_string (func):
    func.restype = c_int
    func.argtypes = [c_int, c_char_p]
    def newfunc (self, str = None):
      if str is None:
         str = ""
      self.dispatcher._setSID()
      self.dispatcher._checkResult(func(self.id, str))
      return
    return Indigo._make_wrapper_func(newfunc, func)

  @staticmethod
  def _member_void_string_string (func):
    func.restype = c_int
    func.argtypes = [c_int, c_char_p, c_char_p]
    def newfunc (self, str1, str2):
      self.dispatcher._setSID()
      self.dispatcher._checkResult(func(self.id, str1, str2))
      return
    return Indigo._make_wrapper_func(newfunc, func)

  @staticmethod
  def _member_int (func):
    func.restype = c_int
    func.argtypes = [c_int]
    def newfunc (self):
      self.dispatcher._setSID()
      return self.dispatcher._checkResult(func(self.id))
    return Indigo._make_wrapper_func(newfunc, func)

  @staticmethod
  def _member_intptr (func):
    func.restype = c_int
    func.argtypes = [c_int, POINTER(c_int)]
    def newfunc (self):
      self.dispatcher._setSID()
      value = c_int()
      res = self.dispatcher._checkResult(func(self.id, pointer(value)))
      if res == 0:
        return None
      return value.value
    return Indigo._make_wrapper_func(newfunc, func)

  @staticmethod
  def _member_intptr_obj (func):
    func.restype = c_int
    func.argtypes = [c_int, c_int, POINTER(c_int)]
    def newfunc (self, param):
      self.dispatcher._setSID()
      value = c_int()
      res = self.dispatcher._checkResult(func(self.id, param.id, pointer(value)))
      if res == 0:
        return None
      return value.value
    return Indigo._make_wrapper_func(newfunc, func)

  @staticmethod
  def _member_int_obj (func):
    func.restype = c_int
    func.argtypes = [c_int, c_int]
    def newfunc (self, param):
      self.dispatcher._setSID()
      return self.dispatcher._checkResult(func(self.id, param.id))
    return Indigo._make_wrapper_func(newfunc, func)
      
  @staticmethod
  def _member_int_obj_int (func):
    func.restype = c_int
    func.argtypes = [c_int, c_int, c_int]
    def newfunc (self, param, param2):
      self.dispatcher._setSID()
      return self.dispatcher._checkResult(func(self.id, param.id, param2))
    return Indigo._make_wrapper_func(newfunc, func)
   
  @staticmethod
  def _member_obj (func):
    func.restype = c_int
    func.argtypes = [c_int]
    def newfunc (self):
      self.dispatcher._setSID()
      newobj = self.dispatcher._checkResult(func(self.id))
      if newobj == 0:
        return None
      return self.dispatcher.IndigoObject(self.dispatcher, newobj, self)
    return Indigo._make_wrapper_func(newfunc, func)

  @staticmethod
  def _member_obj_int (func):
    func.restype = c_int
    func.argtypes = [c_int, c_int]
    def newfunc (self, param):
      self.dispatcher._setSID()
      newobj = self.dispatcher._checkResult(func(self.id, param))
      if newobj == 0:
        return None
      return self.dispatcher.IndigoObject(self.dispatcher, newobj, self)
    return Indigo._make_wrapper_func(newfunc, func)

  @staticmethod
  def _member_obj_int_int (func):
    func.restype = c_int
    func.argtypes = [c_int, c_int]
    def newfunc (self, param, param2):
      self.dispatcher._setSID()
      newobj = self.dispatcher._checkResult(func(self.id, param, param2))
      if newobj == 0:
        return None
      return self.dispatcher.IndigoObject(self.dispatcher, newobj, self)
    return Indigo._make_wrapper_func(newfunc, func)

  @staticmethod
  def _member_int_int_int_int_dint (func):
    func.restype = c_int
    func.argtypes = [c_int, c_int, c_int, c_int, c_int]
    def newfunc (self, param, v1, v2, v3, v4 = -1):
      self.dispatcher._setSID()
      self.dispatcher._checkResult(func(self.id, param, v1, v2, v3, v4))
    return Indigo._make_wrapper_func(newfunc, func)
    
    
  @staticmethod
  def _member_obj_obj (func):
    func.restype = c_int
    func.argtypes = [c_int, c_int]
    def newfunc (self, param):
      self.dispatcher._setSID()
      newobj = self.dispatcher._checkResult(func(self.id, param.id))
      if newobj == 0:
        return None
      return self.dispatcher.IndigoObject(self.dispatcher, newobj, self)
    return Indigo._make_wrapper_func(newfunc, func)

  @staticmethod
  def _member_obj_obj_int (func):
    func.restype = c_int
    func.argtypes = [c_int, c_int, c_int]
    def newfunc (self, param, param2):
      self.dispatcher._setSID()
      newobj = self.dispatcher._checkResult(func(self.id, param.id, param2))
      if newobj == 0:
        return None
      return self.dispatcher.IndigoObject(self.dispatcher, newobj, self)
    return Indigo._make_wrapper_func(newfunc, func)

  @staticmethod
  def _member_obj_string (func):
    func.restype = c_int
    func.argtypes = [c_int, c_char_p]
    def newfunc (self, param):
      self.dispatcher._setSID()
      newobj = self.dispatcher._checkResult(func(self.id, param))
      if newobj == 0:
        return None
      return self.dispatcher.IndigoObject(self.dispatcher, newobj, self)
    return Indigo._make_wrapper_func(newfunc, func)

  @staticmethod
  def _member_obj_iarr (func):
    func.restype = c_int
    func.argtypes = [c_int, c_int, POINTER(c_int)]
    def newfunc (self, intarr):
      arr = (c_int * len(intarr))()
      for i in xrange(len(intarr)):
        arr[i] = intarr[i]
      self.dispatcher._setSID()
      newobj = self.dispatcher._checkResult(func(self.id, len(intarr), arr))
      if newobj == 0:
        return None
      return self.dispatcher.IndigoObject(self.dispatcher, newobj, self)
    return Indigo._make_wrapper_func(newfunc, func)

  @staticmethod
  def _member_obj_iarr_iarr (func):
    func.restype = c_int
    func.argtypes = [c_int, c_int, POINTER(c_int), c_int, POINTER(c_int)]
    def newfunc (self, intarr1, intarr2):
      arr1 = (c_int * len(intarr1))()
      for i in xrange(len(intarr1)):
        arr1[i] = intarr1[i]
      arr2 = (c_int * len(intarr2))()
      for i in xrange(len(intarr2)):
        arr2[i] = intarr2[i]
      self.dispatcher._setSID()
      newobj = self.dispatcher._checkResult(func(self.id, len(intarr1), arr1, len(intarr2), arr2))
      if newobj == 0:
        return None
      return self.dispatcher.IndigoObject(self.dispatcher, newobj, self)
    return Indigo._make_wrapper_func(newfunc, func)

  @staticmethod
  def _member_obj_iarr_iarr_string_string (func):
    func.restype = c_int
    func.argtypes = [c_int, c_int, POINTER(c_int), c_int, POINTER(c_int), c_char_p, c_char_p]
    def newfunc (self, intarr1, intarr2, str1, str2):
      arr1 = (c_int * len(intarr1))()
      for i in xrange(len(intarr1)):
        arr1[i] = intarr1[i]
      arr2 = (c_int * len(intarr2))()
      for i in xrange(len(intarr2)):
        arr2[i] = intarr2[i]
      self.dispatcher._setSID()
      newobj = self.dispatcher._checkResult(func(self.id, len(intarr1), arr1, len(intarr2), arr2, str1, str2))
      if newobj == 0:
        return None
      return self.dispatcher.IndigoObject(self.dispatcher, newobj, self)
    return Indigo._make_wrapper_func(newfunc, func)

  @staticmethod
  def _member_obj_iarr_string (func):
    func.restype = c_int
    func.argtypes = [c_int, c_int, POINTER(c_int), c_char_p]
    def newfunc (self, intarr1, str1):
      arr1 = (c_int * len(intarr1))()
      for i in xrange(len(intarr1)):
        arr1[i] = intarr1[i]
      self.dispatcher._setSID()
      newobj = self.dispatcher._checkResult(func(self.id, len(intarr1), arr1, str1))
      if newobj == 0:
        return None
      return self.dispatcher.IndigoObject(self.dispatcher, newobj, self)
    return Indigo._make_wrapper_func(newfunc, func)
    
  @staticmethod
  def _member_void_iarr (func):
    func.restype = c_int
    func.argtypes = [c_int, c_int, POINTER(c_int)]
    def newfunc (self, intarr):
      arr = (c_int * len(intarr))()
      for i in xrange(len(intarr)):
        arr[i] = intarr[i]
      self.dispatcher._setSID()
      self.dispatcher._checkResult(func(self.id, len(intarr), arr))
    return Indigo._make_wrapper_func(newfunc, func)

  @staticmethod
  def _member_void_float_float_string (func):
    func.restype = c_int
    func.argtypes = [c_int, c_float, c_float, c_char_p]
    def newfunc (self, x, y, s = None):
      self.dispatcher._setSID()
      if s is None:
        s = ''
      return self.dispatcher._checkResult(func(self.id, x, y, s))
    return Indigo._make_wrapper_func(newfunc, func)

  def version (self):
    return Indigo._lib.indigoVersion()

  def similarity (self, item1, item2, metrics = None):
    self._setSID()
    if metrics is None:
      metrics = ''
    return self._checkResultFloat(Indigo._lib.indigoSimilarity(item1.id, item2.id, metrics))

  def writeBuffer (self):
    self._setSID()
    id = self._checkResult(Indigo._lib.indigoWriteBuffer())
    return Indigo.IndigoObject(self, id)

  def writeFile (self, filename):
    self._setSID()
    id = self._checkResult(Indigo._lib.indigoWriteFile(filename))
    return Indigo.IndigoObject(self, id)

  def unserialize (self, arr):
    self._setSID()
    values = (c_char * len(arr))()
    for i in xrange(len(arr)):
      values[i] = arr[i]
    res = Indigo._lib.indigoUnserialize(values, len(arr))
    return Indigo.IndigoObject(self, self._checkResult(res))

  def setOption (self, option, value1, value2 = None, value3 = None):
    self._setSID()
    if type(value1).__name__ == 'str' and value2 is None and value3 is None:
      self._checkResult(Indigo._lib.indigoSetOption(option, value1))
    elif type(value1).__name__ == 'int' and value2 is None and value3 is None:
      self._checkResult(Indigo._lib.indigoSetOptionInt(option, value1))
    elif type(value1).__name__ == 'float' and value2 is None and value3 is None:
      self._checkResult(Indigo._lib.indigoSetOptionFloat(option, value1))
    elif type(value1).__name__ == 'bool' and value2 is None and value3 is None:
      value1_b = 0
      if value1:
        value1_b = 1
      self._checkResult(Indigo._lib.indigoSetOptionBool(option, value1_b))
    elif type(value1).__name__ == 'int' and value2 and \
         type(value2).__name__ == 'int' and value3 is None:
      self._checkResult(Indigo._lib.indigoSetOptionXY(option, value1, value2))
    elif type(value1).__name__ == 'float' and value2 and \
         type(value2).__name__ == 'float' and value3 and \
         type(value3).__name__ == 'float':
      self._checkResult(Indigo._lib.indigoSetOptionColor(option, value1, value2, value3))
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
    return self._checkResultPtr(result)

  def __del__ (self):
    if hasattr(self, '_lib'):
      Indigo._lib.indigoReleaseSessionId(self._sid)
         
  def convertToArray (self, iteratable):
    if isinstance(iteratable, Indigo.IndigoObject):
      return iteratable
    try:
      some_object_iterator = iter(iteratable)
      res = self.createArray()
      for obj in some_object_iterator:
         res.arrayAdd(self.convertToArray(obj))
      return res
    except TypeError, te:
      raise IndigoException("Cannot convert object %s to an array" % (iteratable))
