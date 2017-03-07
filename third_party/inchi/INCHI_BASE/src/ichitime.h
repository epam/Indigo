/*
 * International Chemical Identifier (InChI)
 * Version 1
 * Software version 1.05
 * January 27, 2017
 *
 * The InChI library and programs are free software developed under the
 * auspices of the International Union of Pure and Applied Chemistry (IUPAC).
 * Originally developed at NIST.
 * Modifications and additions by IUPAC and the InChI Trust.
 * Some portions of code were developed/changed by external contributors
 * (either contractor or volunteer) which are listed in the file
 * 'External-contributors' included in this distribution.
 *
 * IUPAC/InChI-Trust Licence No.1.0 for the
 * International Chemical Identifier (InChI)
 * Copyright (C) IUPAC and InChI Trust Limited
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the IUPAC/InChI Trust InChI Licence No.1.0,
 * or any later version.
 *
 * Please note that this library is distributed WITHOUT ANY WARRANTIES
 * whatsoever, whether expressed or implied.
 * See the IUPAC/InChI-Trust InChI Licence No.1.0 for more details.
 *
 * You should have received a copy of the IUPAC/InChI Trust InChI
 * Licence No. 1.0 with this library; if not, please write to:
 *
 * The InChI Trust
 * 8 Cavendish Avenue
 * Cambridge CB1 7US
 * UK
 *
 * or e-mail to alan@inchi-trust.org
 *
 */


#ifndef __ICHITIME_H__
#define __ICHITIME_H__

#ifdef COMPILE_ANSI_ONLY

#ifdef __FreeBSD__
#include <sys/time.h>
#endif

/* get times() */
#ifdef INCHI_USETIMES
#include <sys/times.h>
#endif

/*#include <sys/timeb.h>*/

#include <time.h>

typedef struct tagInchiTime {
    clock_t clockTime;
} inchiTime;

#else

/* Win32 _ftime(): */
#include <time.h>

typedef struct tagInchiTime {
    unsigned long  clockTime; /* Time in seconds since midnight (00:00:00), January 1, 1970;
                                 signed long overflow expected in 2038 */
    long           millitime; /* milliseconds */
} inchiTime;

#endif

#ifndef COMPILE_ALL_CPP
#ifdef __cplusplus
extern "C" {
#endif
#endif


typedef struct tagINCHI_CLOCK
{
    clock_t m_MaxPositiveClock;
    clock_t m_MinNegativeClock;
    clock_t m_HalfMaxPositiveClock;
    clock_t m_HalfMinNegativeClock;
} INCHI_CLOCK;

void InchiTimeGet( inchiTime *TickEnd );

long InchiTimeMsecDiff(INCHI_CLOCK *ic, inchiTime *TickEnd, inchiTime *TickStart );
void InchiTimeAddMsec(INCHI_CLOCK *ic, inchiTime *TickEnd, unsigned long nNumMsec );
int  bInchiTimeIsOver(INCHI_CLOCK *ic, inchiTime *TickEnd );
long InchiTimeElapsed(INCHI_CLOCK *ic, inchiTime *TickStart );

#ifndef COMPILE_ALL_CPP
#ifdef __cplusplus
}
#endif
#endif


#endif /* __ICHITIME_H__ */
