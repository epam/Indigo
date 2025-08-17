[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)

# EPAM Indigo projects #

Copyright (c) 2009-2022 EPAM Systems, Inc.

Licensed under the [Apache License version 2.0](LICENSE)

## Introduction ##

This repository includes:

* Bingo: Chemistry search engine for Oracle, Microsoft SQL Server and PostgreSQL databases
* Bingo-Elastic: Set of APIs for efficient chemistry search in Elasticsearch
  - Java API. Full README is available [here](/bingo/bingo-elastic/java/README.md)
  - Python API. Full README is available [here](/bingo/bingo-elastic/python/README.md)
* Indigo: Universal cheminformatics library with bindings to .NET, Java, Python, R and WebAssembly, and the following tools:
  - Legio: GUI application for combinatorial chemistry
  - ChemDiff: Visual comparison of two SDF or SMILES files
  - indigo-depict: Molecule and reaction rendering utility
  - indigo-cano: Canonical SMILES generator
  - indigo-deco: R-Group deconvolution utility

Detailed documentation is available at <http://lifescience.opensource.epam.com>

Changelog could be found in [CHANGELOG.md](/CHANGELOG.md).


## Download ##
<https://lifescience.opensource.epam.com/download/indigo.html>

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

## Develop using dev container

This is an alternative setup solution next to have it on your PC directly. It builds on having the tools in a docker container and VSCode acts as a client.

### Prerequisites

1. Docker Desktop (or docker engine if you prefer)
2. VSCode

### Installation Steps

1. Clone this repository
2. Open in VSCode
3. Follow recommendation to install the Dev Container Extension
4. Open in Dev Container

The first time takes some minutes but afterwards you can directly develop from VSCode.

***Remark for Windows Users:*** Its a known limitation that on windows the overlay driver is super slow, so if you want to have
a fast IDE, clone your container into a volume rather than natively on the client or in your WSL distribution.

## Preinstalled build tools ##

To build the project from the sources, the following tools should be installed:

>Required:
* GIT 1.8.2+
* C/C++ compilers with C++14 support (GCC, Clang and MSVC are officially supported)
* CMake 3.4+
* Python 3.6+

>Required to build all targets:
* JDK 1.8+
* .NET Standard 2.0+

>Required to build Indigo-WASM:
* Emscripten SDK
* Ninja


#### **Dependencies:**

> **Python**
- wheel package installed. Command: `python -m pip install wheel`
- setuptools version less than 72.0.0 package installed. Command: `python -m pip install setuptools==68.0.0`
- waitress package installed (to run backend API test). Command: `python -m pip install waitress`
- flasgger package installed (to run backend API test). Command: `python -m pip install flasgger`
- psycopg2 package installed (to run backend API test). Command: `python -m pip install psycopg2`
- sqlalchemy package installed (to run backend API test). Command: `python -m pip install sqlalchemy`
- numpy package installed (to run backend API test). Command: `python -m pip install numpy`
- celery package installed (to run backend API test). Command: `python -m pip install celery`
- marshmallow package installed (to run backend API test). Command: `python -m pip install marshmallow`
- redis package installed (to run backend API test). Command: `python -m pip install redis`
- flask_httpauth package installed (to run backend API test). Command: `python -m pip install flask_httpauth`
- pyparsing package installed (to run backend API test). Command: `python -m pip install pyparsing`
- requests package installed (to run backend API test). Command: `python -m pip install requests`

> On Linux use python3 insted of python. Using virtual environment might be required as well.


## Build instruction ##

>On Windows use cmd  to run the commands.

1) Create build folder
    ```
    mkdir build
    ```
2) Move to build folder
    ```
    cd build
    ```

2) Run CMake to configure the project with desired options. For instance:
    ```
    cmake .. -DBUILD_INDIGO=ON -DBUILD_INDIGO_WRAPPERS=ON -DBUILD_INDIGO_UTILS=ON
    ```

3) Build Indigo from console:
    ```
    cmake --build . --config Release --target <target name>
    ```

Replace _**\<target name\>**_ with any of the following targets you need:

- **ALL_BUILD** (on Windows)
- **all** (on Linux)
- **indigo-dotnet**
- **indigo-java**
- **indigo-python**

> Build results could be collected from Indigo/dist folder.

> 'indigo-python' target is commonly used.

## Run tests ##
Befor running any test you have to build and install indigo-python

1) Build indigo-python using `--target indigo-python` or `--target ALL_BUILD`(on Windows) or `--target all`(on Linux). See Build instruction above.
    > - On Windows the package should be in 'Indigo/api/python/dist' folder
    > - On Linux it is located in 'Indigo\dist' folder


> Package will be named like 'epam.indigo-*\<version-arch\>*.whl'. For instance: *epam.indigo-1.29.0.dev2-py3-none-win_amd64.whl*

2) Install package using pip
    > - If Indigo package has been already installed, uninstall it with the following command: `python -m pip uninstall <path-to-.whl-file> -y`

    ```
    python -m pip install <path-to-.whl-file>
    ```

    > Replace _**\<path-to-.whl-file>**_ with the right path to .whl package. For instance: `python -m pip install ../api/python/dist/epam.indigo-1.29.0.dev2-py3-none-win_amd64.whl`

3) Run integration test

    >to run all test

    ```
    python api/tests/integration/test.py -t 1
    ```

    >to run tests by mask use `test_name`
    ```
    python api/tests/integration/test.py -t 1 -p test_name
    ```

### To run backend API test:
1) Build and install indigo-python
2) Set environment variable by running this command:

    > for Linux
    ```
    export INDIGO_SERVICE_URL=http://localhost:5000/v2
    ```
    > for Windows in PowerShell
    ```
    $env:INDIGO_SERVICE_URL="http://localhost:5000/v2"
    ```
3) Run backend service :
    ```
    cd utils/indigo-service/backend/service
    ```
    ```
    cp v2/common/config.py .
    ```
    ```
    waitress-serve --listen="127.0.0.1:5000 [::1]:5000"  app:app
    ```
    > you may use any port instead of 5000

4) Run backend API test:
    ```
    python utils/indigo-service/backend/service/tests/api/indigo_test.py`
    ```
    > use `-k test_name` to run test by pattern.

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
git clone https://github.com/emscripten-core/emsdk.git
```
```
cd emsdk
```
```
./emsdk install latest
```

```
./emsdk activate latest
```
```
source ./emsdk_env.sh
```

Note: On Windows, run `emsdk` instead of `./emsdk`, and `emsdk_env.bat` instead of source `./emsdk_env.sh`, use `cmd` instead of `powershell`.

### Get Indigo sources ###

Clone (or checkout) Indigo repository

```
>git clone https://github.com/epam/Indigo.git
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


## How to build conda package ##

Prepare conda build environment as described at https://docs.conda.io/projects/conda-build/en/stable/install-conda-build.html
Change directory to ```recipe-conda``, set INDIGO_VERSION to existing Indigo version(already published at PyPi), run conda build:
Linux/Mac:
```
>cd conda-recipe
>INDIGO_VERSION=1.29.0 conda build .
```

Windows(using cmd):
```
>cd conda-recipe
>set INDIGO_VERSION=1.29.0
>conda build .
```

To upload packages: install anaconda-client, login, and upload packet using command provided by conda-build in output
```
>conda install anaconda-client
>anaconda login
Enter username:
Enter password:
>anaconda upload /conda-build-dir/packet-name
```
