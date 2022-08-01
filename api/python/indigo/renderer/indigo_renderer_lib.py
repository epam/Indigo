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
from ctypes import CDLL, POINTER, c_char_p, c_int
from typing import Optional

from .._common.lib import Lib


class IndigoRendererLib:
    lib: Optional[CDLL] = None

    def __init__(self) -> None:
        if IndigoRendererLib.lib:
            return

        IndigoRendererLib.lib = Lib.load("indigo-renderer")
        IndigoRendererLib.lib.indigoRendererInit.restype = c_int
        IndigoRendererLib.lib.indigoRendererInit.argtypes = [c_int]
        IndigoRendererLib.lib.indigoRendererDispose.restype = c_int
        IndigoRendererLib.lib.indigoRendererDispose.argtypes = [c_int]
        IndigoRendererLib.lib.indigoRender.restype = c_int
        IndigoRendererLib.lib.indigoRender.argtypes = [c_int, c_int]
        IndigoRendererLib.lib.indigoRenderToFile.restype = c_int
        IndigoRendererLib.lib.indigoRenderToFile.argtypes = [c_int, c_char_p]
        IndigoRendererLib.lib.indigoRenderGrid.restype = c_int
        IndigoRendererLib.lib.indigoRenderGrid.argtypes = [
            c_int,
            POINTER(c_int),
            c_int,
            c_int,
        ]
        IndigoRendererLib.lib.indigoRenderGridToFile.restype = c_int
        IndigoRendererLib.lib.indigoRenderGridToFile.argtypes = [
            c_int,
            POINTER(c_int),
            c_int,
            c_char_p,
        ]
        IndigoRendererLib.lib.indigoRenderReset.restype = c_int
        IndigoRendererLib.lib.indigoRenderReset.argtypes = []
