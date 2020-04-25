import os
import re
import sys
import xml.etree.cElementTree as ElementTree

INDIGO_PATH = os.path.normpath(os.path.join(os.path.abspath(os.path.dirname(__file__)), os.path.pardir))


def update_pom_version(pomFile, newVersion):
    tree = ElementTree.parse(pomFile)
    ElementTree.register_namespace('', 'http://maven.apache.org/POM/4.0.0')
    root = tree.getroot()
    version_changed = False
    for child in root:
        if child.tag.endswith('version'):
            old_version = child.text
            if old_version != newVersion:
                print('Updating Indigo version from {0} to {1} in {2}...'.format(old_version, newVersion, pomFile))
                child.text = newVersion
                version_changed = True
            else:
                print('Indigo version in {0} remains {1}...'.format(pomFile, newVersion))
            break
    if version_changed:
        tree.write(pomFile)


def update_csproj_version(csproj_file, new_version):
    tree = ElementTree.parse(csproj_file)
    root = tree.getroot()
    version_changed = False
    for child in root:
        if child.tag == 'PropertyGroup':
            for cc in child:
                if cc.tag == 'Version':
                    old_version = cc.text
                    if old_version != new_version:
                        print('Updating Indigo version from {0} to {1} in {2}...'.format(old_version, new_version,
                                                                                         csproj_file))
                        cc.text = new_version
                        version_changed = True
                    else:
                        print('Indigo version in {0} remains {1}...'.format(csproj_file, new_version))
                    break
    if version_changed:
        tree.write(csproj_file)


def update_setup_py_version(setup_py_file, new_version):
    with open(setup_py_file) as f:
        data = f.read()
    vr = re.compile("version='(.+)'")
    m = vr.search(data)
    old_version = m.group(1)
    if old_version != new_version:
        data = vr.subn("version='{}'".format(new_version), data)[0]
        print('Updating Indigo version from {0} to {1} in {2}...'.format(old_version, new_version, setup_py_file))
        with open(setup_py_file, 'w') as f:
            f.write(data)
    else:
        print('Indigo version in {0} remains {1}...'.format(setup_py_file, new_version))


def main():
    sys.path.append(os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), 'api'))
    new_version = __import__('get_indigo_version').getIndigoVersion()
    update_pom_version(os.path.join(INDIGO_PATH, 'api', 'java', 'pom.xml'), new_version)
    update_pom_version(os.path.join(INDIGO_PATH, 'api', 'plugins', 'bingo', 'java', 'pom.xml'), new_version)
    update_pom_version(os.path.join(INDIGO_PATH, 'api', 'plugins', 'inchi', 'java', 'pom.xml'), new_version)
    update_pom_version(os.path.join(INDIGO_PATH, 'api', 'plugins', 'renderer', 'java', 'pom.xml'), new_version)
    update_csproj_version(os.path.join(INDIGO_PATH, 'api', 'dotnet', 'Indigo.Net.csproj'), new_version)
    update_setup_py_version(os.path.join(INDIGO_PATH, 'api', 'python', 'setup.py'), new_version)


if __name__ == '__main__':
    main()
