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


def get_indigo_version_tuple_from_git():
    version_raw = subprocess.check_output(r'git describe --tags --long --match "indigo-*" | sed -r "s/indigo-(.+)-(.+)-(.+)/\1\\n\2\\n\3/"', shell=True).decode('utf-8').strip()
    version_splitted = version_raw.split('\n')

    version = version_splitted[0]
    version_suffix = None
    commits = None
    revision = None
    if len(version_splitted) > 1:
        commits = version_splitted[1]
        revision = version_splitted[2]

    if commits == '0':
        commits = None

    if '-' in version:
        version, version_suffix = version.split('-')

    return version, version_suffix, commits, revision
