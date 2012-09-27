import os
import re
import shutil
import subprocess
from os.path import *

from optparse import OptionParser

presets = {
    "win32" : ("Visual Studio 10", ""),
    "win64" : ("Visual Studio 10 Win64", ""),
    "win32-2012" : ("Visual Studio 11", ""),
    "win64-2012" : ("Visual Studio 11 Win64", ""),
    "linux32" : ("Unix Makefiles", "-DSUBSYSTEM_NAME=x86"),
    "linux64" : ("Unix Makefiles", "-DSUBSYSTEM_NAME=x64"),
    "mac10.5" : ("Xcode", "-DSUBSYSTEM_NAME=10.5"),
    "mac10.6" : ("Xcode", "-DSUBSYSTEM_NAME=10.6"),
    }

parser = OptionParser(description='Bingo build script')
parser.add_option('--generator', help='this option is passed as -G option for cmake')
parser.add_option('--params', default="", help='additional build parameters')
parser.add_option('--config', default="Release", help='project configuration')
parser.add_option('--dbms', help='DMBS (oracle, postgres or sqlserver)')
parser.add_option('--nobuild', default=False,
    action="store_true", help='configure without building', dest="nobuild")
parser.add_option('--clean', default=False, action="store_true",
    help='delete all the build data', dest="clean")
parser.add_option('--preset', type="choice", dest="preset",
    choices=presets.keys(), help='build preset %s' % (str(presets.keys())))

(args, left_args) = parser.parse_args()
if len(left_args) > 0:
    print("Unexpected arguments: %s" % (str(left_args)))
    exit()

if args.preset:
    args.generator, args.params = presets[args.preset]
if not args.generator and args.dbms != 'sqlserver':
    print("Generator must be specified")
    exit()

cur_dir = abspath(dirname(__file__))
root = os.path.normpath(join(cur_dir, ".."))
project_dir = join(cur_dir, "bingo-%s" % args.dbms)

if args.dbms != 'sqlserver':
    build_dir = (args.dbms + " " + args.generator + " " + args.params)
    build_dir = build_dir.replace(" ", "_").replace("=", "_").replace("-", "_")
    full_build_dir = os.path.join(root, "build", build_dir)
    if os.path.exists(full_build_dir) and args.clean:
        print("Removing previous project files")
        shutil.rmtree(full_build_dir)
    if not os.path.exists(full_build_dir):
        os.makedirs(full_build_dir)

    if args.generator.find("Unix Makefiles") != -1:
        args.params += " -DCMAKE_BUILD_TYPE=" + args.config

    os.chdir(full_build_dir)
    command = "cmake -G \"%s\" %s %s" % (args.generator, args.params, project_dir)
    print(command)
    subprocess.check_call(command, shell=True)

    if args.nobuild:
        exit(0)

    for f in os.listdir(full_build_dir):
        path, ext = os.path.splitext(f)
        if ext == ".zip":
            os.remove(join(full_build_dir, f))

    subprocess.check_call("cmake --build . --config %s" % args.config, shell=True)

    if args.generator.find("Unix Makefiles") != -1:
        subprocess.check_call("make package", shell=True)
        subprocess.check_call("make install", shell=True)
    elif args.generator.find("Xcode") != -1:
        subprocess.check_call("cmake --build . --target package --config %s" % args.config, shell=True)
        subprocess.check_call("cmake --build . --target install --config %s" % args.config, shell=True)
    elif args.generator.find("Visual Studio") != -1:
        subprocess.check_call("cmake --build . --target PACKAGE --config %s" % args.config, shell=True)
        subprocess.check_call("cmake --build . --target INSTALL --config %s" % args.config, shell=True)
    else:
        print("Do not know how to run package and install target")

    os.chdir(root)
    if not os.path.exists("dist"):
        os.mkdir("dist")
    dist_dir = join(root, "dist")

    for f in os.listdir(full_build_dir):
        path, ext = os.path.splitext(f)
        if ext == ".zip":
            shutil.copy(join(full_build_dir, f), join(dist_dir, f.replace('-shared.zip', '.zip')))
