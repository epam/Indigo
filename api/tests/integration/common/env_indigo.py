# Setup enviroment for using Indigo both for Python, Jython and IronPython
import inspect
import os
import shutil
import sys
import threading
from math import sqrt

from util import REPO_ROOT, getPlatform, isIronPython, isJython

if isIronPython():
    import clr
    import System

    clr.AddReference("System.IO.FileSystem")
    clr.AddReference("System.IO.Compression.FileSystem")
    clr.AddReference("System.Runtime.Extensions")
    from System import Environment
    from System.IO import Directory, DirectoryInfo, File, FileInfo
    from System.IO.Compression import ZipFile
else:
    import subprocess
    import zipfile

    if isJython():
        from util import download_jna, get_indigo_java_version

frame = inspect.stack()[1]
module = inspect.getmodule(frame[0])


def dir_exists(path_):
    if sys.platform == "cli" and os.name != "nt":
        return Directory.Exists(path_)
    else:
        return os.path.isdir(path_)


def file_exists(path):
    if sys.platform == "cli":
        return FileInfo(path).Exists
    else:
        return os.path.exists(path)


def file_size(path):
    if sys.platform == "cli":
        return FileInfo(path).Length
    else:
        return os.path.getsize(path)


def makedirs(path):
    if sys.platform == "cli":
        if not dir_exists(path):
            Directory.CreateDirectory(path)
    else:
        os.makedirs(path)


def unzip(path, parent):
    if sys.platform == "cli":
        out_dir = str(tuple(ZipFile.OpenRead(path).Entries)[0])
        if dir_exists(out_dir):
            Directory.Delete(out_dir, True)
        ZipFile.ExtractToDirectory(path, parent)
        if "latest" in path:
            return out_dir
    else:
        with zipfile.ZipFile(item) as zf:
            zf.extractall()
            if "latest" in path:
                return zf.filelist[0].filename
    return None


def rmdir(path):
    if sys.platform == "cli":
        if not dir_exists(path):
            return
        else:
            Directory.Delete(path, True)
    else:
        shutil.rmtree(path)


if isIronPython():
    import clr

    dll_full_path = lambda: os.path.normpath(
        os.path.abspath(os.environ["INDIGO_PATH"])
    )
    clr.AddReferenceToFileAndPath(dll_full_path())
    from com.epam.indigo import (
        Bingo,
        BingoException,
        BingoObject,
        Indigo,
        IndigoException,
        IndigoInchi,
        IndigoObject,
        IndigoRenderer,
        ReactingCenter,
    )
elif isJython():
    indigo_java_version, jna_version = get_indigo_java_version()
    indigo_path = os.path.normpath(
        os.path.abspath(
            os.getenv("INDIGO_PATH", os.path.join(REPO_ROOT, "dist"))
        )
    )
    download_jna(jna_version, indigo_path)
    jars = []
    for filename in os.listdir(indigo_path):
        if (
            filename.endswith(".jar")
            and not "javadoc" in filename
            and not "sources" in filename
        ):
            jar_path = os.path.join(indigo_path, filename)
            if "jna" in filename:
                jars.insert(0, jar_path)
            else:
                jars.append(jar_path)
    for jar in sorted(jars):
        sys.path.append(jar)
    from com.epam.indigo import (
        Bingo,
        BingoException,
        BingoObject,
        Indigo,
        IndigoException,
        IndigoInchi,
        IndigoObject,
        IndigoRenderer,
    )

    dll_full_path = lambda: sys.path[-4]
else:
    indigo_python_source_folder = os.path.join(REPO_ROOT, "api", "python")
    sys.path.append(indigo_python_source_folder)
    from indigo import Indigo, IndigoException, IndigoObject  # noqa
    from indigo.bingo import Bingo, BingoException, BingoObject  # noqa
    from indigo.inchi import IndigoInchi  # noqa
    from indigo.indigo.indigo_lib import IndigoLib  # noqa
    from indigo.renderer import IndigoRenderer  # noqa

    dll_full_path = lambda: IndigoLib.lib._name


def getIndigoExceptionText(e):
    if isJython():
        return e.message
    elif isIronPython():
        return e.Message.replace("\\'", "'")
    else:
        value = str(e)
        if value[0] == "'" and value[-1] == "'":
            value = value[1:-1]
        if value[0] == '"' and value[-1] == '"':
            value = value[1:-1]
        return value.replace("\\'", "'")


