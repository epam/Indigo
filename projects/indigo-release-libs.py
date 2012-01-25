import os
import shutil
import sys
from os.path import *

config = "Release"

cur_dir = abspath(dirname(__file__))
root = join(cur_dir, "..")
project_dir = join(cur_dir, "indigo-all")

if not os.path.exists("build"):
    os.mkdir("build")
    
generator = ""
if len(sys.argv) < 2:
    print("Generator must be specified")
    
generator = sys.argv[1]

params = ""
if len(sys.argv) > 2:
    params = sys.argv[2]
    
build_dir = (generator + " " + params).replace(" ", "_")

full_build_dir = os.path.join(root, "build", build_dir)
if not os.path.exists(full_build_dir):
    os.mkdir(full_build_dir)

os.chdir(full_build_dir)
os.system("cmake -G \"%s\" %s %s" % (generator, params, project_dir))

os.system("cmake --build . --config %s" % (config))
if params.find("Makefiles") != -1:
    os.system("make package")
    os.system("make install")
else:
    os.system("cmake --build . --target PACKAGE --config %s" % (config))
    os.system("cmake --build . --target INSTALL --config %s" % (config))

os.chdir(root)
if not os.path.exists("dist"):
    os.mkdir("dist")
dist_dir = join(root, "dist")
    
for f in os.listdir(full_build_dir):
    path, ext = os.path.splitext(f)
    if ext == ".zip":
        shutil.copy(join(full_build_dir, f), join(dist_dir, f))
