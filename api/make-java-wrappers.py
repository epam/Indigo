import os
import shutil

root = os.getcwd()

os.chdir(os.path.join(root, "dist"))
if os.path.exists("java"):
    shutil.rmtree("java")
    
os.chdir(os.path.join(root, "java"))
os.system("ant clean")
os.system("ant jar")

os.chdir(os.path.join(root, "renderer", "java"))
os.system("ant clean")
os.system("ant jar")

os.chdir(os.path.join(root, "plugins", "inchi", "java"))
os.system("ant clean")
os.system("ant jar")

os.chdir(os.path.join(root, "dist"))
shutil.copy(os.path.join(root, "LICENSE.GPL"), "java")

os.rename("java", "indigo-java-1.1-beta8")
os.system("zip -r -9 -m indigo-java-1.1-beta8.zip indigo-java-1.1-beta8")
