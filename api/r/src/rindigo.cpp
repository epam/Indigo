#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <fstream>

#include <R.h>
#include <Rdefines.h>
#include <Rinternals.h>

#include "indigo.h"


extern "C"
{
   SEXP version()
   {
      SEXP result = PROTECT(allocVector(STRSXP, 1));
      SET_STRING_ELT(result, 0, mkChar(indigoVersion()));
      UNPROTECT(1);
      return result;
   }

   SEXP canonicalSmiles(SEXP data)
   {
      SEXP result = PROTECT(allocVector(STRSXP, 1));
      int m = indigoLoadMoleculeFromString(CHAR(STRING_ELT(data, 0)));
      SET_STRING_ELT(result, 0, mkChar(indigoCanonicalSmiles(m)));
      indigoFree(m);
      UNPROTECT(1);
      return result;
   }

   SEXP checkSub(SEXP query, SEXP target, SEXP mode)
   {
      int q = indigoLoadQueryMoleculeFromString(CHAR(STRING_ELT(query, 0)));
      int t = indigoLoadMoleculeFromString(CHAR(STRING_ELT(target, 0)));
      int matcher = indigoSubstructureMatcher(t, CHAR(STRING_ELT(mode, 0)));
      SEXP smatch;
      int* match;
      PROTECT( smatch = NEW_INTEGER(1)) ;
      match = INTEGER(smatch);
      match[0] = indigoMatch(matcher, q);
      indigoFree(q);
      indigoFree(t);
      indigoFree(matcher);
      indigoFree(match[0]);
      UNPROTECT(1);
      return smatch;
   }

   SEXP fingerprint(SEXP item, SEXP mode)
   {
      SEXP result = PROTECT(allocVector(STRSXP, 1));
      int i = indigoLoadMoleculeFromString(CHAR(STRING_ELT(item, 0)));
      int fp = indigoFingerprint(i, CHAR(STRING_ELT(mode, 0)));
      SET_STRING_ELT(result, 0, mkChar(indigoOneBitsList(fp)));
      indigoFree(i);
      indigoFree(fp);
      UNPROTECT(1);
      return result;
   }
   
   SEXP fingerprintQuery(SEXP item, SEXP mode)
   {
      SEXP result = PROTECT(allocVector(STRSXP, 1));
      int i = indigoLoadQueryMoleculeFromString(CHAR(STRING_ELT(item, 0)));
      int fp = indigoFingerprint(i, CHAR(STRING_ELT(mode, 0)));
      SET_STRING_ELT(result, 0, mkChar(indigoOneBitsList(fp)));
      indigoFree(i);
      indigoFree(fp);
      UNPROTECT(1);
      return result;
   }

   SEXP molecularWeight(SEXP item)
   {
      SEXP result = PROTECT(allocVector(REALSXP, 1));
      int i = indigoLoadMoleculeFromString(CHAR(STRING_ELT(item, 0)));
      REAL(result)[0] = indigoMolecularWeight(i);
      indigoFree(i);
      UNPROTECT(1);
      return result;
   }
} // extern "C"
