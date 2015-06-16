#
# Copyright (C) 2009-2015 EPAM Systems
#
# This file is part of Indigo toolkit.
#
# This file may be distributed and/or modified under the terms of the
# GNU General Public License version 3 as published by the Free Software
# Foundation and appearing in the file LICENSE.GPL included in the
# packaging of this file.
#
# This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
# WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.

import os
from indigo import *


class BingoException(Exception):

    def __init__(self, value):
        self.value = value

    def __str__(self):
        return repr(self.value.decode('ascii')) if sys.version_info > (3, 0) else repr(self.value)


class Bingo(object):
    def __init__(self, bingoId, indigo, lib):
        self._id = bingoId
        self._indigo = indigo
        self._lib = lib
        self._lib.bingoVersion.restype = c_char_p
        self._lib.bingoVersion.argtypes = None
        self._lib.bingoCreateDatabaseFile.restype = c_int
        self._lib.bingoCreateDatabaseFile.argtypes = [c_char_p, c_char_p, c_char_p]
        self._lib.bingoLoadDatabaseFile.restype = c_int
        self._lib.bingoLoadDatabaseFile.argtypes = [c_char_p, c_char_p]
        self._lib.bingoCloseDatabase.restype = c_int
        self._lib.bingoCloseDatabase.argtypes = [c_int]
        self._lib.bingoInsertRecordObj.restype = c_int
        self._lib.bingoInsertRecordObj.argtypes = [c_int, c_int]
        self._lib.bingoGetRecordObj.restype = c_int
        self._lib.bingoGetRecordObj.argtypes = [c_int, c_int]
        self._lib.bingoInsertRecordObjWithId.restype = c_int
        self._lib.bingoInsertRecordObjWithId.argtypes = [c_int, c_int, c_int]
        self._lib.bingoDeleteRecord.restype = c_int
        self._lib.bingoDeleteRecord.argtypes = [c_int, c_int]
        self._lib.bingoSearchSub.restype = c_int
        self._lib.bingoSearchSub.argtypes = [c_int, c_int, c_char_p]
        self._lib.bingoSearchExact.restype = c_int
        self._lib.bingoSearchExact.argtypes = [c_int, c_int, c_char_p]
        self._lib.bingoSearchMolFormula.restype = c_int
        self._lib.bingoSearchMolFormula.argtypes = [c_int, c_char_p, c_char_p]
        self._lib.bingoSearchSim.restype = c_int
        self._lib.bingoSearchSim.argtypes = [c_int, c_int, c_float, c_float, c_char_p]
        self._lib.bingoNext.restype = c_int
        self._lib.bingoNext.argtypes = [c_int]
        self._lib.bingoGetCurrentId.restype = c_int
        self._lib.bingoGetCurrentId.argtypes = [c_int]
        self._lib.bingoGetObject.restype = c_int
        self._lib.bingoGetObject.argtypes = [c_int]
        self._lib.bingoEndSearch.restype = c_int
        self._lib.bingoEndSearch.argtypes = [c_int]
        self._lib.bingoGetCurrentSimilarityValue.restype = c_float
        self._lib.bingoGetCurrentSimilarityValue.argtypes = [c_int]
        self._lib.bingoOptimize.restype = c_int
        self._lib.bingoOptimize.argtypes = [c_int]
        self._lib.bingoEstimateRemainingResultsCount.restype = c_int
        self._lib.bingoEstimateRemainingResultsCount.argtypes = [c_int]
        self._lib.bingoEstimateRemainingResultsCountError.restype = c_int
        self._lib.bingoEstimateRemainingResultsCountError.argtypes = [c_int]
        self._lib.bingoEstimateRemainingTime.restype = c_int
        self._lib.bingoEstimateRemainingTime.argtypes = [c_int, POINTER(c_float)]

    def __del__(self):
        self.close()

    def close(self):
        self._indigo._setSessionId()
        if self._id >= 0:
            Bingo._checkResult(self._indigo, self._lib.bingoCloseDatabase(self._id))
            self._id = -1

    @staticmethod
    def _checkResult(indigo, result):
        if result < 0:
            raise BingoException(indigo._lib.indigoGetLastError())
        return result

    @staticmethod
    def _checkResultPtr (indigo, result):
        if result is None:
            raise BingoException(indigo._lib.indigoGetLastError())
        return result

    @staticmethod
    def _checkResultString (indigo, result):
        res = Bingo._checkResultPtr(indigo, result)
        return res.decode('ascii') if sys.version_info >= (3, 0) else res.encode('ascii')

    @staticmethod
    def _getLib(indigo):
        if os.name == 'posix' and not platform.mac_ver()[0]:
            _lib = CDLL(indigo.dllpath + "/libbingo.so")
        elif os.name == 'nt':
            _lib = CDLL(indigo.dllpath + "/bingo.dll")
        elif platform.mac_ver()[0]:
            _lib = CDLL(indigo.dllpath + "/libbingo.dylib")
        else:
            raise BingoException("unsupported OS: " + os.name)
        return _lib

    @staticmethod
    def createDatabaseFile(indigo, path, databaseType, options=''):
        indigo._setSessionId()
        if not options:
            options = ''
        lib = Bingo._getLib(indigo)
        lib.bingoCreateDatabaseFile.restype = c_int
        lib.bingoCreateDatabaseFile.argtypes = [c_char_p, c_char_p, c_char_p]
        return Bingo(Bingo._checkResult(indigo, lib.bingoCreateDatabaseFile(path.encode('ascii'), databaseType.encode('ascii'), options.encode('ascii'))), indigo, lib)

    @staticmethod
    def loadDatabaseFile(indigo, path, options=''):
        indigo._setSessionId()
        if not options:
            options = ''
        lib = Bingo._getLib(indigo)
        lib.bingoLoadDatabaseFile.restype = c_int
        lib.bingoLoadDatabaseFile.argtypes = [c_char_p, c_char_p]
        return Bingo(Bingo._checkResult(indigo, lib.bingoLoadDatabaseFile(path.encode('ascii'), options.encode('ascii'))), indigo, lib)

    def version(self):
        self._indigo._setSessionId()
        return Bingo._checkResultString(self._indigo, self._lib.bingoVersion())

    def insert(self, indigoObject, index=None):
        self._indigo._setSessionId()
        if not index:
            return Bingo._checkResult(self._indigo, self._lib.bingoInsertRecordObj(self._id, indigoObject.id))
        else:
            return Bingo._checkResult(self._indigo,
                                      self._lib.bingoInsertRecordObjWithId(self._id, indigoObject.id, index))

    def delete(self, index):
        self._indigo._setSessionId()
        Bingo._checkResult(self._indigo, self._lib.bingoDeleteRecord(self._id, index))

    def searchSub(self, query, options=''):
        self._indigo._setSessionId()
        if not options:
            options = ''
        return BingoObject(Bingo._checkResult(self._indigo, self._lib.bingoSearchSub(self._id, query.id, options.encode('ascii'))),
                           self._indigo, self)

    def searchExact(self, query, options=''):
        self._indigo._setSessionId()
        if not options:
            options = ''
        return BingoObject(Bingo._checkResult(self._indigo, self._lib.bingoSearchExact(self._id, query.id, options.encode('ascii'))),
                           self._indigo, self)

    def searchSim(self, query, minSim, maxSim, metric='tanimoto'):
        self._indigo._setSessionId()
        if not metric:
            metric = 'tanimoto'
        return BingoObject(
            Bingo._checkResult(self._indigo, self._lib.bingoSearchSim(self._id, query.id, minSim, maxSim, metric.encode('ascii'))),
            self._indigo, self)

    def searchMolFormula(self, query, options=''):
        self._indigo._setSessionId()
        if not options:
            options = ''
        return BingoObject(Bingo._checkResult(self._indigo, self._lib.bingoSearchMolFormula(self._id, query.encode('ascii'), options.encode('ascii'))),
                           self._indigo, self)

    def optimize(self):
        self._indigo._setSessionId()
        Bingo._checkResult(self._indigo, self._lib.bingoOptimize(self._id))

    def getRecordById (self, id):
        self._indigo._setSessionId()
        return IndigoObject(self._indigo, Bingo._checkResult(self._indigo, self._lib.bingoGetRecordObj(self._id, id)))

