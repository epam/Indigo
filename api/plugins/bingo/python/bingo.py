#
# Copyright (C) 2010-2013 GGA Software Services LLC
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

from indigo import *


class BingoException(Exception):
    def __init__(self, value):
        self.value = value

    def __str__(self):
        return repr(self.value)


class Bingo(object):
    def __init__(self, id, indigo, lib):
        self.id = id
        self.indigo = indigo
        self._lib = lib
        self._lib.bingoCreateDatabaseFile.restype = c_int
        self._lib.bingoCreateDatabaseFile.argtypes = [c_char_p, c_char_p, c_char_p]
        self._lib.bingoLoadDatabaseFile.restype = c_int
        self._lib.bingoLoadDatabaseFile.argtypes = [c_char_p, c_char_p]
        self._lib.bingoCloseDatabase.restype = c_int
        self._lib.bingoCloseDatabase.argtypes = [c_int]
        self._lib.bingoInsertRecordObj.restype = c_int
        self._lib.bingoInsertRecordObj.argtypes = [c_int, c_int]
        self._lib.bingoDeleteRecord.restype = c_int
        self._lib.bingoDeleteRecord.argtypes = [c_int, c_int]
        self._lib.bingoSearchSub.restype = c_int
        self._lib.bingoSearchSub.argtypes = [c_int, c_int, c_char_p]
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

    def __del__(self):
        self.indigo._checkResult(self._lib.bingoCloseDatabase(self.id))
        self.id = -1

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
    def createDatabaseFile(indigo, path, type, options=''):
        if not options:
            options = ''
        lib = Bingo._getLib(indigo)
        lib.bingoCreateDatabaseFile.restype = c_int
        lib.bingoCreateDatabaseFile.argtypes = [c_char_p, c_char_p, c_char_p]
        return Bingo(indigo._checkResult(lib.bingoCreateDatabaseFile(path, type, options)), indigo, lib)

    @staticmethod
    def loadDatabaseFile(indigo, path, type):
        lib = Bingo._getLib(indigo)
        lib.bingoLoadDatabaseFile.restype = c_int
        lib.bingoLoadDatabaseFile.argtypes = [c_char_p, c_char_p]
        return Bingo(indigo._checkResult(lib.bingoLoadDatabaseFile(path, type)), indigo, lib)

    def insert(self, indigoObject):
        self.indigo._checkResult(self._lib.bingoInsertRecordObj(self.id, indigoObject.id))

    def delete(self, index):
        self.indigo._checkResult(self._lib.bingoDeleteRecord(self.id, index))

    def searchSub(self, query, options=''):
        if not options:
            options = ''
        return BingoObject(self.indigo._checkResult(self._lib.bingoSearchSub(self.id, query.id, options)),
                           self.indigo, self)

    def searchSim(self, query, min, max, metric='tanimoto'):
        if not metric:
            metric = 'tanimoto'
        return BingoObject(self.indigo._checkResult(self._lib.bingoSearchSim(self.id, query.id, min, max, metric)),
                           self.indigo, self)


class BingoObject(object):
    def __init__(self, id, indigo, bingo):
        self.id = id
        self.indigo = indigo
        self.bingo = bingo

    def __del__(self):
        self.indigo._checkResult(self.bingo._lib.bingoEndSearch(self.id))
        self.id = -1

    def next(self):
        return True if self.indigo._checkResult(self.bingo._lib.bingoNext(self.id)) == 1 else False

    def getCurrentId(self):
        return self.indigo._checkResult(self.bingo._lib.bingoGetCurrentId(self.id))

    def getIndigoObject(self):
        return self.indigo.IndigoObject(self.indigo, self.indigo._checkResult(self.bingo._lib.bingoGetObject(self.id)))
