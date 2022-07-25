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
from ctypes import CDLL

from ..indigo.indigo import Indigo
from ..indigo.indigo_lib import IndigoLib
from ..indigo.indigo_object import IndigoObject
from .indigo_inchi_lib import IndigoInchiLib


class IndigoInchi(object):
    def __init__(self, session: Indigo) -> None:
        self._session: Indigo = session
        self._sid: int = self._session.getSessionId()
        self._libraryInstance: IndigoInchiLib = IndigoInchiLib()
        IndigoLib.checkResult(self._lib().indigoInchiInit(self._sid))

    def __del__(self) -> None:
        if self._sid != -1:
            IndigoLib.checkResult(self._lib().indigoInchiDispose(self._sid))
            self._sid = -1

    def _lib(self) -> CDLL:
        self._session._setSessionId()  # noqa
        return self._libraryInstance.lib  # type: ignore

    def resetOptions(self) -> None:
        """Resets options for InChi"""
        IndigoLib.checkResult(self._lib().indigoInchiResetOptions())

    def loadMolecule(self, inchi: str) -> IndigoObject:
        """Loads molecule from InChi string

        Args:
            inchi (str): InChi string

        Returns:
            IndigoObject: molecule object
        """

        res = IndigoLib.checkResult(
            self._lib().indigoInchiLoadMolecule(inchi.encode())
        )
        return IndigoObject(self._session, res)

    def version(self) -> str:
        """Returns InChi version

        Returns:
            str: version string
        """

        return IndigoLib.checkResultString(self._lib().indigoInchiVersion())

    def getInchi(self, molecule: IndigoObject) -> str:
        """Returns InChi string for Indigo molecule

        Args:
            molecule (IndigoObject): molecule object

        Returns:
            str: InChi string
        """

        return IndigoLib.checkResultString(
            self._lib().indigoInchiGetInchi(molecule.id)
        )

    def getInchiKey(self, inchi: str) -> str:
        """Returns InChi key for InChi string

        Args:
            inchi (str): InChi string

        Returns:
            str: InChi key
        """

        return IndigoLib.checkResultString(
            self._lib().indigoInchiGetInchiKey(inchi.encode())
        )

    def getWarning(self) -> str:
        """Returns warning message

        Returns:
            str: warning string
        """

        return IndigoLib.checkResultString(self._lib().indigoInchiGetWarning())

    def getLog(self) -> str:
        """Returns logs while InChi calculation

        Returns:
            str: log string
        """

        return IndigoLib.checkResultString(self._lib().indigoInchiGetLog())

    def getAuxInfo(self) -> str:
        """Returns aux info for the InChi

        Returns:
            str: InChi aux info string
        """

        return IndigoLib.checkResultString(self._lib().indigoInchiGetAuxInfo())
