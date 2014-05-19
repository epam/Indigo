import subprocess
import os
import shutil
import sys
import platform


def getSymbols(libPath):
    return [item.replace('  ', '').split(' ') for item in subprocess.check_output('nm {0}'.format(libPath), shell=True, stderr=subprocess.PIPE if not 'VERBOSE' in os.environ else None).split('\n')]


def getIndigoStdSyms(libRoot):
    libname = 'libstdc++.a' if not platform.mac_ver()[0] else 'libc++.a'
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
                    exit('Duplicate symbol: {0} for {1} and {2}'.format(newName, invMap[newName], item[2]))
            else:
                invMap[newName] = item[2]
                renameSymbols.append((item[2], newName))
    with open('indigostd.syms', 'w') as f:
        for item in renameSymbols:
            f.write('{0} {1}\n'.format(item[0], item[1]))


def linux(compiler, linkFlags, objFiles, linkLibraries, target):
    libstdcppPath = subprocess.check_output('g++ -print-file-name=libstdc++.a', shell=True).replace('\n', '')

    # Find dist root
    libRoot = os.path.dirname(target)
    if not os.path.exists(libRoot):
        os.makedirs(libRoot)
    if not os.path.exists(libRoot):
        sys.exit("Cannot create or find a directory with library files")

    shutil.copy(libstdcppPath, libRoot + '/libstdc++.a')

    getIndigoStdSyms(libRoot)

    for objFile in objFiles:
        subprocess.check_call('objcopy --redefine-syms indigostd.syms {0}'.format(objFile), shell=True)

    if target.find('libindigo.so') != -1:
        subprocess.check_call('objcopy --redefine-syms indigostd.syms {0}/libstdc++.a {0}/libindigostdcpp.a'.format(libRoot), shell=True)
        linkLibraries = linkLibraries + ' -Wl,--whole-archive {0}/libindigostdcpp.a -Wl,--no-whole-archive '.format(libRoot)

    os.remove('{0}/libstdc++.a'.format(libRoot))

    for library in os.listdir(libRoot):
        if not library.endswith('.a') or library == 'libindigostdcpp.a':
            continue
        if 0 in [s[2].find('_ind') if len(s) > 1 else -1 for s in getSymbols(os.path.join(libRoot, library))]:
            continue
        libFile = os.path.join(libRoot, library)
        subprocess.check_call('objcopy --redefine-syms indigostd.syms {0}'.format(libFile), shell=True)

    linkCommand = '{0} -v -shared -L{1}/ -static-libstdc++ {2} {3} {4} -o {5}'.format(compiler, libRoot, linkFlags, ' '.join(objFiles), linkLibraries, target)
    if 'VERBOSE' in os.environ:
        print(linkCommand)
    subprocess.check_call(linkCommand, shell=True, stderr=subprocess.PIPE if not 'VERBOSE' in os.environ else None, stdout=subprocess.PIPE if not 'VERBOSE' in os.environ else None)


def mac(compiler, linkFlags, objFiles, linkLibraries, target):
    def lipoObjconvLipo(binaryFile):
        subprocess.check_call('lipo -thin x86_64 {0} -o {1}'.format(binaryFile, binaryFile + '.tmp.64'), shell=True)
        subprocess.check_call('lipo -thin i386 {0} -o {1}'.format(binaryFile, binaryFile + '.tmp.32'), shell=True)
        os.remove(binaryFile)
        command = 'objconv -v0 -wd1214 -wd1106 -fmacho64 -nf:indigostd.syms {0} {1}'.format(binaryFile + '.tmp.64', binaryFile + '.64')
        if 'VERBOSE' in os.environ:
            print(command)
        subprocess.check_call(command, shell=True, stderr=subprocess.PIPE if not 'VERBOSE' in os.environ else None)
        command = 'objconv -v0 -wd1214 -wd1106 -fmacho32 -nf:indigostd.syms {0} {1}'.format(binaryFile + '.tmp.32', binaryFile + '.32')
        if 'VERBOSE' in os.environ:
            print(command)
        subprocess.check_call(command, shell=True, stderr=subprocess.PIPE if not 'VERBOSE' in os.environ else None)
        command = 'lipo -create {0} {1} -output {2}'.format(binaryFile + '.64', binaryFile + '.32', binaryFile)
        if 'VERBOSE' in os.environ:
            print(command)
        subprocess.check_call(command, shell=True, stderr=subprocess.PIPE if not 'VERBOSE' in os.environ else None)
        os.remove(binaryFile + '.tmp.32')
        os.remove(binaryFile + '.tmp.64')
        os.remove(binaryFile + '.32')
        os.remove(binaryFile + '.64')


    libRoot = os.path.dirname(target)
    shutil.copy('/usr/lib/libc++.a', libRoot + '/libc++.a')
    getIndigoStdSyms(libRoot)

    for objFile in objFiles:
        lipoObjconvLipo(objFile)

    if target.find('libindigo.dylib') != -1:
        lipoObjconvLipo(libRoot + '/libc++.a')
        linkLibraries = linkLibraries + ' -Wl,-all_load {0}/libc++.a -Wl,-noall_load'.format(libRoot)

    for library in os.listdir(libRoot):
        if not library.endswith('.a') or library == 'libindigoc++.a':
            continue
        if 0 in [s[2].find('_ind') if len(s) > 1 else -1 for s in getSymbols(os.path.join(libRoot, library))]:
            continue
        lipoObjconvLipo(os.path.join(libRoot, library))

    cmd = '{0} -L{1}/ -arch i386 -arch x86_64 -undefined dynamic_lookup -nodefaultlibs -lpthread -lc -lm -std=c++11 -mmacosx-version-min=10.7 -dynamiclib {2} {3} {4} -o {5}'.format(compiler, libRoot, linkFlags, ' '.join(objFiles), linkLibraries, target)
    if 'VERBOSE' in os.environ:
        print(cmd)
    subprocess.check_call(cmd, shell=True, stderr=subprocess.PIPE if not 'VERBOSE' in os.environ else None)


def main():
    args = ' '.join(sys.argv).split('|')
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
