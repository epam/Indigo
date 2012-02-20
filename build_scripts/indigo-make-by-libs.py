# This module assumes that you have installed all the 
# libs files in the <source root>/dist directory

import os
import shutil
import sys
import re
from zipfile import *
from os.path import *
from optparse import OptionParser

parser = OptionParser(description='Indigo libraries repacking')
parser.add_option('--libonlyname', help='extract only the library into api/lib')

(args, left_args) = parser.parse_args()
if len(left_args) > 0:
    print("Unexpected arguments: %s" % (str(left_args)))
    exit()

need_join_achieves = (args.libonlyname == None)
need_gen_wrappers = (args.libonlyname == None)
    
# find indigo version
version = ""
cur_dir = split(__file__)[0]
for line in open(join(cur_dir, "..", "api", "indigo-version.cmake")):
    m = re.search('SET\(INDIGO_VERSION "(.*)"', line)
    if m:
        version = m.group(1)

def flatten_directory (dir):
    todelete = []
    for f in os.listdir(dir):
        dir2 = join(dir, f)
        if isdir(dir2):
            for f2 in os.listdir(dir2):
                f2full = join(dir2, f2)
                shutil.move(f2full, dir)
            todelete.append(dir2)
            os.rmdir(dir2)

def move_dir_content (src_dir, dest_dir):
    for f in os.listdir(src_dir):
        f2 = join(src_dir, f)
        destf2 = join(dest_dir, f)
        if isdir(destf2):
            move_dir_content(f2, destf2)
        elif not exists(destf2):
            shutil.move(f2, destf2)
        
    
def join_archives (names, destname):
    for name in names:
        if not exists(name + ".zip"):
            return
    for name in names:
        os.system("unzip %s.zip -d %s" % (name, name))
    os.mkdir(destname)
    for name in names:
        move_dir_content(name, destname)
    if exists(destname + ".zip"):
        os.remove(destname + ".zip")
    os.system("zip -r -9 -m %s.zip %s" % (destname, destname))
    for name in names:
        shutil.rmtree(name)
        os.remove("%s.zip" % (name))
    
os.chdir(join(cur_dir, "../dist"))
#dist = abspath(join("..", "dist"))
if need_join_achieves:
    flatten_directory(".")

def join_archives_by_pattern (pattern, destname):
    archives = []
    for f in os.listdir("."):
        if re.match(pattern, f):
            archives.append(splitext(f)[0])
    if len(archives) == 0:
        return
    print(archives)
    join_archives(archives, destname)
    

arc_joins = [
    ("indigo-libs-%ver%-linux-shared", "indigo-libs-%ver%-linux.+-shared" ),
    ("indigo-libs-%ver%-win-shared", "indigo-libs-%ver%-win.+-shared" ),
    ("indigo-libs-%ver%-mac-shared", "indigo-libs-%ver%-mac.+-shared" ),
    ("indigo-libs-%ver%-linux-static", "indigo-libs-%ver%-linux.+-static" ),
    ("indigo-libs-%ver%-win-static", "indigo-libs-%ver%-win.+-static" ),
    ("indigo-libs-%ver%-mac-static", "indigo-libs-%ver%-mac.+-static" ),
]

if need_join_achieves:
    for dest, pattern in arc_joins:
        p = pattern.replace("%ver%", version) + "\.zip"
        d = dest.replace("%ver%", version)
        join_archives_by_pattern(p, d)

    
print("*** Making wrappers *** ")    

api_dir = abspath("../api")
libs_dir = join(api_dir, "libs")
def clearLibs ():
    for f in os.listdir(libs_dir):
        if f == "readme.txt":
            continue
        ffull = join(libs_dir, f)
        if isdir(ffull):
            shutil.rmtree(ffull)
        else:
            os.remove(ffull)

def unpackToLibs (name):
    if exists("tmp"):
        shutil.rmtree("tmp")
    os.system("unzip %s.zip -d tmp" % (name))
    move_dir_content(join("tmp", name), libs_dir)
    shutil.rmtree("tmp")

wrappers =  [ 
    ("win", ["win"]),
    ("linux", ["linux"]),
    ("mac", ["mac"]),
    ("universal", ["win", "linux", "mac"]),
]    

wrappers_gen = [ "make-java-wrappers.py", "make-python-wrappers.py", 'make-cs-wrappers.py']
for w, libs in wrappers:
    clearLibs()
    if args.libonlyname and w != args.libonlyname:
        continue
    any_exists = False
    for lib in libs:
        name = "indigo-libs-%s-%s-shared" % (version, lib)
        if exists(name + ".zip"):
            any_exists = True
            unpackToLibs(name)
    if not any_exists:
        continue
    if need_gen_wrappers:
        for gen in wrappers_gen:
            if not (w != 'win' and gen == 'make-cs-wrappers.py'):
                os.system('%s %s -s "-%s"' % (sys.executable, join(api_dir, gen), w))
