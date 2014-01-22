import os
import shutil
import sys
import subprocess
from os.path import *

from optparse import OptionParser

presets = {
    "win32" : ("Visual Studio 10", ""),
    "win64" : ("Visual Studio 10 Win64", ""),
    "win32-2012" : ("Visual Studio 11", ""),
    "win64-2012" : ("Visual Studio 11 Win64", ""),
    "win32-2013" : ("Visual Studio 12", ""),
    "win64-2013" : ("Visual Studio 12 Win64", ""),
    "win32-mingw": ("MinGW Makefiles", ""),
    "linux32" : ("Unix Makefiles", "-DSUBSYSTEM_NAME=x86"),
    "linux32-universal" : ("Unix Makefiles", "-DSUBSYSTEM_NAME=x86"),
    "linux64" : ("Unix Makefiles", "-DSUBSYSTEM_NAME=x64"),
    "linux64-universal" : ("Unix Makefiles", "-DSUBSYSTEM_NAME=x64"),
    "mac10.5" : ("Xcode", "-DSUBSYSTEM_NAME=10.5"),
    "mac10.6" : ("Xcode", "-DSUBSYSTEM_NAME=10.6"),
    "mac10.7" : ("Xcode", "-DSUBSYSTEM_NAME=10.7"),
    "mac10.8" : ("Xcode", "-DSUBSYSTEM_NAME=10.8"),
    "mac10.9" : ("Xcode", "-DSUBSYSTEM_NAME=10.9"),
    "mac-universal" : ("Unix Makefiles", "-DSUBSYSTEM_NAME=10.9"),
}

parser = OptionParser(description='Indigo libraries build script')
parser.add_option('--generator', help='this option is passed as -G option for cmake')
parser.add_option('--params', default="", help='additional build parameters')
parser.add_option('--config', default="Release", help='project configuration')
parser.add_option('--nobuild', default=False,
    action="store_true", help='configure without building', dest="nobuild")
parser.add_option('--clean', default=False, action="store_true",
    help='delete all the build data', dest="clean")
parser.add_option('--preset', type="choice", dest="preset",
    choices=list(presets.keys()), help='build preset %s' % (str(list(presets.keys()))))
parser.add_option('--cairo-gl', dest="cairogl",
    default=False, action="store_true", help='Build Cairo with OpenGL support')
parser.add_option('--cairo-vg', dest="cairovg",
    default=False, action="store_true", help='Build Cairo with CairoVG support')
parser.add_option('--cairo-egl', dest="cairoegl",
    default=False, action="store_true", help='Build Cairo with EGL support')
parser.add_option('--cairo-glesv2', dest="cairoglesv2",
    default=False, action="store_true", help='Build Cairo with GLESv2 support')
parser.add_option('--find-cairo', dest="findcairo",
    default=False, action="store_true", help='Find and use system Cairo')
parser.add_option('--find-pixman', dest="findpixman",
    default=False, action="store_true", help='Find and use system Pixman')
if os.name == 'posix':
    parser.add_option('--check-abi', dest='checkabi',
        default=False, action="store_true", help='Check ABI type of Indigo libraries on Linux')

(args, left_args) = parser.parse_args()
if len(left_args) > 0:
    print("Unexpected arguments: %s" % (str(left_args)))
    exit()

if args.preset:
    args.generator, args.params = presets[args.preset]
if not args.generator:
    print("Generator must be specified")
    exit()

cur_dir = abspath(dirname(__file__))
root = join(cur_dir, "..")
project_dir = join(cur_dir, "indigo-all")

#if args.generator.find("Unix Makefiles") != -1 or args.generator.find("MinGW Makefiles") != -1:
args.params += " -DCMAKE_BUILD_TYPE=" + args.config

if args.cairogl:
    args.params += ' -DWITH_CAIRO_GL=TRUE'

if args.cairovg:
    args.params += ' -DWITH_CAIRO_VG=TRUE'

if args.cairoegl:
    args.params += ' -DWITH_CAIRO_EGL=TRUE'

if args.cairoglesv2:
    args.params += ' -DWITH_CAIRO_GLESV2=TRUE'

if args.findcairo:
    args.params += ' -DUSE_SYSTEM_CAIRO=TRUE'

if args.findcairo:
    args.params += ' -DUSE_SYSTEM_PIXMAN=TRUE'

if os.name == 'posix' and args.checkabi:
    args.params += ' -DCHECK_ABI=TRUE'

build_dir = (args.generator + " " + args.params)
build_dir = "indigo_" + build_dir.replace(" ", "_").replace("=", "_").replace("-", "_")

