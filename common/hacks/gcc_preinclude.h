/****************************************************************************
 * Copyright (C) 2009-2013 GGA Software Services LLC
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

/* http://rjpower.org/wordpress/fun-with-shared-libraries-version-glibc_2-14-not-found/ */

#ifndef  __gcc_preinclude_h__
#define __gcc_preinclude_h__

#if defined(__GLIBC__) && defined(__x86_64__)
__asm__(".symver memcpy,memcpy@GLIBC_2.2.5");
#endif

#endif
