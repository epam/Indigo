import re
import os
import sys
import xml.etree.cElementTree as ElementTree

INDIGO_PATH = os.path.normpath(os.path.join(os.path.abspath(os.path.dirname(__file__)), os.path.pardir))


def updatePomVersion(pomFile, newVersion):
    tree = ElementTree.parse(pomFile)
    ElementTree.register_namespace('', 'http://maven.apache.org/POM/4.0.0')
    root = tree.getroot()
    versionChanged = False
    for child in root:
        if child.tag.endswith('version'):
            oldVersion = child.text
            if oldVersion != newVersion:
                print('Updating Indigo version from {0} to {1} in {2}...'.format(oldVersion, newVersion, pomFile))
                child.text = newVersion
                versionChanged = True
            else:
                print('Indigo version in {0} remains {1}...'.format(pomFile, newVersion))
            break
    if versionChanged:
        tree.write(pomFile)


def updateCsProjVersion(csProjFile, newVersion):
    tree = ElementTree.parse(csProjFile)
    root = tree.getroot()
    versionChanged = False
    for child in root:
        if child.tag == 'PropertyGroup':
            for cc in child:
                if cc.tag == 'Version':
                    oldVersion = cc.text
                    if oldVersion != newVersion:
                        print('Updating Indigo version from {0} to {1} in {2}...'.format(oldVersion, newVersion, csProjFile))
                        cc.text = newVersion
                        versionChanged = True
                    else:
                        print('Indigo version in {0} remains {1}...'.format(csProjFile, newVersion))
                    break
    if versionChanged:
        tree.write(csProjFile)


def updateSetupPyVersion(setupPyFile, newVersion):
    with open(setupPyFile) as f:
        data = f.read()
    vr = re.compile("version='(.+)'")
    m = vr.search(data)
    oldVersion = m.group(1)
    if oldVersion != newVersion:
        data = vr.subn("version='{}'".format(newVersion), data)[0]
        print('Updating Indigo version from {0} to {1} in {2}...'.format(oldVersion, newVersion, setupPyFile))
        with open(setupPyFile, 'w') as f:
            f.write(data)
    else:
        print('Indigo version in {0} remains {1}...'.format(setupPyFile, newVersion))


def main(newVersion):
    indigoVersion = newVersion
    updatePomVersion(os.path.join(INDIGO_PATH, 'api', 'java', 'pom.xml'), indigoVersion)
    updatePomVersion(os.path.join(INDIGO_PATH, 'api', 'plugins', 'bingo', 'java', 'pom.xml'), indigoVersion)
    updatePomVersion(os.path.join(INDIGO_PATH, 'api', 'plugins', 'inchi', 'java', 'pom.xml'), indigoVersion)
    updatePomVersion(os.path.join(INDIGO_PATH, 'api', 'plugins', 'renderer', 'java', 'pom.xml'), indigoVersion)
    updateCsProjVersion(os.path.join(INDIGO_PATH, 'api', 'dotnet', 'Indigo.Net.csproj'), indigoVersion)
    updateSetupPyVersion(os.path.join(INDIGO_PATH, 'api', 'python', 'setup.py'), indigoVersion)


if __name__ == '__main__':
    if len(sys.argv) > 1:
        version = sys.argv[1]
    else:
        sys.path.append(os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), 'api'))
        from get_indigo_version import getIndigoVersion
        version = getIndigoVersion()
    main(version)
