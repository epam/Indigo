import glob
import os
import shutil
import subprocess
from os.path import *
from zipfile import ZipFile
from optparse import OptionParser
import re


cur_dir = abspath(dirname(__file__))
root = os.path.normpath(join(cur_dir, ".."))
project_dir = join(cur_dir, "indigo-utils")

os.chdir(root)
if not os.path.exists("dist"):
    os.mkdir("dist")
dist_dir = join(root, "dist")

# Chemdiff
for filename in os.listdir(dist_dir):
    if filename.startswith("indigo-java-") :
        os.chdir(dist_dir)
        if os.path.exists("indigo-java"):
            shutil.rmtree("indigo-java")
        #os.mkdir("indigo-java")
        java_dir = join(dist_dir, "indigo-java")
        fullChemdiffName = "chemdiff-%s" % (filename.replace("indigo-java-%s-" % version , '').replace('.zip', ''))
        uz = ZipFile(join(dist_dir, filename))
        uz.extractall(path=dist_dir)
        os.rename(join(dist_dir, filename)[:-4], "indigo-java")
        if os.path.exists(join(dist_dir, "chemdiff")):
            shutil.rmtree(join(dist_dir, "chemdiff"))
        os.mkdir(join(dist_dir, "chemdiff"))
        os.mkdir(join(dist_dir, "chemdiff", fullChemdiffName))
        os.chdir(join(root, "utils", "chemdiff"))
        subprocess.check_call("ant clean", shell=True)
        subprocess.check_call("ant jar", shell=True)
        shutil.copy(join("dist", "chemdiff.jar"), join(dist_dir, "chemdiff", fullChemdiffName, "chemdiff.jar"))
        if filename.endswith('-win.zip'):
            shutil.copy(join("launch.bat"), join(dist_dir, "chemdiff", fullChemdiffName,"launch.bat"))
        else:
            shutil.copy(join("chemdiff.sh"), join(dist_dir, "chemdiff", fullChemdiffName,"chemdiff.sh"))
        shutil.copy(join("LICENSE.GPL"), join(dist_dir, "chemdiff", fullChemdiffName, "LICENSE.GPL"))
        shutil.copytree(join(root, "utils", "chemdiff", "examples"), join(dist_dir, "chemdiff", fullChemdiffName, "examples"))
        os.chdir(join(dist_dir, "chemdiff", fullChemdiffName))
        os.mkdir("lib")
        for file in glob.glob("../../indigo-java/*.jar"):
            if not (file.endswith('indigo-inchi.jar')):
                shutil.copy(file, "lib")
        shutil.copy(join(root, "common/java/common-controls/dist/common-controls.jar"), "lib")
        if os.name == "nt" and os.path.exists("chemdiff.sh"):
            with open("chemdiff.sh", "rt") as f:
                text = f.read()
            with open("chemdiff.sh", "wt") as f:
                f.write(text.replace("\r\n", "\n"))
        os.chdir(dist_dir)
        shutil.make_archive(fullChemdiffName, "zip", "chemdiff")
        if filename.endswith('-win.zip'):
            os.chdir(join(dist_dir, "chemdiff", fullChemdiffName))
            shutil.copy(join(root, "utils", "chemdiff", "chemdiff_installer.nsi"), join(dist_dir, "chemdiff", fullChemdiffName, "chemdiff_installer.nsi"))
            subprocess.check_call(["makensis", "/DVersion=%s" % version, "chemdiff_installer.nsi"], shell=True)
            shutil.copy("chemdiff-%s-installer.exe" % version, join(dist_dir, "chemdiff-%s-installer.exe" % version))
            os.chdir(dist_dir)
        shutil.rmtree("chemdiff")
        shutil.rmtree("indigo-java")

# Legio
for filename in os.listdir(dist_dir):
    if filename.startswith("indigo-java-") :
        os.chdir(dist_dir)
        if os.path.exists("indigo-java"):
            shutil.rmtree("indigo-java")
        java_dir = join(dist_dir, "indigo-java")
        fullLegioName = "legio-%s" % (filename.replace("indigo-java-%s-" % version , '').replace('.zip', ''))
        uz = ZipFile(join(dist_dir, filename))
        uz.extractall(path=dist_dir)
        os.rename(join(dist_dir, filename)[:-4], "indigo-java")
        if os.path.exists(join(dist_dir, "legio")):
            shutil.rmtree(join(dist_dir, "legio"))
        os.mkdir(join(dist_dir, "legio"))
        os.mkdir(join(dist_dir, "legio", fullLegioName))
        os.chdir(join(root, "utils", "legio"))
        subprocess.check_call("ant clean", shell=True)
        subprocess.check_call("ant jar", shell=True)
        shutil.copy(join("dist", "legio.jar"), join(dist_dir, "legio", fullLegioName, "legio.jar"))
        if filename.endswith('-win.zip'):
            shutil.copy(join("launch.bat"), join(dist_dir, "legio", fullLegioName,"launch.bat"))
        else:
            shutil.copy(join("legio.sh"), join(dist_dir, "legio", fullLegioName,"legio.sh"))
        shutil.copy(join("LICENSE.GPL"), join(dist_dir, "legio", fullLegioName, "LICENSE.GPL"))
        shutil.copytree(join(root, "utils", "legio", "examples"), join(dist_dir, "legio", fullLegioName, "examples"))
        os.chdir(join(dist_dir, "legio", fullLegioName))
        os.mkdir("lib")
        for file in glob.glob("../../indigo-java/*.jar"):
            if not (file.endswith('indigo-inchi.jar')):
                shutil.copy(file, "lib")
        shutil.copy(join(root, "common/java/common-controls/dist/common-controls.jar"), "lib")
        if os.name == "nt" and os.path.exists("legio.sh"):
            with open("legio.sh", "rb") as f:
                text = f.read()
            with open("legio.sh", "wb") as f:
                f.write(text.replace("\r\n", "\n"))
        os.chdir(dist_dir)
        shutil.make_archive(fullLegioName, "zip", "legio")
        if filename.endswith('-win.zip'):
            os.chdir(join(dist_dir, "legio", fullLegioName))
            shutil.copy(join(root, "utils", "legio", "legio_installer.nsi"), join(dist_dir, "legio", fullLegioName, "legio_installer.nsi"))
            subprocess.check_call(["makensis", "/DVersion=%s" % version, "legio_installer.nsi"], shell=True)
            shutil.copy("legio-%s-installer.exe" % version, join(dist_dir, "legio-%s-installer.exe" % version))
            os.chdir(dist_dir)
        shutil.rmtree("legio")
        shutil.rmtree("indigo-java")