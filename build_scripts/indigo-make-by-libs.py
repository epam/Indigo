# This module assumes that you have installed all the
# libs files in the <source root>/dist directory
from contextlib import contextmanager
from distutils.dir_util import copy_tree
from optparse import OptionParser
import os
import re
import shutil
import subprocess
import sys
import zipfile

sys.path.append(os.path.join(os.path.dirname(__file__), "..", "api"))
from get_indigo_version import getIndigoVersion


def flatten_directory(dir):
    todelete = []
    for f in os.listdir(dir):
        if f.find("python") != -1 or f.find("java") != -1 or f.find("dotnet") != -1:
            continue
        dir2 = os.path.join(dir, f)
        if os.path.isdir(dir2):
            for f2 in os.listdir(dir2):
                f2full = os.path.join(dir2, f2)
                shutil.move(f2full, dir)
            todelete.append(dir2)
            os.rmdir(dir2)


def move_dir_content(src_dir, dest_dir):
    for f in os.listdir(src_dir):
        f2 = os.path.join(src_dir, f)
        destf2 = os.path.join(dest_dir, f)
        if os.path.isdir(destf2):
            move_dir_content(f2, destf2)
        elif not os.path.exists(destf2):
            shutil.move(f2, destf2)


def join_archives(names, destname):
    if os.path.exists(destname):
        shutil.rmtree(destname)
    dest_zfname = '{}.zip'.format(destname)
    if os.path.exists(dest_zfname):
        os.remove(dest_zfname)
    for name in names:
        zf_name = '{}.zip'.format(name)
        if not os.path.exists(zf_name):
            raise ValueError('Archive file {} does not exist!'.format(zf_name))
        if os.path.exists(name):
            shutil.rmtree(name)
        os.makedirs(name)
        with zipfile.ZipFile(zf_name) as zf:
            zf.extractall(name)
        copy_tree(name, destname)
        shutil.rmtree(name)
        os.remove(zf_name)
    shutil.make_archive(destname, 'zip', os.path.dirname(destname), destname)
    shutil.rmtree(destname)


def join_archives_by_pattern(pattern, destname):
    archives = []
    for f in os.listdir(os.curdir):
        if re.match(pattern, f):
            archives.append(os.path.splitext(f)[0])
    if len(archives) == 0:
        return
    join_archives(archives, destname)


def clear_libs():
    for f in os.listdir(libs_dir):
        if f == "readme.txt":
            continue
        ffull = os.path.join(libs_dir, f)
        if os.path.isdir(ffull):
            shutil.rmtree(ffull)
        else:
            os.remove(ffull)


def unpack_to_libs(name):
    with zipfile.ZipFile('{}.zip'.format(name)) as zf:
        zf.extractall(libs_dir)
        unzipped_folder = os.path.join(libs_dir, os.path.basename(name))
        for os_name in os.listdir(os.path.join(unzipped_folder, 'shared')):
            shutil.copytree(os.path.join(unzipped_folder, 'shared', os_name), os.path.join(libs_dir, 'shared', os_name))
        shutil.rmtree(unzipped_folder, ignore_errors=True)

@contextmanager
def cwd(path):
    oldpwd = os.getcwd()
    os.chdir(path)
    try:
        yield
    finally:
        os.chdir(oldpwd)


arc_joins = [
    ("./indigo-libs-%ver%-linux-shared", "indigo-libs-%ver%-linux.+-shared"),
    ("./indigo-libs-%ver%-win-shared", "indigo-libs-%ver%-win.+-shared"),
    ("./indigo-libs-%ver%-mac-shared", "indigo-libs-%ver%-mac.+-shared"),
    ("./indigo-libs-%ver%-linux-static", "indigo-libs-%ver%-linux.+-static"),
    ("./indigo-libs-%ver%-win-static", "indigo-libs-%ver%-win.+-static"),
    ("./indigo-libs-%ver%-mac-static", "indigo-libs-%ver%-mac.+-static"),
]

wrappers = [
    ("win", ["win"]),
    ("linux", ["linux"]),
    ("mac", ["mac"]),
    ("universal", ["win", "linux", "mac"]),
]

wrappers_gen = ["make-java-wrappers.py", "make-python-wrappers.py", 'make-dotnet-wrappers.py']


if __name__ == '__main__':
    parser = OptionParser(description='Indigo libraries repacking')
    parser.add_option('--libonlyname', help='extract only the library into api/lib')
    parser.add_option('--config', default="Release", help='project configuration')
    parser.add_option('--type', default='python,java,dotnet', help='wrapper (dotnet, java, python)')

    (args, left_args) = parser.parse_args()

    if not args.type:
        args.type = 'python,java,dotnet'

    if len(left_args) > 0:
        print("Unexpected arguments: %s" % (str(left_args)))
        exit()

    suffix = ""
    if args.config.lower() != "release":
        suffix = "-" + args.config.lower()

    need_join_archieves = args.libonlyname is None
    need_gen_wrappers = args.libonlyname is None

    # Find indigo version
    version = getIndigoVersion()
    with cwd(os.path.join(os.path.split(__file__)[0], '..', 'dist')):

        if need_join_archieves:
            flatten_directory(".")

        if need_join_archieves:
            for dest, pattern in arc_joins:
                p = pattern.replace("%ver%", version) + "\.zip"
                d = dest.replace("%ver%", version) + suffix
                join_archives_by_pattern(p, d)

        print("*** Making wrappers *** ")

        api_dir = os.path.abspath(os.path.join("..", "api"))
        libs_dir = os.path.join(api_dir, "libs")

        for w, libs in wrappers:
            clear_libs()
            if args.libonlyname and w != args.libonlyname:
                continue
            any_exists = True
            for lib in libs:
                name = "indigo-libs-%s-%s-shared%s" % (version, lib, suffix)
                if os.path.exists(name + ".zip"):
                    any_exists = any_exists and True
                    unpack_to_libs(name)
                else:
                    any_exists = any_exists and False
            if not any_exists:
                continue
            if need_gen_wrappers:
                for gen in wrappers_gen:
                    if args.type is not None:
                        for g in args.type.split(','):
                            if gen.find(g) != -1:
                                command = '"%s" "%s" -s "-%s"' % (sys.executable, os.path.join(api_dir, gen), w)
                                print(command)
                                subprocess.check_call(command, shell=True)
