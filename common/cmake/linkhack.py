import subprocess
import os
import shutil
import sys


def getIndigoStdSyms(path):
	libstdcppSymbols = [item.replace('  ', '').split(' ') for item in subprocess.check_output('nm ../../dist/{0}/lib/libstdc++.a'.format(path), shell=True).split('\n')]
	renameSymbols = []
	names = []
	for item in libstdcppSymbols:
		names.append(item[-1])
	for item in libstdcppSymbols:
		if names.count(item[-1]) > 1:
			continue
		if len(item) < 2:
			continue
		if item[1] not in ('u', 'U', 'V', 'r', 'w', 'W', 'n'):
			renameSymbols.append((item[2], '__indigo_{0}'.format(item[2])))
	with open('indigostd.syms', 'wt') as f:
		for item in renameSymbols:
			f.write('{0} {1}\n'.format(item[0], item[1]))

def linux(compiler, linkFlags, arch, objFiles, linkLibraries, target):
	print os.path.normpath(os.path.abspath(os.curdir))
	systemSubsystemName = arch
	libstdcppPath = subprocess.check_output('g++ -print-file-name=libstdc++.a', shell=True).replace('\n', '')
	shutil.copy(libstdcppPath, '../../dist/{0}/lib/libstdc++.a'.format(systemSubsystemName))

	getIndigoStdSyms(systemSubsystemName)

	for objFile in objFiles:
		subprocess.check_call('objcopy --redefine-syms indigostd.syms {0}'.format(objFile), shell=True)
	
	if target.find('libindigo.so') != -1:
		subprocess.check_call('objcopy --redefine-syms indigostd.syms ../../dist/{0}/lib/libstdc++.a ../../dist/{0}/lib/libindigostdcpp.a'.format(systemSubsystemName), shell=True)
		linkLibraries = linkLibraries + ' -Wl,--whole-archive ../../dist/{0}/lib/libindigostdcpp.a -Wl,--no-whole-archive '.format(systemSubsystemName)

	os.remove('../../dist/{0}/lib/libstdc++.a'.format(systemSubsystemName))

	for library in os.listdir('../../dist/{0}/lib/'.format(systemSubsystemName)):
		if not library.endswith('.a') or library == 'libindigostdcpp.a':
			continue
		libFile = os.path.join('../../dist/{0}/lib/'.format(systemSubsystemName), library)
		subprocess.check_call('objcopy --redefine-syms indigostd.syms {0}'.format(libFile), shell=True)
	
	linkCommand = '{0} -v -shared -L../../dist/{1}/lib/ -static-libstdc++ {2} {3} {4} -o {5}'.format(compiler, systemSubsystemName, linkFlags, ' '.join(objFiles), linkLibraries, target)
	print(linkCommand)
	subprocess.check_call(linkCommand, shell=True)

def mac(compiler, linkFlags, arch, objFiles, linkLibraries, target):
	libstdcppPath = subprocess.check_output('g++ -arch {0} -print-file-name=libstdc++.a'.format(arch), shell=True).replace('\n', '')
	shutil.copy(libstdcppPath, '../../dist/Mac/10.6/lib/libstdc++.a')
	getIndigoStdSyms('Mac/10.6')

	objconvArch = 32 if arch == 'i386' else 64

	for objFile in objFiles:
		shutil.move(objFile, objFile + '.tmp')
		subprocess.check_call('objconv -v0 -wd1214 -wd1106 -fmacho{0} -nf:indigostd.syms {1} {2}'.format(objconvArch, objFile + '.tmp', objFile), shell=True)
		os.remove(objFile + '.tmp')

	if target.find('libindigo.dylib') != -1:
		subprocess.check_call('objconv -v0 -wd1214 -wd1106 -fmacho{0} -nf:indigostd.syms {1} {2}'.format(objconvArch, '../../dist/Mac/10.6/lib/libstdc++.a', '../../dist/Mac/10.6/lib/libindigostdcpp.a'), shell=True)
		linkLibraries = linkLibraries + ' -Wl,-all_load ../../dist/Mac/10.6/lib/libindigostdcpp.a -Wl,-noall_load'

	os.remove('../../dist/Mac/10.6/lib/libstdc++.a')

	for library in os.listdir('../../dist/Mac/10.6/lib/'):
		if not library.endswith('.a') or library == 'libindigostdcpp.a':
			continue
		libFile = os.path.join('../../dist/Mac/10.6/lib', library)
		shutil.move(libFile, libFile + '.tmp')
		subprocess.check_call('objconv -v0 -wd1214 -wd1106 -fmacho{0} -nf:indigostd.syms {1} {2}'.format(objconvArch, libFile + '.tmp', libFile), shell=True)
		os.remove(libFile + '.tmp')

	print('{0} -L../../dist/Mac/10.6/lib/ -lc -lgcc_eh -nostdlib -nodefaultlibs -mmacosx-version-min=10.6 -dynamiclib {1} -arch {2} {3} {4} -o {5}'.format(compiler, linkFlags, arch, ' '.join(objFiles), linkLibraries, target))
	subprocess.check_call('{0} -L../../dist/Mac/10.6/lib/ -lc -lgcc_eh -nostdlib -nodefaultlibs -mmacosx-version-min=10.6 -dynamiclib {1} -arch {2} {3} {4} -o {5}'.format(compiler, linkFlags, arch, ' '.join(objFiles), linkLibraries, target), shell=True)


def main():
	args = ' '.join(sys.argv).split('|')
	compiler = args[1]
	linkFlags = args[2]
	arch = args[3].strip()
	objFiles = filter(len, args[4].split(' '))
	linkLibraries = args[5]
	target = args[6]

	if arch.find('Linux') != -1:
		linux(compiler, linkFlags, arch, objFiles, linkLibraries, target)
	else:
		mac(compiler, linkFlags, arch, objFiles, linkLibraries, target)


if __name__ == '__main__':
	main()
