import os
import platform
import re
import sys


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
    pom_path = os.path.join(REPO_ROOT, 'api', 'java', 'pom.xml')
    ElementTree.register_namespace("", "http://maven.apache.org/POM/4.0.0")
    tree = ElementTree.parse(pom_path)
    namespace = r'{http://maven.apache.org/POM/4.0.0}'
    for l1_child in tree.getroot():
        if l1_child.tag == "{}properties".format(namespace):
            for l2_child in l1_child:
                if l2_child.tag == "{}revision".format(namespace):
                    return l2_child.text
    raise ValueError('Could not find version in {}'.format(pom_path))
