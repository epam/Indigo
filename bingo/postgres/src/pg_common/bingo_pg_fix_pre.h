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