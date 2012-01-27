# Setup enviroment for using Indigo both for Python, Jython and IronPython
import sys
import os
import inspect
import threading
from threading import Lock

def isIronPython():
    return sys.version.lower().find("ironpython") != -1

def isJython():
    return os.name == 'java'

if 'INDIGO_COVERAGE' in os.environ:
    from indigo_coverage import IndigoCoverageWrapper as Indigo, IndigoException, IndigoRenderer
else:
    if isIronPython():
        import clr #@UnresolvedImport
        cur_path = os.path.dirname(__file__)
        dll_full_path = os.path.join(cur_path, "../../../api/cs/bin/Release/indigo-cs.dll")
        rdll_full_path = os.path.join(cur_path, "../../../api/renderer/cs/bin/Release/indigo-renderer-cs.dll")
        clr.AddReferenceToFileAndPath(dll_full_path)
        clr.AddReferenceToFileAndPath(rdll_full_path)
        from com.ggasoftware.indigo import *
    elif isJython():
        cur_path = os.path.dirname(__file__)
        jar_full_path = os.path.abspath(os.path.join(cur_path, "../../../api/java/dist/indigo.jar"))
        rjar_full_path = os.path.abspath(os.path.join(cur_path, "../../../api/renderer/java/dist/indigo-renderer.jar"))   
        jna_full_path = os.path.abspath(os.path.join(cur_path, "../../../common/jna/jna.jar"))
        sys.path.append(jar_full_path)
        sys.path.append(rjar_full_path)
        sys.path.append(jna_full_path)    
        from com.ggasoftware.indigo import *
    else:
        cur_path = os.path.dirname(__file__)
        dll_full_path = os.path.join(cur_path, "../../../api/python")
        rdll_full_path = os.path.join(cur_path, "../../../api/renderer/python")
        idll_full_path = os.path.join(cur_path, "../../../api/plugins/inchi/python")
        sys.path.append(dll_full_path)
        sys.path.append(rdll_full_path)
        sys.path.append(idll_full_path)
        #if 'INDIGO_COVERAGE' in os.environ:
        #    from indigo_coverage import IndigoCoverageWrapper as Indigo
        #else:
        from indigo import Indigo
        IndigoObject = Indigo.IndigoObject
        from indigo import IndigoException
        from indigo_renderer import IndigoRenderer
        from indigo_inchi import IndigoInchi

# product function implementation (it is not implemented Python 2.4)
def product(*args, **kwds):
    # product('ABCD', 'xy') --> Ax Ay Bx By Cx Cy Dx Dy
    # product(range(2), repeat=3) --> 000 001 010 011 100 101 110 111
    pools = map(tuple, args) * kwds.get('repeat', 1)
    result = [[]]
    for pool in pools:
        result = [x+[y] for x in result for y in pool]
    for prod in result:
        yield tuple(prod)
        
def getIndigoExceptionText(e):    
    if isJython():
        return e.message
    elif isIronPython():
        return e.Message
    else:
        return e.value 
    
def joinPath(*args):
    frm = inspect.stack()[1][1] 
    return os.path.abspath(os.path.join(os.path.abspath(os.path.dirname(frm)), *args))

def relativePath(databaseFile):
    frm = inspect.stack()[1][1] 
    return os.path.relpath(databaseFile, os.path.dirname(frm)).replace('\\', '/')