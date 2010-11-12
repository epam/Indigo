/* $RCSfile: oratypes.h $ $Date: 20-jul-00.13:44:19 
   ----------------------------------------------------------------------
   ORACLE, Copyright (c) 1982, 1983, 1986, 1990, 1995, 1998, 1999, 2000 
        Oracle Corporation

   ORACLE Utilities, Copyright (c) 1981, 1982, 1983, 1986, 1990,, 2000 1991,
        1995, 1998, 1999, 2000 Oracle Corporation
 
                       *** Restricted Rights ***

    This program is an unpublished work under the Copyright Act of the
    United States and is subject to the terms and conditions stated in
    your  license  agreement  with  ORACORP  including  retrictions on
    use, duplication, and disclosure.
 
    Certain uncopyrighted ideas and concepts are also contained herein.
    These are trade secrets of ORACORP and cannot be  used  except  in
    accordance with the written permission of ORACLE Corporation.
   ---------------------------------------------------------------------- */
 
#ifndef ORATYPES
# define ORATYPES
# define SX_ORACLE
# define SX3_ORACLE 

#ifndef ORASTDDEF
# include <stddef.h>
# define ORASTDDEF
#endif

#ifndef ORALIMITS
# include <limits.h>
# define ORALIMITS
#endif

#ifndef TRUE
# define TRUE  1
# define FALSE 0
#endif

#ifdef lint
# ifndef mips
#  define signed
# endif 
#endif 

#ifdef ENCORE_88K
# ifndef signed
#  define signed
# endif 
#endif 

#if defined(SYSV_386) || defined(SUN_OS)
# ifdef signed
#  undef signed
# endif 
# define signed
#endif 

/* --- Signed/Unsigned one-byte scalar (sb1/ub1) --- */

#ifndef lint
  typedef unsigned char ub1;
  typedef   signed char sb1;
  typedef          char eb1;
#else 
# define ub1 unsigned char
# define sb1 signed char
# define eb1 char
#endif

#define UB1MAXVAL    ((ub1)UCHAR_MAX)
#define UB1MINVAL    ((ub1)       0)
#define SB1MAXVAL    ((sb1)SCHAR_MAX)
#define SB1MINVAL    ((sb1)SCHAR_MIN)

#define MINUB1MAXVAL ((ub1)  255)
#define MAXUB1MINVAL ((ub1)    0)
#define MINSB1MAXVAL ((sb1)  127)
#define MAXSB1MINVAL ((sb1) -127)

#define EB1MAXVAL    ((eb1)SCHAR_MAX)
#define EB1MINVAL    ((eb1)        0)

#define MINEB1MAXVAL ((eb1)  127)
#define MAXEB1MINVAL ((eb1)    0)

#define UB1BITS      CHAR_BIT
#define UB1MASK      ((1 << ((uword)CHAR_BIT)) - 1)

/* backwards compatibility */

#ifndef lint
  typedef sb1 b1;
#else
# define b1 sb1
#endif

#define B1MAXVAL SB1MAXVAL
#define B1MINVAL SB1MINVAL

/* --- Signed/Unsigned two-byte scalar (sb2/ub2) --- */

#ifndef lint 
  typedef unsigned short ub2;
  typedef   signed short sb2;
  typedef          short eb2;
#else
# define ub2 unsigned short
# define sb2 signed short
# define eb2 short
#endif

#define UB2MAXVAL    ((ub2)USHRT_MAX)
#define UB2MINVAL    ((ub2)        0)
#define SB2MAXVAL    ((sb2) SHRT_MAX)
#define SB2MINVAL    ((sb2) SHRT_MIN)

#define MINUB2MAXVAL ((ub2) 65535)
#define MAXUB2MINVAL ((ub2)     0)
#define MINSB2MAXVAL ((sb2) 32767)
#define MAXSB2MINVAL ((sb2)-32767)

#define EB2MAXVAL    ((eb2) SHRT_MAX)
#define EB2MINVAL    ((eb2)        0)

#define MINEB2MAXVAL ((eb2) 32767)
#define MAXEB2MINVAL ((eb2)     0)

/* backwards compatibility */

#ifndef lint
  typedef sb2 b2;
