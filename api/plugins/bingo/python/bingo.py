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
    def __init__(self, indigo):
        self.indigo = indigo
        self.bingo = -1

        if os.name == 'posix' and not platform.mac_ver()[0]:
            self._lib = CDLL(indigo.dllpath + "/libbingo.so")
        elif os.name == 'nt':
            self._lib = CDLL(indigo.dllpath + "\bingo.dll")
        elif platform.mac_ver()[0]:
            self._lib = CDLL(indigo.dllpath + "/libbingo.dylib")
        else:
            raise BingoException("unsupported OS: " + os.name)

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
        self.bingo = -1

    def createDatabaseFile(self, path, type, options=''):
        self.indigo._setSID()
        if not options:
            options = ''
        self.bingo = self.indigo._checkResult(self._lib.bingoCreateDatabaseFile(path, type, options))

    def loadDatabaseFile(self, path, type):
        self.indigo._setSID()
        self.bingo = self.indigo._checkResult(self._lib.bingoLoadDatabaseFile(path, type))

    def closeDatabase(self):
        self.indigo._setSID()
        self.indigo._checkResult(self._lib.bingoCloseDatabase(self.bingo))
        self.bingo = -1

    def insertRecordObj(self, obj):
        self.indigo._setSID()
        self.indigo._checkResult(self._lib.bingoInsertRecordObj(self.bingo, obj.id))

    def deleteRecord(self, index):
        self.indigo._setSID()
        self.indigo._checkResult(self._lib.bingoDeleteRecord(self.bingo, index))

    def searchSub(self, query, options=''):
        self.indigo._setSID()
        if not options:
            options = ''
        return BingoObject(self.indigo._checkResult(self._lib.bingoSearchSub(self.bingo, query.id, options)),
                           self.indigo, self)

    def searchSim(self, query, min, max, metric='tanimoto'):
        self.indigo._setSID()
        if not metric:
            metric = 'tanimoto'
        return BingoObject(self.indigo._checkResult(self._lib.bingoSearchSim(self.bingo, query.id, min, max, metric)),
                           self.indigo, self)


class BingoObject(object):
    def __init__(self, id, indigo, bingo):
        self.id = id
        self.indigo = indigo
        self.bingo = bingo

    def __del__(self):
        self.id = -1

    def next(self):
        self.indigo._setSID()
        return True if self.indigo._checkResult(self.bingo._lib.bingoNext(self.id)) == 1 else False


    def getCurrentId(self):
        self.indigo._setSID()
        return self.indigo._checkResult(self.bingo._lib.bingoGetCurrentId(self.id))

    def getObject(self):
        self.indigo._setSID()
        return self.indigo.IndigoObject(self.indigo, self.indigo._checkResult(self.bingo._lib.bingoGetObject(self.id)))

    def endSearch(self):
        self.indigo._setSID()
        self.indigo._checkResult(self.bingo._lib.bingoEndSearch(self.id))
