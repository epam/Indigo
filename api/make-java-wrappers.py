import os
import shutil
from os.path import *
import re
import subprocess

from optparse import OptionParser

parser = OptionParser(description='Indigo Java libraries build script')
parser.add_option('--suffix', '-s', help='archive suffix', default="")
parser.add_option('--doc', default=False, action='store_true', help='Put documentation into the archive')

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
os.mkdir('java')

os.chdir(os.path.join(api_dir, "java"))
subprocess.check_call("mvn versions:set -DnewVersion=%s" % version, shell=True)
subprocess.check_call("mvn clean package install -Dmaven.test.skip=true", shell=True)
shutil.copy(os.path.join(os.path.abspath(os.curdir), 'target', 'indigo-%s.jar' % version), os.path.join(dist_dir, 'java', 'indigo.jar'))

os.chdir(os.path.join(api_dir, "plugins", "renderer", "java"))
subprocess.check_call("mvn versions:set -DnewVersion=%s" % version, shell=True)
subprocess.check_call("mvn clean package -Dmaven.test.skip=true", shell=True)
shutil.copy(os.path.join(os.path.abspath(os.curdir), 'target', 'indigo-renderer-%s.jar' % version), os.path.join(dist_dir, 'java', 'indigo-renderer.jar'))

os.chdir(os.path.join(api_dir, "plugins", "inchi", "java"))
subprocess.check_call("mvn versions:set -DnewVersion=%s" % version, shell=True)
subprocess.check_call("mvn clean package -Dmaven.test.skip=true", shell=True)
shutil.copy(os.path.join(os.path.abspath(os.curdir), 'target', 'indigo-inchi-%s.jar' % version), os.path.join(dist_dir, 'java', 'indigo-inchi.jar'))

os.chdir(os.path.join(api_dir, "plugins", "bingo", "java")) # TODO: Update when folder will be changed to nosql
subprocess.check_call("mvn versions:set -DnewVersion=%s" % version, shell=True)
subprocess.check_call("mvn clean package -Dmaven.test.skip=true", shell=True)
shutil.copy(os.path.join(os.path.abspath(os.curdir), 'target', 'bingo-nosql-%s.jar' % version), os.path.join(dist_dir, 'java', 'bingo-nosql.jar'))

os.chdir(dist_dir)
shutil.copy(os.path.join(api_dir, "LICENSE.GPL"), "java")
if args.doc:
	doc_dir = join(api_dir, '..', 'doc')
	shutil.copytree(os.path.join(doc_dir, 'build', 'html'), os.path.join('java', 'doc'))

shutil.copy(os.path.join(root, "common", "jna", "jna.jar"), "java")

archive_name = "indigo-java-%s" % (version + args.suffix)
os.rename("java", archive_name)
subprocess.check_call("zip -r -9 -m %s.zip %s" % (archive_name, archive_name), shell=True)
