import os
import shutil
from os.path import *
import re

from optparse import OptionParser

parser = OptionParser(description='Indigo Java libraries build script')
parser.add_option('--suffix', '-s', help='archive suffix', default="")
parser.add_option('--doc', default=False, action='store_true', help='Put documentation into the archive')

(args, left_args) = parser.parse_args()

# Find indigo version
from get_indigo_version import getIndigoVersion
version = getIndigoVersion()

api_dir = abspath(dirname(__file__))
doc_dir = join(api_dir, '..', 'doc')
root = join(api_dir, "..")
dist_dir = join(root, "dist")
if not os.path.exists(dist_dir):
    os.mkdir(dist_dir)

archive_name = "indigo-python-" + version + args.suffix

dest = os.path.join(dist_dir, archive_name)
if os.path.exists(dest):
    shutil.rmtree(dest)
os.mkdir(dest)
shutil.copy(os.path.join(api_dir, "python", "indigo.py"), dest)
shutil.copy(os.path.join(api_dir, "plugins", "renderer", "python", "indigo_renderer.py"), dest)
shutil.copy(os.path.join(api_dir, "plugins", "inchi", "python", "indigo_inchi.py"), dest)
shutil.copy(os.path.join(api_dir, "plugins", "bingo", "python", "bingo.py"), dest)
if args.doc:
	shutil.copytree(os.path.join(doc_dir, 'build', 'html'), os.path.join(dest, 'doc'))
shutil.copytree(os.path.join(api_dir, "libs", "shared"),
    os.path.join(dest, "lib"),
    ignore = shutil.ignore_patterns("*.lib"))

shutil.copy(os.path.join(api_dir, "LICENSE.GPL"), dest)

os.chdir(dist_dir)
if os.path.exists(archive_name + ".zip"):
    os.remove(archive_name + ".zip")
os.system("zip -r -9 -m %s.zip %s" % (archive_name, archive_name))
