/*
 * $Header: cdemorid.h 13-sep-2000.22:27:29 emendez Exp $
 */

/* Copyright (c) 1997, 2000 Oracle Corporation.  All rights reserved.
*/

/* NOTE:  See 'header_template.doc' in the 'doc' dve under the 'forms'
      directory for the header file template that includes instructions.
*/

/*
   NAME
     cdemorid.h - <one-line expansion of the name>

   DESCRIPTION
     test using ROWID with INSERT, UPDATE, DELETE.

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
   emendez     09/13/00 - fix top 5 olint errors
   mjaeger     07/14/99 - bug 808870: OCCS: convert tabs, no long lines
   dchatter    10/16/98 - Creating new demo cdemorid
   dchatter    10/16/98 - Creation


*/

#ifndef CDEMORID
#define CDEMORID

/*------------------------------------------------------------------------
 * Include Files
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <oci.h>

#define MAXBINDS     2
#define MAXROWS      5
/*--------------------------------------------------------------------------
 * Static Function Declarations
 */

sword init_handles(/*_ OCIEnv **envhp,
                       OCIError **errhp,
                       ub4 init_mode _*/);

sword cleanup(/*_ boolean loggedon,
                  OCIEnv *envhp,
                  OCISvcCtx *svchp,
                  OCIError *errhp _*/);

void report_error(/*_ OCIError *errhp _*/);

void checkerr(/*_ OCIError *errhp,
                  sword status _*/);

sword get_all_rows(/*_ OCISvcCtx *svchp,
                       OCIError *errhp,
                       OCIStmt *select_p,
                       OCIRowid **Rowid _*/);

sword update_all_rows(/*_ OCISvcCtx *svchp,
                          OCIError *errhp,
                          OCIStmt *update_p,
                          OCIStmt *select_p,
                          OCIRowid **Rowid _*/);

int main(/*_ int argc, char *argv[] _*/);


#endif                                                           /* CDEMORID */







