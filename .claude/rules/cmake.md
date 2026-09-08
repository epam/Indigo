---
paths:
  - "**/CMakeLists.txt"
  - "cmake/**/*.cmake"
  - "CMakePresets.json"
description: "CMake conventions for the Indigo build: globs, targets, feature flags, third-party, tests"
---

# CMake rules

Configure and build commands: [.memory-bank/build.md](../../.memory-bank/build.md).

## Standards

- `cmake_minimum_required` 3.9 at the root; C++17 is set centrally in `cmake/setup.cmake` — do not
  set `CMAKE_CXX_STANDARD` again in a subdirectory.
- Ninja is the default generator; Visual Studio 2022 on Windows and Unix Makefiles are supported
  fallbacks.

## Globs

```cmake
file(GLOB_RECURSE sources CONFIGURE_DEPENDS src/*.cpp src/*.h)
```

`CONFIGURE_DEPENDS` is mandatory. The misspelling `CONFIUGURE_DEPENDS` is accepted silently by CMake
and produces a build that does not notice new files — grep for it before believing a stale build.

## Targets

- `STATIC` for internal libraries, `SHARED` for the C API libraries that form the FFI boundary.
- Always the target-scoped forms: `target_link_libraries(t PRIVATE|PUBLIC dep)`,
  `target_include_directories(t ...)`, `target_compile_definitions(t ...)`. The directory-scoped
  commands (`link_libraries`, `include_directories`) leak into every target defined afterwards.
- Output names come from `set_target_properties(... OUTPUT_NAME ...)`, output directories from
  `cmake/setup.cmake`. No hard-coded paths — use `${CMAKE_BINARY_DIR}` and friends.

## Feature flags

- Every optional component is an `option(BUILD_XXX "..." ON/OFF)`. Inter-flag dependencies are
  resolved in the root `CMakeLists.txt`, not by expecting the caller to know them.
- Emscripten builds disable everything except the WASM targets; check the flag rather than assuming
  a component is present.

## Third-party and tests

- Standalone mode takes dependencies from `third_party/`; system mode goes through the `Find*.cmake`
  modules in `cmake/`. A new dependency must work in both, or explicitly disable itself in one.
- Register tests with `add_test`, or `gtest_discover_tests(target)` for GoogleTest suites, so CTest
  sees them.
