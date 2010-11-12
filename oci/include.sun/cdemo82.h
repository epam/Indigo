/*
 * $Header: cdemo82.h 14-jul-99.12:41:04 mjaeger Exp $
 */

/* Copyright (c) 1996, 1999, Oracle Corporation.  All rights reserved.
*/

/*
   NAME
     cdemo82.h - header file for oci adt sample program

   MODIFIED   (MM/DD/YY)
   mjaeger     07/14/99 - bug 808870: OCCS: convert tabs, no long lines
   echen       06/03/97 - fix name resolution problem
   azhao       01/30/97 - fix lint error
   echen       01/03/97 - remove obsoleve type
   azhao       07/18/96 - not to include s.h
   dchatter    07/18/96 - delete spurious .h files
   slari       07/15/96 - Creation

*/


#ifndef CDEMO82_ORACLE
# define CDEMO82_ORACLE

#ifndef OCI_ORACLE
#include <oci.h>
#endif

/*---------------------------------------------------------------------------
                     PUBLIC TYPES AND CONSTANTS
  ---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                     PRIVATE TYPES AND CONSTANTS
  ---------------------------------------------------------------------------*/
#define SERVER "ORACLE"
#define ADDRESS_TYPE_NAME "ADDRESS_OBJECT"
#define EMB_ADDRESS_TYPE_NAME "EMBEDDED_ADDRESS"
#define ADDREXT "ADDREXT"
#define EMBADDREXT "EMBADDREXT"
#define RETURN_ON_ERROR(error) if (error) return (error)
#define BIG_RECORD_SIZE 1000



struct address
{
  OCIString   *state;
  OCIString   *zip;
};
typedef struct address address;

struct null_address
{
  sb4    null_adt;
  sb4    null_state;
  sb4    null_zip;
};
typedef struct null_address null_address;

struct embaddress
{
  OCIString   *state;
  OCIString   *zip;
  OCIRef  *preaddrref;
};
typedef struct embaddress embaddress;


struct null_embaddress
{
  sb4     null_state;
  sb4     null_zip;
  sb4     null_preaddrref;
};
typedef struct null_embaddress null_embaddress;

struct person
{
  OCIString        *name;
  OCINumber           age;
  address          addr;
};
typedef struct person person;

struct null_person
{
  sb4              null_name;
  sb4              null_age;
  null_address     null_addr;
};

typedef struct null_person null_person;

static const text *const  names[] =
{(text *) "CUSTOMERVAL", (text *) "ADDRESS", (text *) "STATE"};

static const text *const  selvalstmt = (text *)
                     "SELECT custno, addr FROM customerval";

static const text *const  selobjstmt = (text *)
                     "SELECT custno, addr FROM customerobj";

static const text *const  selref = (text *)
                     "SELECT REF(e) from extaddr e";

static const text *const  deleteref = (text *)
                     "DELETE extaddr";

static const text *const  insertref = (text *)
"insert into extaddr values(address_object('CA', '98765'))";

static const text *const  modifyref = (text *)
"update extaddr set object_column = address_object('TX', '61111')";

static const text *const  selembref = (text *)
                     "SELECT REF(emb) from embextaddr emb";

static const text *const  bndref = (text *)
"update extaddr set object_column.state = 'GA' where object_column = :addrref";

static const text *const  insstmt =
(text *)"INSERT INTO customerval (custno, addr) values (:custno, :addr)";

dvoid *tmp;


/*---------------------------------------------------------------------------
                           PUBLIC FUNCTIONS
  ---------------------------------------------------------------------------*/
OCIRef *cbfunc(/*_ dvoid *context _*/);

/*---------------------------------------------------------------------------
                          PRIVATE FUNCTIONS
  ---------------------------------------------------------------------------*/
static void checkerr(/*_ OCIError *errhp, sword status _*/);
static void selectval(/*_ OCIEnv *envhp, OCISvcCtx *svchp,
                           OCIStmt *stmthp, OCIError *errhp _*/);
static void selectobj(/*_ OCIEnv *envhp, OCISvcCtx *svchp,
                           OCIStmt *stmthp, OCIError *errhp  _*/);
static void insert(/*_ OCIEnv *envhp, OCISvcCtx *svchp,
                        OCIStmt *stmthp, OCIError *errhp,
                        text *insstmt, ub2 nrows _*/);

static void pin_display_addr(/*_ OCIEnv *envhp, OCIError *errhp,
                                  OCIRef *addrref _*/);

int main(/*_ void _*/);



#endif                                              /* cdemo82 */
