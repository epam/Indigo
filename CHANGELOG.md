# Indigo 1.5.1
Released 2021-10-x.

## Features
* PoC implementation of Indigo modern C++ user API written on top of low-level C API. Later it will be used
  in Indigo-WASM and probably other languages.

## Improvements
* C++ unittests were splitted in API and Core parts.
* CMake build system by default tries to enable as many components as possible and warns if building something
  is not possible on the current platform.
* Migrated to modern C++ standard mutexes and locks instead of own-written implementation.
* Start using Rust-like thread-safe hidden objects to improve parallel execution. 
* Lots of small modern C++ related refactorings: added 'override', replaced plain C functions with corresponding from std.

## Bugfixes
* Fixed lots of data races in API and especially in Bingo-NoSQL (#476).


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
