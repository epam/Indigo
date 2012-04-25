from optparse import OptionParser
import os
from os.path import join, abspath, dirname
import re
import shutil
import subprocess

if os.name == 'nt':
    msbuildcommand = 'msbuild /t:Rebuild /p:Configuration=Release'
else:
    # Mono
    msbuildcommand = 'xbuild /t:Rebuild /p:Configuration=Release'

# TODO: Support Mono

parser = OptionParser(description='Indigo C# libraries build script')
parser.add_option('--suffix', '-s', help='archive suffix', default="")

(args, left_args) = parser.parse_args()

api_dir = abspath(dirname(__file__))
root = join(api_dir, "..")
dist_dir = join(root, "dist")
if not os.path.exists(dist_dir):
    os.mkdir(dist_dir)

version = ""
cur_dir = os.path.abspath(os.curdir)
for line in open(join(api_dir, "indigo-version.cmake")):
    m = re.search('SET\(INDIGO_VERSION "(.*)"', line)
    if m:
        version = m.group(1)

os.chdir(dist_dir)
if os.path.exists("cs"):
    shutil.rmtree("cs")
os.mkdir('cs')

# Builld Indigo-cs
indigoCsPath = join(api_dir, "cs")
if os.path.exists(join(indigoCsPath, "Resource")):
    shutil.rmtree(join(indigoCsPath, "Resource"))
os.makedirs(join(indigoCsPath, "Resource", 'Win', 'x64'))
os.makedirs(join(indigoCsPath, "Resource", 'Win', 'x86'))

os.chdir(indigoCsPath)
command = '%s /property:DllPath32=%s /property:DllPath64=%s' % (msbuildcommand, join(api_dir, 'libs', 'shared', 'Win', 'x86'), join(api_dir, 'libs', 'shared', 'Win', 'x64'))
subprocess.check_call(command)


# Build IndigoRendere-cs
indigoRendererCsPath = join(api_dir, "plugins", "renderer", "cs")
if os.path.exists(join(indigoRendererCsPath, "Resource")):
    shutil.rmtree(join(indigoRendererCsPath, "Resource"))
os.makedirs(join(indigoRendererCsPath, "Resource", 'Win', 'x64'))
os.makedirs(join(indigoRendererCsPath, "Resource", 'Win', 'x86'))

os.chdir(indigoRendererCsPath)
command = '%s /property:DllPath32=%s /property:DllPath64=%s' % (msbuildcommand, join(api_dir, 'libs', 'shared', 'Win', 'x86'), join(api_dir, 'libs', 'shared', 'Win', 'x64'))
subprocess.check_call(command)

# Build IndigoInchi-cs
indigoInchiCsPath = join(api_dir, "plugins", "inchi", "cs")
if os.path.exists(join(indigoInchiCsPath, "Resource")):
    shutil.rmtree(join(indigoInchiCsPath, "Resource"))
os.makedirs(join(indigoInchiCsPath, "Resource", 'Win', 'x64'))
os.makedirs(join(indigoInchiCsPath, "Resource", 'Win', 'x86'))

os.chdir(indigoInchiCsPath)
command = '%s /property:DllPath32=%s /property:DllPath64=%s' % (msbuildcommand, join(api_dir, 'libs', 'shared', 'Win', 'x86'), join(api_dir, 'libs', 'shared', 'Win', 'x64'))
subprocess.check_call(command)

os.chdir(dist_dir)
shutil.copy(os.path.join(api_dir, "LICENSE.GPL"), "cs")
shutil.copy(join(indigoCsPath, 'bin', 'Release', 'indigo-cs.dll'), "cs")
shutil.copy(join(indigoRendererCsPath, 'bin', 'Release', 'indigo-renderer-cs.dll'), "cs")
shutil.copy(join(indigoInchiCsPath, 'bin', 'Release', 'indigo-inchi-cs.dll'), "cs")

archive_name = "indigo-cs-%s" % (version + args.suffix)
os.rename("cs", archive_name)
os.system("zip -r -9 -m %s.zip %s" % (archive_name, archive_name))