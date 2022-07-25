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
from ctypes import CDLL, c_char_p, c_int
from typing import Optional

from .._common.lib import Lib


class IndigoInchiLib:
    lib: Optional[CDLL] = None

    def __init__(self) -> None:
        if IndigoInchiLib.lib:
            return

        IndigoInchiLib.lib = Lib.load("indigo-inchi")

        IndigoInchiLib.lib.indigoInchiInit.restype = c_int
        IndigoInchiLib.lib.indigoInchiInit.argtypes = []
        IndigoInchiLib.lib.indigoInchiDispose.restype = c_int
        IndigoInchiLib.lib.indigoInchiDispose.argtypes = []
        IndigoInchiLib.lib.indigoInchiVersion.restype = c_char_p
        IndigoInchiLib.lib.indigoInchiVersion.argtypes = []
        IndigoInchiLib.lib.indigoInchiResetOptions.restype = c_int
        IndigoInchiLib.lib.indigoInchiResetOptions.argtypes = []
        IndigoInchiLib.lib.indigoInchiLoadMolecule.restype = c_int
        IndigoInchiLib.lib.indigoInchiLoadMolecule.argtypes = [c_char_p]
        IndigoInchiLib.lib.indigoInchiGetInchi.restype = c_char_p
        IndigoInchiLib.lib.indigoInchiGetInchi.argtypes = [c_int]
        IndigoInchiLib.lib.indigoInchiGetInchiKey.restype = c_char_p
        IndigoInchiLib.lib.indigoInchiGetInchiKey.argtypes = [c_char_p]
        IndigoInchiLib.lib.indigoInchiGetWarning.restype = c_char_p
        IndigoInchiLib.lib.indigoInchiGetWarning.argtypes = []
        IndigoInchiLib.lib.indigoInchiGetLog.restype = c_char_p
        IndigoInchiLib.lib.indigoInchiGetLog.argtypes = []
        IndigoInchiLib.lib.indigoInchiGetAuxInfo.restype = c_char_p
        IndigoInchiLib.lib.indigoInchiGetAuxInfo.argtypes = []
