import glob
import os
import shutil
import subprocess
from os.path import *
from zipfile import ZipFile
from optparse import OptionParser
import re

version = ""
cur_dir = split(__file__)[0]
for line in open(join(os.path.dirname(os.path.abspath(__file__)), "..", "api", "indigo-version.cmake")):
    m = re.search('SET\(INDIGO_VERSION "(.*)"', line)
    if m:
        version = m.group(1)

presets = {
    "win32" : ("Visual Studio 10", ""),
    "win64" : ("Visual Studio 10 Win64", ""),
    "linux32" : ("Unix Makefiles", "-DSUBSYSTEM_NAME=x86"),
    "linux64" : ("Unix Makefiles", "-DSUBSYSTEM_NAME=x64"),
    "mac10.5" : ("Xcode", "-DSUBSYSTEM_NAME=10.5"),
    "mac10.6" : ("Xcode", "-DSUBSYSTEM_NAME=10.6"),
}

parser = OptionParser(description='Indigo utilities build script')
parser.add_option('--generator', help='this option is passed as -G option for cmake')
parser.add_option('--params', default="", help='additional build parameters')
parser.add_option('--config', default="Release", help='project configuration')
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
if not args.generator:
    print("Generator must be specified")
    exit()

cur_dir = abspath(dirname(__file__))
root = join(cur_dir, "..")
project_dir = join(cur_dir, "indigo-utils")

if args.generator.find("Unix Makefiles") != -1:
    args.params += " -DCMAKE_BUILD_TYPE=" + args.config
    
build_dir = (args.generator + " " + args.params)
build_dir = "indigo_utils_" + build_dir.replace(" ", "_").replace("=", "_").replace("-", "_")

full_build_dir = os.path.join(root, "build", build_dir)
if os.path.exists(full_build_dir) and args.clean:
    print("Removing previous project files")
    shutil.rmtree(full_build_dir)
if not os.path.exists(full_build_dir):
    os.makedirs(full_build_dir)

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

subprocess.call("cmake --build . --config %s" % (args.config), shell=True)
if args.generator.find("Unix Makefiles") != -1:
    subprocess.check_call("make package", shell=True)
elif args.generator.find("Xcode") != -1:
    subprocess.check_call("cmake --build . --target package --config %s" % (args.config), shell=True)
elif args.generator.find("Visual Studio") != -1:
    subprocess.check_call("cmake --build . --target PACKAGE --config %s" % (args.config), shell=True)
else:
    print("Do not know how to run package and install target")
                                                              
os.chdir(root)
if not os.path.exists("dist"):
    os.mkdir("dist")
dist_dir = join(root, "dist")
    
for f in os.listdir(full_build_dir):
    path, ext = os.path.splitext(f)
    if ext == ".zip":
        shutil.copy(join(full_build_dir, f), join(dist_dir, f.replace('-shared', '')))

# Chemdiff
for filename in os.listdir(dist_dir):
    if filename.startswith("indigo-java") :
        os.chdir(dist_dir)
        if os.path.exists("indigo-java"):
            shutil.rmtree("indigo-java")
        os.mkdir("indigo-java")
        java_dir = join(dist_dir, "indigo-java")
        if filename.endswith("-win.zip") or filename.endswith("-universal.zip"):
            uz = ZipFile(join(dist_dir, filename))
            uz.extractall(path=dist_dir)
            os.rename(join(dist_dir, filename)[:-4], "indigo-java")
            if filename.endswith('-universal.zip'):
                os.chdir(join(root, "utils", "chemdiff"))
                subprocess.check_call(["ant", "clean"], shell=True)
                subprocess.check_call(["ant", "jar"], shell=True)
                shutil.copyfile(join("dist", "chemdiff.jar"), join(dist_dir, "chemdiff.jar"))
                shutil.copyfile(join("chemdiff.sh"), join(dist_dir, "chemdiff.sh"))
                shutil.copyfile(join("LICENSE.GPL"), join(dist_dir, "LICENSE.GPL"))
                os.chdir(dist_dir)
                os.mkdir("lib")
                for file in glob.glob("indigo-java/*.jar"):
                    shutil.copy(file, "lib")
                shutil.copy(join(root, "common/java/common-controls/dist/common-controls.jar"), "lib")
                with ZipFile("chemdiff-%s-universal.zip" % version, 'w') as zip:
                    zip.write("chemdiff.jar")
                    zip.write("chemdiff.sh")
                    zip.write("LICENSE.GPL")
                    zip.write("lib/")
                os.remove("chemdiff.jar")
                os.remove("chemdiff.sh")
                os.remove("LICENSE.GPL")
                shutil.rmtree("lib")
            else:
                continue
                # TODO: Add Windows support
        else:
            continue