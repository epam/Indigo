/*
 * $Header: ocikpr.h 04-dec-2000.17:39:01 porangas Exp $ 
 */

/* Copyright (c) 1991, 1995, 1996, 1998 by Oracle Corporation */
/*
   NAME
     ocikpr.h - header of K & R compilers
   MODIFIED   (MM/DD/YY)
    porangas   12/04/00 - Forward merge bug#974710 to 9i
    sgollapu   05/19/98 - Change text to OraText
    dchatter   04/21/96 -
    dchatter   11/10/95 -  add ognfd() - get native fd
    lchidamb   04/06/95 -  drop maxdsz from obindps/odefinps
    slari      04/07/95 -  add opinit
    dchatter   03/08/95 -  osetpi and ogetpi
    lchidamb   12/09/94 -  add obindps() and odefinps()
    dchatter   03/06/95 -  merge changes from branch 1.1.720.2
    dchatter   11/14/94 -  merge changes from branch 1.1.720.1
    dchatter   02/08/95 -  olog call; drop onblon
    dchatter   10/31/94 -  new functions for non-blocking oci
    rkooi2     11/27/92 -  Changing datatypes (in comments) and return types 
    rkooi2     10/26/92 -  More portability mods 
    rkooi2     10/18/92 -  Changed to agree with oci.c 
    sjain      03/16/92 -  Creation 
*/

/*
 *  Declare the OCI functions.
 *  Prototype information is commented out.
 *  Use this header for non-ANSI C compilers.
 *  Note that you will need to include ocidfn.h in the .c files
 *    to get the definition for cda_def.
 */

#ifndef OCIKPR
#define OCIKPR

#include <oratypes.h>

/*
 * Oci BIND (Piecewise or with Skips) 
 */
sword  obindps( /*_ struct cda_def *cursor, ub1 opcode, OraText *sqlvar, 
		  sb4 sqlvl, ub1 *pvctx, sb4 progvl, 
		  sword ftype, sword scale,
		  sb2 *indp, ub2 *alen, ub2 *arcode, 
		  sb4 pv_skip, sb4 ind_skip, sb4 alen_skip, sb4 rc_skip,
		  ub4 maxsiz, ub4 *cursiz, 
		  OraText *fmt, sb4 fmtl, sword fmtt _*/ );
sword  obreak( /*_ struct cda_def *lda _*/ );
sword  ocan  ( /*_ struct cda_def *cursor _*/ );
sword  oclose( /*_ struct cda_def *cursor _*/ );
sword  ocof  ( /*_ struct cda_def *lda _*/ );
sword  ocom  ( /*_ struct cda_def *lda _*/ );
sword  ocon  ( /*_ struct cda_def *lda _*/ );


/*
 * Oci DEFINe (Piecewise or with Skips) 
 */
sword  odefinps( /*_ struct cda_def *cursor, ub1 opcode, sword pos,ub1 *bufctx,
		   sb4 bufl, sword ftype, sword scale, 
		   sb2 *indp, OraText *fmt, sb4 fmtl, sword fmtt, 
		   ub2 *rlen, ub2 *rcode,
		   sb4 pv_skip, sb4 ind_skip, sb4 alen_skip, sb4 rc_skip _*/ );
sword  odescr( /*_ struct cda_def *cursor, sword pos, sb4 *dbsize,
                   sb2 *dbtype, sb1 *cbuf, sb4 *cbufl, sb4 *dsize,
                   sb2 *prec, sb2 *scale, sb2 *nullok _*/ );
sword  odessp( /*_ struct cda_def *cursor, OraText *objnam, size_t onlen,
                   ub1 *rsv1, size_t rsv1ln, ub1 *rsv2, size_t rsv2ln,
                   ub2 *ovrld, ub2 *pos, ub2 *level, OraText **argnam,
                   ub2 *arnlen, ub2 *dtype, ub1 *defsup, ub1* mode,
                   ub4 *dtsiz, sb2 *prec, sb2 *scale, ub1 *radix,
                   ub4 *spare, ub4 *arrsiz _*/ );
sword  oerhms( /*_ struct cda_def *lda, sb2 rcode, OraText *buf,
                   sword bufsiz _*/ );
sword  oermsg( /*_ sb2 rcode, OraText *buf _*/ );
sword  oexec ( /*_ struct cda_def *cursor _*/ );
sword  oexfet( /*_ struct cda_def *cursor, ub4 nrows,
                   sword cancel, sword exact _*/ );
