/*
 * $Header: cdemo6.h 08-aug-2001.16:30:28 slari Exp $
 */

/* Copyright (c) 1995, 2001, Oracle Corporation.  All rights reserved.  
*/

/*
   NAME
     cdemo6.h - header file for C++ Demo Program
   MODIFIED   (MM/DD/YY)
    slari      08/08/01  - b1737025: move extern declaration
    mjaeger    07/14/99 -  bug 808870: OCCS: convert tabs, no long lines
    slari      04/25/95 -  merge changes from branch 1.1.720.1
    slari      04/24/95 -  Branch_for_patch
    slari      04/21/95 -  Creation
*/


#include <string.h>
#include <oratypes.h>
#include <ocidfn.h>
#include <ocidem.h>

/* oparse flags */
#define  DEFER_PARSE        1
#define  NATIVE             1
#define  VERSION_7          2

/* Class forward declarations */
class connection;
class cursor;

/*
 * This class represents a connection to ORACLE database.
 *
 * NOTE: This connection class is just given as an example and all possible
 *       operations on a connection have not been defined.
 */
class connection
{
  friend class cursor;
  public:
    connection()
      { state = not_connected; memset(hda,'\0', HDA_SIZE); }
    ~connection();
    sword connect(const text *username, const text *password);
    sword disconnect();
    void display_error(FILE* file) const;
  private:
    Lda_Def lda;
    ub1 hda[HDA_SIZE];
    enum conn_state
    {
      not_connected,
      connected
    };
    conn_state state;
};

/*
 * This class represents an ORACLE cursor.
 *
 * NOTE: This cursor class is just given as an example and all possible
 *       operations on a cursor have not been defined.
 */
class cursor
{
  public:
    cursor()
      {state = not_opened; conn = (connection *)0; }
    ~cursor();
    sword open(connection *conn_param);
    sword close();
    sword parse(const text *stmt)
      { return (oparse(&cda, (text *)stmt, (sb4)-1,
                       DEFER_PARSE, (ub4) VERSION_7)); }
    /* bind an input variable */
    sword bind_by_position(sword sqlvnum, ub1 *progvar, sword progvarlen,
                           sword datatype, sword scale, sb2 *indicator)
      { return (obndrn(&cda, sqlvnum, progvar, progvarlen, datatype, scale,
                       indicator, (text *)0, -1, -1)); }
    /* define an output variable */
    sword define_by_position(sword position, ub1 *buf, sword bufl,
                             sword datatype, sword scale, sb2 *indicator,
                             ub2 *rlen, ub2 *rcode)
      { return (odefin(&cda, position, buf, bufl, datatype, scale, indicator,
                       (text *)0, -1, -1, rlen, rcode)); }
    sword describe(sword position, sb4 *dbsize, sb2 *dbtype, sb1 *cbuf,
                   sb4 *cbufl, sb4 *dsize, sb2 *prec, sb2 *scale, sb2 *nullok)
      { return (odescr(&cda, position, dbsize, dbtype, cbuf, cbufl, dsize,
                       prec, scale, nullok)); }
    sword execute()
      { return (oexec(&cda)); }
    sword fetch()
      { return (ofetch(&cda)); }
    sword get_error_code() const
      { return (cda.rc); }
    void display_error( FILE* file) const;
  private:
    Cda_Def cda;
    connection *conn;
    enum cursor_state
    {
      not_opened,
      opened
    };
    cursor_state state;
};

/*
 * Error number macros
 */
#define CONERR_ALRCON -1                                /* already connected */
#define CONERR_NOTCON -2                                    /* not connected */
#define CURERR_ALROPN -3                           /* cursor is already open */
#define CURERR_NOTOPN -4                             /* cursor is not opened */
