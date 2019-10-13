import os
import shutil
import subprocess
import sys
from optparse import OptionParser


def make_zips(api_dir, dist_dir):
    parser = OptionParser(description='Indigo Python libraries build script')
    parser.add_option('--suffix', '-s', help='archive suffix', default="")

    (args, left_args) = parser.parse_args()

    # Find indigo version
    from get_indigo_version import getIndigoVersion
    version = getIndigoVersion()

    if not os.path.exists(dist_dir):
        os.mkdir(dist_dir)

    archive_name = "./indigo-python-" + version + args.suffix

    dest = os.path.join(dist_dir, archive_name)
    if os.path.exists(dest):
        shutil.rmtree(dest)
    os.mkdir(dest)
    os.mkdir(os.path.join(dest, 'indigo'))
    shutil.copy(os.path.join(api_dir, "python", 'indigo.py'), os.path.join(dest, 'indigo', '__init__.py'))
    shutil.copy(os.path.join(api_dir, "plugins", "renderer", "python", "indigo_renderer.py"), dest)
    shutil.copy(os.path.join(api_dir, "plugins", "renderer", "python", "indigo_renderer.py"), os.path.join(dest, 'indigo', 'renderer.py'))
    shutil.copy(os.path.join(api_dir, "plugins", "inchi", "python", "indigo_inchi.py"), dest)
    shutil.copy(os.path.join(api_dir, "plugins", "inchi", "python", "indigo_inchi.py"), os.path.join(dest, 'indigo', 'inchi.py'))
    shutil.copy(os.path.join(api_dir, "plugins", "bingo", "python", "bingo.py"), dest)
    shutil.copy(os.path.join(api_dir, "plugins", "bingo", "python", "bingo.py"), os.path.join(dest, 'indigo', 'bingo.py'))
    shutil.copytree(os.path.join(api_dir, "libs", "shared"), os.path.join(dest, "lib"), ignore=shutil.ignore_patterns("*.lib"))

    shutil.copy(os.path.join(api_dir, "LICENSE"), dest)
    os.chdir(dist_dir)
    if os.path.exists(archive_name + ".zip"):
        os.remove(archive_name + ".zip")
    shutil.make_archive(archive_name, 'zip', os.path.dirname(archive_name), archive_name)
    shutil.rmtree(archive_name)
    full_archive_name = os.path.normpath(os.path.join(dist_dir, archive_name))
    print('Archive {}.zip created'.format(full_archive_name))


def make_wheels(api_dir, dest):
    if os.path.exists(dest):
        shutil.rmtree(dest)
    os.makedirs(dest)
    os.makedirs(os.path.join(dest, 'indigo'))

    shutil.copy(os.path.join(api_dir, "LICENSE"), dest)
    shutil.copy(os.path.join(api_dir, "python", "indigo.py"), os.path.join(dest, 'indigo', '__init__.py'))
    shutil.copy(os.path.join(api_dir, "plugins", "renderer", "python", "indigo_renderer.py"), os.path.join(dest, 'indigo', 'renderer.py'))
    shutil.copy(os.path.join(api_dir, "plugins", "inchi", "python", "indigo_inchi.py"), os.path.join(dest, 'indigo', 'inchi.py'))
    shutil.copy(os.path.join(api_dir, "plugins", "bingo", "python", "bingo.py"), os.path.join(dest, 'indigo', 'bingo.py'))
    shutil.copy(os.path.join(api_dir, "python", "setup.py"), dest)
    shutil.copytree(os.path.join(api_dir, "libs", "shared"), os.path.join(dest, 'indigo', "lib"), ignore=shutil.ignore_patterns("*.lib"))
    cur_dir = os.path.abspath(os.curdir)
    os.chdir(dest)
    subprocess.check_call([sys.executable, 'setup.py', 'bdist_wheel', '--plat-name=win32'])
    subprocess.check_call([sys.executable, 'setup.py', 'bdist_wheel', '--plat-name=win_amd64'])
    subprocess.check_call([sys.executable, 'setup.py', 'bdist_wheel', '--plat-name=manylinux1_x86_64'])
    subprocess.check_call([sys.executable, 'setup.py', 'bdist_wheel', '--plat-name=manylinux1_i686'])
    subprocess.check_call([sys.executable, 'setup.py', 'bdist_wheel', '--plat-name=macosx_10_7_intel'])
    os.chdir(cur_dir)


if __name__ == '__main__':
    api_dir = os.path.abspath(os.path.dirname(__file__))
    root = os.path.normpath(os.path.join(api_dir, ".."))
    dist_dir = os.path.join(root, "dist")
    make_zips(api_dir, dist_dir)
    if sys.argv[1] == '-s' and sys.argv[2] == '-universal':
        make_wheels(api_dir, os.path.join(dist_dir, 'epam.indigo'))
