import subprocess
import os
import shutil
import sys


def getIndigoStdSyms(libRoot):
    libstdcppSymbols = [item.replace('  ', '').split(' ') for item in subprocess.check_output('nm {0}/libstdc++.a'.format(libRoot), shell=True).split('\n')]
    renameSymbols = []

    invMap = {}
    for item in libstdcppSymbols:
        if len(item) < 2:
            continue
        if item[1] not in ('u', 'U', 'r', 'n') and item[2].find('pthread') == -1:
            newName = 'i' + item[2][1:]
            if newName in invMap:
                if invMap[newName] != item[2]:
                    exit('Duplicate symbol: {0} for {1} and {2}'.format(newName, invMap[newName], item[2]))
            else:
                invMap[newName] = item[2]
                renameSymbols.append((item[2], newName))
    with open('indigostd.syms', 'wt') as f:
        for item in renameSymbols:
            f.write('{0} {1}\n'.format(item[0], item[1]))

def linux(compiler, linkFlags, arch, objFiles, linkLibraries, target):
    print os.path.normpath(os.path.abspath(os.curdir))
    systemSubsystemName = arch
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
        libFile = os.path.join(libRoot, library)
        subprocess.check_call('objcopy --redefine-syms indigostd.syms {0}'.format(libFile), shell=True)
    
    # TODO: Same for Mac
    staticLib = ''
    if target.find('libindigo.so') != -1:
        staticLib = '-static-libstdc++'

    linkCommand = '{0} -v -shared -L{1}/ {6} {2} {3} {4} -o {5}'.format(compiler, libRoot, linkFlags, ' '.join(objFiles), linkLibraries, target, staticLib)
    print(linkCommand)
    subprocess.check_call(linkCommand, shell=True)

def mac(compiler, linkFlags, arch, objFiles, linkLibraries, target):
    libstdcppPath = subprocess.check_output('g++ -arch {0} -print-file-name=libstdc++.a'.format(arch), shell=True).replace('\n', '')

    libRoot = os.path.dirname(target)
    if not os.path.exists(libRoot):
        sys.exit("Cannot file a directory with library files")
    shutil.copy(libstdcppPath, libRoot + '/libstdc++.a')
    getIndigoStdSyms(libRoot)

    objconvArch = 32 if arch == 'i386' else 64

    for objFile in objFiles:
        shutil.move(objFile, objFile + '.tmp')
        subprocess.check_call('objconv -v0 -wd1214 -wd1106 -fmacho{0} -nf:indigostd.syms {1} {2}'.format(objconvArch, objFile + '.tmp', objFile), shell=True)
        os.remove(objFile + '.tmp')

    if target.find('libindigo.dylib') != -1:
        subprocess.check_call('objconv -v0 -wd1214 -wd1106 -fmacho{0} -nf:indigostd.syms {1} {2}'.format(objconvArch, '../../dist/Mac/10.6/lib/libstdc++.a', '../../dist/Mac/10.6/lib/libindigostdcpp.a'), shell=True)
        linkLibraries = linkLibraries + ' -Wl,-all_load {0}/libindigostdcpp.a -Wl,-noall_load'.format(libRoot)

    os.remove(libRoot + '/libstdc++.a')

    for library in os.listdir(libRoot):
        if not library.endswith('.a') or library == 'libindigostdcpp.a':
            continue
        libFile = os.path.join(libRoot, library)
        shutil.move(libFile, libFile + '.tmp')
        subprocess.check_call('objconv -v0 -wd1214 -wd1106 -fmacho{0} -nf:indigostd.syms {1} {2}'.format(objconvArch, libFile + '.tmp', libFile), shell=True)
        os.remove(libFile + '.tmp')

    cmd = '{0} -L{1}/ -lc -lgcc_eh -nostdlib -nodefaultlibs -mmacosx-version-min=10.6 -dynamiclib {2} -arch {3} {4} {5} -o {6}'.format(compiler, libRoot, linkFlags, arch, ' '.join(objFiles), linkLibraries, target)
    print(cmd)
    subprocess.check_call(cmd, shell=True)


def main():
    args = ' '.join(sys.argv).split('|')
    compiler = args[1]
    linkFlags = args[2]
    arch = args[3].strip()
    objFiles = filter(len, args[4].split(' '))
    linkLibraries = args[5]
    target = args[6].strip()

    if arch.find('Linux') != -1:
        linux(compiler, linkFlags, arch, objFiles, linkLibraries, target)
    else:
        mac(compiler, linkFlags, arch, objFiles, linkLibraries, target)


if __name__ == '__main__':
    main()
