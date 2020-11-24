import os
import re
import shutil
import subprocess


def getIndigoVersion():
    version = "unknown"
    indigo_version_h = os.path.join(os.path.dirname(__file__), "src", "indigo_version.h")
    if os.path.exists(indigo_version_h):
        for line in open(indigo_version_h):
            m = re.search('INDIGO_VERSION "(.*)-.*"', line)
            if m:
                version = m.group(1)
    elif 'INDIGO_VERSION' in os.environ:
        version = os.environ['INDIGO_VERSION']
    else:
        # If we are in repo
        if shutil.which('git') and os.path.exists(os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), '.git')) and os.name == 'posix':
            version = subprocess.check_output(r'git describe --tags --long --match "indigo-*" | sed -r "s/indigo-(.+)-(.+)-(.+)/\1\.\2/"', shell=True).decode('utf-8').strip()
    return version
