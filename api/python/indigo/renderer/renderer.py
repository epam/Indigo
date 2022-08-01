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
from ctypes import CDLL, c_int
from typing import Sequence

from ..indigo.indigo import Indigo
from ..indigo.indigo_exception import IndigoException
from ..indigo.indigo_lib import IndigoLib
from ..indigo.indigo_object import IndigoObject
from .indigo_renderer_lib import IndigoRendererLib


class IndigoRenderer:
    def __init__(self, session: Indigo) -> None:
        self._session: Indigo = session
        self._sid: int = self._session.getSessionId()
        self._libraryInstance: IndigoRendererLib = IndigoRendererLib()
        IndigoLib.checkResult(self._lib().indigoRendererInit(self._sid))

    def __del__(self) -> None:
        if self._sid != -1:
            IndigoLib.checkResult(self._lib().indigoRendererDispose(self._sid))
            self._sid = -1

    def _lib(self) -> CDLL:
        self._session._setSessionId()  # noqa
        return self._libraryInstance.lib  # type: ignore

    def renderToBuffer(self, obj: IndigoObject) -> bytes:
        """Renders object to buffer

        Args:
            obj (IndigoObject): object to render

        Returns:
            buffer with byte array
        """
        wb = self._session.writeBuffer()
        IndigoLib.checkResult(self._lib().indigoRender(obj.id, wb.id))
        return wb.toBuffer()

    def renderToString(self, obj: IndigoObject) -> str:
        """Renders object to string

        Args:
            obj (IndigoObject): object to render

        Returns:
            str: string with rendered data
        """
        return self.renderToBuffer(obj).decode()

    def renderToFile(self, obj: IndigoObject, filename: str) -> None:
        """Renders to file

        Args:
            obj (IndigoObject): object to render
            filename (str): full file path
        """
        IndigoLib.checkResult(
            self._lib().indigoRenderToFile(obj.id, filename.encode("ascii"))
        )

    def renderGridToFile(
        self,
        objects: IndigoObject,
        refatoms: Sequence[int],
        ncolumns: int,
        filename: str,
    ) -> None:
        """Renders grid to file

        Args:
            objects (IndigoObject): array of objects
            refatoms (Sequence[int]): array or reference atoms
            ncolumns (int): number of columns
            filename (str): full file path

        Raises:
            IndigoException: if any error while rendering
        """
        arr = None
        if refatoms:
            if len(refatoms) != objects.count():
                raise IndigoException(
                    "renderGridToFile(): "
                    "refatoms[] size must be equal to the number of objects"
                )
            arr = (c_int * len(refatoms))()
            for i in range(len(refatoms)):
                arr[i] = refatoms[i]
        IndigoLib.checkResult(
            self._lib().indigoRenderGridToFile(
                objects.id, arr, ncolumns, filename.encode("ascii")
            )
        )

    def renderGridToBuffer(
        self, objects: IndigoObject, refatoms: Sequence[int], ncolumns: int
    ) -> bytes:
        """Renders grid to buffer

        Args:
            objects (IndigoObject): array of objects
            refatoms (Sequence[int]): array or reference atoms
            ncolumns (int): number of columns

        Raises:
            IndigoException: if any error while rendering

        Returns:
            list: buffer byte array
        """
        arr = None
        if refatoms:
            if len(refatoms) != objects.count():
                raise IndigoException(
                    "renderGridToBuffer(): "
                    "refatoms[] size must be equal to the number of objects"
                )
            arr = (c_int * len(refatoms))()
            for i in range(len(refatoms)):
                arr[i] = refatoms[i]
        wb = self._session.writeBuffer()
        IndigoLib.checkResult(
            self._lib().indigoRenderGrid(objects.id, arr, ncolumns, wb.id)
        )
        return wb.toBuffer()
