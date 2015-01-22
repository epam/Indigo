import os
import re
import sys
import xml.etree.cElementTree as ElementTree


INDIGO_PATH = os.path.normpath(os.path.join(os.path.abspath(os.path.dirname(__file__)), os.path.pardir))


def getIndigoCMakeVersion():
    with open(os.path.join(INDIGO_PATH, 'api', 'indigo-version.cmake'), 'r') as f:
        m = re.match('SET\(INDIGO_VERSION \"(.*)\"\).*', f.read())
        return m.group(1)


def setIndigoCMakeVersion(newVersion):
    cmakeFile = os.path.join(INDIGO_PATH, 'api', 'indigo-version.cmake')
    with open(cmakeFile, 'r') as f:
        cmakeText = f.read()
        m = re.match('SET\(INDIGO_VERSION \"(.*)\"\).*', cmakeText)
        result = re.sub('SET\(INDIGO_VERSION \"(.*)\"\).*', 'SET(INDIGO_VERSION "{0}")'.format(newVersion), cmakeText)
    print('Updating Indigo version from {0} to {1} in {2}...'.format(m.group(1), newVersion, cmakeFile))
    with open(cmakeFile, 'w') as f:
        f.write(result)

def updatePomVersion(pomFile, newVersion):
    tree = ElementTree.parse(pomFile)
    ElementTree.register_namespace('', 'http://maven.apache.org/POM/4.0.0')
    root = tree.getroot()
    for child in root:
        if child.tag.endswith('version'):
            print('Updating Indigo version from {0} to {1} in {2}...'.format(child.text, newVersion, pomFile))
            child.text = newVersion
            break
    tree.write(pomFile)


def main(newVersion=None):
    indigoVersion = newVersion if newVersion else getIndigoCMakeVersion()
    updatePomVersion(os.path.join(INDIGO_PATH, 'api', 'java', 'pom.xml'), indigoVersion)
    updatePomVersion(os.path.join(INDIGO_PATH, 'api', 'plugins', 'bingo', 'java', 'pom.xml'), indigoVersion)
    updatePomVersion(os.path.join(INDIGO_PATH, 'api', 'plugins', 'inchi', 'java', 'pom.xml'), indigoVersion)
    updatePomVersion(os.path.join(INDIGO_PATH, 'api', 'plugins', 'renderer', 'java', 'pom.xml'), indigoVersion)
    if newVersion:
        setIndigoCMakeVersion(newVersion)

if __name__ == '__main__':
    main(sys.argv[1]) if len(sys.argv) == 2 else main()
