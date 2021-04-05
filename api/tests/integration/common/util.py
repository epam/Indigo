import os
import platform
import re
import sys


REPO_ROOT = os.path.normpath(os.path.join(os.path.abspath(os.path.dirname(__file__)), '..', '..', '..', '..'))


def isIronPython():
    try:
        import clr
        return True
    except:
        return False


def isJython():
    try:
        import java
        return True
    except:
        return False


def getIndigoVersion():
    version = ""
    cur_dir = os.path.split(__file__)[0]
    if not os.path.exists(os.path.join(cur_dir, "../../../indigo/api/indigo-version.cmake")):
        return version
    for line in open(os.path.join(cur_dir, "../../../indigo/api/indigo-version.cmake")):
        m = re.search('SET\(INDIGO_VERSION "(.*)"', line)
        if m:
            version = m.group(1)
    return version


def getCpuCount():
    if os.name == 'java':
        from java.lang import Runtime
        runtime = Runtime.getRuntime()
        cpu_count = runtime.availableProcessors()
    else:
        import multiprocessing
        cpu_count = multiprocessing.cpu_count()
    return cpu_count


def getPlatform():
    system_name = None
    if isJython():
        import java.lang.System
        osname = java.lang.System.getProperty('os.name')
        if osname.startswith('Windows'):
            system_name = 'win'
        elif osname == 'Mac OS X':
            system_name = 'mac'
        elif osname == 'Linux':
            system_name = 'linux'
        else:
            raise EnvironmentError('Unsupported operating system %s' % osname)
    else:
        if os.name == 'nt':
            system_name = 'win'
        elif os.name == 'posix':
            if platform.mac_ver()[0]:
                system_name = 'mac'
            else:
                system_name = 'linux'
        else:
            raise EnvironmentError('Unsupported operating system %s' % os.name)
    return system_name


def get_indigo_java_version():
    import xml.etree.cElementTree as ElementTree
    pom_path = os.path.join(REPO_ROOT, 'api', 'java', 'pom.xml')
    ElementTree.register_namespace("", "http://maven.apache.org/POM/4.0.0")
    tree = ElementTree.parse(pom_path)
    namespace = r'{http://maven.apache.org/POM/4.0.0}'
    indigo_version = None
    jna_version = None
    for l1_child in tree.getroot():
        if l1_child.tag == "{}properties".format(namespace):
            for l2_child in l1_child:
                if l2_child.tag == "{}revision".format(namespace):
                    indigo_version = l2_child.text
        if l1_child.tag == "{}dependencies".format(namespace):
            for l2_child in l1_child:
                if l2_child.tag == "{}dependency".format(namespace):
                    jna_found = False
                    for l3_child in l2_child:
                        if l3_child.tag == "{}artifactId".format(namespace) and l3_child.text == 'jna':
                            jna_found = True
                            break
                    if jna_found:
                        for l3_child in l2_child:
                            if l3_child.tag == "{}version".format(namespace):
                                jna_version = l3_child.text
    if not indigo_version:
        raise ValueError('Could not find Indigo version in {}'.format(pom_path))
    if not jna_version:
        raise ValueError('Could not find JNA version in {}'.format(pom_path))
    return indigo_version, jna_version


def file_sha1(path):
    import hashlib
    with open(path, 'rb') as f:
        return hashlib.sha1(f.read()).hexdigest()


def download_jna(jna_version, path):
    import urllib

    output_path = os.path.join(path, 'jna-{}.jar'.format(jna_version))
    jna_url = "https://search.maven.org/remotecontent?filepath=net/java/dev/jna/jna/{0}/jna-{0}.jar".format(jna_version)
    need_to_download = False

    if os.path.exists(output_path):
        jna_sha1_url = "{}.sha1".format(jna_url)
        jna_sha1 = urllib.urlopen(jna_sha1_url).read()
        if jna_sha1_url != file_sha1(output_path):
            need_to_download = True

    if not need_to_download:
        return
    try:
        with open(output_path, 'wb') as of:
            f = urllib.urlopen(jna_url)
            of.write(f.read())
            f.close()
    except Exception as e:
        os.remove(output_path)
        raise e
