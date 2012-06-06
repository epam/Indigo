import glob
import os
import shutil
import subprocess
from os.path import *
from zipfile import ZipFile
from optparse import OptionParser
import re

version = ""
cur_dir = split(__file__)[0]
for line in open(join(os.path.dirname(os.path.abspath(__file__)), "..", "api", "indigo-version.cmake")):
    m = re.search('SET\(INDIGO_VERSION "(.*)"', line)
    if m:
        version = m.group(1)

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
        distVersion = filename.replace("indigo-java-%s-" % version , '').replace('.zip', '')
        fullChemdiffName = "chemdiff-%s-%s" % (version, distVersion)
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
        os.chdir(join(dist_dir, "chemdiff", fullChemdiffName))
        os.mkdir("lib")
        for file in glob.glob("../../indigo-java/*.jar"):
            if not (file.endswith('indigo-inchi.jar')):
                shutil.copy(file, "lib")
        shutil.copy(join(root, "common/java/common-controls/dist/common-controls.jar"), "lib")
        os.chdir(dist_dir)
        shutil.make_archive(fullChemdiffName, "zip", "chemdiff")
        if filename.endswith('-win.zip'):
            os.chdir(join(dist_dir, "chemdiff", fullChemdiffName))
            shutil.copy(join(root, "utils", "chemdiff", "chemdiff_installer.nsi"), join(dist_dir, "chemdiff", fullChemdiffName, "chemdiff_installer.nsi"))
            shutil.copytree(join(root, "utils", "chemdiff", "tests"), join(dist_dir, "chemdiff", fullChemdiffName, "tests"))
            subprocess.check_call(["makensis", "/DVersion=%s" % version, "chemdiff_installer.nsi"], shell=True)
            shutil.copy("chemdiff-%s-installer.exe" % version, join(dist_dir, "chemdiff-%s-installer.exe" % version))
            os.chdir(dist_dir)
        shutil.rmtree("chemdiff")
		
# Legio
for filename in os.listdir(dist_dir):
    if filename.startswith("indigo-java-") :
        os.chdir(dist_dir)
        if os.path.exists("indigo-java"):
            shutil.rmtree("indigo-java")
        java_dir = join(dist_dir, "indigo-java")
        distVersion = filename.replace("indigo-java-%s-" % version , '').replace('.zip', '')
        fullChemdiffName = "legio-%s-%s" % (version, distVersion)
        uz = ZipFile(join(dist_dir, filename))
        uz.extractall(path=dist_dir)
        os.rename(join(dist_dir, filename)[:-4], "indigo-java")
        if os.path.exists(join(dist_dir, "legio")):
            shutil.rmtree(join(dist_dir, "legio"))
        os.mkdir(join(dist_dir, "legio"))
        os.mkdir(join(dist_dir, "legio", fullChemdiffName))
        os.chdir(join(root, "utils", "legio"))
        subprocess.check_call("ant clean", shell=True)
        subprocess.check_call("ant jar", shell=True)
        shutil.copy(join("dist", "legio.jar"), join(dist_dir, "legio", fullChemdiffName, "legio.jar"))
        if filename.endswith('-win.zip'):
            shutil.copy(join("launch.bat"), join(dist_dir, "legio", fullChemdiffName,"launch.bat"))
        else:
            shutil.copy(join("legio.sh"), join(dist_dir, "legio", fullChemdiffName,"legio.sh"))
        shutil.copy(join("LICENSE.GPL"), join(dist_dir, "legio", fullChemdiffName, "LICENSE.GPL"))
        os.chdir(join(dist_dir, "legio", fullChemdiffName))
        os.mkdir("lib")
        for file in glob.glob("../../indigo-java/*.jar"):
            if not (file.endswith('indigo-inchi.jar')):
                shutil.copy(file, "lib")
        shutil.copy(join(root, "common/java/common-controls/dist/common-controls.jar"), "lib")
        os.chdir(dist_dir)
        shutil.make_archive(fullChemdiffName, "zip", "legio")
        if filename.endswith('-win.zip'):
            os.chdir(join(dist_dir, "legio", fullChemdiffName))
            shutil.copy(join(root, "utils", "legio", "legio_installer.nsi"), join(dist_dir, "legio", fullChemdiffName, "legio_installer.nsi"))
            shutil.copytree(join(root, "utils", "legio", "tests"), join(dist_dir, "legio", fullChemdiffName, "tests"))
            subprocess.check_call(["makensis", "/DVersion=%s" % version, "legio_installer.nsi"], shell=True)
            shutil.copy("legio-%s-installer.exe" % version, join(dist_dir, "legio-%s-installer.exe" % version))
            os.chdir(dist_dir)
        shutil.rmtree("legio")
        shutil.rmtree("indigo-java")