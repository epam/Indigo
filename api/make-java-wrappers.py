import os
import shutil
from os.path import *
import re

# find indigo version
version = ""
for line in open("indigo-version.cmake"):
    m = re.search('SET\(INDIGO_VERSION "(.*)"', line)
    if m:
        version = m.group(1)

api_dir = abspath(dirname(__file__))
root = join(api_dir, "..")
dist_dir = join(root, "dist")
if not os.path.exists(dist_dir):
    os.mkdir(dist_dir)

os.chdir(dist_dir)
if os.path.exists("java"):
    shutil.rmtree("java")
    
os.chdir(os.path.join(api_dir, "java"))
os.system("ant clean")
os.system("ant jar")

os.chdir(os.path.join(api_dir, "renderer", "java"))
os.system("ant clean")
os.system("ant jar")

os.chdir(os.path.join(api_dir, "plugins", "inchi", "java"))
os.system("ant clean")
os.system("ant jar")

os.chdir(dist_dir)
shutil.copy(os.path.join(api_dir, "LICENSE.GPL"), "java")

os.rename("java", "indigo-java-%s" % (version))
os.system("zip -r -9 -m indigo-java-%s.zip indigo-java-%s" % (version, version))
