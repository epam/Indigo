# This module assumes that you have installed all the
# libs files in the <source root>/dist directory

import os
import shutil
import sys
import re
import subprocess
import inspect
from zipfile import *
from os.path import *
from optparse import OptionParser

def make_doc():
    curdir = abspath(os.curdir)
    script_dir = os.path.dirname(os.path.abspath(inspect.getfile(inspect.currentframe())))
    root_dir = os.path.join(script_dir, "..")

    os.chdir(os.path.join(root_dir, 'api/python'))
    subprocess.check_call('"%s" copy-libs.py' % sys.executable, shell=True)
    os.chdir('../../doc')
    subprocess.check_call('"%s" builder.py' % sys.executable, shell=True)
    os.chdir(curdir)

def copy_doc(destname):
    shutil.copytree('../doc/build/html', join(curdir, destname, 'doc'))

def flatten_directory(dir):
    todelete = []
    for f in os.listdir(dir):
        if f.find("python") != -1 or f.find("java") != -1 or f.find("dotnet") != -1:
            continue
        dir2 = join(dir, f)
        if isdir(dir2):
            for f2 in os.listdir(dir2):
                f2full = join(dir2, f2)
                shutil.move(f2full, dir)
            todelete.append(dir2)
            os.rmdir(dir2)


def move_dir_content(src_dir, dest_dir):
    for f in os.listdir(src_dir):
        f2 = join(src_dir, f)
        destf2 = join(dest_dir, f)
        if isdir(destf2):
            move_dir_content(f2, destf2)
        elif not exists(destf2):
            shutil.move(f2, destf2)


def join_archives(names, destname):
    for name in names:
        if not exists(name + ".zip"):
            return
    for name in names:
        subprocess.check_call("unzip %s.zip -d %s" % (name, name), shell=True)
    os.mkdir(destname)
    for name in names:
        move_dir_content(name, destname)
    if exists(destname + ".zip"):
        os.remove(destname + ".zip")
    if args.doc:
        copy_doc(destname)
    subprocess.check_call("zip -r -9 -m %s.zip %s" % (destname, destname), shell=True)
    for name in names:
        shutil.rmtree(name)
        os.remove("%s.zip" % name)


def join_archives_by_pattern(pattern, destname):
    archives = []
    for f in os.listdir("."):
        if re.match(pattern, f):
            archives.append(splitext(f)[0])
    if len(archives) == 0:
        return
    print(archives)
    join_archives(archives, destname)


def clearLibs():
    for f in os.listdir(libs_dir):
        if f == "readme.txt":
            continue
        ffull = join(libs_dir, f)
        if isdir(ffull):
            shutil.rmtree(ffull)
        else:
            os.remove(ffull)


def unpackToLibs(name):
    if exists("tmp"):
        shutil.rmtree("tmp")
    subprocess.check_call("unzip %s.zip -d tmp" % (name), shell=True)
    move_dir_content(join("tmp", name), libs_dir)
    shutil.rmtree("tmp")


parser = OptionParser(description='Indigo libraries repacking')
parser.add_option('--libonlyname', help='extract only the library into api/lib')
parser.add_option('--config', default="Release", help='project configuration')
parser.add_option('--type', default='python,java,dotnet', help='wrapper (dotnet, java, python)')
parser.add_option('--doc', default=False, action='store_true', help='Build documentation')

(args, left_args) = parser.parse_args()

if args.doc:
    make_doc()

if len(left_args) > 0:
    print("Unexpected arguments: %s" % (str(left_args)))
    exit()

suffix = ""
if args.config.lower() != "release":
    suffix = "-" + args.config.lower()

need_join_archieves = (args.libonlyname == None)
need_gen_wrappers = (args.libonlyname == None)

# find indigo version
version = ""
cur_dir = split(__file__)[0]
for line in open(join(cur_dir, "..", "api", "indigo-version.cmake")):
    m = re.search('SET\(INDIGO_VERSION "(.*)"', line)
    if m:
        version = m.group(1)

os.chdir(join(cur_dir, "../dist"))
#dist = abspath(join("..", "dist"))
if need_join_archieves:
    flatten_directory(".")

arc_joins = [
    ("indigo-libs-%ver%-linux-shared", "indigo-libs-%ver%-linux.+-shared" ),
    ("indigo-libs-%ver%-win-shared", "indigo-libs-%ver%-win.+-shared" ),
    ("indigo-libs-%ver%-mac-shared", "indigo-libs-%ver%-mac.+-shared" ),
    ("indigo-libs-%ver%-linux-static", "indigo-libs-%ver%-linux.+-static" ),
    ("indigo-libs-%ver%-win-static", "indigo-libs-%ver%-win.+-static" ),
    ("indigo-libs-%ver%-mac-static", "indigo-libs-%ver%-mac.+-static" ),
]

if need_join_archieves:
    for dest, pattern in arc_joins:
        p = pattern.replace("%ver%", version) + "\.zip"
        d = dest.replace("%ver%", version) + suffix
        join_archives_by_pattern(p, d)

print("*** Making wrappers *** ")

api_dir = abspath("../api")
libs_dir = join(api_dir, "libs")

wrappers = [
    ("win", ["win"]),
    ("linux", ["linux"]),
    ("mac", ["mac"]),
    ("universal", ["win", "linux", "mac"]),
]

wrappers_gen = ["make-java-wrappers.py", "make-python-wrappers.py", 'make-dotnet-wrappers.py']

for w, libs in wrappers:
    clearLibs()
    if args.libonlyname and w != args.libonlyname:
        continue
    any_exists = True
    for lib in libs:
        name = "indigo-libs-%s-%s-shared%s" % (version, lib, suffix)
        if exists(name + ".zip"):
            any_exists = any_exists and True
            unpackToLibs(name)
        else:
            any_exists = any_exists and False
    if not any_exists:
        continue
    if need_gen_wrappers:
        for gen in wrappers_gen:
            if args.type is not None and gen not in args.type.split(','):
                continue
            subprocess.check_call('"%s" %s -s "-%s" %s' % (sys.executable, join(api_dir, gen), w, '--doc' if args.doc else ''), shell=True)
