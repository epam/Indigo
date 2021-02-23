![Build Status](https://github.com/epam/indigo/workflows/CI/badge.svg)
[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)

# EPAM Indigo projects #

Copyright (c) 2009-2021 EPAM Systems, Inc.

Licensed under the [Apache License version 2.0](LICENSE)

## Introduction ##

This repository includes:

* Bingo: Chemistry search engine for Oracle, Microsoft SQL Server and PostgreSQL databases
* Bingo-Elastic: Set of APIs for efficient chemistry search in Elasticsearch
  - Java API. Full README is available [here](/api/plugins/bingo-elastic/java/README.md)
  - Python API. Full README is available [here](/api/plugins/bingo-elastic/python/README.md)
* Indigo: Universal cheminformatics library with bindings to .NET, Java, Python, R and WebAssembly, and the following tools:
  - Legio: GUI application for combinatorial chemistry
  - ChemDiff: Visual comparison of two SDF or SMILES files
  - indigo-depict: Molecule and reaction rendering utility
  - indigo-cano: Canonical SMILES generator
  - indigo-deco: R-Group deconvolution utility

Detailed documentation is available at <http://lifescience.opensource.epam.com>

Changelogs could be found here:
* [Indigo API](/api/CHANGELOG.md)
* [Bingo](/bingo/CHANGELOG.md)

## Download ##
<https://lifescience.opensource.epam.com/download/indigo/index.html>

### Bindings in public repositories:
* .NET: <https://www.nuget.org/packages/Indigo.Net>
* Java: <https://search.maven.org/search?q=g:com.epam.indigo>
* Python: <https://pypi.org/project/epam.indigo/>

## Source code organization ##

Main directory structure layout:
* `api`: Indigo API sources
* `bingo`: Bingo sources
* `core`: Core algorithms and data structures sources
* `third_party`: sources for third-party libraries
* `utils`: utilities sources

Each project is placed in the corresponding directory with CMakeList.txt configuration
file, that does not include other projects. In order to build the whole project with the
correct references you need to use CMake configurations from the build_scripts directory.

## Preinstalled build tools ##

To build the project from the sources, the following tools should be installed:

* GIT 1.8.2+
* C/C++ compilers with C++11 support (GCC, Clang and MSVC are officially supported)
* CMake 3.4+
* Python 2.7+
* JDK 1.8+
* .NET Standard 2.0+

## Build instructions ##