#else
# define b2 sb2
#endif

#define B2MAXVAL SB2MAXVAL
#define B2MINVAL SB2MINVAL

/* --- Signed/Unsigned four-byte scalar (sb4/ub4) --- */

#ifndef lint 
  typedef unsigned int ub4;
  typedef   signed int sb4;
  typedef          int eb4;
#else
# define ub4 unsigned int
# define sb4 signed int
# define eb4 int
#endif

#define UB4MAXVAL    ((ub4)UINT_MAX)
#define UB4MINVAL    ((ub4)       0)
#define SB4MAXVAL    ((sb4) INT_MAX)
#define SB4MINVAL    ((sb4) INT_MIN)

#define MINUB4MAXVAL ((ub4) 4294967295)
#define MAXUB4MINVAL ((ub4)          0)
#define MINSB4MAXVAL ((sb4) 2147483647)
#define MAXSB4MINVAL ((sb4)-2147483647)

#define EB4MAXVAL    ((eb4) INT_MAX)
#define EB4MINVAL    ((eb4)       0)

#define MINEB4MAXVAL ((eb4) 2147483647)
#define MAXEB4MINVAL ((eb4)          0)

/* backwards compatibility */

#ifndef lint
  typedef sb4 b4;
#else
# define b4 sb4
#endif

#define B4MAXVAL SB4MAXVAL
#define B4MINVAL SB4MINVAL

/* --- Unsigned 8-byte scalar (ub8) --- */

#ifndef lint
# if (__STDC__ != 1)
#  define SLU8NATIVE
#  define SLS8NATIVE
# endif
#endif

#ifdef SLU8NATIVE

#ifdef SS_64BIT_SERVER
# ifndef lint
   typedef unsigned long ub8;
# else
#  define ub8 unsigned long
# endif 
#else
# ifndef lint
   typedef unsigned long long ub8;
# else
#  define ub8 unsigned long long
# endif 
#endif

#define UB8ZERO      ((ub8)0)

#define UB8MINVAL    ((ub8)0)
#define UB8MAXVAL    ((ub8)18446744073709551615)

#define MAXUB8MINVAL ((ub8)0)
#define MINUB8MAXVAL ((ub8)18446744073709551615)

#endif 

/* --- Signed 8-byte scalar (sb8) --- */

#ifdef SLS8NATIVE

#ifdef SS_64BIT_SERVER
# ifndef lint
   typedef signed long sb8;
# else
#  define sb8 signed long
# endif 
#else
# ifndef lint
   typedef signed long long sb8;
# else
#  define sb8 signed long long
# endif 
#endif

#define SB8ZERO      ((sb8)0)

#define SB8MINVAL    ((sb8)-9223372036854775808)
#define SB8MAXVAL    ((sb8) 9223372036854775807)

#define MAXSB8MINVAL ((sb8)-9223372036854775807)
#define MINSB8MAXVAL ((sb8) 9223372036854775807)

#endif 

/* --- Bits, masks, bit vectors --- */

typedef ub1   bitvec;    
#define BITVEC(n) (((n)+(UB1BITS-1))>>3)

#ifndef uiXT
  typedef  ub1 BITS8;
  typedef  ub2 BITS16;
  typedef  ub4 BITS32;
#endif

/* --- Character pointer --- */

#ifdef lint
# define oratext unsigned char
#else
  typedef unsigned char oratext;
#endif

#if !defined(LUSEMFC)
# ifdef lint
#  define text    unsigned char
#  define OraText oratext
# else
   typedef oratext text;
   typedef oratext OraText;
# endif
#endif

#if !defined(MOTIF) && !defined(LISPL)  && !defined(__cplusplus) && \
    !defined(LUSEMFC)
  typedef OraText *string;        
#endif 

#ifndef lint
  typedef unsigned short utext;
#else
# define utext  unsigned short
#endif

/* --- Boolean --- */

#ifndef boolean
# ifndef lint
   typedef int boolean;
# else
#  define boolean int
# endif
#endif

#ifndef lint
  typedef sb4 dboolean;
#else
# define dboolean sb4
#endif

/* --- Backwards compatibility scalars --- */

#ifndef lint
  typedef eb4     deword;
  typedef ub4     duword;
  typedef sb4     dsword;
  typedef dsword  dword;
