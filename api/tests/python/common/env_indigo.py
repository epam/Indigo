# Setup enviroment for using Indigo both for Python, Jython and IronPython
import sys
import os
import shutil
import inspect
import threading
from math import sqrt

from util import isIronPython, isJython, getPlatform, getIndigoVersion, getCpuCount

if sys.platform == 'cli':
    import clr
    import System
    clr.AddReference("System.IO.FileSystem")
    clr.AddReference("System.IO.Compression.FileSystem")
    clr.AddReference("System.Runtime.Extensions")
    from System.IO import FileInfo, File, Directory, DirectoryInfo
    from System.IO.Compression import ZipFile
    from System import Environment
else:
    import subprocess
    import zipfile


frame = inspect.stack()[1]
module = inspect.getmodule(frame[0])


def dir_exists(path_):
    if sys.platform == 'cli' and os.name != 'nt':
        return Directory.Exists(path_)
    else:
        return os.path.isdir(path_)


def file_exists(path):
    if sys.platform == 'cli':
        return FileInfo(path).Exists
    else:
        return os.path.exists(path)


def file_size(path):
    if sys.platform == 'cli':
        return FileInfo(path).Length
    else:
        return os.path.getsize(path)


def makedirs(path):
    if sys.platform == 'cli':
        if not dir_exists(path):
            Directory.CreateDirectory(path)
    else:
        os.makedirs(path)


def unzip(path, parent):
    if sys.platform == 'cli':
        out_dir = str(tuple(ZipFile.OpenRead(path).Entries)[0])
        if dir_exists(out_dir):
            Directory.Delete(out_dir, True)
        ZipFile.ExtractToDirectory(path, parent)
        if 'latest' in path:
            return out_dir
    else:
        with zipfile.ZipFile(item) as zf:
            zf.extractall()
            if 'latest' in path:
                return zf.filelist[0].filename
    return None


def rmdir(path):
    if sys.platform == 'cli':
        if not dir_exists(path):
            return
        else:
            Directory.Delete(path, True)
    else:
        shutil.rmtree(path)


def cdll_indigo(path):
    def lib_name():
        return "{}{}{}".format(prefix, lib, suffix)

    prefix = None
    suffix = None
    runtimes_folder = None
    libs = ['indigo',
                   'indigo-renderer',
                   'indigo-inchi',
                   'bingo']
    libs_instances = []
    if getPlatform() == 'win':
        prefix = ''
        suffix = '.dll'
        # runtimes_folder = 'win-x64'
        libs += ['vcruntime140', 'vcruntime_140_1',
                      'msvcp140', 'concrt140']
    elif getPlatform() == 'linux':
        prefix = 'lib'
        suffix = '.so'
        # runtimes_folder = 'linux-x64'
    elif getPlatform() == 'mac':
        prefix = 'lib'
        suffix = '.dylib'
        # runtimes_folder = 'osx-x64'
    else:
        raise RuntimeError('Invalid platform: {}'.format(getPlatform()))

    import ctypes
    for lib in libs:
        lib_path = os.path.normpath(os.path.join(path, '..', 'lib', 'netstandard2.0', lib_name()))
        if file_exists(lib_path):
            libs_instances.append(ctypes.CDLL(lib_path))
    return libs_instances

if 'INDIGO_COVERAGE' in os.environ:
    from indigo_coverage import IndigoCoverageWrapper as Indigo, IndigoObjectCoverageWrapper as IndigoObject, IndigoException, IndigoRenderer, IndigoInchi
    Indigo.IndigoObject = IndigoObject
