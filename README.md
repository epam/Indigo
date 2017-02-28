[![Build Status](https://travis-ci.org/epam/Indigo.svg?branch=master)](https://travis-ci.org/epam/Indigo) [![Build Status](https://ci.appveyor.com/api/projects/status/github/epam/indigo?branch=master&svg=true)](https://ci.appveyor.com/project/mkviatkovskii/indigo)

# EPAM Indigo projects #

Copyright (c) 2009-2015 EPAM Systems
GNU General Public License version 3

## Introduction ##

This repository includes:
 * Bingo: Chemistry search engine for Oracle, Microsoft SQL Server and PostgreSQL databases
 * Indigo: Universal cheminformatics library, and the following tools:
  - Legio: GUI application for combinatorial chemistry
  - ChemDiff: Visual comparison of two SDF or SMILES files
  - indigo-depict: Molecule and reaction rendering utility
  - indigo-cano: Canonical SMILES generator
  - indigo-deco: R-Group deconvolution utility

Detailed documentations is available at http://lifescience.opensource.epam.com

Main directory structure layout:
 * api: Indigo API sources
 * bingo: Bingo sources
 * build_scripts: CMake and python scripts for building all the sources
 * third_party: sources for third-party libraries
 * utils: utilities sources
 * common|graph|layout|molecule|reaction|render2d: indigo-core sources

## Source code organization ##

Each project is placed in the corresponding directory with CMakeList.txt configuration
file, that does not include other projects. In order to build the whole project with the
correct references you need to use CMake configurations from the build_scripts directory.

## Build instructions ##

All the cmake projects are placed in build_scripts directories. You can use them manually,
or execute preconfigured scripts that does all the job.

## Bingo build instructions ##

To generate project configuration, build the source code, and create the archives for
installation you need to execute build_scripts\bingo-release.py:

	build_scripts\bingo-release.py --preset=linux32 --dbms=[postgres|oracle|sqlserver]

The are different cmake presets:
	linux32, linux64, win32, win64, mac10.7 (and also all later mac10.x versions)

## Indigo build instructions ##

To generate project configuration, build the source code, and create the archives for
installation you need to execute build_scripts\indigo-release-libs.py and
build_scripts\indigo-release-utils.py:


### Linux (gcc 4.7+)

	build_scripts\indigo-release-libs.py --preset=linux64
	build_scripts\indigo-release-utils.py --preset=linux64

### Windows (Microsoft Visual Studio 2013+)

	build_scripts\indigo-release-libs.py --preset=win64-2013
	build_scripts\indigo-release-utils.py --preset=win64-2013

### Mac OS (Clang 3.0+)

	build_scripts\indigo-release-libs.py --preset=mac10.10
	build_scripts\indigo-release-utils.py --preset=mac10.10

### Other

There are different cmake presets:
	win32-2013, win64-2013, win32-2015, win64-2015, win32-mingw, linux32, linux32-universal, linux64,
	linux64-universal, mac10.7, mac10.8, mac10.9, mac10.10, mac10.11, mac10.12

### Wrappers

To generate Java, C#, or Python wrappers after build the binaries you need to execute

    build_scripts\indigo-make-by-libs.py --type=java

Available types: java, dotnet, python

