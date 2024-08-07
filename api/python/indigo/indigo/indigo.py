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

__version__ = "1.7.4"

import warnings
from ctypes import CDLL, c_byte, c_double, c_float, c_int, c_ubyte, pointer

from .indigo_exception import IndigoException
from .indigo_lib import IndigoLib
from .indigo_object import IndigoObject


class Indigo:
    ABS = 1
    OR = 2
    AND = 3
    EITHER = 4
    UP = 5
    DOWN = 6
    CIS = 7
    TRANS = 8
    CHAIN = 9
    RING = 10
    ALLENE = 11

    SINGLET = 101
    DOUBLET = 102
    TRIPLET = 103
    RC_NOT_CENTER = -1
    RC_UNMARKED = 0
    RC_CENTER = 1
    RC_UNCHANGED = 2
    RC_MADE_OR_BROKEN = 4
    RC_ORDER_CHANGED = 8

    SG_TYPE_GEN = 0
    SG_TYPE_DAT = 1
    SG_TYPE_SUP = 2
    SG_TYPE_SRU = 3
    SG_TYPE_MUL = 4
    SG_TYPE_MON = 5
    SG_TYPE_MER = 6
    SG_TYPE_COP = 7
    SG_TYPE_CRO = 8
    SG_TYPE_MOD = 9
    SG_TYPE_GRA = 10
    SG_TYPE_COM = 11
    SG_TYPE_MIX = 12
    SG_TYPE_FOR = 13
    SG_TYPE_ANY = 14

    def getSessionId(self) -> int:
        return self._sid

    def _setSessionId(self):
        IndigoLib.lib.indigoSetSessionId(self._sid)

    def _lib(self) -> CDLL:
        self._setSessionId()
        return self._libraryInstance.lib  # type: ignore

    def __init__(self):
        self._libraryInstance: IndigoLib = IndigoLib()
        self._sid = self._libraryInstance.lib.indigoAllocSessionId()

    def __del__(self):
        if self._sid != -1:
            self._lib().indigoReleaseSessionId(self._sid)
            self._sid = -1

    def deserialize(self, arr: bytes) -> IndigoObject:
        """Creates molecule or reaction object from binary serialized CMF
           format

        Args:
            arr (bytes): array of bytes

        Returns:
            IndigoObject: molecule or reaction object
        """
        values = (c_ubyte * len(arr))()
        for i in range(len(arr)):
            values[i] = arr[i]
        res = self._lib().indigoUnserialize(values, len(arr))
        return IndigoObject(self, IndigoLib.checkResult(res))

    def unserialize(self, arr: bytes) -> IndigoObject:
        """[DEPRECATED] Creates molecule or reaction object from binary
           serialized CMF format

        Args:
            arr (list): array of bytes

        Returns:
            IndigoObject: molecule or reaction object
        """
        warnings.warn(
            "unserialize() is deprecated, use deserialize() instead",
            DeprecationWarning,
        )
        return self.deserialize(arr)

    def setOption(self, option, value1, value2=None, value3=None):
        """Sets option value

        Args:
            option (str): option name
            value1 (int, str, bool, float): option value
            value2 (int, float): option value for tuples. Optional, defaults
                                 to None.
            value3 (float): option value for triple. Optional,
                            defaults to None.

        Raises:
            IndigoException: if option does not exist
        """

        opt = option.encode()
        if (
            isinstance(value1, float)
            and isinstance(value2, float)
            and isinstance(value3, float)
        ):
            IndigoLib.checkResult(
                self._lib().indigoSetOptionColor(opt, value1, value2, value3)
            )
        elif (
            isinstance(value1, int)
            and isinstance(value2, int)
            and value3 is None
        ):
            IndigoLib.checkResult(
                self._lib().indigoSetOptionXY(opt, value1, value2)
            )
        elif value2 is None and value3 is None:
            if isinstance(value1, str):
                setOpt = self._lib().indigoSetOption
                value = value1.encode()
            elif isinstance(value1, int):
                setOpt = self._lib().indigoSetOptionInt
                value = value1
            elif isinstance(value1, float):
                setOpt = self._lib().indigoSetOptionFloat
                value = value1
            elif isinstance(value1, bool):
                value1 = 0
                if value1:
                    value1 = 1
                setOpt = self._lib().indigoSetOptionBool
            else:
                raise IndigoException("bad option")
            IndigoLib.checkResult(setOpt(opt, value))
        else:
            raise IndigoException("bad option")

    def getOption(self, option):
        """Returns option value by name

        Args:
            option (str): option name

        Returns:
            str: option value
        """

        return IndigoLib.checkResultString(
            self._lib().indigoGetOption(option.encode())
        )

    def getOptionInt(self, option):
        """Returns option integer value by name

        Args:
            option (str): option name

        Returns:
            int: option value
        """

        value = c_int()
        IndigoLib.checkResult(
            self._lib().indigoGetOptionInt(option.encode(), pointer(value))
        )
        return value.value

    def getOptionBool(self, option):
        """Returns option boolean value by name

        Args:
            option (str): option name

        Returns:
            bool: option value
        """

        value = c_int()
        IndigoLib.checkResult(
            self._lib().indigoGetOptionBool(option.encode(), pointer(value))
        )
        if value.value == 1:
            return True
        return False

    def getOptionFloat(self, option):
        """Returns option float value by name

        Args:
            option (str): option name

        Returns:
            float: option value
        """

        value = c_float()
        IndigoLib.checkResult(
            self._lib().indigoGetOptionFloat(option.encode(), pointer(value))
        )
        return value.value

    def getOptionType(self, option):
        """Returns option value type by name

        Args:
            option (str): option name

        Returns:
            str: option type string
        """

        return IndigoLib.checkResultString(
            self._lib().indigoGetOptionType(option.encode())
        )

    def resetOptions(self):
        """Resets options to default state"""

        IndigoLib.checkResult(self._lib().indigoResetOptions())

    def convertToArray(self, iterable):
        """Converts iterable object to array

        Args:
            iterable (IndigoObject): iterable object

        Raises:
            IndigoException: if object is not iterable

        Returns:
            IndigoObject: array of objects
        """
        if isinstance(iterable, IndigoObject):
            return iterable
        try:
            some_object_iterator = iter(iterable)
            res = self.createArray()
            for obj in some_object_iterator:
                res.arrayAdd(self.convertToArray(obj))
            return res
        except TypeError:
            raise IndigoException(
                "Cannot convert object %s to an array" % iterable
            )

    def dbgBreakpoint(self):
        return self._lib().indigoDbgBreakpoint()

    def version(self):
        """Returns Indigo version

        Returns:
            str: version string
        """

        return IndigoLib.checkResultString(self._lib().indigoVersion())

    def versionInfo(self):
        """Returns Indigo version info

        Returns:
            str: version info string
        """

        return IndigoLib.checkResultString(self._lib().indigoVersionInfo())

    def countReferences(self):
        """Returns the number of objects in pool

        Returns:
            int: number of objects
        """

        return IndigoLib.checkResult(self._lib().indigoCountReferences())

    def writeFile(self, filename):
        """Creates file writer object

        Args:
            filename (str): full path to the file

        Returns:
            IndigoObject: file writer object
        """

        return IndigoObject(
            self,
            IndigoLib.checkResult(
                self._lib().indigoWriteFile(filename.encode())
            ),
        )

    def writeBuffer(self):
        """Creates buffer to write an object

        Returns:
            IndigoObject: buffer object
        """

        return IndigoObject(
            self, IndigoLib.checkResult(self._lib().indigoWriteBuffer())
        )

    def createMolecule(self):
        """Creates molecule object

        Returns:
            IndigoObject: molecule object
        """

        return IndigoObject(
            self, IndigoLib.checkResult(self._lib().indigoCreateMolecule())
        )

    def createQueryMolecule(self):
        """Creates query molecule object

        Returns:
            IndigoObject: query molecule
        """

        return IndigoObject(
            self,
            IndigoLib.checkResult(self._lib().indigoCreateQueryMolecule()),
        )

    def loadMolecule(self, string):
        """Loads molecule from string. Format is automatically recognized.

        Args:
            string (str): molecule format

        Returns:
            IndigoObject: molecule object

        Raises:
            IndigoException: Exception if structure format is incorrect
        """

        return IndigoObject(
            self,
            IndigoLib.checkResult(
                self._lib().indigoLoadMoleculeFromString(string.encode())
            ),
        )

    def loadMoleculeFromFile(self, filename):
        """Loads molecule from file. Automatically detects input format.

        Args:
            filename (str): full path to a file

        Returns:
            IndigoObject: loaded molecular structure

        Raises:
            IndigoException: Exception if structure format is incorrect
        """

        return IndigoObject(
            self,
            IndigoLib.checkResult(
                self._lib().indigoLoadMoleculeFromFile(filename.encode())
            ),
        )

    def loadMoleculeFromBuffer(self, data):
        """Loads molecule from buffer. Automatically detects input format.

        Args:
            data (bytes): input byte array

        Returns:
            IndigoObject: loaded molecular structure

        Raises:
            IndigoException: Exception if structure format is incorrect

        Examples:
            .. code-block:: python

                with open (..), 'rb') as f:
                    m = indigo.loadMoleculeFromBuffer(f.read())
        """
        buf = data
        values = (c_byte * len(buf))()
        for i in range(len(buf)):
            values[i] = buf[i]

        return IndigoObject(
            self,
            IndigoLib.checkResult(
                self._lib().indigoLoadMoleculeFromBuffer(values, len(buf))
            ),
        )

    def loadQueryMolecule(self, string):
        """Loads query molecule from string. Format will be automatically
           recognized.

        Args:
            string (str): molecule format

        Returns:
            IndigoObject: query molecule object

        Raises:
            IndigoException: Exception if structure format is incorrect
        """

        return IndigoObject(
            self,
            IndigoLib.checkResult(
                self._lib().indigoLoadQueryMoleculeFromString(string.encode())
            ),
        )

    def loadQueryMoleculeFromFile(self, filename):
        """
        Loads query molecule from file. Automatically detects input format.

        Args:
            filename (str): full path to a file

        Returns:
            IndigoObject: loaded query molecular structure

        Raises:
            IndigoException: Exception if structure format is incorrect
        """

        return IndigoObject(
            self,
            IndigoLib.checkResult(
                self._lib().indigoLoadQueryMoleculeFromFile(filename.encode())
            ),
        )

    def loadSmarts(self, string):
        """Loads query molecule from string in SMARTS format

        Args:
            string (str): smarts string

        Returns:
            IndigoObject: loaded query molecular structure

        Raises:
            IndigoException: Exception if structure format is incorrect
        """

        return IndigoObject(
            self,
            IndigoLib.checkResult(
                self._lib().indigoLoadSmartsFromString(string.encode())
            ),
        )

    def loadSmartsFromFile(self, filename):
        """Loads query molecule from file in SMARTS format

        Args:
            filename (str): full path to the file with smarts strings

        Returns:
            IndigoObject: loaded query molecular structure

        Raises:
            IndigoException: Exception if structure format is incorrect
        """

        return IndigoObject(
            self,
            IndigoLib.checkResult(
                self._lib().indigoLoadSmartsFromFile(filename.encode())
            ),
        )

    def loadMonomerLibrary(self, string):
        """Loads monomer library from ket string

        Args:
            string (str): ket

        Returns:
            IndigoObject: loaded monomer library

        Raises:
            IndigoException: Exception if structure format is incorrect
        """

        return IndigoObject(
            self,
            IndigoLib.checkResult(
                self._lib().indigoLoadMonomerLibraryFromString(string.encode())
            ),
        )

    def loadMonomerLibraryFromFile(self, filename):
        """Loads monomer library from from file in ket format

        Args:
            string (str): full path to the file with ket

        Returns:
            IndigoObject: loaded monomer library

        Raises:
            IndigoException: Exception if structure format is incorrect
        """

        return IndigoObject(
            self,
            IndigoLib.checkResult(
                self._lib().indigoLoadMonomerLibraryFromFile(filename.encode())
            ),
        )

    def loadKetDocument(self, string):
        """Loads ket document from ket string

        Args:
            string (str): ket

        Returns:
            IndigoObject: loaded ket document

        Raises:
            IndigoException: Exception if structure format is incorrect
        """

        return IndigoObject(
            self,
            IndigoLib.checkResult(
                self._lib().indigoLoadKetDocumentFromString(string.encode())
            ),
        )

    def loadKetDocumentFromFile(self, filename):
        """Loads ket document from from file in ket format

        Args:
            string (str): full path to the file with ket

        Returns:
            IndigoObject: loaded ket document

        Raises:
            IndigoException: Exception if structure format is incorrect
        """

        return IndigoObject(
            self,
            IndigoLib.checkResult(
                self._lib().indigoLoadKetDocumentFromFile(filename.encode())
            ),
        )

    def loadSequence(self, string, seq_type, library):
        """Loads molecule from DNA/RNA/PEPTIDE sequence string

        Args:
            string (str): sequence string
            seq_type (str): sequence type (RNA/DNA/PEPTIDE)

        Returns:
            IndigoObject: loaded query molecular structure

        Raises:
            IndigoException: Exception if structure format is incorrect
        """

        return IndigoObject(
            self,
            IndigoLib.checkResult(
                self._lib().indigoLoadSequenceFromString(
                    string.encode(), seq_type.encode(), library.id
                )
            ),
        )

    def loadSequenceFromFile(self, filename, seq_type, library):
        """Loads query molecule from file in sequence format

        Args:
            filename (str): full path to the file with sequence string
            seq_type (str): sequence type (RNA/DNA/PEPTIDE)

        Returns:
            IndigoObject: loaded query molecular structure

        Raises:
            IndigoException: Exception if structure format is incorrect
        """

        return IndigoObject(
            self,
            IndigoLib.checkResult(
                self._lib().indigoLoadSequenceFromFile(
                    filename.encode(), seq_type.encode(), library.id
                )
            ),
        )

    def loadFasta(self, string, seq_type, library):
        """Loads molecule from DNA/RNA/PEPTIDE sequence string

        Args:
            string (str): sequence string
            seq_type (str): sequence type (RNA/DNA/PEPTIDE)

        Returns:
            IndigoObject: loaded query molecular structure

        Raises:
            IndigoException: Exception if structure format is incorrect
        """

        return IndigoObject(
            self,
            IndigoLib.checkResult(
                self._lib().indigoLoadFastaFromString(
                    string.encode(), seq_type.encode(), library.id
                )
            ),
        )

    def loadFastaFromFile(self, filename, seq_type, library):
        """Loads query molecule from file in sequence format

        Args:
            filename (str): full path to the file with sequence string
            seq_type (str): sequence type (RNA/DNA/PEPTIDE)

        Returns:
            IndigoObject: loaded query molecular structure

        Raises:
            IndigoException: Exception if structure format is incorrect
        """

        return IndigoObject(
            self,
            IndigoLib.checkResult(
                self._lib().indigoLoadFastaFromFile(
                    filename.encode(), seq_type.encode(), library.id
                )
            ),
        )

    def loadIdt(self, string, library):
        """Loads molecule from IDT string

        Args:
            string (str): sequence string

        Returns:
            IndigoObject: loaded query molecular structure

        Raises:
            IndigoException: Exception if structure format is incorrect
        """

        return IndigoObject(
            self,
            IndigoLib.checkResult(
                self._lib().indigoLoadIdtFromString(
                    string.encode(), library.id
                )
            ),
        )

    def loadIdtFromFile(self, filename, library):
        """Loads query molecule from file in IDT sequence format

        Args:
            filename (str): full path to the file with sequence string

        Returns:
            IndigoObject: loaded query molecular structure

        Raises:
            IndigoException: Exception if structure format is incorrect
        """

        return IndigoObject(
            self,
            IndigoLib.checkResult(
                self._lib().indigoLoadIdtFromFile(
                    filename.encode(), library.id
                )
            ),
        )

    def loadHelm(self, string, library):
        """Loads molecule from HELM string

        Args:
            string (str): sequence string

        Returns:
            IndigoObject: loaded query molecular structure

        Raises:
            IndigoException: Exception if structure format is incorrect
        """

        return IndigoObject(
            self,
            IndigoLib.checkResult(
                self._lib().indigoLoadHelmFromString(
                    string.encode(), library.id
                )
            ),
        )

    def loadHelmFromFile(self, filename, library):
        """Loads query molecule from file in HELM sequence format

        Args:
            filename (str): full path to the file with sequence string

        Returns:
            IndigoObject: loaded query molecular structure

        Raises:
            IndigoException: Exception if structure format is incorrect
        """

        return IndigoObject(
            self,
            IndigoLib.checkResult(
                self._lib().indigoLoadHelmFromFile(
                    filename.encode(), library.id
                )
            ),
        )

    def loadReaction(self, string):
        """Loads reaction from string. Format will be automatically recognized.

        Args:
            string (str): reaction format

        Returns:
            IndigoObject: reaction object

        Raises:
            IndigoException: Exception if structure format is incorrect
        """

        return IndigoObject(
            self,
            IndigoLib.checkResult(
                self._lib().indigoLoadReactionFromString(string.encode())
            ),
        )

    def loadReactionFromFile(self, filename):
        """Loads reaction from file

        Args:
            filename (str): full path to a file

        Returns:
            IndigoObject: loaded reaction

        Raises:
            IndigoException: Exception if structure format is incorrect
        """

        return IndigoObject(
            self,
            IndigoLib.checkResult(
                self._lib().indigoLoadReactionFromFile(filename.encode())
            ),
        )

    def loadQueryReaction(self, string):
        """Loads query reaction from string. Format will be automatically
           recognized.

        Args:
            string (str): reaction format

        Returns:
            IndigoObject: query reaction object
        """

        return IndigoObject(
            self,
            IndigoLib.checkResult(
                self._lib().indigoLoadQueryReactionFromString(string.encode())
            ),
        )

    def loadQueryReactionFromFile(self, filename):
        """Loads query reaction from file. Automatically detects input format.

        Args:
            filename (str): full path to a file

        Returns:
            IndigoObject: loaded query reaction object

        Raises:
            IndigoException: Exception if structure format is incorrect
        """

        return IndigoObject(
            self,
            IndigoLib.checkResult(
                self._lib().indigoLoadQueryReactionFromFile(filename.encode())
            ),
        )

    def loadReactionSmarts(self, string):
        """Loads query reaction from string in SMARTS format

        Args:
            string (str): smarts string

        Returns:
            IndigoObject: loaded query reaction

        Raises:
            IndigoException: Exception if structure format is incorrect
        """

        return IndigoObject(
            self,
            IndigoLib.checkResult(
                self._lib().indigoLoadReactionSmartsFromString(string.encode())
            ),
        )

    def loadReactionSmartsFromFile(self, filename):
        """Loads query reaction from file in SMARTS format

        Args:
            filename (str): full path to the file with smarts strings

        Returns:
            IndigoObject: loaded query reaction

        Raises:
            IndigoException: Exception if structure format is incorrect
        """

        return IndigoObject(
            self,
            IndigoLib.checkResult(
                self._lib().indigoLoadReactionSmartsFromFile(filename.encode())
            ),
        )

    def loadStructure(self, structure_str, parameter=None):
        """Loads structure from string

        Args:
            structure_str (str): string with structure format
            parameter (str): parameters for loading. Optional, defaults to
                             None.

        Returns:
            IndigoObject: loaded object
        """

        parameter = "" if parameter is None else parameter
        return IndigoObject(
            self,
            IndigoLib.checkResult(
                self._lib().indigoLoadStructureFromString(
                    structure_str.encode(),
                    parameter.encode(),
                )
            ),
        )

    def loadStructureFromBuffer(self, structure_data, parameter=None):
        """Loads structure object from buffer

        Args:
            structure_data (list): array of bytes
            parameter (str): parameters for loading. Optional, defaults to
                             None.

        Returns:
            IndigoObject: loaded object
        """
        buf = structure_data
        values = (c_byte * len(buf))()
        for i in range(len(buf)):
            values[i] = buf[i]

        parameter = "" if parameter is None else parameter
        return IndigoObject(
            self,
            IndigoLib.checkResult(
                self._lib().indigoLoadStructureFromBuffer(
                    values, len(buf), parameter.encode()
                )
            ),
        )

    def loadStructureFromFile(self, filename, parameter=None):
        """Loads structure object from file

        Args:
            filename (str): full path with structure information
            parameter (str): parameters to load. Optional, defaults to None.

        Returns:
            IndigoObject: loaded object
        """

        parameter = "" if parameter is None else parameter
        return IndigoObject(
            self,
            IndigoLib.checkResult(
                self._lib().indigoLoadStructureFromFile(
                    filename.encode(),
                    parameter.encode(),
                )
            ),
        )

    def checkStructure(self, structure, props=""):
        """Runs validation for the given structure

        Args:
            structure (str): structure object
            props (str): Parameters for validation. Optional, defaults to "".

        Returns:
           str: validation results string
        """
        if props is None:
            props = ""

        return IndigoLib.checkResultString(
            self._lib().indigoCheckStructure(
                structure.encode(),
                props.encode(),
            )
        )

    def loadFingerprintFromBuffer(self, buffer):
        """Creates a fingerprint from the supplied binary data

        Args:
            buffer (list): array of bytes

        Returns:
            IndigoObject: fingerprint object
        """

        length = len(buffer)

        values = (c_byte * length)()
        for i in range(length):
            values[i] = buffer[i]

        return IndigoObject(
            self,
            IndigoLib.checkResult(
                self._lib().indigoLoadFingerprintFromBuffer(values, length)
            ),
        )

    def loadFingerprintFromDescriptors(self, descriptors, size, density):
        """Packs a list of molecule descriptors into a fingerprint object

        Args:
            descriptors (list): list of normalized numbers (roughly) between
                                0.0 and 1.0
            size (int): size of the fingerprint in bytes
            density (float): approximate density of '1's vs '0's in the
                             fingerprint

        Returns:
            IndigoObject: fingerprint object
        """

        length = len(descriptors)

        descr_arr = (c_double * length)()
        for i in range(length):
            descr_arr[i] = descriptors[i]

        result = self._lib().indigoLoadFingerprintFromDescriptors(
            descr_arr, length, size, density
        )
        return IndigoObject(self, IndigoLib.checkResult(result))

    def createReaction(self):
        """Creates reaction object

        Returns:
            IndigoObject: reaction object
        """

        return IndigoObject(
            self, IndigoLib.checkResult(self._lib().indigoCreateReaction())
        )

    def createQueryReaction(self):
        """Creates query reaction object

        Returns:
            IndigoObject: query reaction object
        """

        return IndigoObject(
            self,
            IndigoLib.checkResult(self._lib().indigoCreateQueryReaction()),
        )

    def exactMatch(self, item1, item2, flags=""):
        """Creates match object for the given structures

        Args:
            item1 (IndigoObject): first target structure (molecule or reaction)
            item2 (IndigoObject): second target structure
                                  (molecule or reaction)
            flags (str): exact match options. Optional, defaults to "".

        Returns:
            IndigoObject: match object
        """
        if flags is None:
            flags = ""

        new_obj = IndigoLib.checkResult(
            self._lib().indigoExactMatch(item1.id, item2.id, flags.encode())
        )
        if new_obj == 0:
            return None
        else:
            return IndigoObject(self, new_obj, [item1, item2, self])

    def setTautomerRule(self, index, beg, end):
        """Sets tautomer rules

        Args:
            index (int): tau rule index
            beg (str): begin value
            end (str): end value

        Returns:
            int: 1 if there are no errors
        """

        return IndigoLib.checkResult(
            self._lib().indigoSetTautomerRule(
                index, beg.encode(), end.encode()
            )
        )

    def removeTautomerRule(self, index):
        """Removes tautomer rule

        Args:
            index (int): tau rule index

        Returns:
            int: 1 if there are no errors
        """

        return IndigoLib.checkResult(
            self._lib().indigoRemoveTautomerRule(index)
        )

    def clearTautomerRules(self):
        """Clears all tautomer rules

        Returns:
            int: 1 if there are no errors
        """

        return IndigoLib.checkResult(self._lib().indigoClearTautomerRules())

    def commonBits(self, fingerprint1, fingerprint2):
        """Returns the number of common 1 bits for the given fingerprints

        Args:
            fingerprint1 (IndigoObject): first fingerprint object
            fingerprint2 (IndigoObject): second fingerprint object

        Returns:
            int: number of common bits
        """

        return IndigoLib.checkResult(
            self._lib().indigoCommonBits(fingerprint1.id, fingerprint2.id)
        )

    def similarity(self, item1, item2, metrics=""):
        """Returns the similarity measure between two structures.
        Accepts two molecules, two reactions, or two fingerprints.

        Args:
            item1 (IndigoObject): molecule, reaction or fingerprint object
            item2 (IndigoObject): molecule, reaction or fingerprint object
            metrics (str): "tanimoto", "tversky", "tversky <alpha> <beta>",
                           "euclid-sub" or "normalized-edit".
                           Optional, defaults to "tanimoto".

        Returns:
            float: [description]
        """
        if metrics is None:
            metrics = ""

        return IndigoLib.checkResultFloat(
            self._lib().indigoSimilarity(item1.id, item2.id, metrics.encode())
        )

    def iterateSDFile(self, filename):
        """Returns iterator for SDF files

        Args:
            filename (str): full file path

        Returns:
            IndigoObject: SD iterator object
        """

        return IndigoObject(
            self,
            IndigoLib.checkResult(
                self._lib().indigoIterateSDFile(filename.encode())
            ),
        )

    def iterateRDFile(self, filename):
        """Returns iterator for RDF files

        Args:
            filename (str): full file path

        Returns:
            IndigoObject: RD iterator object
        """

        return IndigoObject(
            self,
            IndigoLib.checkResult(
                self._lib().indigoIterateRDFile(filename.encode())
            ),
        )

    def iterateSmilesFile(self, filename):
        """Returns iterator for smiles files

        Args:
            filename (str): full file path

        Returns:
            IndigoObject: smiles iterator object
        """

        return IndigoObject(
            self,
            IndigoLib.checkResult(
                self._lib().indigoIterateSmilesFile(filename.encode())
            ),
        )

    def iterateCMLFile(self, filename):
        """Returns iterator for CML files

        Args:
            filename (str): full file path

        Returns:
            IndigoObject: CML iterator object
        """

        return IndigoObject(
            self,
            IndigoLib.checkResult(
                self._lib().indigoIterateCMLFile(filename.encode())
            ),
        )

    def iterateCDXFile(self, filename):
        """Returns iterator for CDX files

        Args:
            filename (str): full file path

        Returns:
            IndigoObject: CDX iterator object
        """

        return IndigoObject(
            self,
            IndigoLib.checkResult(
                self._lib().indigoIterateCDXFile(filename.encode())
            ),
        )

    def createFileSaver(self, filename, format_):
        """Creates file saver object

        Args:
            filename (str): full file path
            format_ (str): file format

        Returns:
            IndigoObject: file saver object
        """

        return IndigoObject(
            self,
            IndigoLib.checkResult(
                self._lib().indigoCreateFileSaver(
                    filename.encode(),
                    format_.encode(),
                )
            ),
        )

    def createSaver(self, obj, format_):
        """Creates saver object

        Args:
            obj (IndigoObject): output object
            format_ (str): format settings

        Returns:
            IndigoObject: saver object
        """

        return IndigoObject(
            self,
            IndigoLib.checkResult(
                self._lib().indigoCreateSaver(obj.id, format_.encode())
            ),
        )

    def createArray(self):
        """Creates array object

        Returns:
            IndigoObject: array object
        """

        return IndigoObject(
            self, IndigoLib.checkResult(self._lib().indigoCreateArray())
        )

    def substructureMatcher(self, target, mode=""):
        """Creates substructure matcher

        Args:
            target (IndigoObject): target molecule or reaction
            mode (str): substructure mode. Optional, defaults to "".

        Returns:
            IndigoObject: substructure matcher
        """
        if mode is None:
            mode = ""

        return IndigoObject(
            self,
            IndigoLib.checkResult(
                self._lib().indigoSubstructureMatcher(target.id, mode.encode())
            ),
            target,
        )

    def extractCommonScaffold(self, structures, options=""):
        """Extracts common scaffold for the given structures

        Args:
            structures (IndigoObject): array object of molecule structures
            options (str): extraction options. Optional, defaults to "".

        Returns:
            IndigoObject: scaffold object
        """
        structures = self.convertToArray(structures)
        if options is None:
            options = ""

        new_obj = IndigoLib.checkResult(
            self._lib().indigoExtractCommonScaffold(
                structures.id, options.encode()
            )
        )
        if new_obj == 0:
            return None
        else:
            return IndigoObject(self, new_obj, self)

    def decomposeMolecules(self, scaffold, structures):
        """Creates deconvolution object for the given structures

        Args:
            scaffold (IndigoObject): query molecule object
            structures (IndigoObject): array of molecule structures

        Returns:
            IndigoObject: deconvolution object
        """
        structures = self.convertToArray(structures)

        return IndigoObject(
            self,
            IndigoLib.checkResult(
                self._lib().indigoDecomposeMolecules(
                    scaffold.id, structures.id
                )
            ),
            scaffold,
        )

    def rgroupComposition(self, molecule, options=""):
        """Creates composition iterator

        Args:
            molecule (IndigoObject): target molecule object
            options (str): rgroup composition options. Optional,
                           defaults to "".

        Returns:
            IndigoObject: composition iterator
        """
        if options is None:
            options = ""

        new_obj = IndigoLib.checkResult(
            self._lib().indigoRGroupComposition(molecule.id, options.encode())
        )
        if new_obj == 0:
            return None
        else:
            return IndigoObject(self, new_obj, self)

    def getFragmentedMolecule(self, elem, options=""):
        """Returns fragmented molecule for the given composition element

        Args:
            elem (IndigoObject): composition element object
            options (str): Fragmentation options. Optional, defaults to "".

        Returns:
            IndigoObject: fragmented structure object
        """
        if options is None:
            options = ""

        new_obj = IndigoLib.checkResult(
            self._lib().indigoGetFragmentedMolecule(elem.id, options.encode())
        )
        if new_obj == 0:
            return None
        else:
            return IndigoObject(self, new_obj, self)

    def createDecomposer(self, scaffold):
        """Creates deconvolution object for the given scaffold

        Args:
            scaffold (IndigoObject): scaffold molecular structure

        Returns:
            IndigoObject: deconvolution object
        """

        return IndigoObject(
            self,
            IndigoLib.checkResult(
                self._lib().indigoCreateDecomposer(scaffold.id)
            ),
            scaffold,
        )

    def reactionProductEnumerate(self, replaced_action, monomers):
        """Creates reaction product enumeration iterator

        Args:
            replaced_action (IndigoObject): query reaction for the enumeration
            monomers (IndigoObject): array of objects to enumerate

        Returns:
            IndigoObject: result products iterator
        """

        monomers = self.convertToArray(monomers)
        return IndigoObject(
            self,
            IndigoLib.checkResult(
                self._lib().indigoReactionProductEnumerate(
                    replaced_action.id, monomers.id
                )
            ),
            replaced_action,
        )

    def transform(self, reaction, monomers):
        """Transforms the given monomers by reaction

        Args:
            reaction (IndigoObject): query reaction
            monomers (IndigoObject): array of objects to transform

        Returns:
            IndigoObject: mapping object
        """

        new_obj = IndigoLib.checkResult(
            self._lib().indigoTransform(reaction.id, monomers.id)
        )
        if new_obj == 0:
            return None
        else:
            return IndigoObject(self, new_obj, self)

    def loadBuffer(self, buf):
        """Creates scanner object from buffer

        Args:
            buf(list): array of bytes

        Returns:
            IndigoObject: scanner object
        """
        buf = list(buf)
        values = (c_byte * len(buf))()
        for i in range(len(buf)):
            values[i] = buf[i]

        return IndigoObject(
            self,
            IndigoLib.checkResult(
                self._lib().indigoLoadBuffer(values, len(buf))
            ),
        )

    def loadString(self, string):
        """Creates scanner object from string

        Args:
            string (str): string with information

        Returns:
            IndigoObject: scanner object
        """

        return IndigoObject(
            self,
            IndigoLib.checkResult(
                self._lib().indigoLoadString(string.encode())
            ),
        )

    def iterateSDF(self, reader):
        """Creates SDF iterator from scanner object

        Args:
            reader (IndigoObject): scanner object

        Returns:
            IndigoObject: SD iterator object
        """

        result = IndigoLib.checkResult(self._lib().indigoIterateSDF(reader.id))
        if not result:
            return None
        return IndigoObject(self, result, reader)

    def iterateSmiles(self, reader):
        """Creates smiles iterator from scanner object

        Args:
            reader (IndigoObject): scanner object

        Returns:
            IndigoObject: smiles iterator object
        """

        result = IndigoLib.checkResult(
            self._lib().indigoIterateSmiles(reader.id)
        )
        if not result:
            return None
        return IndigoObject(self, result, reader)

    def iterateCML(self, reader):
        """Creates CML iterator from scanner object

        Args:
            reader (IndigoObject): scanner object

        Returns:
            IndigoObject: CML iterator object
        """

        result = IndigoLib.checkResult(self._lib().indigoIterateCML(reader.id))
        if not result:
            return None
        return IndigoObject(self, result, reader)

    def iterateCDX(self, reader):
        """Creates CDX iterator from scanner object

        Args:
            reader (IndigoObject): scanner object

        Returns:
            IndigoObject: CDX iterator object
        """

        result = IndigoLib.checkResult(self._lib().indigoIterateCDX(reader.id))
        if not result:
            return None
        return IndigoObject(self, result, reader)

    def iterateRDF(self, reader):
        """Creates RDF iterator from scanner object

        Args:
            reader (IndigoObject): scanner object

        Returns:
            IndigoObject: RD iterator object
        """

        result = IndigoLib.checkResult(self._lib().indigoIterateRDF(reader.id))
        if not result:
            return None
        return IndigoObject(self, result, reader)

    def iterateTautomers(self, molecule, params):
        """Iterates tautomers for the given molecule

        Args:
            molecule (IndigoObject): molecule to find tautomers from
            params (str): tau iteration parameters. "INCHI" or "RSMARTS".
                          Defaults to "RSMARTS"

        Returns:
            IndigoObject: molecule iterator object
        """

        return IndigoObject(
            self,
            IndigoLib.checkResult(
                self._lib().indigoIterateTautomers(
                    molecule.id, params.encode()
                )
            ),
            molecule,
        )

    def nameToStructure(self, name, params=None):
        """
        Converts a chemical name into a corresponding structure

        Args:
            name (str): a name to parse
            params (str): a string (optional) containing parsing options or
                          None if no options are changed

        Raises:
            IndigoException: if parsing fails or no structure is found

        """
        if params is None:
            params = ""

        return IndigoObject(
            self,
            IndigoLib.checkResult(
                self._lib().indigoNameToStructure(
                    name.encode(),
                    params.encode(),
                )
            ),
        )

    def buildPkaModel(self, level, threshold, filename):
        """Builds pKa model into file

        Args:
            level (int): max level
            threshold (float): threshold value
            filename (str): full path to the file

        Returns:
            int: 1 if level > 0, 0 otherwise
        """

        return IndigoLib.checkResult(
            self._lib().indigoBuildPkaModel(
                level, threshold, filename.encode()
            )
        )

    def transformHELMtoSCSR(self, item):
        """Transforms HELM to SCSR object

        Args:
            item (IndigoObject): object with HELM information

        Returns:
            IndigoObject: molecule with SCSR object
        """

        return IndigoObject(
            self,
            IndigoLib.checkResult(
                self._lib().indigoTransformHELMtoSCSR(item.id)
            ),
        )

    def check(self, mol_str, check_flags="", props=""):
        """Validates the given structure

        Args:
            mol_str (str): input structure string
            check_flags (str): validation flags. Optional, defaults to "".
            props (str): validation properties. Optional, defaults to "".

        Returns:
            str: validation string
        """
        if props is None:
            props = ""
        if check_flags is None:
            check_flags = ""

        return IndigoLib.checkResultString(
            self._lib().indigoCheck(
                mol_str.encode(),
                check_flags.encode(),
                props.encode(),
            )
        )
