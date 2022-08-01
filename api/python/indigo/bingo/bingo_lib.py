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
from ctypes import CDLL, POINTER, c_char_p, c_float, c_int
from typing import Optional

from .._common.lib import Lib


class BingoLib:
    lib: Optional[CDLL] = None

    def __init__(self) -> None:
        if BingoLib.lib:
            return

        BingoLib.lib = Lib.load("bingo-nosql")
        BingoLib.lib.bingoVersion.restype = c_char_p
        BingoLib.lib.bingoVersion.argtypes = []
        BingoLib.lib.bingoCreateDatabaseFile.restype = c_int
        BingoLib.lib.bingoCreateDatabaseFile.argtypes = [
            c_char_p,
            c_char_p,
            c_char_p,
        ]
        BingoLib.lib.bingoLoadDatabaseFile.restype = c_int
        BingoLib.lib.bingoLoadDatabaseFile.argtypes = [c_char_p, c_char_p]
        BingoLib.lib.bingoCloseDatabase.restype = c_int
        BingoLib.lib.bingoCloseDatabase.argtypes = [c_int]
        BingoLib.lib.bingoInsertRecordObj.restype = c_int
        BingoLib.lib.bingoInsertRecordObj.argtypes = [c_int, c_int]
        BingoLib.lib.bingoInsertRecordObjWithExtFP.restype = c_int
        BingoLib.lib.bingoInsertRecordObjWithExtFP.argtypes = [
            c_int,
            c_int,
            c_int,
        ]
        BingoLib.lib.bingoGetRecordObj.restype = c_int
        BingoLib.lib.bingoGetRecordObj.argtypes = [c_int, c_int]
        BingoLib.lib.bingoInsertRecordObjWithId.restype = c_int
        BingoLib.lib.bingoInsertRecordObjWithId.argtypes = [
            c_int,
            c_int,
            c_int,
        ]
        BingoLib.lib.bingoInsertRecordObjWithIdAndExtFP.restype = c_int
        BingoLib.lib.bingoInsertRecordObjWithIdAndExtFP.argtypes = [
            c_int,
            c_int,
            c_int,
            c_int,
        ]
        BingoLib.lib.bingoDeleteRecord.restype = c_int
        BingoLib.lib.bingoDeleteRecord.argtypes = [c_int, c_int]
        BingoLib.lib.bingoSearchSub.restype = c_int
        BingoLib.lib.bingoSearchSub.argtypes = [c_int, c_int, c_char_p]
        BingoLib.lib.bingoSearchExact.restype = c_int
        BingoLib.lib.bingoSearchExact.argtypes = [c_int, c_int, c_char_p]
        BingoLib.lib.bingoSearchMolFormula.restype = c_int
        BingoLib.lib.bingoSearchMolFormula.argtypes = [
            c_int,
            c_char_p,
            c_char_p,
        ]
        BingoLib.lib.bingoSearchSim.restype = c_int
        BingoLib.lib.bingoSearchSim.argtypes = [
            c_int,
            c_int,
            c_float,
            c_float,
            c_char_p,
        ]
        BingoLib.lib.bingoSearchSimWithExtFP.restype = c_int
        BingoLib.lib.bingoSearchSimWithExtFP.argtypes = [
            c_int,
            c_int,
            c_float,
            c_float,
            c_int,
            c_char_p,
        ]
        BingoLib.lib.bingoSearchSimTopN.restype = c_int
        BingoLib.lib.bingoSearchSimTopN.argtypes = [
            c_int,
            c_int,
            c_int,
            c_float,
            c_char_p,
        ]
        BingoLib.lib.bingoSearchSimTopNWithExtFP.restype = c_int
        BingoLib.lib.bingoSearchSimTopNWithExtFP.argtypes = [
            c_int,
            c_int,
            c_int,
            c_float,
            c_int,
            c_char_p,
        ]
        BingoLib.lib.bingoEnumerateId.restype = c_int
        BingoLib.lib.bingoEnumerateId.argtypes = [c_int]
        BingoLib.lib.bingoNext.restype = c_int
        BingoLib.lib.bingoNext.argtypes = [c_int]
        BingoLib.lib.bingoGetCurrentId.restype = c_int
        BingoLib.lib.bingoGetCurrentId.argtypes = [c_int]
        BingoLib.lib.bingoGetObject.restype = c_int
        BingoLib.lib.bingoGetObject.argtypes = [c_int]
        BingoLib.lib.bingoEndSearch.restype = c_int
        BingoLib.lib.bingoEndSearch.argtypes = [c_int]
        BingoLib.lib.bingoGetCurrentSimilarityValue.restype = c_float
        BingoLib.lib.bingoGetCurrentSimilarityValue.argtypes = [c_int]
        BingoLib.lib.bingoOptimize.restype = c_int
        BingoLib.lib.bingoOptimize.argtypes = [c_int]
        BingoLib.lib.bingoEstimateRemainingResultsCount.restype = c_int
        BingoLib.lib.bingoEstimateRemainingResultsCount.argtypes = [c_int]
        BingoLib.lib.bingoEstimateRemainingResultsCountError.restype = c_int
        BingoLib.lib.bingoEstimateRemainingResultsCountError.argtypes = [c_int]
        BingoLib.lib.bingoEstimateRemainingTime.restype = c_int
        BingoLib.lib.bingoEstimateRemainingTime.argtypes = [
            c_int,
            POINTER(c_float),
        ]
        BingoLib.lib.bingoContainersCount.restype = c_int
        BingoLib.lib.bingoContainersCount.argtypes = [c_int]
        BingoLib.lib.bingoCellsCount.restype = c_int
        BingoLib.lib.bingoCellsCount.argtypes = [c_int]
        BingoLib.lib.bingoCurrentCell.restype = c_int
        BingoLib.lib.bingoCurrentCell.argtypes = [c_int]
        BingoLib.lib.bingoMinCell.restype = c_int
        BingoLib.lib.bingoMinCell.argtypes = [c_int]
        BingoLib.lib.bingoMaxCell.restype = c_int
        BingoLib.lib.bingoMaxCell.argtypes = [c_int]
