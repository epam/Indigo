/****************************************************************************
 * Copyright (C) 2009-2012 GGA Software Services LLC
 * 
 * This file is part of Indigo toolkit.
 * 
 * This file may be distributed and/or modified under the terms of the
 * GNU General Public License version 3 as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPL included in the
 * packaging of this file.
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 ***************************************************************************/

#ifndef __os_thread_h__
#define __os_thread_h__

//
// Crossplatform thread support
//

#ifdef _WIN32

#define THREAD_RET unsigned long
#define THREAD_MOD __stdcall
#define THREAD_END return 0

#else

#include <pthread.h>

#define THREAD_RET void*
#define THREAD_MOD 
#define THREAD_END pthread_exit(NULL); return 0

#endif

#ifdef __cplusplus
extern "C" {
#endif

void osThreadCreate (THREAD_RET (THREAD_MOD *func)(void *param), void *param);

#ifdef __cplusplus
}
#endif

#endif // __os_thread_h__