class BingoObject(object):
    def __init__(self, objId, indigo, bingo):
        self._id = objId
        self._indigo = indigo
        self._bingo = bingo

    def __del__(self):
        self.close()

    def close(self):
        self._indigo._setSessionId()
        if self._id >= 0:
            Bingo._checkResult(self._indigo, self._bingo._lib.bingoEndSearch(self._id))
            self._id = -1

    def next(self):
        self._indigo._setSessionId()
        return True if Bingo._checkResult(self._indigo, self._bingo._lib.bingoNext(self._id)) == 1 else False

    def getCurrentId(self):
        self._indigo._setSessionId()
        return Bingo._checkResult(self._indigo, self._bingo._lib.bingoGetCurrentId(self._id))

    def getIndigoObject(self):
        self._indigo._setSessionId()
        return IndigoObject(self._indigo, Bingo._checkResult(self._indigo, self._bingo._lib.bingoGetObject(self._id)))

    def getCurrentSimilarityValue(self):
        self._indigo._setSessionId()
        return Bingo._checkResult(self._indigo, self._bingo._lib.bingoGetCurrentSimilarityValue(self._id))

    def estimateRemainingResultsCount(self):
        self._indigo._setSessionId()
        return Bingo._checkResult(self._indigo, self._bingo._lib.bingoEstimateRemainingResultsCount(self._id))

    def estimateRemainingResultsCountError(self):
        self._indigo._setSessionId()
        return Bingo._checkResult(self._indigo, self._bingo._lib.bingoEstimateRemainingResultsCountError(self._id))

    def estimateRemainingTime(self):
        self._indigo._setSessionId()
        value = c_float()
        Bingo._checkResult(self._indigo, self._bingo._lib.bingoEstimateRemainingTime(self._id, pointer(value)))
        return value.value
