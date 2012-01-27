import os
import shutil
from os.path import *

cur_dir = split(__file__)[0]
lib_dir = join(cur_dir, "lib")
if exists(lib_dir):
    shutil.rmtree(lib_dir)
    
srclibs_dir = join(cur_dir, "..", "libs", "shared")
shutil.copytree(srclibs_dir, lib_dir)
