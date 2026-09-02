# Build

How to compile Indigo and its components, including WASM and the recommended dev container setup.

## Prerequisites

- CMake 3.9+
- C++14 compiler
- Python 3.6+
- Ninja (required when using CMake presets)
- JDK 1.8+ (only for the Java wrapper)
- .NET Standard 2.0+ (only for the .NET wrapper)

## Configure and build

```bash
# Configure (using preset)
cmake --preset indigo-debug

# Or manually
mkdir build && cd build
cmake .. -DBUILD_INDIGO=ON -DBUILD_INDIGO_WRAPPERS=ON -DBUILD_INDIGO_UTILS=ON

# Build a specific target
cmake --build build --config Release --target indigo-python
# Other targets: indigo-java, indigo-dotnet, all (Linux) / ALL_BUILD (Windows)
```

Build output lands in `dist/` (Linux) or `api/python/dist/` (Windows).

## Notable CMake options

`BUILD_INDIGO`, `BUILD_INDIGO_WRAPPERS`, `BUILD_INDIGO_UTILS`, `BUILD_BINGO`, `BUILD_BINGO_POSTGRES`, `BUILD_BINGO_ORACLE`, `BUILD_BINGO_SQLSERVER`, `BUILD_BINGO_ELASTIC`, `BUILD_STANDALONE`.

The Bingo-SqlServer cartridge (`bingo/sqlserver/`) is C# and only builds on Windows with MSVC + Visual Studio. On other platforms `BUILD_BINGO_SQLSERVER` is silently skipped.

## WASM build

Requires the Emscripten SDK installed and activated:

```bash
emcmake cmake .. -DCMAKE_BUILD_TYPE=Debug -G Ninja
ninja indigo-ketcher-js-test
```

## Dev container

`.devcontainer/` configures a Debian 12 container (`linux/amd64`). The Python venv lives at `/opt/venv`. The devcontainer is the recommended setup for cross-platform development — particularly on macOS, where it avoids native toolchain drift.

## Related

- [claude-docs/testing.md](testing.md) — running the test suites against built artifacts
- [claude-docs/oracle.md](oracle.md) — the Bingo-Oracle build via the Docker harness
