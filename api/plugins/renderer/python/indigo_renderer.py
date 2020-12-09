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
from ctypes import CDLL, POINTER, c_char_p, c_int

from indigo import IndigoException


class IndigoRenderer(object):
    def __init__(self, indigo):
        self.indigo = indigo

        if (
            os.name == "posix"
            and not platform.mac_ver()[0]
            and not platform.system().startswith("CYGWIN")
        ):
            self._lib = CDLL(indigo.dllpath + "/libindigo-renderer.so")
        elif os.name == "nt" or platform.system().startswith("CYGWIN"):
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
        self._lib.indigoRenderGrid.argtypes = [
            c_int,
            POINTER(c_int),
            c_int,
            c_int,
        ]
        self._lib.indigoRenderGridToFile.restype = c_int
        self._lib.indigoRenderGridToFile.argtypes = [
            c_int,
            POINTER(c_int),
            c_int,
            c_char_p,
        ]
        self._lib.indigoRenderReset.restype = c_int
        self._lib.indigoRenderReset.argtypes = [c_int]

    def renderToBuffer(self, obj):
        self.indigo._setSessionId()
        wb = self.indigo.writeBuffer()
        try:
            self.indigo._checkResult(self._lib.indigoRender(obj.id, wb.id))
            return wb.toBuffer()
        finally:
            wb.dispose()

    def renderToFile(self, obj, filename):
        self.indigo._setSessionId()
        self.indigo._checkResult(
            self._lib.indigoRenderToFile(obj.id, filename.encode("ascii"))
        )

    def renderGridToFile(self, objects, refatoms, ncolumns, filename):
        self.indigo._setSessionId()
        arr = None
        if refatoms:
            if len(refatoms) != objects.count():
                raise IndigoException(
                    "renderGridToFile(): refatoms[] size must be equal to the number of objects"
                )
            arr = (c_int * len(refatoms))()
            for i in range(len(refatoms)):
                arr[i] = refatoms[i]
        self.indigo._checkResult(
            self._lib.indigoRenderGridToFile(
                objects.id, arr, ncolumns, filename.encode("ascii")
            )
        )

    def renderGridToBuffer(self, objects, refatoms, ncolumns):
        self.indigo._setSessionId()
        arr = None
        if refatoms:
            if len(refatoms) != objects.count():
                raise IndigoException(
                    "renderGridToBuffer(): refatoms[] size must be equal to the number of objects"
                )
            arr = (c_int * len(refatoms))()
            for i in range(len(refatoms)):
                arr[i] = refatoms[i]
        wb = self.indigo.writeBuffer()
        try:
            self.indigo._checkResult(
                self._lib.indigoRenderGrid(objects.id, arr, ncolumns, wb.id)
            )
            return wb.toBuffer()
        finally:
            wb.dispose()
