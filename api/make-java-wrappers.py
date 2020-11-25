import os
import shutil
from os.path import *
import subprocess

from optparse import OptionParser
import xml.etree.ElementTree as ElementTree

parser = OptionParser(description='Indigo Java libraries build script')
parser.add_option('--suffix', '-s', help='archive suffix', default="")
parser.add_option('--publish', help='Publish jar',  default=False, action='store_true')
(args, left_args) = parser.parse_args()


# Find indigo version
def get_pom_version_from_curdir():
    tree = ElementTree.parse(os.path.join(os.curdir, 'pom.xml'))
    ElementTree.register_namespace('', 'http://maven.apache.org/POM/4.0.0')
    root = tree.getroot()
    version = None
    for child in root:
        if child.tag.endswith('version'):
            version = child.text
    return version


api_dir = abspath(dirname(__file__))
root = join(api_dir, "..")
dist_dir = join(root, "dist")
if not os.path.exists(dist_dir):
    os.mkdir(dist_dir)

os.chdir(dist_dir)
if os.path.exists("java"):
    shutil.rmtree("java")
os.mkdir('java')

mvn_cmd = 'mvnw.cmd' if os.name == 'nt' else './mvnw'

publish = '' if not args.publish else 'deploy -P sign-artifacts -Dgpg.passphrase={} -Dossrh.user={} -Dossrh.password={}'.format(
    os.environ['GPG_PASSPHRASE'], os.environ['MAVEN_USER'], os.environ['MAVEN_PASSWORD'])

os.chdir(os.path.join(api_dir, "java"))
version = get_pom_version_from_curdir()
subprocess.check_call("%s -X -B clean package verify install %s" % (mvn_cmd, publish), shell=True)
shutil.copy(os.path.join(os.path.abspath(os.curdir), 'target', 'indigo-%s.jar' % version), os.path.join(dist_dir, 'java', 'indigo.jar'))

os.chdir(os.path.join(api_dir, "plugins", "renderer", "java"))
version = get_pom_version_from_curdir()
subprocess.check_call("%s -X -B clean package verify install %s" % (mvn_cmd, publish), shell=True)
shutil.copy(os.path.join(os.path.abspath(os.curdir), 'target', 'indigo-renderer-%s.jar' % version), os.path.join(dist_dir, 'java', 'indigo-renderer.jar'))

os.chdir(os.path.join(api_dir, "plugins", "inchi", "java"))
version = get_pom_version_from_curdir()
subprocess.check_call("%s -X -B clean package verify install %s" % (mvn_cmd, publish), shell=True)
shutil.copy(os.path.join(os.path.abspath(os.curdir), 'target', 'indigo-inchi-%s.jar' % version), os.path.join(dist_dir, 'java', 'indigo-inchi.jar'))

os.chdir(os.path.join(api_dir, "plugins", "bingo", "java"))
version = get_pom_version_from_curdir()
subprocess.check_call("%s -X -B clean package verify install %s" % (mvn_cmd, publish), shell=True)
shutil.copy(os.path.join(os.path.abspath(os.curdir), 'target', 'bingo-nosql-%s.jar' % version), os.path.join(dist_dir, 'java', 'bingo-nosql.jar'))

if args.publish:
    os.chdir(os.path.join(api_dir, "plugins", "bingo-elastic", "java"))
    version = get_pom_version_from_curdir()
    subprocess.check_call("%s -B clean package verify install %s" % (mvn_cmd, publish), shell=True)

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
