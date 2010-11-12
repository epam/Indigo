/*
 * $Header: cdemodsc.h 14-jul-99.12:45:26 mjaeger Exp $
 */

/* Copyright (c) 1997, 1999, Oracle Corporation.  All rights reserved.
*/

/* NOTE:  See 'header_template.doc' in the 'doc' dve under the 'forms'
      directory for the header file template that includes instructions.
*/

/*
   NAME
     cdemodsc.h - header file for cdemodsc.c

   DESCRIPTION
     This file will contain the header information for the cdemodsc.c

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
   svedala     09/09/98 - lines longer than 79 chars reformatted - bug 722491
   echen       06/04/97 - fix the include files
   cchau       05/29/97 - Creation.
*/

/*----------------------------------------------------------------------*/
#ifndef CDEMODSC
#define CDEMODSC

#ifndef OCI_ORACLE
#include <oci.h>
#endif

/*----------------------------------------------------------------------*/
/*
** #define
*/

#define MAXNAME       30
#define MAXOBJLEN     60
#define MAXOBJS        7
#define NPOS          40
#define SPACING       for (glindex = 0; glindex < tab; glindex++)\
                        printf(" ")

/*----------------------------------------------------------------------*/
/*
** Prototypes for functions in cdemodsc.c
*/
static void chk_column(/*_ OCIEnv *envhp, OCIError *errhp,
                           OCISvcCtx *svchp, dvoid *dschp, ub4 parmcnt  _*/);
static void chk_method(/*_ OCIEnv *envhp, OCIError *errhp,
                    OCISvcCtx *svchp, dvoid *dschp, const text *comment _*/);
static void chk_methodlst(/*_ OCIEnv *envhp, OCIError *errhp,
         OCISvcCtx *svchp, dvoid *dschp, ub4 count, const text *comment _*/);
static void chk_arglst(/*_ OCIEnv *envhp, OCIError *errhp,
                                         OCISvcCtx *svchp, dvoid *dschp _*/);
static void chk_arg(/*_ OCIEnv *envhp, OCIError *errhp, OCISvcCtx *svchp,
                             dvoid *dschp, ub1 type, ub4 start, ub4 end _*/);
static void chk_collection (/*_ OCIEnv *envhp, OCIError *errhp,
                         OCISvcCtx *svchp, dvoid *dschp, sword is_array _*/);
static void tst_desc_type(/*_ OCIEnv *envhp, OCIError *errhp,
                                        OCISvcCtx *svchp, text *objname _*/);
static void checkerr(/*_ OCIError *errhp, sword status _*/);

/* Prototype for main function */
int main(/*_ int argc, char *argv[] _*/);
int tab;
int glindex;

#endif  /* CDEMODSC */

