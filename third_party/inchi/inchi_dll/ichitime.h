/*
 * International Chemical Identifier (InChI)
 * Version 1
 * Software version 1.04
 * September 9, 2011
 *
 * The InChI library and programs are free software developed under the
 * auspices of the International Union of Pure and Applied Chemistry (IUPAC).
 * Originally developed at NIST. Modifications and additions by IUPAC 
 * and the InChI Trust.
 *
 * IUPAC/InChI-Trust Licence No.1.0 for the 
 * International Chemical Identifier (InChI) Software version 1.04
 * Copyright (C) IUPAC and InChI Trust Limited
 * 
 * This library is free software; you can redistribute it and/or modify it 
 * under the terms of the IUPAC/InChI Trust InChI Licence No.1.0, 
 * or any later version.
 * 
 * Please note that this library is distributed WITHOUT ANY WARRANTIES 
 * whatsoever, whether expressed or implied.  See the IUPAC/InChI Trust 
 * Licence for the International Chemical Identifier (InChI) Software 
 * version 1.04, October 2011 ("IUPAC/InChI-Trust InChI Licence No.1.0") 
 * for more details.
 * 
 * You should have received a copy of the IUPAC/InChI Trust InChI 
 * Licence No. 1.0 with this library; if not, please write to:
 * 
 * The InChI Trust
 * c/o FIZ CHEMIE Berlin
 *
 * Franklinstrasse 11
 * 10587 Berlin
 * GERMANY
 *
 * or email to: ulrich@inchi-trust.org.
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
#include <sys/timeb.h>

typedef struct tagInchiTime {
    unsigned long  clockTime; /* Time in seconds since midnight (00:00:00), January 1, 1970;
                                 signed long overflow expected in 2038 */
    long           millitime; /* milliseconds */

} inchiTime;

#endif


#ifdef TARGET_EXE_USING_API

#define InchiTimeGet           e_InchiTimeGet
#define InchiTimeMsecDiff      e_InchiTimeMsecDiff
#define InchiTimeAddMsec       e_InchiTimeAddMsec
#define bInchiTimeIsOver       e_bInchiTimeIsOver
#define InchiTimeElapsed       e_InchiTimeElapsed

#define FullMaxClock           e_FullMaxClock
#define HalfMaxClock           e_HalfMaxClock
#define MaxPositiveClock       e_MaxPositiveClock
#define MinNegativeClock       e_MinNegativeClock
#define HalfMaxPositiveClock   e_HalfMaxPositiveClock
#define HalfMinNegativeClock   e_HalfMinNegativeClock



#endif

#ifndef COMPILE_ALL_CPP
#ifdef __cplusplus
extern "C" {
#endif
#endif


void InchiTimeGet( inchiTime *TickEnd );
long InchiTimeMsecDiff( inchiTime *TickEnd, inchiTime *TickStart );
void InchiTimeAddMsec( inchiTime *TickEnd, unsigned long nNumMsec );
int  bInchiTimeIsOver( inchiTime *TickEnd );
long InchiTimeElapsed( inchiTime *TickStart );

#ifndef COMPILE_ALL_CPP
#ifdef __cplusplus
}
#endif
#endif


#endif /* __ICHITIME_H__ */