inspectStackLock = threading.RLock()


def dataPath(args):
    return os.path.normpath(
        os.path.abspath(
            os.path.join(
                os.path.abspath(__file__),
                "..",
                "..",
                "..",
                "..",
                "..",
                "data",
                args,
            )
        )
    )


def joinPathPy(args, file_py):
    return os.path.normpath(
        os.path.abspath(
            os.path.join(os.path.abspath(os.path.dirname(file_py)), args)
        )
    ).replace("\\", "/")


def relativePath(args):
    inspectStackLock.acquire()
    frm = inspect.stack()[1][1]
    inspectStackLock.release()
    return os.path.normpath(
        os.path.relpath(args, os.path.dirname(frm))
    ).replace("\\", "/")


def dist_vec(a, b):
    return sqrt(sum((a - b) ** 2 for a, b in zip(a, b)))


def moleculeLayoutDiff(indigo, mol, ref, delta=0.01, ref_is_file=True):
    if ref_is_file:
        ref_name = getRefFilepath2(ref)
        m2 = indigo.loadMoleculeFromFile(ref_name)
    else:
        m2 = indigo.loadMolecule(ref)

    error_buf = []

    for a1 in mol.iterateAtoms():
        a2 = m2.getAtom(a1.index())
        a = a1.xyz()
        b = a2.xyz()
        d = dist_vec(a, b)
        if d > delta:
            error_buf.append(
                "atom #{} ({}) contains delta {} > {}".format(
                    a1.index(), a1.symbol(), d, delta
                )
            )

    for s in mol.iterateDataSGroups():
        s2 = m2.getDataSGroup(s.index())
        a = s.getSGroupCoords()
        b = s2.getSGroupCoords()
        d = dist_vec(a, b)
        if d > delta:
            error_buf.append(
                "data sgroup #{} contains delta {} > {}".format(
                    s.index(), d, delta
                )
            )

    if len(error_buf) > 0:
        return "Error: {}".format("\n   ".join(error_buf))

    return "Success"


def reactionLayoutDiff(indigo, rxn, ref, delta=0.001, ref_is_file=True):
    if ref_is_file:
        ref_name = getRefFilepath2(ref)
        r2 = indigo.loadReactionFromFile(ref_name)
    else:
        r2 = indigo.loadReaction(ref)
    error_buf = []
    for m in rxn.iterateMolecules():
        res = moleculeLayoutDiff(
            indigo,
            m,
            r2.getMolecule(m.index()).molfile(),
            delta,
            ref_is_file=False,
        )
        error_buf.append("Molecule #{}: {}".format(m.index(), res))
    return "\n   ".join(error_buf)


def getRefFilepath(filename):
    """
    Returns:
        reference path for the specified filename. Platform specific if necessary
    """
    with inspectStackLock:
        frm = inspect.stack()[1][1]
    ref_path = os.path.abspath(os.path.join(os.path.dirname(frm), "ref"))
    sys_name = getPlatform()
    if file_exists(os.path.join(ref_path, sys_name, filename)):
        return os.path.normpath(
            os.path.abspath(os.path.join(ref_path, sys_name, filename))
        )

    if file_exists(os.path.join(ref_path, filename)):
        return os.path.normpath(
            os.path.abspath(os.path.join(ref_path, filename))
        )

    raise RuntimeError(
        'Can not find a file "%s" neither at "%s" or "%s"'
        % (
            filename,
            ref_path,
            os.path.abspath(os.path.join(ref_path, sys_name)),
        )
    )


def getRefFilepath2(filename):
    """
    Returns:
        reference path for the specified filename in stack 2. Platform specific if necessary
    """
    with inspectStackLock:
        frm = inspect.stack()[2][1]
    ref_path = os.path.abspath(os.path.join(os.path.dirname(frm), "ref"))
    if file_exists(os.path.join(ref_path, filename)):
        return os.path.normpath(
            os.path.abspath(os.path.join(ref_path, filename))
        )
    sys_name = getPlatform()
    if file_exists(os.path.join(ref_path, sys_name, filename)):
        return os.path.normpath(
            os.path.abspath(os.path.join(ref_path, sys_name, filename))
        )

    raise RuntimeError(
        'Can not find a file "%s" neither at "%s" or "%s"'
        % (
            filename,
            ref_path,
            os.path.abspath(os.path.join(ref_path, sys_name)),
        )
    )
