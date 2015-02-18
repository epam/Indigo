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
   static void _setStringToSTRSXP(SEXP *result, const char *str)
   {
      if (str != NULL) 
         SET_STRING_ELT(*result, 0, mkChar(str));
      else
         SET_STRING_ELT(*result, 0, NA_STRING);
   }
   
   SEXP r_indigoVersion()
   {
      SEXP result = PROTECT(allocVector(STRSXP, 1));
      _setStringToSTRSXP(&result, indigoVersion());
      UNPROTECT(1);
      return result;
   }
   
   SEXP r_indigoGetLastError()
   {
      SEXP result = PROTECT(allocVector(STRSXP, 1));
      _setStringToSTRSXP(&result, indigoGetLastError());
      UNPROTECT(1);
      return result;
   }
   
   SEXP r_indigoAllocSessionId()
   {
      SEXP result = PROTECT(allocVector(INTSXP, 1));
      INTEGER(result)[0] = indigoAllocSessionId();
      UNPROTECT(1);
      return result;
   }
      
   void r_indigoSetSessionId(SEXP id)
   {
      indigoSetSessionId(INTEGER(id)[0]);
   }
   
   void r_indigoReleaseSessionId(SEXP id)
   {
      indigoReleaseSessionId(INTEGER(id)[0]);
   }
   
   void r_indigoFree(SEXP obj_id)
   {
      indigoFree(INTEGER(obj_id)[0]);
   }

   SEXP r_indigoLoadMolecule(SEXP data)
   {
      SEXP result = PROTECT(allocVector(INTSXP, 1));
      INTEGER(result)[0] = indigoLoadMoleculeFromString(CHAR(STRING_ELT(data, 0)));
      UNPROTECT(1);
      return result;
   }
   
   SEXP r_indigoLoadQueryMolecule(SEXP data)
   {
      SEXP result = PROTECT(allocVector(INTSXP, 1));
      INTEGER(result)[0] = indigoLoadQueryMoleculeFromString(CHAR(STRING_ELT(data, 0)));
      UNPROTECT(1);
      return result;
   }
   
   SEXP r_indigoCanonicalSmiles(SEXP mol)
   {
      const char *con_smiles = indigoCanonicalSmiles(INTEGER(mol)[0]);
      
      SEXP result = PROTECT(allocVector(STRSXP, 1));
      _setStringToSTRSXP(&result, con_smiles);
         
      UNPROTECT(1);
      return result;
   }
   
   SEXP r_indigoFingerprint(SEXP mol, SEXP mode)
   {
      SEXP result = PROTECT(allocVector(INTSXP, 1));
      INTEGER(result)[0] = indigoFingerprint(INTEGER(mol)[0], CHAR(STRING_ELT(mode, 0)));
      UNPROTECT(1);
      return result;
   }
   
   SEXP r_indigoMolecularWeight(SEXP mol)
   {
      SEXP result = PROTECT(allocVector(REALSXP, 1));
      REAL(result)[0] = indigoMolecularWeight(INTEGER(mol)[0]);
      UNPROTECT(1);
      return result;
   }

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

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
      PROTECT( smatch = NEW_INTEGER(1)) ;
      
      int match_res = indigoMatch(matcher, q);
      
      INTEGER(smatch)[0] = match_res;
      
      indigoFree(q);
      indigoFree(t);
      indigoFree(matcher);
      indigoFree(match_res);
      
      UNPROTECT(1);
      return smatch;
   }
   
   const char * _makeOnesListStr(int mol, const char *mode)
   {
      if (mol == -1)
         return NULL;
     
      int fp = indigoFingerprint(mol, mode);
      if (fp == -1)
         return NULL;    
      
      const char *ones_list = indigoOneBitsList(fp);
      
      indigoFree(fp);
      return ones_list;
   }

   SEXP fingerprint(SEXP item, SEXP mode)
   {
      SEXP result = PROTECT(allocVector(STRSXP, 1));
      SET_STRING_ELT(result, 0, mkChar(""));
      
      int i = indigoLoadMoleculeFromString(CHAR(STRING_ELT(item, 0)));

      const char *ones_list =  _makeOnesListStr(i, CHAR(STRING_ELT(mode, 0)));

      if (ones_list != NULL)
         SET_STRING_ELT(result, 0, mkChar(ones_list));
               
      indigoFree(i);  
      UNPROTECT(1);
      
      return result;
   }
   
   SEXP fingerprintQuery(SEXP item, SEXP mode)
   {
      SEXP result = PROTECT(allocVector(STRSXP, 1));
      SET_STRING_ELT(result, 0, mkChar(""));
      
      int i = indigoLoadQueryMoleculeFromString(CHAR(STRING_ELT(item, 0)));

      const char *ones_list =  _makeOnesListStr(i, CHAR(STRING_ELT(mode, 0)));

      if (ones_list != NULL)
         SET_STRING_ELT(result, 0, mkChar(ones_list));
               
      indigoFree(i);  
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
