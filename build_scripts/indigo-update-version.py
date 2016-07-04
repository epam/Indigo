import os
import re
import sys
import xml.etree.cElementTree as ElementTree


INDIGO_PATH = os.path.normpath(os.path.join(os.path.abspath(os.path.dirname(__file__)), os.path.pardir))


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


def main(newVersion):
    indigoVersion = newVersion
    updatePomVersion(os.path.join(INDIGO_PATH, 'api', 'java', 'pom.xml'), indigoVersion)
    updatePomVersion(os.path.join(INDIGO_PATH, 'api', 'plugins', 'bingo', 'java', 'pom.xml'), indigoVersion)
    updatePomVersion(os.path.join(INDIGO_PATH, 'api', 'plugins', 'inchi', 'java', 'pom.xml'), indigoVersion)
    updatePomVersion(os.path.join(INDIGO_PATH, 'api', 'plugins', 'renderer', 'java', 'pom.xml'), indigoVersion)


if __name__ == '__main__':
    main(sys.argv[1])