sword  oexn  ( /*_ struct cda_def *cursor, sword iters, sword rowoff _*/ );
sword  ofen  ( /*_ struct cda_def *cursor, sword nrows _*/ );
sword  ofetch( /*_ struct cda_def *cursor _*/ );
sword  oflng ( /*_ struct cda_def *cursor, sword pos, ub1 *buf,
                   sb4 bufl, sword dtype, ub4 *retl, sb4 offset _*/ );
sword  ogetpi( /*_ struct cda_def *cursor, ub1 *piecep, dvoid **ctxpp, 
                   ub4 *iterp, ub4 *indexp _*/ );
sword  opinit( /*_ ub4 mode _*/ );
sword  olog  ( /*_ struct cda_def *lda, ub1 *hst, 
                   OraText *uid, sword uidl,
                   OraText *psw, sword pswl, 
                   OraText *conn, sword connl, 
                   ub4 mode _*/ );
sword  ologof( /*_ struct cda_def *lda _*/ );
sword  oopen ( /*_ struct cda_def *cursor, struct cda_def *lda,
                   OraText *dbn, sword dbnl, sword arsize,
                   OraText *uid, sword uidl _*/ );
sword  oopt  ( /*_ struct cda_def *cursor, sword rbopt, sword waitopt _*/ );
sword  oparse( /*_ struct cda_def *cursor, OraText *sqlstm, sb4 sqllen,
                   sword defflg, ub4 lngflg _*/ );
sword  orol  ( /*_ struct cda_def *lda _*/ );
sword  osetpi( /*_ struct cda_def *cursor, ub1 piece, dvoid *bufp, 
                   ub4 *lenp _*/ );
void sqlld2  ( /*_ struct cda_def *lda, OraText *cname, sb4 *cnlen _*/ );
void sqllda  ( /*_ struct cda_def *lda _*/ );

/* non-blocking functions */
sword onbset( /*_ struct cda_def *lda _*/ ); 
sword onbtst( /*_ struct cda_def *lda _*/ ); 
sword onbclr( /*_ struct cda_def *lda _*/ ); 
sword ognfd ( /*_ struct cda_def *lda, dvoid *fdp _*/ );



/* 
 * OBSOLETE FUNCTIONS 
 */

/* 
 * OBSOLETE BIND CALLS-- use obindps() 
 */
sword  obndra( /*_ struct cda_def *cursor, OraText *sqlvar, sword sqlvl,
                 ub1 *progv, sword progvl, sword ftype, sword scale,
                 sb2 *indp, ub2 *alen, ub2 *arcode, ub4 maxsiz,
                 ub4 *cursiz, OraText *fmt, sword fmtl, sword fmtt _*/ );
sword  obndrn( /*_ struct cda_def *cursor, sword sqlvn, ub1 *progv,
                 sword progvl, sword ftype, sword scale, sb2 *indp,
                 OraText *fmt, sword fmtl, sword fmtt _*/ );
sword  obndrv( /*_ struct cda_def *cursor, OraText *sqlvar, sword sqlvl,
                 ub1 *progv, sword progvl, sword ftype, sword scale,
                 sb2 *indp, OraText *fmt, sword fmtl, sword fmtt _*/ );

/* 
 * OBSOLETE DEFINE CALLS-- use odefinps() 
 */
sword  odefin( /*_ struct cda_def *cursor, sword pos, ub1 *buf,
                 sword bufl, sword ftype, sword scale, sb2 *indp,
                 OraText *fmt, sword fmtl, sword fmtt, ub2 *rlen, 
                 ub2 *rcode _*/ );


/* older calls ; preferred equivalent calls above */
sword  odsc  ( /*_ struct cda_def *cursor, sword pos, sb2 *dbsize,
                   sb2 *fsize, sb2 *rcode, sb2 *dtype, sb1 *buf,
                   sb2 *bufl, sb2 *dsize _*/ );
sword  oname ( /*_ struct cda_def *cursor, sword pos, sb1 *tbuf,
                   sb2 *tbufl, sb1 *buf, sb2 *bufl _*/ );
sword  olon  ( /*_ struct cda_def *lda, OraText *uid, sword uidl,
                   OraText *pswd, sword pswdl, sword audit _*/ );
sword  orlon ( /*_ struct cda_def *lda, ub1 *hda, OraText *uid,
                   sword uidl, OraText *pswd, sword pswdl, sword audit _*/ );
sword  osql3 ( /*_ struct cda_def *cda, OraText *sqlstm, sword sqllen _*/ );







#endif  /* OCIKPR */







