import os
import shutil
from os.path import *
import re
from optparse import OptionParser

def outputName(outputBaseName):
    #targetPlatform = args.suffix[1:]
    #if targetPlatform == 'win':
    suffix = '.dll'
    prefix = ''
    #elif targetPlatform == 'mac':
    #    suffix = '.dylib'
    #    prefix = 'lib'
    #elif targetPlatform == 'linux':
    #    suffix = '.so'
    #    prefix = 'lib'
    #else:
    #    raise ValueError('Unsupported system: %s' % os.name)
    return prefix + outputBaseName + suffix

def buildInPath(path, outputBaseName, resourceList, reference=None):
    os.chdir(path)
    if os.path.exists('dist'):
        shutil.rmtree('dist')
    os.mkdir('dist')
    execCommand = "csc /unsafe /target:library /optimize /out:%s/%s" % (join(dist_dir, 'cs'), outputName(outputBaseName))
    if reference:
        execCommand += ' /reference:%s' % reference
    for resource in resourceList:
        if resource != '':
            execCommand += ' /resource:%s' % resource
    execCommand += " *.cs"
    print execCommand    
    os.system(execCommand)
        
parser = OptionParser(description='Indigo C# libraries build script')
parser.add_option('--suffix', '-s', help='archive suffix', default="")

(args, left_args) = parser.parse_args()

# find indigo version
version = ""
cur_dir = split(__file__)[0]
for line in open(join(cur_dir, "indigo-version.cmake")):
    m = re.search('SET\(INDIGO_VERSION "(.*)"', line)
    if m:
        version = m.group(1)

api_dir = abspath(dirname(__file__))
root = join(api_dir, "..")
dist_dir = join(root, "dist")
if not os.path.exists(dist_dir):
    os.mkdir(dist_dir)

os.chdir(dist_dir)
if os.path.exists("cs"):
    shutil.rmtree("cs")
os.mkdir('cs')

if os.path.exists(join(api_dir, "cs", "Resource")):
    shutil.rmtree(join(api_dir, "cs", "Resource"), ignore_errors=True)
if os.path.exists(join(api_dir, "renderer", "cs", "Resource")):
    shutil.rmtree(join(api_dir, "renderer", "cs", "Resource"),  ignore_errors=True)
        
for file in os.listdir(join(api_dir, 'libs', 'shared')):
    shutil.copytree(join(api_dir, 'libs', 'shared', file), join(api_dir, "cs", "Resource", file))
    shutil.copytree(join(api_dir, 'libs', 'shared', file), join(api_dir, "renderer", "cs", "Resource", file))
    
buildInPath(os.path.join(api_dir, "cs"), 
            'indigo-cs',
            [join('Resource', 'Win', 'x64', 'indigo.dll')+','+join('Win', 'x64', 'indigo.dll'),
             join('Resource', 'Win', 'x86', 'indigo.dll')+','+join('Win', 'x86', 'indigo.dll')])
buildInPath(os.path.join(api_dir, "renderer", 'cs'), 
            'indigo-renderer-cs',
            [join('Resource', 'Win', 'x64', 'indigo-renderer.dll')+','+join('Win', 'x64', 'indigo-renderer.dll'),
             join('Resource', 'Win', 'x86', 'indigo-renderer.dll')+','+join('Win', 'x86', 'indigo-renderer.dll')],       
            os.path.join(dist_dir, "cs", outputName('indigo-cs')))
# TODO: Add Indigo C# InChI plugin

os.chdir(dist_dir)
shutil.copy(os.path.join(api_dir, "LICENSE.GPL"), "cs")

archive_name = "indigo-cs-%s" % (version + args.suffix)
os.rename("cs", archive_name)
os.system("zip -r -9 -m %s.zip %s" % (archive_name, archive_name))
