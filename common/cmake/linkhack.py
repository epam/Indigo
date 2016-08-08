#!/usr/bin/python
import subprocess
import os
import shutil
import sys
import platform


def getSymbols(libPath):
    stderr = None
    if 'VERBOSE' not in os.environ:
        stderr = subprocess.PIPE
    return [item.replace('  ', '').split(' ') for item in subprocess.Popen('nm %s' % (libPath), shell=True, stdout=subprocess.PIPE, stderr=stderr).communicate()[0].split('\n')]


def getIndigoStdSyms():
    libname = 'libc++.a'
    if not platform.mac_ver()[0]:
        libname = 'libstdc++.a'
    libstdcppSymbols = getSymbols(libname)
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

    shutil.copy(libstdcppPath, 'libstdc++.a')

    getIndigoStdSyms()

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
            subprocess.call('objcopy --redefine-syms indigostd.syms libstdc++.a libindigostdcpp.a', shell=True)
            linkLibraries = linkLibraries + ' -Wl,--whole-archive libindigostdcpp.a -Wl,--no-whole-archive '
            break

    os.remove('libstdc++.a')

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

    verboseParam = ''
    if 'VERBOSE' in os.environ:
        verboseParam = ' -v '
    linkCommand = '%s %s -static-libstdc++ %s %s %s -o %s' % (compiler, verboseParam, linkFlags, ' '.join(objFiles), linkLibraries, target)
    stderr = None
    stdout = None
    if 'VERBOSE' in os.environ:
        print(linkCommand)
        stderr = subprocess.PIPE
        stdout = subprocess.PIPE
    subprocess.call(linkCommand, shell=True, stderr=stderr, stdout=stdout)


def main():
    args = ' '.join(sys.argv).split('|')
    compiler = args[1]
    linkFlags = args[2]
    objFiles = filter(len, args[3].split(' '))
    linkLibraries = args[4].strip()
    target = args[5].strip()

    if platform.mac_ver()[0]:
        print('linkhack for Mac is not supported anymore, build with 10.7 SDK instead')
    else:
        linux(compiler, linkFlags, objFiles, linkLibraries, target)


if __name__ == '__main__':
    main()