else:
    cur_path = os.path.abspath(os.path.dirname(__file__))
    distPaths = [
        os.path.normpath(os.path.join(cur_path, '../../../../dist')),
    ]
    success = False

    indigo_root = os.path.dirname(os.path.dirname(os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))))
    if isIronPython():
        if not 'INDIGO_PATH' in os.environ:
            import clr
            target_framework = 'netstandard2.0'
            dll_full_path = os.path.normpath(os.path.abspath(os.path.join(indigo_root, "api/dotnet/bin/Release/{}/Indigo.Net.dll".format(target_framework))))
            if not os.path.exists(dll_full_path):
                for distPath in distPaths:
                    if not dir_exists(distPath):
                        continue
                    dll_full_path_base = None
                    for item in os.listdir(distPath):
                        if item.startswith('indigo-dotnet-') and item.endswith('.zip') and (getPlatform() in item or 'universal' in item) and (target_framework in item):
                            curdir = os.path.abspath(os.curdir)
                            os.chdir(distPath)
                            top_file = item.replace('.zip', '')
                            top_file = unzip(item, os.curdir) or top_file
                            os.chdir(curdir)
                            dll_full_path_base = os.path.abspath(os.path.join(cur_path, distPath, top_file))
                            break
                    if dll_full_path_base:
                        dll_full_path = os.path.abspath(os.path.join(cur_path, "%s/Indigo.Net.dll" % (dll_full_path_base)))

                    if not os.path.exists(dll_full_path):
                        continue
                    break
            os.environ['INDIGO_PATH'] = dll_full_path

        import clr
        for item in os.environ['INDIGO_PATH'].split(os.pathsep):
            cdll_indigo(item)
            clr.AddReferenceToFileAndPath(item)
        from com.epam.indigo import *
        success = True

    elif isJython():
        if not 'INDIGO_PATH' in os.environ:
            dll_full_path = os.path.normpath(os.path.abspath(os.path.join(indigo_root, "api/java/target/indigo-%s.jar" % getIndigoVersion())))
            rdll_full_path = os.path.normpath(os.path.abspath(os.path.join(indigo_root, "api/plugins/renderer/java/target/indigo-renderer-%s.jar" % getIndigoVersion())))
            idll_full_path = os.path.normpath(os.path.abspath(os.path.join(indigo_root, "api/plugins/inchi/java/target/indigo-inchi-%s.jar" % getIndigoVersion())))
            bdll_full_path = os.path.normpath(os.path.abspath(os.path.join(indigo_root, "api/plugins/bingo/java/target/bingo-nosql-%s.jar" % getIndigoVersion())))
            jna_full_path = os.path.normpath(os.path.abspath(os.path.join(indigo_root, "common/jna/jna.jar")))

            if not (os.path.exists(dll_full_path) and os.path.exists(rdll_full_path) and os.path.exists(idll_full_path) and os.path.exists(bdll_full_path) and os.path.exists(jna_full_path)):
                for distPath in distPaths:
                    if not os.path.exists(distPath):
                        continue
                    dll_full_path_base = '%s/java' % (distPath)
                    for item in os.listdir(distPath):
                        if item.startswith('indigo-java-') and item.endswith('.zip') and (item.find(getPlatform()) != -1 or item.find('universal') != -1):
                            curdir = os.path.abspath(os.curdir)
                            os.chdir(distPath)
                            top_file = item.replace('.zip', '')
                            with zipfile.ZipFile(item) as zf:
                                zf.extractall()
                                if item.find('latest') != -1:
                                    top_file = zf.filelist[0].filename
                            os.chdir(curdir)
                            dll_full_path_base = os.path.abspath(os.path.join(cur_path, distPath, top_file))
                            break

                    dll_full_path = os.path.abspath(os.path.join(cur_path, "%s/indigo.jar" % (dll_full_path_base)))
                    rdll_full_path = os.path.abspath(os.path.join(cur_path, "%s/indigo-renderer.jar" % (dll_full_path_base)))
                    idll_full_path = os.path.abspath(os.path.join(cur_path, "%s/indigo-inchi.jar" % (dll_full_path_base)))
                    bdll_full_path = os.path.abspath(os.path.join(cur_path, "%s/bingo-nosql.jar" % (dll_full_path_base)))
                    jna_full_path = os.path.abspath(os.path.join(cur_path, "%s/jna.jar" % (dll_full_path_base)))
                    if not os.path.exists(dll_full_path):
                        continue
                    break
            os.environ['INDIGO_PATH'] = os.pathsep.join((dll_full_path, rdll_full_path, idll_full_path, bdll_full_path, jna_full_path))

        for item in os.environ['INDIGO_PATH'].split(os.pathsep):
            sys.path.insert(0, item)

        devnull = open(os.devnull, 'w')
        sys.stdout = devnull
        from com.epam.indigo import *
        sys.stdout = sys.__stdout__
        success = True

    else:
        if 'INDIGO_PATH' not in os.environ:
            dll_full_path = os.path.normpath(os.path.join(indigo_root, "api/python"))
            rdll_full_path = os.path.normpath(os.path.join(indigo_root, "api/plugins/renderer/python"))
            idll_full_path = os.path.normpath(os.path.join(indigo_root, "api/plugins/inchi/python"))
            bdll_full_path = os.path.normpath(os.path.join(indigo_root, "api/plugins/bingo/python"))
            _lib_path = os.path.join(dll_full_path, 'lib')
            if os.path.exists(_lib_path):
                shutil.rmtree(_lib_path)
            _shared_libs_path = os.path.join(indigo_root, 'api/libs/shared')
            if os.path.exists(_shared_libs_path) and len(os.listdir(_shared_libs_path)) > 0:
                shutil.copytree(_shared_libs_path, _lib_path)
            if not os.path.exists(os.path.join(dll_full_path, 'lib')):
                for distPath in distPaths:
                    distPath = os.path.normpath(distPath)
                    if not os.path.exists(distPath):
                        continue
                    dll_full_path = '%s/python' % (distPath)
                    for item in os.listdir(distPath):
                        if item.startswith('indigo-python-') and item.endswith('.zip') and (getPlatform() in item or 'universal' in item):
                            curdir = os.path.abspath(os.curdir)
                            os.chdir(distPath)
                            top_file = item.replace('.zip', '')
                            with zipfile.ZipFile(item) as zf:
                                zf.extractall()
                                if item.find('latest') != -1:
                                    top_file = zf.filelist[0].filename
                            os.environ['INDIGO_PATH'] = os.path.abspath(os.path.join(cur_path, distPath, top_file))
                            os.chdir(curdir)
                            break
                    if not os.path.exists(dll_full_path):
                        continue
                    break
            else:
                os.environ['INDIGO_PATH'] = os.pathsep.join((dll_full_path, rdll_full_path, idll_full_path, bdll_full_path))

        if not os.environ.get('INDIGO_PATH'):
            raise ValueError('Could not find Indigo libraries for Python')
        for item in os.environ['INDIGO_PATH'].split(os.pathsep):
            sys.path.insert(0, item)
        from indigo import Indigo, IndigoObject, IndigoException
        from indigo_renderer import IndigoRenderer
        from indigo_inchi import IndigoInchi
        from bingo import Bingo, BingoException, BingoObject
        success = True

    if not success:
        raise RuntimeError('Indigo not found at %s' % distPaths)


