#
# Copyright (C) 2009-2015 EPAM Systems
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

from indigo import *


class IndigoInchi(object):
    def __init__(self, indigo):
        self.indigo = indigo

        if os.name == 'posix' and not platform.mac_ver()[0]:
            self._lib = CDLL(indigo.dllpath + "/libindigo-inchi.so")
        elif os.name == 'nt':
            self._lib = CDLL(indigo.dllpath + "\indigo-inchi.dll")
        elif platform.mac_ver()[0]:
            self._lib = CDLL(indigo.dllpath + "/libindigo-inchi.dylib")
        else:
            raise IndigoException("unsupported OS: " + os.name)

        self._lib.indigoInchiVersion.restype = c_char_p
        self._lib.indigoInchiVersion.argtypes = []
        self._lib.indigoInchiResetOptions.restype = c_int
        self._lib.indigoInchiResetOptions.argtypes = []
        self._lib.indigoInchiLoadMolecule.restype = c_int
        self._lib.indigoInchiLoadMolecule.argtypes = [c_char_p]
        self._lib.indigoInchiGetInchi.restype = c_char_p
        self._lib.indigoInchiGetInchi.argtypes = [c_int]
        self._lib.indigoInchiGetInchiKey.restype = c_char_p
        self._lib.indigoInchiGetInchiKey.argtypes = [c_char_p]
        self._lib.indigoInchiGetWarning.restype = c_char_p
        self._lib.indigoInchiGetWarning.argtypes = []
        self._lib.indigoInchiGetLog.restype = c_char_p
        self._lib.indigoInchiGetLog.argtypes = []
        self._lib.indigoInchiGetAuxInfo.restype = c_char_p
        self._lib.indigoInchiGetAuxInfo.argtypes = []

    def resetOptions(self):
        self.indigo._setSessionId()
        self.indigo._checkResult(self._lib.indigoInchiResetOptions())

    def loadMolecule(self, inchi):
        self.indigo._setSessionId()
        res = self.indigo._checkResult(self._lib.indigoInchiLoadMolecule(inchi.encode('ascii')))
        if res == 0:
            return None
        return self.indigo.IndigoObject(self.indigo, res)

    def version(self):
        self.indigo._setSessionId()
        return self.indigo._checkResultString(self._lib.indigoInchiVersion())

    def getInchi(self, molecule):
        self.indigo._setSessionId()
        return self.indigo._checkResultString(self._lib.indigoInchiGetInchi(molecule.id))

    def getInchiKey(self, inchi):
        self.indigo._setSessionId()
        return self.indigo._checkResultString(self._lib.indigoInchiGetInchiKey(inchi.encode('ascii')))

    def getWarning(self):
        self.indigo._setSessionId()
        return self.indigo._checkResultString(self._lib.indigoInchiGetWarning())

    def getLog(self):
        self.indigo._setSessionId()
        return self.indigo._checkResultString(self._lib.indigoInchiGetLog())

    def getAuxInfo(self):
        self.indigo._setSessionId()
        return self.indigo._checkResultString(self._lib.indigoInchiGetAuxInfo())
