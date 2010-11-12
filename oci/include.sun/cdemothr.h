/*
 * $Header: cdemothr.h 14-jul-99.12:48:01 mjaeger Exp $
 */

/* Copyright (c) 1997, 1999, Oracle Corporation.  All rights reserved.
*/

/* NOTE:  See 'header_template.doc' in the 'doc' dve under the 'forms'
      directory for the header file template that includes instructions.
*/

/*
   NAME
     cdemothr.h - C Demo for ociTHRead header file

   DESCRIPTION
     This file contains the definition of the thread context needed by
     the demo program for OCIThread

   RELATED DOCUMENTS

   INSPECTION STATUS
     Inspection date:
     Inspection status:
     Estimated increasing cost defects per page:
     Rule sets:

   ACCEPTANCE REVIEW STATUS
     Review date:
     Review status:
     Reviewers:

   PUBLIC FUNCTION(S)
     <list of external functions declared/defined - with one-line descriptions>

   PRIVATE FUNCTION(S)
     <list of static functions defined in .c file - with one-line descriptions>

   EXAMPLES

   NOTES
     <other useful comments, qualifications, etc.>

   MODIFIED   (MM/DD/YY)
   mjaeger     07/14/99 - bug 808870: OCCS: convert tabs, no long lines
   nramakri    12/17/97 - Creation

*/

#ifndef CDEMOTHR_ORACLE
#define CDEMOTHR_ORACLE

#include <oci.h>

/*---------------------------------------------------------------------------
                     PUBLIC TYPES AND CONSTANTS
  ---------------------------------------------------------------------------*/

/*--------------------------- CDEMOTHR_NUMTHREADS ---------------------------*/
/***
  Constant for the number of threads that will be spawned in demo in
  a multi-threaded environment
***/
#define CDEMOTHR_NUMTHREADS       32

/*-------------------------------- CDemoThrCtx---------------------------*/
/*
 CDemoThrCtx - C Demo Thread Context

 One instance of this structure is created and shared among all the threads
 that are created in the demo ('CDemoThr()').
*/

struct CDemoThrCtx
{
   OCIThreadId      *mainTID_CDemoThrCtx;   /* ID for the main thread */
   /* All of the spawned threads do a check to make sure that their thread  */
   /* ID is different from that of the main thread.                        */

   OCIThreadMutex       *tidArMx_CDemoThrCtx;  /* Mutex for 'tidAr_'   */

   /* Array of thread IDs */
   OCIThreadId    *tidAr_CDemoThrCtx[CDEMOTHR_NUMTHREADS];
   /* As threads are spawned, the array is filled up with their IDs.  The   */
   /* mutex must be held in order for it be safe to access the array.       */

   sword  tnumAr_CDemoThrCtx[CDEMOTHR_NUMTHREADS];  /* Array of thread #'s  */
   /* A thread whose ID is in position 'i' in 'tidAr_' will put 'i' in      */
   /* position 'i' of this array.  The main thread checks this to ensure    */
   /* that everything went OK.                                              */

   OCIThreadKey       *key_CDemoThrCtx;          /* Thread key              */
   ub1   keyAr_CDemoThrCtx[CDEMOTHR_NUMTHREADS];   /* Values for thread key */
};
typedef struct CDemoThrCtx CDemoThrCtx;

/*---------------------------------------------------------------------------
                     PRIVATE TYPES AND CONSTANTS
  ---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------
                           PUBLIC FUNCTIONS
  ---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------
                          PRIVATE FUNCTIONS
  ---------------------------------------------------------------------------*/

#endif                                              /* CDEMOTHR_ORACLE */
