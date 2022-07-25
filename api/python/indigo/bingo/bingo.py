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
from .bingo_exception import BingoException
from .bingo_lib import BingoLib
from .bingo_object import BingoObject


class Bingo:
    def __init__(self, bingo_id: int, session: Indigo, lib: BingoLib) -> None:
        self._id: int = bingo_id
        self.session: Indigo = session
        self._libraryInstance: BingoLib = lib
        # init
        self.version()

    def __del__(self):
        self.close()

    def _lib(self) -> CDLL:
        self.session._setSessionId()  # noqa
        return self._libraryInstance.lib  # type: ignore

    def close(self):
        if self._id != -1:
            IndigoLib.checkResult(
                self._lib().bingoCloseDatabase(self._id), BingoException
            )
            self._id = -1

    @staticmethod
    def createDatabaseFile(indigo, path, databaseType, options=""):
        libraryHandle = BingoLib()
        return Bingo(
            IndigoLib.checkResult(
                libraryHandle.lib.bingoCreateDatabaseFile(
                    path.encode(),
                    databaseType.encode(),
                    options.encode(),
                ),
                BingoException,
            ),
            indigo,
            libraryHandle,
        )

    @staticmethod
    def loadDatabaseFile(indigo, path, options=""):
        libraryHandle = BingoLib()
        return Bingo(
            IndigoLib.checkResult(
                libraryHandle.lib.bingoLoadDatabaseFile(
                    path.encode(), options.encode()
                ),
                BingoException,
            ),
            indigo,
            libraryHandle,
        )

    def version(self):
        return IndigoLib.checkResultString(
            self._lib().bingoVersion(), BingoException
        )

    def insert(self, indigoObject, index=None):
        if not index:
            return IndigoLib.checkResult(
                self._lib().bingoInsertRecordObj(self._id, indigoObject.id),
                BingoException,
            )
        else:
            return IndigoLib.checkResult(
                self._lib().bingoInsertRecordObjWithId(
                    self._id, indigoObject.id, index
                ),
                BingoException,
            )

    def insertWithExtFP(self, indigoObject, ext_fp, index=None):
        if not index:
            return IndigoLib.checkResult(
                self._lib().bingoInsertRecordObjWithExtFP(
                    self._id, indigoObject.id, ext_fp.id
                ),
                BingoException,
            )
        else:
            return IndigoLib.checkResult(
                self._lib().bingoInsertRecordObjWithIdAndExtFP(
                    self._id, indigoObject.id, index, ext_fp.id
                ),
                BingoException,
            )

    def delete(self, index):
        IndigoLib.checkResult(
            self._lib().bingoDeleteRecord(self._id, index), BingoException
        )

    def searchSub(self, query, options=""):
        return BingoObject(
            IndigoLib.checkResult(
                self._lib().bingoSearchSub(
                    self._id, query.id, options.encode()
                ),
                BingoException,
            ),
            self,
        )

    def searchExact(self, query, options=""):
        return BingoObject(
            IndigoLib.checkResult(
                self._lib().bingoSearchExact(
                    self._id, query.id, options.encode()
                ),
                BingoException,
            ),
            self,
        )

    def searchSim(self, query, minSim, maxSim, metric="tanimoto"):
        return BingoObject(
            IndigoLib.checkResult(
                self._lib().bingoSearchSim(
                    self._id, query.id, minSim, maxSim, metric.encode()
                ),
                BingoException,
            ),
            self,
        )

    def searchSimWithExtFP(
        self, query, minSim, maxSim, ext_fp, metric="tanimoto"
    ):
        return BingoObject(
            IndigoLib.checkResult(
                self._lib().bingoSearchSimWithExtFP(
                    self._id,
                    query.id,
                    minSim,
                    maxSim,
                    ext_fp.id,
                    metric.encode(),
                ),
                BingoException,
            ),
            self,
        )

    def searchSimTopN(self, query, limit, minSim, metric="tanimoto"):
        return BingoObject(
            IndigoLib.checkResult(
                self._lib().bingoSearchSimTopN(
                    self._id, query.id, limit, minSim, metric.encode()
                ),
                BingoException,
            ),
            self,
        )

    def searchSimTopNWithExtFP(
        self, query, limit, minSim, ext_fp, metric="tanimoto"
    ):
        return BingoObject(
            IndigoLib.checkResult(
                self._lib().bingoSearchSimTopNWithExtFP(
                    self._id,
                    query.id,
                    limit,
                    minSim,
                    ext_fp.id,
                    metric.encode(),
                ),
                BingoException,
            ),
            self,
        )

    def enumerateId(self):
        return BingoObject(
            IndigoLib.checkResult(
                self._lib().bingoEnumerateId(self._id), BingoException
            ),
            self,
        )

    def searchMolFormula(self, query, options=""):
        return BingoObject(
            IndigoLib.checkResult(
                self._lib().bingoSearchMolFormula(
                    self._id, query.encode(), options.encode()
                ),
                BingoException,
            ),
            self,
        )

    def optimize(self):
        IndigoLib.checkResult(
            self._lib().bingoOptimize(self._id), BingoException
        )

    def getRecordById(self, id):
        return IndigoObject(
            self.session,
            IndigoLib.checkResult(
                self._lib().bingoGetRecordObj(self._id, id), BingoException
            ),
        )
