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

#ifndef __bingo_pg_fix_post_h__
#define __bingo_pg_fix_post_h__

#ifdef qsort
#undef qsort
#endif

#ifdef printf
#undef printf
#endif

#ifdef vprintf
#undef vprintf
#endif

#ifdef snprintf
#undef snprintf
#endif

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

#ifdef send
#undef send
#endif

#ifdef IGNORE
#undef IGNORE
#endif

#endif //__bingo_pg_fix_post_h__