def getIndigoExceptionText(e):
    if isJython():
        return e.message
    elif isIronPython():
        return e.Message.replace("\\'", "'")
    else:
        value = str(e)
        if value[0] == '\'' and value[-1] == '\'':
            value = value[1:-1]
        if value[0] == '"' and value[-1] == '"':
            value = value[1:-1]
        return value.replace("\\'", "'")


absPathDict = {}
relPathDict = {}
inspectStackLock = threading.RLock()


def joinPath(*args):
    inspectStackLock.acquire()
    frm = inspect.stack()[1][1]
    inspectStackLock.release()
    return os.path.normpath(os.path.abspath(os.path.join(os.path.abspath(os.path.dirname(frm)), *args))).replace('\\', '/')


def relativePath(args):
    inspectStackLock.acquire()
    frm = inspect.stack()[1][1]
    inspectStackLock.release()
    return os.path.normpath(os.path.relpath(args, os.path.dirname(frm))).replace('\\', '/')


def dist_vec(a, b):
    return sqrt(sum((a - b)**2 for a, b in zip(a, b)))


def moleculeLayoutDiff(indigo, mol, ref, delta=0.001, ref_is_file=True):
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
            error_buf.append('atom #{} ({}) contains delta {} > {}'.format(a1.index(), a1.symbol(), d, delta))

    for s in mol.iterateDataSGroups():
        s2 = m2.getDataSGroup(s.index())
        a = s.getSGroupCoords()
        b = s2.getSGroupCoords()
        d = dist_vec(a, b)
        if d > delta:
            error_buf.append('data sgroup #{} contains delta {} > {}'.format(s.index(), d, delta))

    if len(error_buf) > 0:
        return 'Error: {}'.format("\n   ".join(error_buf))

    return 'Success'


