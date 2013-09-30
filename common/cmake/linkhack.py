import subprocess
import os
import shutil
import sys

def main():
	args = ' '.join(sys.argv).split('|')
	compiler = args[1]
	linkFlags = args[2]
	arch = args[3].strip()
	objFiles = filter(len, args[4].split(' '))
	linkLibraries = args[5]
	target = args[6]

	libstdcppPath = subprocess.check_output('g++ -arch {0} -print-file-name=libstdc++.a'.format(arch), shell=True).replace('\n', '')
	shutil.copy(libstdcppPath, '../../dist/Mac/10.6/lib/libstdc++.a')
	libstdcppSymbols = [item.replace('  ', '').split(' ') for item in subprocess.check_output('nm ../../dist/Mac/10.6/lib/libstdc++.a', shell=True).split('\n')]
	renameSymbols = []
	for item in libstdcppSymbols:
		if len(item) < 2:
			continue
		if item[1] not in ('u', 'U'):
			renameSymbols.append((item[2], '__indigo_{0}'.format(item[2])))
	objconvArch = 32 if arch == 'i386' else 64
	nrString = ''
	with open('indigostd.syms', 'wt') as f:
		for item in renameSymbols:
			f.write('{0} {1}\n'.format(item[0], item[1]))

	for objFile in objFiles:
		shutil.move(objFile, objFile + '.tmp')
		subprocess.check_call('objconv -v0 -wd1214 -wd1106 -fmacho{0} -nf:indigostd.syms {1} {2}'.format(objconvArch, objFile + '.tmp', objFile), shell=True)
		os.remove(objFile + '.tmp')

	if target.find('libindigo.dylib') != -1:
		subprocess.check_call('objconv -v0 -wd1214 -wd1106 -fmacho{0} -nf:indigostd.syms {1} {2}'.format(objconvArch, '../../dist/Mac/10.6/lib/libstdc++.a', '../../dist/Mac/10.6/lib/libindigostdcpp.a'), shell=True)
		linkLibraries = linkLibraries + ' -Wl,-all_load ../../dist/Mac/10.6/lib/libindigostdcpp.a -Wl,-noall_load'
		#subprocess.check_call('g++ -all_load -arch {0} -lc -lgcc_eh -nostdlib -nodefaultlibs -mmacosx-version-min=10.6 -dynamiclib ../../dist/Mac/10.6/lib/libindigostdcpp.a -o ../../dist/Mac/10.6/lib/libindigostdcpp.dylib'.format(arch), shell=True)
		#print(os.path.abspath(os.curdir))
		#print os.path.normpath(os.path.join(os.path.abspath(os.curdir), '../../../../api/libs/shared/Mac/10.6/lib/libindigostdcpp.dylib'))
		#if not os.path.exists(os.path.normpath(os.path.join(os.path.abspath(os.curdir), '../../../../api/libs/shared/Mac/10.6/'))):
		#os.makedirs(os.path.normpath(os.path.join(os.path.abspath(os.curdir), '../../../../api/libs/shared/Mac/10.6/')))
		#shutil.copy('../../dist/Mac/10.6/lib/libindigostdcpp.dylib', os.path.normpath(os.path.join(os.path.abspath(os.curdir), '../../../../api/libs/shared/Mac/10.6/libindigostdcpp.dylib')))
		#os.remove('../../dist/Mac/10.6/lib/libindigostdcpp.a')

	os.remove('../../dist/Mac/10.6/lib/libstdc++.a')

	for library in os.listdir('../../dist/Mac/10.6/lib/'):
		if not library.endswith('.a'):
			continue
		libFile = os.path.join('../../dist/Mac/10.6/lib', library)
		shutil.move(libFile, libFile + '.tmp')
		subprocess.check_call('objconv -v0 -wd1214 -wd1106 -fmacho{0} -nf:indigostd.syms {1} {2}'.format(objconvArch, libFile + '.tmp', libFile), shell=True)
		os.remove(libFile + '.tmp')

	print('{0} -L../../dist/Mac/10.6/lib/ -lc -lgcc_eh -nostdlib -nodefaultlibs -mmacosx-version-min=10.6 -dynamiclib {1} -arch {2} {3} {4} -o {5}'.format(compiler, linkFlags, arch, ' '.join(objFiles), linkLibraries, target))
	subprocess.check_call('{0} -L../../dist/Mac/10.6/lib/ -lc -lgcc_eh -nostdlib -nodefaultlibs -mmacosx-version-min=10.6 -dynamiclib {1} -arch {2} {3} {4} -o {5}'.format(compiler, linkFlags, arch, ' '.join(objFiles), linkLibraries, target), shell=True)

if __name__ == '__main__':
	main()