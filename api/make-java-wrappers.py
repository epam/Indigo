import os
import shutil
from os.path import *
import subprocess

from optparse import OptionParser

parser = OptionParser(description='Indigo Java libraries build script')
parser.add_option('--suffix', '-s', help='archive suffix', default="")

(args, left_args) = parser.parse_args()

# Find indigo version
from get_indigo_version import getIndigoVersion
version = getIndigoVersion()

api_dir = abspath(dirname(__file__))
root = join(api_dir, "..")
dist_dir = join(root, "dist")
if not os.path.exists(dist_dir):
    os.mkdir(dist_dir)

os.chdir(dist_dir)
if os.path.exists("java"):
    shutil.rmtree("java")
os.mkdir('java')

os.chdir(os.path.join(api_dir, "java"))
subprocess.check_call("mvn -q versions:set -DnewVersion=%s" % version, shell=True)
subprocess.check_call("mvn -q clean package install -Dmaven.test.skip=true", shell=True)
shutil.copy(os.path.join(os.path.abspath(os.curdir), 'target', 'indigo-%s.jar' % version), os.path.join(dist_dir, 'java', 'indigo.jar'))

os.chdir(os.path.join(api_dir, "plugins", "renderer", "java"))
subprocess.check_call("mvn -q versions:set -DnewVersion=%s" % version, shell=True)
subprocess.check_call("mvn -q clean package -Dmaven.test.skip=true", shell=True)
shutil.copy(os.path.join(os.path.abspath(os.curdir), 'target', 'indigo-renderer-%s.jar' % version), os.path.join(dist_dir, 'java', 'indigo-renderer.jar'))

os.chdir(os.path.join(api_dir, "plugins", "inchi", "java"))
subprocess.check_call("mvn -q versions:set -DnewVersion=%s" % version, shell=True)
subprocess.check_call("mvn -q clean package -Dmaven.test.skip=true", shell=True)
shutil.copy(os.path.join(os.path.abspath(os.curdir), 'target', 'indigo-inchi-%s.jar' % version), os.path.join(dist_dir, 'java', 'indigo-inchi.jar'))

os.chdir(os.path.join(api_dir, "plugins", "bingo", "java"))  # TODO: Update when folder will be changed to nosql
subprocess.check_call("mvn -q versions:set -DnewVersion=%s" % version, shell=True)
subprocess.check_call("mvn -q clean package -Dmaven.test.skip=true", shell=True)
shutil.copy(os.path.join(os.path.abspath(os.curdir), 'target', 'bingo-nosql-%s.jar' % version), os.path.join(dist_dir, 'java', 'bingo-nosql.jar'))

os.chdir(dist_dir)
shutil.copy(os.path.join(api_dir, "LICENSE"), "java")

shutil.copy(os.path.join(root, "common", "jna", "jna.jar"), "java")

archive_name = "./indigo-java-%s-%s" % (version, args.suffix)
os.rename("java", archive_name)
if os.path.exists(archive_name + ".zip"):
    os.remove(archive_name + ".zip")
shutil.make_archive(archive_name, 'zip', os.path.dirname(archive_name), archive_name)
shutil.rmtree(archive_name)
full_archive_name = os.path.normpath(os.path.join(dist_dir, archive_name))
print('Archive {}.zip created'.format(full_archive_name))
