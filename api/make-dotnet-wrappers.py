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
    if os.path.exists(join(target_basepath, "lib")):
        shutil.rmtree(join(target_basepath, "lib"))
    os.mkdir(join(target_basepath, "lib"))

    if 'win' in wrappers:
        shutil.copytree(os.path.join(native_library_path, 'Win'), join(target_basepath, "lib", "Win"))
    if 'linux' in wrappers:
        shutil.copytree(os.path.join(native_library_path, 'Linux'), join(target_basepath, "lib", "Linux"))
    if 'mac' in wrappers:
        shutil.copytree(os.path.join(native_library_path, 'Mac'), join(target_basepath, "lib", "Mac"))


if __name__ == '__main__':
    msbuild_command = ['dotnet', 'build', '-t:restore', '/t:Rebuild', '/p:Configuration=Release']

    parser = OptionParser(description='Indigo .NET libraries build script')
    parser.add_option('--suffix', '-s', help='archive suffix', default="")
    (args, left_args) = parser.parse_args()
    wrapper = args.suffix
    if  wrapper == "universal":
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
    # Copy native libraries to IndigoRenderer.Net
    indigoRendererDotNetPath = join(api_dir, "plugins", "renderer", "dotnet")
    copy_libs(libraryPath, indigoRendererDotNetPath, explicit_wrappers)
    # Copy native libraries to IndigoInchi.Net
    indigoInchiDotNetPath = join(api_dir, "plugins", "inchi", "dotnet")
    copy_libs(libraryPath, indigoInchiDotNetPath, explicit_wrappers)
    # Copy native libraries to Bingo.Net
    bingoDotNetPath = join(api_dir, "plugins", "bingo", "dotnet")
    copy_libs(libraryPath, bingoDotNetPath, explicit_wrappers)

    # Build
    os.chdir(indigoDotNetPath)
    subprocess.check_call(msbuild_command + ['Indigo.Net.sln', ])

    # Zip nupkg
    if wrapper == 'universal':
        indigoDotNetVersion = xml_to_dict(os.path.join(indigoDotNetPath, 'Indigo.Net.csproj'))['PropertyGroup']['Version']
        os.chdir(dist_dir)
        if os.path.exists("dotnet_nupkg"):
            shutil.rmtree("dotnet_nupkg")
        os.mkdir('dotnet_nupkg')
        shutil.copy(os.path.join(api_dir, "LICENSE"), "dotnet_nupkg")
        shutil.copy(join(indigoDotNetPath, 'bin', 'Release', 'Indigo.Net.{}.nupkg'.format(indigoDotNetVersion)), "dotnet_nupkg")
        shutil.copy(join(indigoRendererDotNetPath, 'bin', 'Release', 'IndigoRenderer.Net.{}.nupkg'.format(indigoDotNetVersion)), "dotnet_nupkg")
        shutil.copy(join(indigoInchiDotNetPath, 'bin', 'Release', 'IndigoInchi.Net.{}.nupkg'.format(indigoDotNetVersion)), "dotnet_nupkg")
        shutil.copy(join(bingoDotNetPath, 'bin', 'Release', 'Bingo.Net.{}.nupkg'.format(indigoDotNetVersion)), "dotnet_nupkg")
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
        ignore_patterns = shutil.ignore_patterns('*.json', 'test.db')
        shutil.copytree(join(indigoDotNetPath, 'bin', 'Release', dotnet_target), target_dir, ignore=ignore_patterns, dirs_exist_ok=True)
        shutil.copytree(join(indigoRendererDotNetPath, 'bin', 'Release', dotnet_target), target_dir, ignore=ignore_patterns,  dirs_exist_ok=True)
        shutil.copytree(join(indigoInchiDotNetPath, 'bin', 'Release', dotnet_target), target_dir, ignore=ignore_patterns,  dirs_exist_ok=True)
        shutil.copytree(join(bingoDotNetPath, 'bin', 'Release', dotnet_target), target_dir, ignore=ignore_patterns, dirs_exist_ok=True)
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
