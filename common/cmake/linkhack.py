#!/usr/bin/python
import subprocess
import os
import shutil
import sys
import platform


def getSymbols(libPath):
    stderr = None
    if not 'VERBOSE' in os.environ:
        stderr = subprocess.PIPE
    return [item.replace('  ', '').split(' ') for item in subprocess.Popen('nm %s' % (libPath), shell=True, stdout=subprocess.PIPE, stderr=stderr).communicate()[0].split('\n')]


def getIndigoStdSyms(libRoot):
    libname = 'libc++.a'
    if not platform.mac_ver()[0]:
        libname = 'libstdc++.a'
    libstdcppSymbols = getSymbols(os.path.join(libRoot, libname))
    renameSymbols = []

    invMap = {}
    for item in libstdcppSymbols:
        if len(item) < 2:
            continue
        if item[1] != 'U' and item[2].find('pthread') == -1:
            newName = '_ind_' + item[2]
            if newName in invMap:
                if invMap[newName] != item[2]:
                    exit('Duplicate symbol: %s for %s and %s' % (newName, invMap[newName], item[2]))
            else:
                invMap[newName] = item[2]
                renameSymbols.append((item[2], newName))
    f = open('indigostd.syms', 'w')
    for item in renameSymbols:
        f.write('%s %s\n' % (item[0], item[1]))
    f.close()

def linux(compiler, linkFlags, objFiles, linkLibraries, target):
    libstdcppPath = subprocess.Popen('g++ -print-file-name=libstdc++.a', shell=True, stdout=subprocess.PIPE).communicate()[0].replace('\n', '')

    # Find dist root
    libRoot = os.path.dirname(target)
    if not os.path.exists(libRoot):
        os.makedirs(libRoot)
    if not os.path.exists(libRoot):
        sys.exit("Cannot create or find a directory with library files")

    shutil.copy(libstdcppPath, libRoot + '/libstdc++.a')

    getIndigoStdSyms(libRoot)

    for objFile in objFiles:
        subprocess.call('objcopy --redefine-syms indigostd.syms %s' % (objFile), shell=True)

    for libname in ('libindigo.so', 
                    'bingo_postgres.so',
                    'libbingo-oracle.so', 
                    'libketcher-server.so', 
                    'indigo-cano', 
                    'indigo-deco', 
                    'indigo-depict', 
                    'rindigo.so'):
        if target.find(libname) != -1:
            subprocess.call('objcopy --redefine-syms indigostd.syms %s/libstdc++.a %s/libindigostdcpp.a' % (libRoot, libRoot), shell=True)
            linkLibraries = linkLibraries + ' -Wl,--whole-archive %s/libindigostdcpp.a -Wl,--no-whole-archive ' % (libRoot)
            break

    os.remove('%s/libstdc++.a' % (libRoot))

    for library in os.listdir(libRoot):
        if not library.endswith('.a') or library == 'libindigostdcpp.a':
            continue

        symlist = []
        for s in getSymbols(os.path.join(libRoot, library)):
            if len(s) > 1:
                symlist.append(s[2].find('_ind'))
        if 0 in symlist:
            continue
        libFile = os.path.join(libRoot, library)
        subprocess.call('objcopy --redefine-syms indigostd.syms %s' % (libFile), shell=True)

    linkCommand = '%s -v -L%s/ -static-libstdc++ %s %s %s -o %s' % (compiler, libRoot, linkFlags, ' '.join(objFiles), linkLibraries, target)
    stderr = None
    stdout = None
    if 'VERBOSE' in os.environ:
        print(linkCommand)
        stderr = subprocess.PIPE
        stdout = subprocess.PIPE
    subprocess.call(linkCommand, shell=True, stderr=stderr, stdout=stdout)


def mac(compiler, linkFlags, objFiles, linkLibraries, target):
    def lipoObjconvLipo(binaryFile):
        subprocess.call('lipo -thin x86_64 %s -o %s' % (binaryFile, binaryFile + '.tmp.64'), shell=True)
        subprocess.call('lipo -thin i386 %s -o %s' % (binaryFile, binaryFile + '.tmp.32'), shell=True)
        os.remove(binaryFile)
        command = 'objconv -v0 -wd1214 -wd1106 -fmacho64 -nf:indigostd.syms %s %s' % (binaryFile + '.tmp.64', binaryFile + '.64')
        stderr=None
        if 'VERBOSE' in os.environ:
            print(command)
            stderr = subprocess.PIPE
        subprocess.call(command, shell=True, stderr=stderr)
        command = 'objconv -v0 -wd1214 -wd1106 -fmacho32 -nf:indigostd.syms %s %s' % (binaryFile + '.tmp.32', binaryFile + '.32')
        if 'VERBOSE' in os.environ:
            print(command)
        subprocess.call(command, shell=True, stderr=sterr)
        command = 'lipo -create %s %s -output %s' % (binaryFile + '.64', binaryFile + '.32', binaryFile)
        if 'VERBOSE' in os.environ:
            print(command)
        subprocess.call(command, shell=True, stderr=stderr)
        os.remove(binaryFile + '.tmp.32')
        os.remove(binaryFile + '.tmp.64')
        os.remove(binaryFile + '.32')
        os.remove(binaryFile + '.64')


    libRoot = os.path.dirname(target)
    shutil.copy('/usr/lib/libc++.a', libRoot + '/libc++.a')
    getIndigoStdSyms(libRoot)

    for objFile in objFiles:
        lipoObjconvLipo(objFile)

    for libname in ('libindigo.dylib',
                    'bingo_postgres.dylib',
                    'libketcher-server.dylib',
                    'indigo-cano',
                    'indigo-deco',
                    'indigo-depict'):
        if target.find(libname) != -1:
            lipoObjconvLipo(libRoot + '/libc++.a')
            linkLibraries = linkLibraries + ' -Wl,-all_load %s/libc++.a -Wl,-noall_load' % (libRoot)
            break

    for library in os.listdir(libRoot):
        if not library.endswith('.a') or library == 'libindigoc++.a':
            continue
        symlist = []
        for s in getSymbols(os.path.join(libRoot, library)):
            if len(s) > 1:
                symlist.append(s[2].find('_ind'))
        if 0 in symlist:
            continue
        lipoObjconvLipo(os.path.join(libRoot, library))

    cmd = ('%s -L%s/ -arch i386 -arch x86_64 -undefined dynamic_lookup -nodefaultlibs -lc -lm -std=c++11 -mmacosx-version-min=10.7 %s %s %s -o %s' % 
        (compiler, libRoot, linkFlags, ' '.join(objFiles), linkLibraries, target))
    stderr=None
    if 'VERBOSE' in os.environ:
        print(cmd)
        stderr = subprocess.PIPE
    subprocess.call(cmd, shell=True, stderr=stderr)


def main():
    args = ' '.join(sys.argv).split('|')
    print args
    compiler = args[1]
    linkFlags = args[2]
    objFiles = filter(len, args[3].split(' '))
    linkLibraries = args[4].strip()
    target = args[5].strip()

    if platform.mac_ver()[0]:
        mac(compiler, linkFlags, objFiles, linkLibraries, target)
    else:
        linux(compiler, linkFlags, objFiles, linkLibraries, target)


if __name__ == '__main__':
    main()
