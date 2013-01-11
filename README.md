# GGA Indigo projects #

Copyright (c) 2009-2012 GGA Software Services LLC
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

Detailed documentations is available at http://ggasoftware.com/opensource

Main directory structure:
	* api: Indigo API sources
	* bingo: Bingo sources
	* build_scripts: CMake and python scripts for building all the sources

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
	linux32, linux64, win32, win64, mac10.5, mac10.6 (for 10.7 also)

## Indigo build instructions ##

To generate project configuration, build the source code, and create the archives for 
installation you need to execute build_scripts\bingo-release.py:

	build_scripts\indigo-release-libs.py --preset=linux32
	build_scripts\indigo-release-utils.py --preset=win32

There are different cmake presets:
	linux32, linux64, win32, win64, mac10.5, mac10.6 (for 10.7 also)

To generate Java, C#, or Python wrappers after build the binaries you need to execute
	api\make-cs-wrappers.py
	make-java-wrappers.py
	make-python-wrappers.py