def build(params=[], install=True):
    full_build_dir = os.path.join(root, "build", build_dir)
    if params:
        full_build_dir += "_UNIVERSAL_"
    if 'INDIGO_CMAKE_OSX_ARCHITECTURES' in params:
        full_build_dir += params['INDIGO_CMAKE_OSX_ARCHITECTURES']
    if os.path.exists(full_build_dir) and args.clean:
        print("Removing previous project files")
        shutil.rmtree(full_build_dir)
    if not os.path.exists(full_build_dir):
        os.makedirs(full_build_dir)

    os.chdir(full_build_dir)

    paramString = ''
    if params:
        for key, value in params.items():
            paramString += '{0}={1} '.format(key, value)
    if params:
        command = "%s %s cmake -G \"%s\" %s %s" % ('CC=gcc CXX=g++' if args.preset != 'mac-universal' else '', paramString, args.generator, args.params, project_dir)
    else:
        command = "%s cmake -G \"%s\" %s %s" % (paramString, args.generator, args.params, project_dir)
    print(command)
    subprocess.check_call(command, shell=True)

    if args.nobuild:
        exit(0)

    for f in os.listdir(full_build_dir):
        path, ext = os.path.splitext(f)
        if ext == ".zip":
            os.remove(join(full_build_dir, f))

    if args.generator.find("Unix Makefiles") != -1:
        subprocess.check_call("make package", shell=True)
        subprocess.check_call("make install", shell=True)
    elif args.generator.find("Xcode") != -1:
        subprocess.check_call("cmake --build . --target package --config %s" % (args.config), shell=True)
        subprocess.check_call("cmake --build . --target install --config %s" % (args.config), shell=True)
    elif args.generator.find("Visual Studio") != -1:
        subprocess.check_call("cmake --build . --target PACKAGE --config %s" % (args.config), shell=True)
        subprocess.check_call("cmake --build . --target INSTALL --config %s" % (args.config), shell=True)
    elif args.generator.find("MinGW Makefiles") != -1:
        subprocess.check_call("mingw32-make package", shell=True)
        subprocess.check_call("mingw32-make install", shell=True)
    else:
        print("Do not know how to run package and install target")
    subprocess.check_call("ctest -V --timeout 20 -C %s ." % (args.config), shell=True)

    if not params or 'INDIGO_CMAKE_OSX_ARCHITECTURES' not in params:
        os.chdir(root)
        if not os.path.exists("dist"):
            os.mkdir("dist")
        dist_dir = join(root, "dist")

        for f in os.listdir(full_build_dir):
            path, ext = os.path.splitext(f)
            if ext == ".zip":
                shutil.copy(join(full_build_dir, f), join(dist_dir, f))
    else:
        return full_build_dir


# if args.preset == 'mac-universal':
#     amd64Path = build({'UNIVERSAL': 'TRUE'}) #, 'INDIGO_CMAKE_OSX_ARCHITECTURES': 'x86_64'}, install=False)
#     #i386Path = build({'UNIVERSAL': 'TRUE'}), 'INDIGO_CMAKE_OSX_ARCHITECTURES': 'i386'}, install=False)

#     full_build_dir = os.path.join(root, "build", build_dir)
#     if os.path.exists(full_build_dir) and args.clean:
#         print("Removing previous project files")
#         shutil.rmtree(full_build_dir)
#     if not os.path.exists(full_build_dir):
#         os.makedirs(full_build_dir)
#     os.chdir(full_build_dir)
#     if os.path.exists('shared'):
#         shutil.rmtree('shared')
#     if os.path.exists('static'):
#         shutil.rmtree('static')

#     os.makedirs('shared/Mac/10.6/')
#     os.makedirs('static/Mac/10.6/')
#     for item in os.listdir(os.path.join(i386Path, 'dist', 'Mac', '10.6', 'lib')):
#         if item.endswith('.dylib'):
#             print(item)
#             subprocess.check_call('lipo -create -arch i386 {0}/dist/Mac/10.6/lib/{2} -arch x86_64 {1}/dist/Mac/10.6/lib/{2} -o shared/Mac/10.6/{2}'.format(i386Path, amd64Path, item), shell=True)
#         elif item.endswith('.a'):
#             subprocess.check_call('lipo -create -arch i386 {0}/dist/Mac/10.6/lib/{2} -arch x86_64 {1}/dist/Mac/10.6/lib/{2} -o static/Mac/10.6/{2}'.format(i386Path, amd64Path, item), shell=True)

#     for item in os.listdir(i386Path):
#         if item.endswith('.zip'):
#             if item.find('static') != -1:
#                 staticName = item
#             elif item.find('shared') != -1:
#                 sharedName = item
#             shutil.copy(os.path.join(i386Path, item), '.')

#     subprocess.check_call('zip -u {0} -r static'.format(staticName), shell=True)
#     subprocess.check_call('zip -u {0} -r shared'.format(sharedName), shell=True)

#     dist_dir = join(root, "dist")
#     if not os.path.exists(dist_dir):
#         os.mkdir(dist_dir)

#     for f in os.listdir('.'):
#         path, ext = os.path.splitext(f)
#         if ext == ".zip":
#             shutil.copy(join(full_build_dir, f), join(dist_dir, f))

if args.preset in ('linux64-universal', 'linux32-universal', 'mac-universal'):
    build({'UNIVERSAL': 'TRUE'})
else:
    build()
