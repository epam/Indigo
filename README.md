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
* C/C++ compilers with C++14 support (GCC, Clang and MSVC are officially supported)
* CMake 3.4+
* Python 3.6+
* JDK 1.8+
* .NET Standard 2.0+
* Emscripten SDK
* Ninja

## Build instruction ##

Create build folder and use cmake with desired options. For instance:

```
Indigo/build>cmake .. -DBUILD_INDIGO=ON -DBUILD_INDIGO_WRAPPERS=ON -DBUILD_INDIGO_UTILS=ON
```

To build Indigo from console:
```
Indigo/build>cmake --build . --config Release --target all
```

or any of the following targets could be specified: --target { indigo-dotnet | indigo-java | indigo-python }
Build results could be collected from Indigo/dist folder.

## How to build Indigo-WASM ##

### Build tools prerequisites ###

* Git

Make sure git is running from path:
```
>git --version
git version 2.26.2.windows.1
```

* Python (https://www.python.org/downloads/)

Make sure python is running from path:
```
>python --version
Python 3.9.0
```

* cmake (https://cmake.org/download/)

Make sure cmake is running from path:
```
>cmake --version
cmake version 3.18.4
```

* Install ninja (https://github.com/ninja-build/ninja/releases)

Download corresponding ninja-xxx.zip and unpack to folder on path.
Make sure it's running from path:
```
>ninja --version
1.10.2
```

* Install emscripten sdk (https://github.com/emscripten-core/emsdk)

```
>git clone https://github.com/emscripten-core/emsdk.git
>cd emsdk
>./emsdk install latest
>./emsdk activate latest
>source ./emsdk_env.sh
```

Note: On Windows, run `emsdk` instead of `./emsdk`, and `emsdk_env.bat` instead of source `./emsdk_env.sh`.

### Get Indigo sources ###

Clone (or checkout) Indigo repository, branch `feature/246-indigo-ketcher`

```
>git clone https://github.com/epam/Indigo.git --branch feature/246-indigo-ketcher 
```

### Build Indigo ###

For each new session, set environment anew:
```
>cd emsdk
>./emsdk activate latest
```

If fresh build:
```
>mkdir build
>cd build
```

Now build:
```
>emcmake cmake .. -DCMAKE_BUILD_TYPE=Debug -G Ninja
>ninja indigo-ketcher-js-test
```
