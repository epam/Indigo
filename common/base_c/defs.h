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

#ifndef __defs_h__
#define __defs_h__

#if !defined(__max)
#define __max(a, b) (((a) > (b)) ? (a) : (b))
#endif
#if !defined(__min)
#define __min(a, b) (((a) < (b)) ? (a) : (b))
#endif
#if !defined(__min3)
#define __min3(a, b, c) (__min(a, __min(b, c)))
#endif
#if !defined(__max3)
#define __max3(a, b, c) (__max(a, __max(b, c)))
#endif
#if !defined(__sign)
#define __sign(a) (a > 0 ? 1 : (a < 0 ? -1 : 0))
#endif

#define __swap(a, b, tmp) ((tmp) = (b), (b) = (a), (a) = (tmp))

#if defined(_WIN32) && !defined(__MINGW32__)
//#define vsnprintf _vsnprintf
#if _MSC_VER < 1900
#define snprintf _snprintf
#endif

#define strcasecmp _stricmp
#define strncasecmp _strnicmp
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define NELEM(arr) ((int)(sizeof(arr) / sizeof(arr[0])))

#ifndef dword
typedef unsigned int dword;
#endif
#ifndef __byte_typedef__
#define __byte_typedef__
typedef unsigned char byte;
#endif

#ifndef EXPORT_SYMBOL
#ifdef _WIN32
#define EXPORT_SYMBOL __declspec(dllexport)
#elif (defined __GNUC__ || defined __APPLE__)
#define EXPORT_SYMBOL __attribute__((visibility("default")))
#else
#define EXPORT_SYMBOL
#endif
#endif

#ifndef DLLEXPORT
#ifdef _WIN32
#ifdef INDIGO_PLUGIN
#define DLLEXPORT __declspec(dllimport)
#else
#define DLLEXPORT EXPORT_SYMBOL
#endif
#else
#define DLLEXPORT EXPORT_SYMBOL
#endif
#endif

#ifndef CEXPORT
#ifndef __cplusplus
#define CEXPORT EXPORT_SYMBOL
#else
#define CEXPORT extern "C" EXPORT_SYMBOL
#endif
#endif

#if defined(_WIN32) && !defined(__MINGW32__)
#define qword unsigned __int64
#else
#define qword unsigned long long
#endif

typedef unsigned short word;

#if !defined(NULL)
#define NULL 0L
#endif

#endif
