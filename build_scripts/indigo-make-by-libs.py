import os
import re
import runpy
import shutil
import subprocess
import sys
import zipfile
from contextlib import contextmanager
from optparse import OptionParser


def flatten_directory(dir_):
    to_delete = []
    for f in os.listdir(dir_):
        if f.find("python") != -1 or f.find("java") != -1 or f.find("dotnet") != -1:
            continue
        dir2 = os.path.join(dir_, f)
        if os.path.isdir(dir2):
            for f2 in os.listdir(dir2):
                f2full = os.path.join(dir2, f2)
                shutil.move(f2full, dir_)
            to_delete.append(dir2)
            os.rmdir(dir2)


def move_dir_content(src_dir, dest_dir):
    for f in os.listdir(src_dir):
        f2 = os.path.join(src_dir, f)
        destf2 = os.path.join(dest_dir, f)
        if os.path.isdir(destf2):
            move_dir_content(f2, destf2)
        elif not os.path.exists(destf2):
            shutil.move(f2, destf2)


def copytree(src, dst, symlinks=False, ignore=None):
    for item in os.listdir(src):
        if not os.path.exists(dst):
            os.makedirs(dst)
        s = os.path.join(src, item)
        d = os.path.join(dst, item)
        if os.path.isdir(s):
            copytree(s, d, symlinks, ignore)
        else:
            shutil.copy(s, d)


def join_archives(names, dest_name):
    if os.path.exists(dest_name):
        shutil.rmtree(dest_name)
    dest_zf_name = '{}.zip'.format(dest_name)
    if os.path.exists(dest_zf_name):
        os.remove(dest_zf_name)
    for name in names:
        zf_name = '{}.zip'.format(name)
        if not os.path.exists(zf_name):
            raise ValueError('Archive file {} does not exist!'.format(zf_name))
        if os.path.exists(name):
            shutil.rmtree(name)
        os.makedirs(name)
        with zipfile.ZipFile(zf_name) as zf:
            zf.extractall(name)
        copytree(name, dest_name)
        shutil.rmtree(name)
        # os.remove(zf_name)
    shutil.make_archive(dest_name, 'zip', os.path.dirname(dest_name), dest_name)
    shutil.rmtree(dest_name)


def join_archives_by_pattern(pattern, dest_name):
    archives = []
    for f in os.listdir(os.curdir):
        if re.match(pattern, f):
            archives.append(os.path.splitext(f)[0])
    if len(archives) == 0:
        return
    join_archives(archives, dest_name)


def clear_libs(libs_dir):
    for f in os.listdir(libs_dir):
        if f == "readme.txt":
            continue
        ffull = os.path.join(libs_dir, f)
        if os.path.isdir(ffull):
            shutil.rmtree(ffull)
        else:
            os.remove(ffull)


def unpack_to_libs(name, libs_dir):
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


def main():
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

    runpy.run_path(os.path.join(os.path.dirname(__file__), 'indigo-update-version.py'), run_name="__main__")

    parser = OptionParser(description='Indigo libraries repacking')
    parser.add_option('--libonlyname', help='extract only the library into api/lib')
    parser.add_option('--config', default="Release", help='project configuration')
    parser.add_option('--type', default='python,java,dotnet', help='wrapper (dotnet, java, python)')
    parser.add_option('--wrappers-arch', default='win,linux,mac,universal',
                      help='wrappers arch (win, linux, mac, universal')

    (args, left_args) = parser.parse_args()

    required_wrappers_arches = args.wrappers_arch.split(',')
    if not args.type:
        args.type = 'python,java,dotnet'

    if len(left_args) > 0:
        print("Unexpected arguments: %s" % (str(left_args)))
        exit()

    suffix = ""
    if args.config.lower() != "release":
        suffix = "-" + args.config.lower()

    need_join_archives = args.libonlyname is None
    need_gen_wrappers = args.libonlyname is None

    # Find Indigo version
    sys.path.append(os.path.join(os.path.dirname(__file__), os.pardir, "api"))
    get_indigo_version = __import__('get_indigo_version').getIndigoVersion
    version = get_indigo_version()
    with cwd(os.path.join(os.path.split(__file__)[0], '..', 'dist')):

        if need_join_archives:
            flatten_directory(".")

        if need_join_archives:
            for dest, pattern in arc_joins:
                p = pattern.replace("%ver%", version) + ".zip"
                d = dest.replace("%ver%", version) + suffix
                join_archives_by_pattern(p, d)

        print("*** Making wrappers *** ")

        api_dir = os.path.abspath(os.path.join("..", "api"))
        libs_dir = os.path.join(api_dir, "libs")

        for w, libs in wrappers:
            if not w in required_wrappers_arches:
                continue
            clear_libs(libs_dir)
            if args.libonlyname and w != args.libonlyname:
                continue
            any_exists = True
            for lib in libs:
                name = "indigo-libs-%s-%s-shared%s" % (version, lib, suffix)
                if os.path.exists(name + ".zip"):
                    any_exists = any_exists and True
                    unpack_to_libs(name, libs_dir)
                else:
                    any_exists = any_exists and False
            if not any_exists:
                continue
            if need_gen_wrappers:
                for gen in wrappers_gen:
                    if args.type is not None:
                        for g in args.type.split(','):
                            if gen.find(g) != -1:
                                command = '"%s" "%s" -s "%s"' % (sys.executable, os.path.join(api_dir, gen), w)
                                print(command)
                                subprocess.check_call(command, shell=True)


if __name__ == '__main__':
    main()
