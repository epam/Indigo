import os
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


def updateCsProjVersion(csProjFile, newVersion):
    tree = ElementTree.parse(csProjFile)
    root = tree.getroot()
    for child in root:
        print(child.tag)
        if child.tag == 'PropertyGroup':
            for cc in child:
                if cc.tag == 'Version':
                    print('Updating Indigo version from {0} to {1} in {2}...'.format(cc.text, newVersion, csProjFile))
                    cc.text = newVersion
                    break
    tree.write(csProjFile)


def main(newVersion):
    indigoVersion = newVersion
    updatePomVersion(os.path.join(INDIGO_PATH, 'api', 'java', 'pom.xml'), indigoVersion)
    updatePomVersion(os.path.join(INDIGO_PATH, 'api', 'plugins', 'bingo', 'java', 'pom.xml'), indigoVersion)
    updatePomVersion(os.path.join(INDIGO_PATH, 'api', 'plugins', 'inchi', 'java', 'pom.xml'), indigoVersion)
    updatePomVersion(os.path.join(INDIGO_PATH, 'api', 'plugins', 'renderer', 'java', 'pom.xml'), indigoVersion)
    updateCsProjVersion(os.path.join(INDIGO_PATH, 'api', 'dotnet', 'Indigo.Net.csproj'), indigoVersion)
    updateCsProjVersion(os.path.join(INDIGO_PATH, 'api', 'plugins', 'bingo', 'dotnet', 'Bingo.Net.csproj'), indigoVersion)
    updateCsProjVersion(os.path.join(INDIGO_PATH, 'api', 'plugins', 'inchi', 'dotnet', 'IndigoInchi.Net.csproj'), indigoVersion)
    updateCsProjVersion(os.path.join(INDIGO_PATH, 'api', 'plugins', 'renderer', 'dotnet', 'IndigoRenderer.Net.csproj'), indigoVersion)


if __name__ == '__main__':
    if len(sys.argv) > 1:
        version = sys.argv[1]
    else:
        sys.path.append(os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), 'api'))
        from get_indigo_version import getIndigoVersion
        version = getIndigoVersion()
    main(version)
