/****************************************************************************
 * Copyright (C) from 2009 to Present EPAM Systems.
 *
 * This file is part of Indigo toolkit.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ***************************************************************************/

#ifndef __indigo_version__
#define __indigo_version__

// Detect compiler
#if _MSC_VER
#if __clang__
#define COMPILER clangcl
#define COMPILER_VERSION __clang_major__.__clang_minor__.__clang_patchlevel__
#else
#define COMPILER msvc
#define COMPILER_VERSION _MSC_VER
#endif
#elif __clang__
#if __apple_build_version__
#define COMPILER appleclang
#define COMPILER_VERSION __apple_build_version__
#else
#define COMPILER clang
#define COMPILER_VERSION __clang_major__.__clang_minor__.__clang_patchlevel__
#endif
#elif __GNUC__
#define COMPILER gnu
#define COMPILER_VERSION __GNUC__.__GNUC_MINOR__.__GNUC_PATCHLEVEL__
#else
#define COMPILER unknown
#define COMPILER_VERSION unknown
#endif

// Detect OS
#if _WIN32
#define OS win
#elif __MSYS__
#define OS win_msys
#elif __CYGWIN__
#define OS win_cygwin
#elif __linux__
#undef linux
#define OS linux
#elif __APPLE__
#define OS darwin
#elif __EMSCRIPTEN__
#define OS wasm
#else
#define OS unknown
#endif

// Detect Arch
#if __x86_64__ || _M_AMD64
#define ARCH x86_64
#elif __i386__ || _M_IX86
#define ARCH x86
#elif __aarch64__
#define ARCH aarch64
#elif __arm__ || _M_ARM
#define ARCH arm
#elif __EMSCRIPTEN__
#define ARCH wasm32
#else
#define ARCH unknown
#endif

#define STRINGIFY2(x) #x
#define STRINGIFY(x) STRINGIFY2(x)

#define INDIGO_PLATFORM STRINGIFY(ARCH) "-" STRINGIFY(OS) "-" STRINGIFY(COMPILER) "-" STRINGIFY(COMPILER_VERSION)

#define INDIGO_VERSION "${INDIGO_VERSION_EXT}"

#endif
