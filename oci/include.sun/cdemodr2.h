/*
 * $Header: cdemodr2.h 14-jul-99.12:44:25 mjaeger Exp $
 */

/* Copyright (c) 1997, 1999, Oracle Corporation.  All rights reserved.
*/

/* NOTE:  See 'header_template.doc' in the 'doc' dve under the 'forms'
      directory for the header file template that includes instructions.
*/

/*
   NAME
     tkp8rv2.c - DML Returning Test 2.

   DESCRIPTION
     Demonstrate INSERT/UPDATE/DELETE statements RETURNING LOBS.

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
   svedala     10/01/98 - include stdlib.h - bug 714175
   azhao       06/03/97 - new file
   azhao       06/03/97 - Creation

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <oci.h>


#define MAXBINDS       25
#define MAXROWS         5           /* max no of rows returned per iter */
#define MAXCOLS         4
#define MAXITER        10           /* max no of iters in execute */
#define MAXBUFLEN      40

int main(/*_ int argc, char *argv[] _*/);
sword init_handles(/*_ OCIEnv **envhp, OCISvcCtx **svchp,
                               OCIError **errhp, OCIServer **svrhp,
                               OCISession **authp, ub4 mode _*/);

sword attach_server(/*_ ub4 mode, OCIServer *srvhp,
                        OCIError *errhp, OCISvcCtx *svchp _*/);
sword log_on(/*_ OCISession *authp, OCIError *errhp, OCISvcCtx *svchp,
                        text *uid, text *pwd, ub4 credt, ub4 mode _*/);
sword alloc_bind_handle(/*_ OCIStmt *stmthp, OCIBind *bndhp[],
                                   int nbinds _*/);
void print_raw(/*_ ub1 *raw, ub4 rawlen _*/);

void free_handles(/*_ OCIEnv *envhp, OCISvcCtx *svchp, OCIServer *srvhp,
                      OCIError *errhp, OCISession *authp, OCIStmt *stmthp _*/);
void report_error(/*_ OCIError *errhp _*/);
void logout_detach_server(/*_ OCISvcCtx *svchp, OCIServer *srvhp,
                              OCIError *errhp, OCISession *authp,
                              text *userid _*/);
sword finish_demo(/*_ boolean loggedon, OCIEnv *envhp, OCISvcCtx *svchp,
                      OCIServer *srvhp, OCIError *errhp, OCISession *authp,
                      OCIStmt *stmthp, text *userid _*/);
sword demo_insert(/*_ OCISvcCtx *svchp, OCIStmt *stmthp,
                              OCIBind *bndhp[], OCIError *errhp _*/);
sword demo_update(/*_ OCISvcCtx *svchp, OCIStmt *stmthp,
                              OCIBind *bndhp[], OCIError *errhp _*/);
sword demo_delete(/*_ OCISvcCtx *svchp, OCIStmt *stmthp,
                              OCIBind *bndhp[], OCIError *errhp _*/);
sword bind_name(/*_ OCIStmt *stmthp, OCIBind *bndhp[], OCIError *errhp _*/);
sword bind_in_name(/*_ OCIStmt *stmthp, OCIBind *bndhp[], OCIError *errhp _*/);
sword bind_low_high(/*_ OCIStmt *stmthp, OCIBind *bndhp[],
                        OCIError *errhp _*/);
sword bind_input(/*_ OCIStmt *stmthp, OCIBind *bndhp[], OCIError *errhp _*/);
sword bind_output(/*_ OCIStmt *stmthp, OCIBind *bndhp[], OCIError *errhp _*/);
sword bind_array(/*_ OCIBind *bndhp[], OCIError *errhp _*/);
sword bind_dynamic(/*_ OCIBind *bndhp[], OCIError *errhp _*/);
sb4 cbf_no_data(/*_ dvoid *ctxp, OCIBind *bindp, ub4 iter, ub4 index,
                 dvoid **bufpp, ub4 *alenpp, ub1 *piecep, dvoid **indpp _*/);
sb4 cbf_get_data(/*_ dvoid *ctxp, OCIBind *bindp, ub4 iter, ub4 index,
                             dvoid **bufpp, ub4 **alenpp, ub1 *piecep,
                             dvoid **indpp, ub2 **rcodepp _*/);
sword alloc_buffer(/*_ ub4 pos, ub4 iter, ub4 rows _*/);
sword read_piece(/*_ OCISvcCtx *svchp, OCILobLocator *lob, ub2 lobtype _*/);
sword write_piece(/*_ OCISvcCtx *svchp, OCILobLocator *lob, ub2 lobtype _*/);
sword check_lob(/*_ int iter, OCISvcCtx *svchp _*/);
sword locator_alloc(/*_ OCILobLocator *lob[], ub4 nlobs _*/);
sword locator_free(/*_ OCILobLocator *lob[], ub4 nlobs _*/);
sword select_locator(/*_ OCISvcCtx *svchp, OCIStmt *stmthp,
                                 OCIBind *bndhp[], OCIError *errhp _*/);

