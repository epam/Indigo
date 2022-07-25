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

from ctypes import CDLL, c_float, pointer
from typing import TYPE_CHECKING

from ..indigo.indigo_lib import IndigoLib
from ..indigo.indigo_object import IndigoObject
from .bingo_exception import BingoException

if TYPE_CHECKING:
    from .bingo import Bingo


class BingoObject:
    def __init__(self, id_: int, db: "Bingo"):
        self._id = id_
        self._db = db

    def __del__(self):
        self.close()

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.close()

    def __iter__(self):
        return self

    def __next__(self):
        next_item = self.next()
        if next_item:
            return self
        raise StopIteration

    def _lib(self) -> CDLL:
        return self._db._lib()  # noqa

    def close(self):
        if self._id != -1:
            IndigoLib.checkResult(
                self._lib().bingoEndSearch(self._id), BingoException
            )
            self._id = -1

    def next(self):
        return (
            IndigoLib.checkResult(
                self._lib().bingoNext(self._id), BingoException
            )
            == 1
        )

    def getCurrentId(self):
        return IndigoLib.checkResult(
            self._lib().bingoGetCurrentId(self._id), BingoException
        )

    def getIndigoObject(self):
        return IndigoObject(
            self._db.session,
            IndigoLib.checkResult(
                self._lib().bingoGetObject(self._id), BingoException
            ),
        )

    def getCurrentSimilarityValue(self):
        return IndigoLib.checkResult(
            self._lib().bingoGetCurrentSimilarityValue(self._id),
            BingoException,
        )

    def estimateRemainingResultsCount(self):
        return IndigoLib.checkResult(
            self._lib().bingoEstimateRemainingResultsCount(self._id),
            BingoException,
        )

    def estimateRemainingResultsCountError(self):
        return IndigoLib.checkResult(
            self._lib().bingoEstimateRemainingResultsCountError(self._id),
            BingoException,
        )

    def estimateRemainingTime(self):
        value = c_float()
        IndigoLib.checkResult(
            self._lib().bingoEstimateRemainingTime(self._id, pointer(value)),
            BingoException,
        )
        return value.value

    def containersCount(self):
        return IndigoLib.checkResult(
            self._lib().bingoContainersCount(self._id), BingoException
        )

    def cellsCount(self):
        return IndigoLib.checkResult(
            self._lib().bingoCellsCount(self._id), BingoException
        )

    def currentCell(self):
        return IndigoLib.checkResult(
            self._lib().bingoCurrentCell(self._id), BingoException
        )

    def minCell(self):
        return IndigoLib.checkResult(
            self._lib().bingoMinCell(self._id), BingoException
        )

    def maxCell(self):
        return IndigoLib.checkResult(
            self._lib().bingoMaxCell(self._id), BingoException
        )
