import os
import re


def getIndigoVersion():
    version = "unknown"
    indigo_version_h = os.path.join(os.path.dirname(__file__), "src", "indigo_version.h")
    if os.path.exists(indigo_version_h):
        for line in open():
            m = re.search('INDIGO_VERSION "(.*)-.*"', line)
            if m:
                version = m.group(1)
    elif 'INDIGO_VERSION' in os.environ:
        version = os.environ['INDIGO_VERSION']

    return version
