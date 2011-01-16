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

import indigo
from indigo import *

class IndigoRenderer:

  def __init__ (self, indigo):
    self.indigo = indigo

    if os.name == 'posix' and not platform.mac_ver()[0]:
      self._lib = CDLL(indigo.dllpath + "/libindigo-renderer.so")
    elif os.name == 'nt':
      self._lib = CDLL(indigo.dllpath + "\indigo-renderer.dll")
    elif platform.mac_ver()[0]:
      self._lib = CDLL(indigo.dllpath + "/libindigo-renderer.dylib")
    else:
      raise IndigoException("unsupported OS: " + os.name)

    self._lib.indigoRender.restype = c_int
    self._lib.indigoRender.argtypes = [c_int, c_int]
    self._lib.indigoRenderToFile.restype = c_int
    self._lib.indigoRenderToFile.argtypes = [c_int, c_char_p]
    self._lib.indigoRenderGrid.restype = c_int
    self._lib.indigoRenderGrid.argtypes = [c_int, POINTER(c_int), c_int, c_int]
    self._lib.indigoRenderGridToFile.restype = c_int
    self._lib.indigoRenderGridToFile.argtypes = [c_int, POINTER(c_int), c_int, c_char_p]
    self._lib.indigoRenderReset.restype = c_int
    self._lib.indigoRenderReset.argtypes = [c_int]
    
  def renderToBuffer (self, obj):
    self.indigo._setSID()
    wb = self.indigo.writeBuffer()
    self.indigo._checkResult(self._lib.indigoRender(obj.id, wb.id))
    return wb.toBuffer()
  
  def renderToFile (self, obj, filename):
    self.indigo._setSID()
    self.indigo._checkResult(self._lib.indigoRenderToFile(obj.id, filename))

  def renderGridToFile (self, objects, refatoms, ncolumns, filename):
    self.indigo._setSID()
    arr = None
    if refatoms:
      if len(refatoms) != objects.size():
        raise IndigoException("renderGridToFile(): refatoms[] size must be equal to the number of objects")
      arr = (c_int * len(refatoms))()
      for i in xrange(len(refatoms)):
        arr[i] = refatoms[i]
    self.indigo._checkResult(
      self._lib.indigoRenderGridToFile(objects.id, arr, ncolumns, filename))

  def renderGridToBuffer (self, objects, refatoms, ncolumns):
    self.indigo._setSID()
    arr = None
    if refatoms:
      if len(refatoms) != objects.size():
        raise IndigoException("renderGridToBuffer(): refatoms[] size must be equal to the number of objects")
      arr = (c_int * len(refatoms))()
      for i in xrange(len(refatoms)):
        arr[i] = refatoms[i]
    wb = self.indigo.writeBuffer()
    self.indigo._checkResult(
      self._lib.indigoRenderGrid(objects.id, arr, ncolumns, wb.id))
    return wb.toBuffer()
