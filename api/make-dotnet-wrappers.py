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

parser = OptionParser(description='Indigo .NET libraries build script')
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
if os.path.exists("dotnet"):
    shutil.rmtree("dotnet")
os.mkdir('dotnet')

# Build Indigo-dotnet
indigoDotNetPath = join(api_dir, "dotnet")
if os.path.exists(join(indigoDotNetPath, "Resource")):
    shutil.rmtree(join(indigoDotNetPath, "Resource"))
os.makedirs(join(indigoDotNetPath, "Resource", 'Win', 'x64'))
os.makedirs(join(indigoDotNetPath, "Resource", 'Win', 'x86'))

os.chdir(indigoDotNetPath)
command = '%s /property:DllPath32=%s /property:DllPath64=%s' % (msbuildcommand, join(api_dir, 'libs', 'shared', 'Win', 'x86'), join(api_dir, 'libs', 'shared', 'Win', 'x64'))
subprocess.check_call(command)


# Build IndigoRenderer-dotnet
indigoRendererDotNetPath = join(api_dir, "plugins", "renderer", "dotnet")
if os.path.exists(join(indigoRendererDotNetPath, "Resource")):
    shutil.rmtree(join(indigoRendererDotNetPath, "Resource"))
os.makedirs(join(indigoRendererDotNetPath, "Resource", 'Win', 'x64'))
os.makedirs(join(indigoRendererDotNetPath, "Resource", 'Win', 'x86'))

os.chdir(indigoRendererDotNetPath)
command = '%s /property:DllPath32=%s /property:DllPath64=%s' % (msbuildcommand, join(api_dir, 'libs', 'shared', 'Win', 'x86'), join(api_dir, 'libs', 'shared', 'Win', 'x64'))
subprocess.check_call(command)

# Build IndigoInchi-dotnet
indigoInchiDotNetPath = join(api_dir, "plugins", "inchi", "dotnet")
if os.path.exists(join(indigoInchiDotNetPath, "Resource")):
    shutil.rmtree(join(indigoInchiDotNetPath, "Resource"))
os.makedirs(join(indigoInchiDotNetPath, "Resource", 'Win', 'x64'))
os.makedirs(join(indigoInchiDotNetPath, "Resource", 'Win', 'x86'))

os.chdir(indigoInchiDotNetPath)
command = '%s /property:DllPath32=%s /property:DllPath64=%s' % (msbuildcommand, join(api_dir, 'libs', 'shared', 'Win', 'x86'), join(api_dir, 'libs', 'shared', 'Win', 'x64'))
subprocess.check_call(command)

os.chdir(dist_dir)
shutil.copy(os.path.join(api_dir, "LICENSE.GPL"), "dotnet")
shutil.copy(join(indigoDotNetPath, 'bin', 'Release', 'indigo-dotnet.dll'), "dotnet")
shutil.copy(join(indigoRendererDotNetPath, 'bin', 'Release', 'indigo-renderer-dotnet.dll'), "dotnet")
shutil.copy(join(indigoInchiDotNetPath, 'bin', 'Release', 'indigo-inchi-dotnet.dll'), "dotnet")

archive_name = "indigo-dotnet-%s" % (version + args.suffix)
os.rename("dotnet", archive_name)
os.system("zip -r -9 -m %s.zip %s" % (archive_name, archive_name))