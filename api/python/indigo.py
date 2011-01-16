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

class Indigo:
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
        self._lib.indigoAlignAtoms(self.id, len(atoms), atoms, xyz))

    def __enter__ (self):
      return self
    def __exit__ (self, exc_type, exc_value, traceback):
      self.dispatcher._setSID()
      self._lib.indigoClose(self.id)
    def __del__ (self):
      self.dispatcher._setSID()
      self._lib.indigoFree(self.id)
        
    def __iter__ (self):
      return self
    def __next__ (self):
      obj = self._next()
      if obj == None:
        raise StopIteration
      return obj
    def next (self):
      return self.__next__()

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
    self._lib.indigoWriteFile.restype = c_int
    self._lib.indigoWriteFile.argtypes = [c_char_p]
    self._lib.indigoWriteBuffer.restype = c_int
    self._lib.indigoWriteBuffer.argtypes = None
    self._lib.indigoSaveMDLCT.restype = c_int
    self._lib.indigoSaveMDLCT.argtypes = [c_int, c_int]
    self._lib.indigoXYZ.restype = POINTER(c_float)
    self._lib.indigoXYZ.argtypes = [c_int]
    self._lib.indigoAlignAtoms.restype = c_float
    self._lib.indigoAlignAtoms.argtypes = [c_int, c_int, POINTER(c_int), POINTER(c_float)]
    self._lib.indigoToString.restype = c_char_p
    self._lib.indigoToString.argtypes = [c_int]
    self._lib.indigoToBuffer.restype = c_int
    self._lib.indigoToBuffer.argtypes = [c_int, POINTER(POINTER(c_char)), POINTER(c_int)]
    self._lib.indigoDbgBreakpoint.restype = None
    self._lib.indigoDbgBreakpoint.argtypes = None
 
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

    self.substructureMatcher = self._static_obj_obj_string(self._lib.indigoSubstructureMatcher)

    self.extractCommonScaffold = self._static_obj_obj_string(self._lib.indigoExtractCommonScaffold)
    self.decomposeMolecules = self._static_obj_obj_obj(self._lib.indigoDecomposeMolecules)
    self.reactionProductEnumerate = self._static_obj_obj_obj(self._lib.indigoReactionProductEnumerate)

    self.dbgBreakpoint = self._lib.indigoDbgBreakpoint
    
    self.IndigoObject.close = self._member_void(self._lib.indigoClose)
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
    self.IndigoObject.iterateStereocenters = self._member_obj(self._lib.indigoIterateStereocenters)
    self.IndigoObject.iterateRGroups = self._member_obj(self._lib.indigoIterateRGroups)
    self.IndigoObject.iterateRGroupFragments = self._member_obj(self._lib.indigoIterateRGroupFragments)
    self.IndigoObject.countAttachmentPoints = self._member_int(self._lib.indigoCountAttachmentPoints)
    self.IndigoObject.isPseudoatom = self._member_bool(self._lib.indigoIsPseudoatom)
    self.IndigoObject.isRSite = self._member_bool(self._lib.indigoIsRSite)
    self.IndigoObject.stereocenterType = self._member_int(self._lib.indigoStereocenterType)
    self.IndigoObject.singleAllowedRGroup = self._member_int(self._lib.indigoSingleAllowedRGroup)
    self.IndigoObject.symbol = self._member_string(self._lib.indigoSymbol)

    self.IndigoObject.degree = self._member_int(self._lib.indigoDegree)
    self.IndigoObject.charge = self._member_intptr(self._lib.indigoGetCharge)
    self.IndigoObject.explicitValence = self._member_intptr(self._lib.indigoGetExplicitValence)
    self.IndigoObject.radicalElectrons = self._member_intptr(self._lib.indigoGetRadicalElectrons)
    self.IndigoObject.atomicNumber = self._member_int(self._lib.indigoAtomicNumber)
    self.IndigoObject.isotope = self._member_int(self._lib.indigoIsotope)

    self.IndigoObject.countSuperatoms = self._member_int(self._lib.indigoCountSuperatoms)
    self.IndigoObject.countDataSGroups = self._member_int(self._lib.indigoCountDataSGroups)
    self.IndigoObject.iterateDataSGroups = self._member_obj(self._lib.indigoIterateDataSGroups)
    self.IndigoObject.description = self._member_string(self._lib.indigoDescription)
    self.IndigoObject.remove = self._member_void(self._lib.indigoRemove)

    self.IndigoObject.resetCharge = self._member_void(self._lib.indigoResetCharge)
    self.IndigoObject.resetExplicitValence = self._member_void(self._lib.indigoResetExplicitValence)
    self.IndigoObject.resetRadical = self._member_void(self._lib.indigoResetRadical)
    self.IndigoObject.resetIsotope = self._member_void(self._lib.indigoResetIsotope)
    self.IndigoObject.resetStereo = self._member_void(self._lib.indigoResetStereo)
    self.IndigoObject.invertStereo = self._member_void(self._lib.indigoInvertStereo)

    self.IndigoObject.setAttachmentPoint = self._member_void_int(self._lib.indigoSetAttachmentPoint)

    self.IndigoObject.removeConstraints = self._member_void_string(self._lib.indigoRemoveConstraints)
    self.IndigoObject.addConstraint = self._member_void_string_string(self._lib.indigoAddConstraint)
    self.IndigoObject.addConstraintNot = self._member_void_string_string(self._lib.indigoAddConstraintNot)

    self.IndigoObject.countAtoms = self._member_int(self._lib.indigoCountAtoms)
    self.IndigoObject.countBonds = self._member_int(self._lib.indigoCountBonds)
    self.IndigoObject.countPseudoatoms = self._member_int(self._lib.indigoCountPseudoatoms)
    self.IndigoObject.countRSites = self._member_int(self._lib.indigoCountRSites)

    self.IndigoObject.iterateBonds = self._member_obj(self._lib.indigoIterateBonds)
    self.IndigoObject.bondOrder = self._member_int(self._lib.indigoBondOrder)
    self.IndigoObject.topology = self._member_int(self._lib.indigoTopology)
    self.IndigoObject.bondStereo = self._member_int(self._lib.indigoBondStereo)

    self.IndigoObject.iterateNeighbors = self._member_obj(self._lib.indigoIterateNeighbors)
    self.IndigoObject.bond = self._member_obj(self._lib.indigoBond)
    self.IndigoObject.getAtom = self._member_obj_int(self._lib.indigoGetAtom)
    self.IndigoObject.getBond = self._member_obj_int(self._lib.indigoGetBond)
    self.IndigoObject.source = self._member_obj(self._lib.indigoSource)
    self.IndigoObject.destination = self._member_obj(self._lib.indigoDestination)

    self.IndigoObject.clearCisTrans = self._member_void(self._lib.indigoClearCisTrans)
    self.IndigoObject.clearStereocenters = self._member_void(self._lib.indigoClearStereocenters)
    self.IndigoObject.countStereocenters = self._member_int(self._lib.indigoCountStereocenters)
    self.IndigoObject.resetSymmetricCisTrans = self._member_int(self._lib.indigoResetSymmetricCisTrans)
    self.IndigoObject.unseparateCharges = self._member_int(self._lib.indigoUnseparateCharges)

    self.IndigoObject.countComponents = self._member_int(self._lib.indigoCountComponents)
    self.IndigoObject.componentIndex = self._member_int(self._lib.indigoComponentIndex)
    self.IndigoObject.component = self._member_obj_int(self._lib.indigoComponent)
    self.IndigoObject.iterateComponents = self._member_obj(self._lib.indigoIterateComponents)

    self.IndigoObject.countHeavyAtoms = self._member_int(self._lib.indigoCountHeavyAtoms)
    self.IndigoObject.molecularWeight = self._member_float(self._lib.indigoMolecularWeight)
    self.IndigoObject.monoisotopicMass = self._member_float(self._lib.indigoMonoisotopicMass)
    self.IndigoObject.mostAbundantMass = self._member_float(self._lib.indigoMostAbundantMass)

    self.IndigoObject.canonicalSmiles = self._member_string(self._lib.indigoCanonicalSmiles)
    self.IndigoObject.layeredCode = self._member_string(self._lib.indigoLayeredCode)

    self.IndigoObject.hasZCoord = self._member_bool(self._lib.indigoHasZCoord)
    self.IndigoObject.isChiral = self._member_bool(self._lib.indigoIsChiral)
    
    self.IndigoObject.aromatize = self._member_bool(self._lib.indigoAromatize)
    self.IndigoObject.dearomatize = self._member_bool(self._lib.indigoDearomatize)
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
    self.IndigoObject.count = self._member_int(self._lib.indigoCount)
    self.IndigoObject.clear = self._member_void(self._lib.indigoClear)
    self.IndigoObject.arrayAdd = self._member_void_obj(self._lib.indigoArrayAdd)
    self.IndigoObject.at = self._member_obj_int(self._lib.indigoAt)
    
    self.IndigoObject.match = self._member_obj_obj(self._lib.indigoMatch)
    self.IndigoObject.countMatches = self._member_int_obj(self._lib.indigoCountMatches)
    self.IndigoObject.iterateMatches = self._member_obj_obj(self._lib.indigoIterateMatches)
    self.IndigoObject.highlightedTarget = self._member_obj(self._lib.indigoHighlightedTarget);
    self.IndigoObject.mapAtom = self._member_obj_obj(self._lib.indigoMapAtom);
    self.IndigoObject.mapBond = self._member_obj_obj(self._lib.indigoMapBond);	
	
    self.IndigoObject.allScaffolds = self._member_obj(self._lib.indigoAllScaffolds);
    self.IndigoObject.decomposedMoleculeScaffold = self._member_obj(self._lib.indigoDecomposedMoleculeScaffold)
    self.IndigoObject.iterateDecomposedMolecules = self._member_obj(self._lib.indigoIterateDecomposedMolecules)
    self.IndigoObject.iterateDecomposedMolecules = self._member_obj(self._lib.indigoIterateDecomposedMolecules)
    self.IndigoObject.decomposedMoleculeHighlighted = self._member_obj(self._lib.indigoDecomposedMoleculeHighlighted)
    self.IndigoObject.decomposedMoleculeWithRGroups = self._member_obj(self._lib.indigoDecomposedMoleculeWithRGroups)
    
    self.IndigoObject.index = self._member_int(self._lib.indigoIndex)

    self.IndigoObject.createSubmolecule = self._member_obj_iarr(self._lib.indigoCreateSubmolecule)
    self.IndigoObject.createEdgeSubmolecule = self._member_obj_iarr_iarr(self._lib.indigoCreateEdgeSubmolecule)
    self.IndigoObject.removeAtoms = self._member_void_iarr(self._lib.indigoRemoveAtoms)
    self.IndigoObject.addDataSGroup = self._member_obj_iarr_iarr_string_string(self._lib.indigoAddDataSGroup)
    self.IndigoObject.setDataSGroupXY = self._member_void_float_float_string(self._lib.indigoSetDataSGroupXY)

    self.IndigoObject._next = self._member_obj(self._lib.indigoNext)
    
  def _make_wrapper_func (self, wrapper, func):
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
      return self.IndigoObject(self, res)
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
   
  def _static_obj_string (self, func):
    func.restype = c_int
    func.argtypes = [c_char_p]
    def newfunc (str):
      self._setSID()
      res = self._checkResult(func(str))
      if res == 0:
        return None
      return self.IndigoObject(self, res)
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
      return self.IndigoObject(self, self._checkResult(res))
    return self._make_wrapper_func(newfunc, func)

  def _static_obj_obj (self, func):
    func.restype = c_int
    func.argtypes = [c_int]
    def newfunc (obj):
      self._setSID()
      res = func(obj.id)
      if res == 0:
        return None
      return self.IndigoObject(self, self._checkResult(res))
    return self._make_wrapper_func(newfunc, func)
		
  def _static_obj_obj_obj (self, func):
    func.restype = c_int
    func.argtypes = [c_int, c_int]
    def newfunc (obj1, obj2):
      self._setSID()
      res = func(obj1.id, obj2.id)
      if res == 0:
        return None
      return self.IndigoObject(self, self._checkResult(res))
    return self._make_wrapper_func(newfunc, func)

  def _setSID (self):
    self._lib.indigoSetSessionId(self._sid)

  def _member_string (self, func): 
    func.restype = c_char_p
    func.argtypes = [c_int]
    dispatcher = self
    def newfunc (self):
      dispatcher._setSID()
      return dispatcher._checkResultString(func(self.id))
    return self._make_wrapper_func(newfunc, func)

  def _member_bool (self, func):
    func.restype = c_int
    func.argtypes = [c_int]
    dispatcher = self
    def newfunc (self):
      dispatcher._setSID()
      res = dispatcher._checkResult(func(self.id))
      return res == 1                                                 
    return self._make_wrapper_func(newfunc, func)

  def _member_bool_string (self, func):
    func.restype = c_int
    func.argtypes = [c_int, c_char_p]
    dispatcher = self
    def newfunc (self, str):
      dispatcher._setSID()
      res = dispatcher._checkResult(func(self.id, str))
      return res == 1                                                 
    return self._make_wrapper_func(newfunc, func)

  def _member_string_string (self, func):
    func.restype = c_char_p
    func.argtypes = [c_int, c_char_p]
    dispatcher = self
    def newfunc (self, str):
      dispatcher._setSID()
      return dispatcher._checkResultString(func(self.id, str))
    return self._make_wrapper_func(newfunc, func)

  def _member_float (self, func):
    func.restype = c_float
    func.argtypes = [c_int]
    dispatcher = self
    def newfunc (self):
      dispatcher._setSID()
      return dispatcher._checkResultFloat(func(self.id))
    return self._make_wrapper_func(newfunc, func)

  def _member_void (self, func):
    func.restype = c_int
    func.argtypes = [c_int]
    dispatcher = self
    def newfunc (self):
      dispatcher._setSID()
      dispatcher._checkResult(func(self.id))
      return
    return self._make_wrapper_func(newfunc, func)

  def _member_void_int (self, func):
    func.restype = c_int
    func.argtypes = [c_int]
    dispatcher = self
    def newfunc (self, param):
      dispatcher._setSID()
      dispatcher._checkResult(func(self.id, param))
      return
    return self._make_wrapper_func(newfunc, func)

  def _member_void_obj (self, func):
    func.restype = c_int
    func.argtypes = [c_int, c_int]
    dispatcher = self
    def newfunc (self, other):
      dispatcher._setSID()
      dispatcher._checkResult(func(self.id, other.id))
      return
    return self._make_wrapper_func(newfunc, func)

  def _member_void_string (self, func):
    func.restype = c_int
    func.argtypes = [c_int, c_char_p]
    dispatcher = self
    def newfunc (self, str):
      dispatcher._setSID()
      dispatcher._checkResult(func(self.id, str))
      return
    return self._make_wrapper_func(newfunc, func)

  def _member_void_string_string (self, func):
    func.restype = c_int
    func.argtypes = [c_int, c_char_p, c_char_p]
    dispatcher = self
    def newfunc (self, str1, str2):
      dispatcher._setSID()
      dispatcher._checkResult(func(self.id, str1, str2))
      return
    return self._make_wrapper_func(newfunc, func)

  def _member_int (self, func):
    func.restype = c_int
    func.argtypes = [c_int]
    dispatcher = self
    def newfunc (self):
      dispatcher._setSID()
      return dispatcher._checkResult(func(self.id))
    return self._make_wrapper_func(newfunc, func)

  def _member_intptr (self, func):
    func.restype = c_int
    func.argtypes = [c_int, POINTER(c_int)]
    dispatcher = self
    def newfunc (self):
      dispatcher._setSID()
      value = c_int()
      res = dispatcher._checkResult(func(self.id, pointer(value)))
      if res == 0:
        return None
      return value.value
    return self._make_wrapper_func(newfunc, func)

  def _member_obj (self, func):
    func.restype = c_int
    func.argtypes = [c_int]
    dispatcher = self
    def newfunc (self):
      dispatcher._setSID()
      newobj = dispatcher._checkResult(func(self.id))
      if newobj == 0:
        return None
      return dispatcher.IndigoObject(dispatcher, newobj)
    return self._make_wrapper_func(newfunc, func)

  def _member_obj_int (self, func):
    func.restype = c_int
    func.argtypes = [c_int, c_int]
    dispatcher = self
    def newfunc (self, param):
      dispatcher._setSID()
      newobj = dispatcher._checkResult(func(self.id, param))
      if newobj == 0:
        return None
      return dispatcher.IndigoObject(dispatcher, newobj)
    return self._make_wrapper_func(newfunc, func)

  def _member_int_obj (self, func):
    func.restype = c_int
    func.argtypes = [c_int, c_int]
    dispatcher = self
    def newfunc (self, param):
      dispatcher._setSID()
      return dispatcher._checkResult(func(self.id, param.id))
    return self._make_wrapper_func(newfunc, func)
	
  def _member_obj_obj (self, func):
    dispatcher = self
    func.restype = c_int
    func.argtypes = [c_int, c_int]
    def newfunc (self, param):
      dispatcher._setSID()
      newobj = dispatcher._checkResult(func(self.id, param.id))
      if newobj == 0:
        return None
      return dispatcher.IndigoObject(dispatcher, newobj)
    return self._make_wrapper_func(newfunc, func)

  def _member_obj_string (self, func):
    dispatcher = self
    func.restype = c_int
    func.argtypes = [c_int, c_char_p]
    def newfunc (self, param):
      dispatcher._setSID()
      newobj = dispatcher._checkResult(func(self.id, param))
      if newobj == 0:
        return None
      return dispatcher.IndigoObject(dispatcher, newobj)
    return self._make_wrapper_func(newfunc, func)

  def _member_void_iarr (self, func):
    dispatcher = self
    func.restype = c_int
    func.argtypes = [c_int, c_int, POINTER(c_int)]
    def newfunc (self, intarr):
      arr = (c_int * len(intarr))()
      for i in xrange(len(intarr)):
        arr[i] = intarr[i]
      dispatcher._setSID()
      dispatcher._checkResult(func(self.id, len(intarr), arr))
    return self._make_wrapper_func(newfunc, func)

  def _member_obj_iarr (self, func):
    func.restype = c_int
    func.argtypes = [c_int, c_int, POINTER(c_int)]
    dispatcher = self
    def newfunc (self, intarr):
      arr = (c_int * len(intarr))()
      for i in xrange(len(intarr)):
        arr[i] = intarr[i]
      dispatcher._setSID()
      newobj = dispatcher._checkResult(func(self.id, len(intarr), arr))
      if newobj == 0:
        return None
      return dispatcher.IndigoObject(dispatcher, newobj)
    return self._make_wrapper_func(newfunc, func)

  def _member_obj_iarr_iarr (self, func):
    dispatcher = self
    func.restype = c_int
    func.argtypes = [c_int, c_int, POINTER(c_int), c_int, POINTER(c_int)]
    def newfunc (self, intarr1, intarr2):
      arr1 = (c_int * len(intarr1))()
      for i in xrange(len(intarr1)):
        arr1[i] = intarr1[i]
      arr2 = (c_int * len(intarr2))()
      for i in xrange(len(intarr2)):
        arr2[i] = intarr2[i]
      dispatcher._setSID()
      newobj = dispatcher._checkResult(func(self.id, len(intarr1), arr1, len(intarr2), arr2))
      if newobj == 0:
        return None
      return dispatcher.IndigoObject(dispatcher, newobj)
    return self._make_wrapper_func(newfunc, func)

  def _member_obj_iarr_iarr_string_string (self, func):
    dispatcher = self
    func.restype = c_int
    func.argtypes = [c_int, c_int, POINTER(c_int), c_int, POINTER(c_int), c_char_p, c_char_p]
    def newfunc (self, intarr1, intarr2, str1, str2):
      arr1 = (c_int * len(intarr1))()
      for i in xrange(len(intarr1)):
        arr1[i] = intarr1[i]
      arr2 = (c_int * len(intarr2))()
      for i in xrange(len(intarr2)):
        arr2[i] = intarr2[i]
      dispatcher._setSID()
      newobj = dispatcher._checkResult(func(self.id, len(intarr1), arr1, len(intarr2), arr2, str1, str2))
      if newobj == 0:
        return None
      return dispatcher.IndigoObject(dispatcher, newobj)
    return self._make_wrapper_func(newfunc, func)

  def _member_void_float_float_string (self, func):
    func.restype = c_int
    func.argtypes = [c_int, c_float, c_float, c_char_p]
    dispatcher = self
    def newfunc (self, x, y, s):
      dispatcher._setSID()
      return dispatcher._checkResult(func(self.id, x, y, s))
    return self._make_wrapper_func(newfunc, func)

  def version (self):
    return self._lib.indigoVersion()

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
