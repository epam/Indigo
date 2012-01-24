import os
import shutil

root = os.getcwd()

version = "1.1-beta8"
archive_name = "indigo-python-" + version

if not os.path.exists("dist"):
    os.mkdir("dist")
dest = os.path.join(root, "dist", archive_name)
if os.path.exists(dest):
    shutil.rmtree(dest)
os.mkdir(dest)
shutil.copy(os.path.join(root, "python", "indigo.py"), dest)
shutil.copy(os.path.join(root, "renderer", "python", "indigo_renderer.py"), dest)
shutil.copy(os.path.join(root, "plugins", "inchi", "python", "indigo_inchi.py"), dest)
shutil.copytree(os.path.join(root, "libs", "shared"), os.path.join(dest, "lib"), ignore = shutil.ignore_patterns("*.lib"))

shutil.copy(os.path.join(root, "LICENSE.GPL"), dest)

os.chdir("dist")
if os.path.exists(archive_name + ".zip"):
    os.remove(archive_name + ".zip")
os.system("zip -r -9 -m %s.zip %s" % (archive_name, archive_name))
