import os
import shutil
from os.path import *
import re

from optparse import OptionParser

parser = OptionParser(description='Indigo Java libraries build script')
parser.add_option('--suffix', '-s', help='archive suffix', default="")

(args, left_args) = parser.parse_args()

# find indigo version
version = ""
cur_dir = split(__file__)[0]
for line in open(join(cur_dir, "indigo-version.cmake")):
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

shutil.copy(os.path.join(root, "common", "jna", "jna.jar"), "java")

archive_name = "indigo-java-%s" % (version + args.suffix)
os.rename("java", archive_name)
os.system("zip -r -9 -m %s.zip %s" % (archive_name, archive_name))