else:
    dllPath = {}
    
    vsversion = 'Visual Studio'
    if args.preset.find("2012") != -1:
        vsversion += ' 11'
    else:
        vsversion += ' 10'
    
    for arch, generator in (('x86', vsversion), ('x64', vsversion + ' Win64')):
        build_dir = (args.dbms + " " + generator + " " + args.params)
        build_dir = build_dir.replace(" ", "_").replace("=", "_").replace("-", "_")
        full_build_dir = os.path.join(root, "build", build_dir)
        dllPath[arch] = os.path.normpath(os.path.join(full_build_dir, 'dist', 'Win', arch, 'shared', args.config))
        if os.path.exists(full_build_dir) and args.clean:
            print("Removing previous project files")
            shutil.rmtree(full_build_dir)
        if not os.path.exists(full_build_dir):
            os.makedirs(full_build_dir)
        os.chdir(full_build_dir)
        command = "cmake -G \"%s\" %s %s" % (generator, args.params, project_dir)
        print(command)
        subprocess.check_call(command, shell=True)

        if args.nobuild:
            exit(0)

        for f in os.listdir(full_build_dir):
            path, ext = os.path.splitext(f)
            if ext == ".zip":
                os.remove(join(full_build_dir, f))

        subprocess.check_call("cmake --build . --config %s" % args.config, shell=True)

    os.chdir(join(root, 'bingo', 'sqlserver'))
    command = 'msbuild /t:Rebuild /p:Configuration=%s /property:DllPath32=%s /property:DllPath64=%s' % (args.config, dllPath['x86'], dllPath['x64'])
    print os.path.abspath(os.curdir), command
    subprocess.check_call(command)

    os.chdir(root)
    if not os.path.exists("dist"):
        os.mkdir("dist")
    dist_dir = join(root, "dist")
    if not os.path.exists(join(root, 'dist', 'bingo-sqlserver')):
        os.mkdir(join(root, 'dist', 'bingo-sqlserver'))
    else:
        shutil.rmtree(join(root, 'dist', 'bingo-sqlserver'))
        os.mkdir(join(root, 'dist', 'bingo-sqlserver'))

    os.mkdir(join(root, 'dist', 'bingo-sqlserver', 'assembly'))
    for item in os.listdir(join(root, 'bingo', 'sqlserver', 'sql')):
        if item.endswith('.sql') or item.endswith('.bat'):
            shutil.copyfile(join(root, 'bingo', 'sqlserver', 'sql', item), join(root, 'dist', 'bingo-sqlserver', item))
    if not os.path.exists(join(root, 'bingo', 'sqlserver', 'bin', args.config, 'bingo-sqlserver.dll')):
        print 'Warning: File %s does not exist, going to use empty stub instead' % join(root, 'dist', 'bingo-sqlserver', 'assembly', 'bingo-sqlserver.dll')
        open(join(root, 'dist', 'bingo-sqlserver', 'assembly', 'bingo-sqlserver.dll', 'w')).close()
    else:
        shutil.copyfile(join(root, 'bingo', 'sqlserver', 'bin', args.config, 'bingo-sqlserver.dll'), join(root, 'dist', 'bingo-sqlserver', 'assembly', 'bingo-sqlserver.dll'))
        
    # Get version
    version = ""
    for line in open(join('bingo', "bingo-version.cmake")):
        m = re.search('SET\(BINGO_VERSION "([^\"]*)\"', line)
        if m:
            version = m.group(1)
    os.chdir('dist')
    if os.path.exists('bingo-sqlserver-%s.zip' % version):
        os.remove('bingo-sqlserver-%s.zip' % version)
    shutil.make_archive('bingo-sqlserver-%s' % version, format='zip', root_dir=join(root, 'dist', 'bingo-sqlserver'))
    shutil.rmtree('bingo-sqlserver')
    os.chdir(root)