def reactionLayoutDiff(indigo, rxn, ref, delta=0.001, ref_is_file=True):
    if ref_is_file:
        ref_name = getRefFilepath2(ref)
        r2 = indigo.loadReactionFromFile(ref_name)
    else:
        r2 = indigo.loadReaction(ref)
    error_buf = []
    for m in rxn.iterateMolecules():
        res = moleculeLayoutDiff(indigo, m, r2.getMolecule(m.index()).molfile(), delta, ref_is_file=False)
        error_buf.append('Molecule #{}: {}'.format(m.index(), res))
    return "\n   ".join(error_buf)


def getRefFilepath(filename):
    """
    Returns:
        reference path for the specified filename. Platform specific if necessary
    """
    with inspectStackLock:
        frm = inspect.stack()[1][1]
    ref_path = os.path.abspath(os.path.join(os.path.dirname(frm), 'ref'))
    if file_exists(os.path.join(ref_path, filename)):
        return os.path.normpath(os.path.abspath(os.path.join(ref_path, filename)))
    sys_name = getPlatform()
    if file_exists(os.path.join(ref_path, sys_name, filename)):
        return os.path.normpath(os.path.abspath(os.path.join(ref_path, sys_name, filename)))

    raise RuntimeError('Can not find a file "%s" neither at "%s" or "%s"' % (filename, ref_path, os.path.abspath(os.path.join(ref_path, sys_name))))


def getRefFilepath2(filename):
    """
    Returns:
        reference path for the specified filename in stack 2. Platform specific if necessary
    """
    with inspectStackLock:
        frm = inspect.stack()[2][1]
    ref_path = os.path.abspath(os.path.join(os.path.dirname(frm), 'ref'))
    if file_exists(os.path.join(ref_path, filename)):
        return os.path.normpath(os.path.abspath(os.path.join(ref_path, filename)))
    sys_name = getPlatform()
    if file_exists(os.path.join(ref_path, sys_name, filename)):
        return os.path.normpath(os.path.abspath(os.path.join(ref_path, sys_name, filename)))

    raise RuntimeError('Can not find a file "%s" neither at "%s" or "%s"' % (filename, ref_path, os.path.abspath(os.path.join(ref_path, sys_name))))


def subprocess_communicate(name, arguments, wd, env):
    if sys.platform == 'cli':
        p = System.Diagnostics.Process()
        p.StartInfo.UseShellExecute = False
        p.StartInfo.RedirectStandardError = True
        p.StartInfo.RedirectStandardOutput = True
        p.StartInfo.FileName = name
        p.StartInfo.Arguments = ' '.join(arguments)
        p.StartInfo.WorkingDirectory = wd
        for key, value in env.items():
            p.StartInfo.EnvironmentVariables[str(key)] = str(value)
        p.Start()
        stdout = p.StandardOutput.ReadToEnd()
        stderr = p.StandardError.ReadToEnd()
        p.WaitForExit()
    else:
        p = subprocess.Popen(
            [name, ] + arguments,
            cwd=wd,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            env=dict(os.environ).update(env))
        stdout, stderr = p.communicate()
    return stdout, stderr


def open_file_utf8(filename):
    import codecs
    if isJython():
        return open(filename, 'wt')
    elif isIronPython():
        # TODO: FIXME (maybe on C# site?)
        return codecs.open(filename, 'wb', encoding='utf-8') # FAILED: wrong symbols
        # return open(filename, 'wt') # ERROR: UnicodeEncodeError: 'ascii' codec can't encode character '\uFFFD' in position 814: ordinal not in range(128)
        # return open(filename, 'wb') # ERROR: UnicodeEncodeError: 'ascii' codec can't encode character '\uFFFD' in position 724: ordinal not in range(128)
        # return codecs.open(filename, 'wb') #  # ERROR: UnicodeEncodeError: 'ascii' codec can't encode character '\uFFFD' in position 724: ordinal not in range(128)
    else:
        if sys.version_info.major < 3:
            return open(filename, 'wt')
        else:
            return codecs.open(filename, 'wb', encoding='utf-8')
