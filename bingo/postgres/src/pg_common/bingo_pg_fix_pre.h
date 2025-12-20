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

#ifndef __bingo_pg_fix_pre_h__
#define __bingo_pg_fix_pre_h__

// Visual Studio 2013 has isnan and isinf functions defined in math.h.
// PostgreSQL defines isnan and isind macroses, so we need to include math.h
// before PostgreSQL includes
//
// See also:
// http://www.postgresql.org/message-id/529D05CC.7070806@gmx.de
// http://www.postgresql.org/message-id/attachment/31194/VS2013_01.patch
#if (_MSC_VER >= 1800)

#include <math.h>
// PostgeSQL 14+ patches system files and thus postgres.h must be included
// before inclusion of <sys/stat.h> or patch fails.
// Thus <functional> must not be included here as this file is included
// before postgres.h
// #include <functional>

#endif

#endif // #ifndef __bingo_pg_fix_pre_h__