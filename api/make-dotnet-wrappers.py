import fnmatch
from optparse import OptionParser
import os
from os.path import join, abspath, dirname
import shutil
import subprocess
from xml.etree import cElementTree as ElementTree

from get_indigo_version import getIndigoVersion


class XmlListConfig(list):
    def __init__(self, aList):
        for element in aList:
            if element:
                # treat like dict
                if len(element) == 1 or element[0].tag != element[1].tag:
                    self.append(XmlDictConfig(element))
                # treat like list
                elif element[0].tag == element[1].tag:
                    self.append(XmlListConfig(element))
            elif element.text:
                text = element.text.strip()
                if text:
                    self.append(text)


class XmlDictConfig(dict):
    def __init__(self, parent_element):
        if parent_element.items():
            self.update(dict(parent_element.items()))
        for element in parent_element:
            if element:
                # treat like dict - we assume that if the first two tags
                # in a series are different, then they are all different.
                if len(element) == 1 or element[0].tag != element[1].tag:
                    aDict = XmlDictConfig(element)
                # treat like list - we assume that if the first two tags
                # in a series are the same, then the rest are the same.
                else:
                    # here, we put the list in dictionary; the key is the
                    # tag name the list elements all share in common, and
                    # the value is the list itself
                    aDict = {element[0].tag: XmlListConfig(element)}
                # if the tag has attributes, add those to the dict
                if element.items():
                    aDict.update(dict(element.items()))
                self.update({element.tag: aDict})
            # this assumes that if you've got an attribute in a tag,
            # you won't be having any text. This may or may not be a
            # good idea -- time will tell. It works for the way we are
            # currently doing XML configuration files...
            elif element.items():
                self.update({element.tag: dict(element.items())})
            # finally, if there are no child tags and no attributes, extract
            # the text
            else:
                self.update({element.tag: element.text})


def xml_to_dict(path):
    return XmlDictConfig(ElementTree.parse(path).getroot())


def copy_libs(native_library_path, target_basepath, wrappers):
    print('Cleaning up previous native libraries in ' + target_basepath)
    for f in os.listdir(target_basepath):
        if os.path.splitext(f)[1] in ('.dll', '.so', '.dylib'):
            print('Removing ' + os.path.join(target_basepath, f))
            os.remove(os.path.join(target_basepath, f))
    print('Cleaning up finished')

    if 'win' in wrappers:
        for f in os.listdir(os.path.join(native_library_path, 'Win', "x64")):
            if f.endswith('.dll'):
                shutil.copy(os.path.join(native_library_path, 'Win', "x64", f), target_basepath)
    if 'linux' in wrappers:
        for f in os.listdir(os.path.join(native_library_path, 'Linux', "x64")):
            if f.endswith('.so'):
                shutil.copy(os.path.join(native_library_path, 'Linux', "x64", f), target_basepath)
    if 'mac' in wrappers:
        for f in os.listdir(os.path.join(native_library_path, 'Mac', "10.7")):
            if f.endswith('.dylib'):
                shutil.copy(os.path.join(native_library_path, 'Mac', "10.7", f), target_basepath)


def copytree(src, dst, symlinks=False, ignore=None):
    for item in os.listdir(src):
        if not os.path.exists(dst):
            os.makedirs(dst)
        if ignore:
            for pattern in ignore:
                if fnmatch.fnmatch(item, pattern):
                    continue
        s = os.path.join(src, item)
        d = os.path.join(dst, item)
        if os.path.isdir(s):
            copytree(s, d, symlinks, ignore)
        else:
            shutil.copy(s, d)


if __name__ == '__main__':
    msbuild_command = ['dotnet', 'build', '-t:Build', '-t:Pack', '-p:Configuration=Release']

    parser = OptionParser(description='Indigo .NET libraries build script')
    parser.add_option('--suffix', '-s', help='archive suffix', default="")
    (args, left_args) = parser.parse_args()
    wrapper = args.suffix
    if wrapper == "universal":
        explicit_wrappers = ('win', 'linux', 'mac')
    else:
        explicit_wrappers = (wrapper, )

    api_dir = abspath(dirname(__file__))
    root = join(api_dir, "..")
    dist_dir = join(root, "dist")
    if not os.path.exists(dist_dir):
        os.mkdir(dist_dir)

    cur_dir = os.path.abspath(os.curdir)

    # Find indigo version
    indigoVersion = getIndigoVersion()

    libraryPath = join(api_dir, 'libs', 'shared')

    # Copy native libraries to Indigo.Net
    indigoDotNetPath = join(api_dir, 'dotnet')
    copy_libs(libraryPath, indigoDotNetPath, explicit_wrappers)

    # Build
    os.chdir(indigoDotNetPath)
    subprocess.check_call(msbuild_command + ['Indigo.Net.sln', ])

    os.chdir(dist_dir)

    # Zip nupkg
    if wrapper == 'universal':
        indigoDotNetVersion = xml_to_dict(os.path.join(indigoDotNetPath, 'Indigo.Net.csproj'))['PropertyGroup']['Version']
        if os.path.exists("dotnet_nupkg"):
            shutil.rmtree("dotnet_nupkg")
        os.mkdir('dotnet_nupkg')
        shutil.copy(os.path.join(api_dir, "LICENSE"), "dotnet_nupkg")
        shutil.copy(join(indigoDotNetPath, 'bin', 'Release', 'Indigo.Net.{}.nupkg'.format(indigoDotNetVersion)), "dotnet_nupkg")
        archive_name = "./indigo-dotnet-{}-nupkg-{}".format(indigoDotNetVersion, wrapper)
        os.rename("dotnet_nupkg", archive_name)
        if os.path.exists(archive_name + ".zip"):
            os.remove(archive_name + ".zip")
        shutil.make_archive(archive_name, 'zip', os.path.dirname(archive_name), archive_name)
        shutil.rmtree(archive_name)
        full_archive_name = os.path.normpath(os.path.join(dist_dir, archive_name))
        print('Archive {}.zip created'.format(full_archive_name))

    # Zip libraries per .NET target
    for dotnet_target in ("netstandard2.0",):
        target_dir = "dotnet_{}".format(dotnet_target)
        if os.path.exists(target_dir):
            shutil.rmtree(target_dir)
        os.mkdir(target_dir)
        shutil.copy(os.path.join(api_dir, "LICENSE"), target_dir)
        ignore_patterns = ('*.json', 'test.db')
        copytree(join(indigoDotNetPath, 'bin', 'Release', dotnet_target), target_dir, ignore=ignore_patterns)
        archive_name = "./indigo-dotnet-{}-{}-{}".format(indigoVersion, dotnet_target, wrapper)
        if os.path.exists(archive_name):
            shutil.rmtree(archive_name)
        os.rename(target_dir, archive_name)
        if os.path.exists(archive_name + ".zip"):
            os.remove(archive_name + ".zip")
        shutil.make_archive(archive_name, 'zip', os.path.dirname(archive_name), archive_name)
        shutil.rmtree(archive_name)
        full_archive_name = os.path.normpath(os.path.join(dist_dir, archive_name))
        print('Archive {}.zip created'.format(full_archive_name))
