import os
import runpy
import shutil
import sys
from optparse import OptionParser
from os.path import *

parser = OptionParser(description='Indigo Python libraries build script')
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

archive_name = "./indigo-python-" + version + args.suffix

dest = os.path.join(dist_dir, archive_name)
if os.path.exists(dest):
    shutil.rmtree(dest)
os.mkdir(dest)
shutil.copy(os.path.join(api_dir, "python", "indigo.py"), dest)
shutil.copy(os.path.join(api_dir, "plugins", "renderer", "python", "indigo_renderer.py"), dest)
shutil.copy(os.path.join(api_dir, "plugins", "inchi", "python", "indigo_inchi.py"), dest)
shutil.copy(os.path.join(api_dir, "plugins", "bingo", "python", "bingo.py"), dest)
shutil.copytree(os.path.join(api_dir, "libs", "shared"),
                os.path.join(dest, "lib"),
                ignore=shutil.ignore_patterns("*.lib"))

shutil.copy(os.path.join(api_dir, "LICENSE"), dest)
os.chdir(dist_dir)
if os.path.exists(archive_name + ".zip"):
    os.remove(archive_name + ".zip")
shutil.make_archive(archive_name, 'zip', os.path.dirname(archive_name), archive_name)
shutil.rmtree(archive_name)
full_archive_name = os.path.normpath(os.path.join(dist_dir, archive_name))
print('Archive {}.zip created'.format(full_archive_name))


def make_wheels(api_dir, dest):
    os.makedirs(os.path.join(dest), exist_ok=True)
    os.makedirs(os.path.join(dest, 'indigo'), exist_ok=True)

    shutil.copy(os.path.join(api_dir, "LICENSE"), dest)
    shutil.copy(os.path.join(api_dir, "python", "indigo.py"), os.path.join(dest, 'indigo', '__init__.py'))

    shutil.copy(os.path.join(api_dir, "plugins", "renderer", "python", "indigo_renderer.py"), os.path.join(dest, 'indigo', 'renderer.py'))
    shutil.copy(os.path.join(api_dir, "plugins", "inchi", "python", "indigo_inchi.py"), os.path.join(dest, 'indigo', 'inchi.py'))
    shutil.copy(os.path.join(api_dir, "plugins", "bingo", "python", "bingo.py"), os.path.join(dest, 'indigo', 'bingo.py'))
    shutil.copy(os.path.join(api_dir, "python", "setup.py"), dest)
    shutil.copytree(os.path.join(api_dir, "libs", "shared"), os.path.join(dest, 'indigo', "lib"), ignore=shutil.ignore_patterns("*.lib"))
    curdir = os.path.abspath(os.curdir)
    os.chdir(dest)
    sysargv = sys.argv
    sys.argv[1:] = ['bdist_wheel', '--plat-name=win32']
    runpy.run_path('setup.py')
    sys.argv[1:] = ['bdist_wheel', '--plat-name=win_amd64']
    runpy.run_path('setup.py')
    sys.argv[1:] = ['bdist_wheel', '--plat-name=manylinux1_x86_64']
    runpy.run_path('setup.py')
    sys.argv[1:] = ['bdist_wheel', '--plat-name=manylinux1_i686']
    runpy.run_path('setup.py')
    sys.argv[1:] = ['bdist_wheel', '--plat-name=macosx_10_7']
    runpy.run_path('setup.py')
    sys.argv[1:] = sysargv[1:]
    os.chdir(curdir)


if __name__ == '__main__':
    make_wheels(api_dir, os.path.join(dist_dir, 'epam.indigo'))
