import ctypes

class Molecule(IndigoObject):
    api.Python_com_dc_indigo_molecule_lib_createFromMolFile.argtypes = (ctypes.c_char_p,)
    api.Python_com_dc_indigo_molecule_lib_createFromMolFile.restype = ctypes.c_long

    api.Python_com_dc_indigo_molecule_lib_aromatize.argtypes = (ctypes.c_long,)
    api.Python_com_dc_indigo_molecule_lib_aromatize.restype = None

    api.Python_com_dc_indigo_molecule_lib_molfile.argtypes = (ctypes.c_long,)
    api.Python_com_dc_indigo_molecule_lib_molfile.restype = ctypes.c_char_p

    def __init__(self, handle):
        IndigoObject(self, handle)

    @staticmethod
    def createFromMolFile(string):
        return Molecule(api.Python_com_dc_indigo_molecule_lib_createFromMolFile(string));

    def aromatize(self):
        api.Python_com_dc_indigo_molecule_lib_aromatize(self.handle)

    def molfile(self):
        _result = api.Python_com_dc_indigo_molecule_lib_molfile(self.handle)
        result = _result.value
        libc.free(_result)
        return result
    
###### Not generated, should be in sources. Just shown here. Base class for all library objects ######
class IndigoObject(object):
    api = ctypes.cdll.LoadLibrary('./indigo.so')
    libc = ctypes.CDLL(ctypes.util.find_library('c'))
    api.Python_com_dc_indigo_object_lib_release.argtypes = (ctypes.long,)
    api.Python_com_dc_indigo_object_lib_release.restype = None
    def __init__(self, handle):
        self.handle = handle
    def __del__(self):
        api.Python_com_dc_indigo_object_lib_release(self.handle)


