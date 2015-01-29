import os
import sys
import shutil
import subprocess

cur_dir = os.path.dirname(os.path.abspath(__file__))
root = os.path.join(cur_dir, "..")
api_dir = os.path.join(root, "api")
r_dir = os.path.join(api_dir, "r")
r_src_dir = os.path.join(r_dir, "src")

rellibs = __import__("indigo-release-libs")
indigo_pack_array = rellibs.build_libs(sys.argv[1:])

os.chdir(r_src_dir)
if not os.path.exists("dist"):
   os.mkdir("dist")

r_dist_dir = os.path.join(r_src_dir, "dist")

for pack in indigo_pack_array:
   if pack.find("static") >= 0:
      shutil.copy(pack, r_dist_dir)
 
shutil.copy(os.path.join(api_dir, "indigo.h"), r_src_dir)
shutil.copy(os.path.join(root, "common", "cmake", "linkhack.py"), r_src_dir)
os.chmod(os.path.join(r_src_dir, "linkhack.py"), 0777)

os.chdir(r_dir)
if not os.path.exists("package"):
   os.mkdir("package")
os.chdir(os.path.join(r_dir, "package"))

subprocess.check_call("R CMD build %s" % (r_dir), shell=True)