import os
import re


def getIndigoVersion():
    version = "unknown"

    for line in open(os.path.join(os.path.dirname(__file__), "src", "indigo_version.h")):
        m = re.search('INDIGO_VERSION "(.*)-.*"', line)
        if m:
            version = m.group(1)
    return version
