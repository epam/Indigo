import os
import platform
import re
import sys


def isIronPython():
    return sys.platform == 'cli'


def isJython():
    return os.name == 'java'


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
