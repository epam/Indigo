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

import os
import platform
from ctypes import CDLL, c_char_p, c_int

from indigo import IndigoException


class IndigoInchi(object):
    def __init__(self, indigo):
        self.indigo = indigo

        if (
            os.name == "posix"
            and not platform.mac_ver()[0]
            and not platform.system().startswith("CYGWIN")
        ):
            self._lib = CDLL(indigo._dll_dir + "/libindigo-inchi.so")
        elif os.name == "nt" or platform.system().startswith("CYGWIN"):
            self._lib = CDLL(indigo._dll_dir + "/indigo-inchi.dll")
        elif platform.mac_ver()[0]:
            self._lib = CDLL(indigo._dll_dir + "/libindigo-inchi.dylib")
        else:
            raise IndigoException("unsupported OS: " + os.name)

        self._lib.indigoInchiInit.restype = c_int
        self._lib.indigoInchiInit.argtypes = []
        self._lib.indigoInchiDispose.restype = c_int
        self._lib.indigoInchiDispose.argtypes = []
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

        # Init Indigo-InChI context and options
        self.indigo._setSessionId()
        self.indigo._checkResult(self._lib.indigoInchiInit())
        self._initialized = True

    def __del__(self):
        if self._initialized:
            self.indigo._setSessionId()
            self.indigo._checkResult(self._lib.indigoInchiDispose())
            self._initialized = False

    def resetOptions(self):
        """Resets options for InChi"""
        self.indigo._setSessionId()
        self.indigo._checkResult(self._lib.indigoInchiResetOptions())

    def loadMolecule(self, inchi):
        """Loads molecule from InChi string

        Args:
            inchi (str): InChi string

        Returns:
            IndigoObject: molecule object
        """
        self.indigo._setSessionId()
        res = self.indigo._checkResult(
            self._lib.indigoInchiLoadMolecule(inchi.encode("ascii"))
        )
        if res < 0:
            return None
        return self.indigo.IndigoObject(self.indigo, res)

    def version(self):
        """Returns InChi version

        Returns:
            str: version string
        """
        self.indigo._setSessionId()
        return self.indigo._checkResultString(self._lib.indigoInchiVersion())

    def getInchi(self, molecule):
        """Returns InChi string for Indigo molecule

        Args:
            molecule (IndigoObject): molecule object

        Returns:
            str: InChi string
        """
        self.indigo._setSessionId()
        return self.indigo._checkResultString(
            self._lib.indigoInchiGetInchi(molecule.id)
        )

    def getInchiKey(self, inchi):
        """Returns InChi key for InChi string

        Args:
            inchi (str): InChi string

        Returns:
            str: InChi key
        """
        self.indigo._setSessionId()
        return self.indigo._checkResultString(
            self._lib.indigoInchiGetInchiKey(inchi.encode("ascii"))
        )

    def getWarning(self):
        """Returns warning message

        Returns:
            str: warning string
        """
        self.indigo._setSessionId()
        return self.indigo._checkResultString(
            self._lib.indigoInchiGetWarning()
        )

    def getLog(self):
        """Returns logs while InChi calculation

        Returns:
            str: log string
        """
        self.indigo._setSessionId()
        return self.indigo._checkResultString(self._lib.indigoInchiGetLog())

    def getAuxInfo(self):
        """Returns aux info for the InChi

        Returns:
            str: InChi aux info string
        """
        self.indigo._setSessionId()
        return self.indigo._checkResultString(
            self._lib.indigoInchiGetAuxInfo()
        )