#else
# define deword eb4
# define duword ub4
# define dsword sb4
# define dword  dsword
#endif

#define  DUWORDMAXVAL       UB4MAXVAL
#define  DUWORDMINVAL       UB4MINVAL
#define  DSWORDMAXVAL       SB4MAXVAL
#define  DSWORDMINVAL       SB4MINVAL

#define  MINDUWORDMAXVAL    MINUB4MAXVAL
#define  MAXDUWORDMINVAL    MAXUB4MINVAL
#define  MINDSWORDMAXVAL    MINSB4MAXVAL
#define  MAXDSWORDMINVAL    MAXSB4MINVAL

#define  DEWORDMAXVAL       EB4MAXVAL
#define  DEWORDMINVAL       EB4MINVAL
#define  MINDEWORDMAXVAL    MINEB4MAXVAL
#define  MAXDEWORDMINVAL    MAXEB4MINVAL

#define  DWORDMAXVAL        DSWORDMAXVAL
#define  DWORDMINVAL        DSWORDMINVAL

#ifndef lint
  typedef          int eword;
  typedef unsigned int uword;
  typedef   signed int sword;
#else
# define eword int
# define uword unsigned int
# define sword signed int
#endif 

#define  EWORDMAXVAL  ((eword) INT_MAX)
#define  EWORDMINVAL  ((eword)       0)

#define  UWORDMAXVAL  ((uword)UINT_MAX)
#define  UWORDMINVAL  ((uword)       0)

#define  SWORDMAXVAL  ((sword) INT_MAX)
#define  SWORDMINVAL  ((sword) INT_MIN)

#define  MINEWORDMAXVAL  ((eword)  2147483647)
#define  MAXEWORDMINVAL  ((eword)           0)

#define  MINUWORDMAXVAL  ((uword)  4294967295)
#define  MAXUWORDMINVAL  ((uword)           0)

#define  MINSWORDMAXVAL  ((sword)  2147483647)
#define  MAXSWORDMINVAL  ((sword) -2147483647)

#ifndef lint
  typedef unsigned long  ubig_ora;             
  typedef   signed long  sbig_ora;             
#else
# define ubig_ora unsigned long
# define sbig_ora signed long
#endif 

#define UBIG_ORAMAXVAL ((ubig_ora)ULONG_MAX)
#define UBIG_ORAMINVAL ((ubig_ora)        0)

#define SBIG_ORAMAXVAL ((sbig_ora) LONG_MAX)
#define SBIG_ORAMINVAL ((sbig_ora) LONG_MIN)

#define MINUBIG_ORAMAXVAL ((ubig_ora) 4294967295)
#define MAXUBIG_ORAMINVAL ((ubig_ora)          0)

#define MINSBIG_ORAMAXVAL ((sbig_ora) 2147483647)
#define MAXSBIG_ORAMINVAL ((sbig_ora)-2147483647)

#define UBIGORABITS      (UB1BITS * sizeof(ubig_ora))

/* --- Const --- */

#undef CONST
#define CONST const

/* --- Misc --- */

#ifndef lint
  typedef ub4 dsize_t;
#else
# define dsize_t ub4
#endif

#define DSIZE_TMAXVAL    UB4MAXVAL
#define MINDSIZE_TMAXVAL (dsize_t)65535

#ifndef lint
  typedef ub4 dptr_t;
#else
# define dptr_t ub4
#endif

#define M_IDEN 30

#ifdef AIXRIOS
# define SLMXFNMLEN 256
#else
# define SLMXFNMLEN 512
#endif 

#ifdef lint
# define dvoid void
#else
# ifdef UTS2
#  define dvoid char
# else
#  define dvoid void
# endif
#endif

typedef void (*lgenfp_t)( void );

#ifndef ORASYS_TYPES
# include <sys/types.h>
# define ORASYS_TYPES
#endif 

#ifndef SIZE_TMAXVAL
# define SIZE_TMAXVAL UBIG_ORAMAXVAL
#endif

#ifndef MINSIZE_TMAXVAL
# define MINSIZE_TMAXVAL (size_t)4294967295
#endif

#endif /* ORATYPES */
