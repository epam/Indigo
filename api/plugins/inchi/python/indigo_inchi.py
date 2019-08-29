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

from indigo import *


class IndigoInchi(object):
    def __init__(self, indigo):
        self.indigo = indigo

        if os.name == 'posix' and not platform.mac_ver()[0] and not platform.system().startswith("CYGWIN"):
            self._lib = CDLL(indigo.dllpath + "/libindigo-inchi.so")
        elif os.name == 'nt' or platform.system().startswith("CYGWIN"):
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
