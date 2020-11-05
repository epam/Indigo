![Build Status](https://github.com/epam/indigo/workflows/CI/badge.svg)
[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)

# EPAM Indigo projects #

Copyright (c) 2009-2020 EPAM Systems, Inc.

Licensed under the [Apache License version 2.0](LICENSE)

## Introduction ##

This repository includes:

* Bingo: Chemistry search engine for Oracle, Microsoft SQL Server and PostgreSQL databases
* Bingo-elastic: Set of APIs for efficient chemistry search in Elasticsearch
  - Java API. Full README is available [here](/api/plugins/bingo-elastic/java/README.md)
* Indigo: Universal cheminformatics library with bindings to .NET, Java and Python, and the following tools:
  - Legio: GUI application for combinatorial chemistry
  - ChemDiff: Visual comparison of two SDF or SMILES files
  - indigo-depict: Molecule and reaction rendering utility
  - indigo-cano: Canonical SMILES generator
  - indigo-deco: R-Group deconvolution utility

Detailed documentation is available at <http://lifescience.opensource.epam.com>

## Download ##
<https://lifescience.opensource.epam.com/download/indigo/index.html>

### Bindings:
* .NET: <https://www.nuget.org/packages/Indigo.Net>
* Java: <https://search.maven.org/search?q=g:com.epam.indigo>
* Python: <https://pypi.org/project/epam.indigo/>

## Source code organization ##

Main directory structure layout:
* `api`: Indigo API sources
* `bingo`: Bingo sources
* `build_scripts`: CMake and python scripts for building all the sources
* `third_party`: sources for third-party libraries
* `utils`: utilities sources
* `common`|`graph`|`layout`|`molecule`|`reaction`|`render2d`: indigo-core sources

Each project is placed in the corresponding directory with CMakeList.txt configuration
file, that does not include other projects. In order to build the whole project with the
correct references you need to use CMake configurations from the build_scripts directory.

## Preinstalled build tools ##

To build the project from the sources, the following tools should be installed:

* GIT 1.8.2+
* GIT-LFS, so GIT can handle Large File Storage. Refer to <https://github.com/git-lfs/git-lfs/wiki/Installation>
* C/C++ compiler
    * Linux:   GCC version 4.9+ (GCC 4.8.4 is used in builds) or Clang version 3.5+
    * Windows: Visual Studio 2013+ (2015 is used in builds) or MinGW with GCC version 4.9+
    * macOS:   Clang 3.5+ (AppleClang 9.1.0 is used in builds)
* CMake 3.0+
* Python 2.7+
* JDK 1.8+
* .NET Standard 2.0
* Maven 3.1+

## Build instructions ##

All the CMake projects are placed in `build_scripts` directories. You can use them manually,
or execute preconfigured scripts those do all the job.

## Bingo build instructions ##

To generate project configuration, build the source code, and create the archives for
installation, execute `build_scripts/bingo-release.py`:
```bash
python build_scripts/bingo-release.py --preset=linux32 --dbms=[postgres|oracle|sqlserver]
```
There are different cmake presets: 
`linux32`, `linux64`, `win32`, `win64`, `mac10.7` (and also all later mac10.x versions)

There are different dbms targets:
* `postgres` - PostgreSQL
* `oracle` - Oracle
* `sqlserver` - Microsoft SQL Server

## Indigo build instructions ##

To generate project configuration, build the source code, and create the archives for
installation, execute `build_scripts/indigo-release-libs.py` and 
`build_scripts/indigo-release-utils.py`:


### Linux (GCC 4.9+ or Clang 3.5+)
APT-based requirements (Debian or Ubuntu):
```bash
sudo apt install cmake libfreetype6-dev libfontconfig1-dev
```
RPM-based requirements (RedHat, CentOS, Fedora): install Developer Toolset if your OS does not have GCC 4.9+ and then install dependencies:
```bash
sudo yum install cmake freetype-devel fontconfig-devel
```
Build libraries and utils:
```bash
python build_scripts/indigo-release-libs.py --preset=linux64
python build_scripts/indigo-release-utils.py --preset=linux64
```

### Windows (Microsoft Visual Studio 2013+ or MinGW with GCC version 4.9+)
```bash
python build_scripts/indigo-release-libs.py --preset=win64-2013
python build_scripts/indigo-release-utils.py --preset=win64-2013
```

### Mac OS (Clang 3.5+)
```bash
python build_scripts/indigo-release-libs.py --preset=mac10.14
python build_scripts/indigo-release-utils.py --preset=mac10.14
```
### Other presets

There are different presets for the build scripts:
* `win32-2013`, `win64-2013`: Visual Studio 2013
* `win32-2015`, `win64-2015`: Visual Studio 2015
* `win32-2017`, `win64-2017`: Visual Studio 2017
* `win32-2019`, `win64-2019`: Visual Studio 2019
* `win32-mingw`: MinGW
* `win64-mingw`: MinGW-w64 
* `linux32`, `linux64`: GCC or Clang on Linux with C++11 support
* `linux32-universal`, `linux64-universal`: GCC on Linux with statically linked libstdc++ for using on older Linux systems without C++11 support
* `mac10.7`, `mac10.8`, `mac10.9`, `mac10.10`, `mac10.11`, `mac10.12`, `mac10.12`, `mac10.13`, `mac10.14`, `mac10.15`: target Mac OS X or macOS
* `mac-universal`: targeting Mac OS X 10.7 as first version with C++11 support, should work on all Mac OS X 10.7+ systems

## Wrappers build instructions

To generate Java, C#, or Python wrappers after building the binaries, execute:
```bash
python build_scripts/indigo-make-by-libs.py --type=[java|dotnet|python]
```
Available wrapper types: `java`, `dotnet`, `python`.
