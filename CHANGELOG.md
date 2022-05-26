# Indigo 1.7.0
Released 2022-05-26

## Features
* API web service: added /render endpoint for rendering compounds and reactions
* API: added logP calculation to Python API
* API: added atom hybridization calculation to Python API
* API: added salt stripping method to Python API
* Core: added support for multistep reactions
* Ketcher WASM API: added InChIKey calculation method
* API: Added Jupyter notebooks with examples of using Indigo in machine learning
* API: Added initial version of graph neural networks featurizers

## Improvements
* ZLib updated to 1.2.12
* LibPNG updated to 1.6.37
* TinyXML updated to TinyXML2 9.0.0
* Bingo PostgreSQL support to Postgres 13 and 144 added, thanks @SPKorhonen,
  dropped support for Postgres 9.6

## Bugfixes
* Bingo Elastic: fixed exact search (#644)
* Core: ketcher format loader: options handling fixed (#588)
* API: Fixed `name()` calling for RXNV3000 format (#678)
* Numerous fixes for Ketcher data format (#689, #711, #733, #734)
* API web service: fixed descriptors calcuation


# Indigo 1.6.1
Released 2021-12-28

## Features
* PoC implementation of Indigo modern C++ user API written on top of low-level C API. Later it will be used
  in Indigo-WASM and probably other languages.
* New Indigo service added as preview. Modernized Indigo service implements JSON:API protocol and can be installed as 
  Docker image `epmlsop/indigo-service:enhanced-latest`.
* Indigo API ported to ARM64 processor architecture. Python, Java and C# wrappers now contain required native libraries 
  for macOS (Apple M1) and Linux.
* Implemented loader for CDXML format.
* Dative and hydrogen bonds are now supported.
* Implemented partial aromatization/dearomatization for the structures with superatoms.
* Multifragment support for KET-format.
* Simple objects support for KET-format.
* Atom's aliases and functional groups' attributes support for KET-format.
* Indigo-Python: initial version of inorganic salt checker added.

## Improvements
* Bingo-NoSQL major refactoring with significant multithreading performance improvements.
* C++ unittests were separated in API and Core parts.
* CMake build system by default tries to enable as many components as possible and warns if building something
  is not possible on the current platform.
* Migrated to modern C++ standard mutexes and locks instead of own-written implementation.
* Using thread-safe objects in Indigo API instead of raw mutexes to guarantee thread safety.
* C++ code modernization: added 'override', replaced plain C functions with corresponding from std, etc.
* Indigo API integration tests engine parallelized. 
* Indigo WASM API for Ketcher reached stable status and is now published to NPM public repository.
* Indigo i386 libraries for Windows prepared.
* CI/CD: automatic code style checks and linters added for Python and C++ code.

## Bugfixes
* Fixed multiple data races in API and especially in Bingo-NoSQL (#476).
* InChI library bugfix for empty string support
* Multiple small bugfixes in Indigo-Ketcher WASM module and Indigo Service.
* Bingo-Elastic-Java: updated all dependencies to fix log4j security issue.
* Fixed an occasional error in RPE.
* Bingo-NoSQL: fixed `enumerateId()` in Java.


# Indigo 1.5.0
Released 2021-09-06.

## Features
* InChI updated to 1.06
* Added WebAssembly support to run Indigo in a web browser. 
* Added Java and Python API for Bingo Elasticsearch cartridge
* Added JSON-based data format for interacting with Ketcher supporting enhanced stereochemistry, simple graphics,
  reactions and r-groups.

## Improvements
* Added C++ unittests.
* Added multiple API integration tests.
* CMake build system reworked, now all components, including Bingo cartridges and Python, Java, C# API.
  could be built using single CMake command.
* Migrated to standard modern C++ smart pointers. Changed AutoPtr to std::unique_ptr.
* Unified molecule check function and changed the result format.
* Miscellaneous modern C++ related refactorings: added 'override', replaced plain C functions with corresponding from std.
* Optimized the adding of elements to atoms and bonds arrays.
* Exposed oneBitsList in .NET API.
* Implemented context manager and iterator for Bingo object.
* Added Bingo CI for Postgres 9.6.

## Bugfixes
* api: sessions fixed, now options are session-scoped, not global.


Previous versions: 
* [Indigo API](/api/CHANGELOG.md)
* [Bingo](/bingo/CHANGELOG.md)
