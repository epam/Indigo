# Indigo 1.6.1
Released 2021-12-30
## Features
* Multifragment support for KET-format
* Simple objects support for KET-format
* Atom's aliases and functional groups' attributes support for KET-format
* Dative and hydrogen bonds support
* Partial aromatisation/dearomatization for the structures with superatoms

## Improvements
* Multiple ket-format improvements

## Bugfixes
* Inchi bugfix for empty string support
* Multiple bugfixes in WASM and indigo remote service API

# Indigo 1.6.0
Released 2021-11-24.

## Features
* PoC implementation of Indigo modern C++ user API written on top of low-level C API. Later it will be used
  in Indigo-WASM and probably other languages.
* New Indigo service added as preview. Modernized Indigo service implements JSON:API protocol and can be installed as 
  Docker image `epmlsop/indigo-service:enhanced-latest`.
* Indigo API ported to ARM64 processor architecture. Python, Java and C# wrappers now contain required native libraries 
  for macOS (Apple M1) and Linux.
* Implemented loader for CDXML format.

## Improvements
* Bingo-NoSQL major refactoring with significant multithreading performance improvements.
* C++ unittests were splitted in API and Core parts.
* CMake build system by default tries to enable as many components as possible and warns if building something
  is not possible on the current platform.
* Migrated to modern C++ standard mutexes and locks instead of own-written implementation.
* Using thread-safe objects in Indigo API instead of raw mutexes to guarantee thread safety.
* C++ code modernization: added 'override', replaced plain C functions with corresponding from std, etc.
* Indigo API integration tests engine parallelized. 
* Indigo WASM API for Ketcher reached stable status and is now published to NPM public repository.
* Indigo i386 libraries for Windows prepared.

## Bugfixes
* Fixed multiple data races in API and especially in Bingo-NoSQL (#476).


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
* Miscelaneous modern C++ related refactorings: added 'override', replaced plain C functions with corresponding from std.
* Optimized the adding of elements to atoms and bonds arrays.
* Exposed oneBitsList in .NET API.
* Implemented context manager and iterator for Bingo object.
* Added Bingo CI for Postgres 9.6.

## Bugfixes
* api: sessions fixed, now options are session-scoped, not global.
