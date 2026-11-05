# Build

> **Read when:** you are configuring, compiling, adding a target, or touching CMake/WASM/devcontainer.
> **Skip when:** the change does not affect compilation.

How to compile Indigo and its components, including WASM and the recommended dev container setup.

## Prerequisites

- CMake 3.12+ (required by the bundled FreeType 2.14.3)
- C++17 compiler (`cmake/setup.cmake` sets `CMAKE_CXX_STANDARD 17`)
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

**A source file added or removed reaches the build one build late under Visual Studio.** The globs use
`CONFIGURE_DEPENDS` (`core/indigo-core/CMakeLists.txt#CONFIGURE_DEPENDS`), so every build re-checks
them and re-runs CMake when a file appeared or went away. Under the Visual Studio 2022 generator that
re-run happens inside the build that noticed it, and MSBuild finishes that build with the old project:
an added file is compiled by the next build, and a deleted one fails the noticing build with C1083
(cannot open source file) before the next one succeeds. Other generators were not checked. Until
`core/indigo-core/` and five `api/` targets were corrected, they spelled the flag `CONFIUGURE_DEPENDS`,
which CMake accepts silently as one more glob pattern and which froze the source list at configure
time ([.claude/rules/cmake.md](../.claude/rules/cmake.md)).

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

- [testing.md](./testing.md) — running the test suites against built artifacts
- [modules/bingo-oracle.md](./modules/bingo-oracle.md) — the Bingo-Oracle build via the Docker harness
