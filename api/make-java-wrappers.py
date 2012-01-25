import os
import shutil
from os.path import *

api_dir = abspath(dirname(__file__))
root = join(api_dir, "..")

dist_dir = join(root, "dist")
if not os.path.exists(dist_dir):
    os.mkdir(dist_dir)

os.chdir(dist_dir)
if os.path.exists("java"):
    shutil.rmtree("java")
    
os.chdir(os.path.join(api_dir, "java"))
os.system("ant clean")
os.system("ant jar")

os.chdir(os.path.join(api_dir, "renderer", "java"))
os.system("ant clean")
os.system("ant jar")

os.chdir(os.path.join(api_dir, "plugins", "inchi", "java"))
os.system("ant clean")
os.system("ant jar")

os.chdir(dist_dir)
shutil.copy(os.path.join(api_dir, "LICENSE.GPL"), "java")

os.rename("java", "indigo-java-1.1-beta8")
os.system("zip -r -9 -m indigo-java-1.1-beta8.zip indigo-java-1.1-beta8")
