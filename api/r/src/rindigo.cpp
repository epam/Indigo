#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <fstream>

#include <R.h>
#include <Rdefines.h>
#include <Rinternals.h>

#include "indigo.h"
#include "indigo-renderer.h"

#ifndef REXPORT_SYMBOL
   #ifdef _WIN32
      #define REXPORT_SYMBOL __declspec(dllexport)
   #else
      #define REXPORT_SYMBOL
   #endif
#endif

#ifndef REXPORT
   #ifndef __cplusplus
      #define REXPORT REXPORT_SYMBOL
   #else
      #define REXPORT extern "C" REXPORT_SYMBOL
   #endif
#endif


static void _setStringToSTRSXP(SEXP *result, const char *str)
{
   if (str != NULL) 
      SET_STRING_ELT(*result, 0, mkChar(str));
   else
      SET_STRING_ELT(*result, 0, NA_STRING);
}
   
REXPORT SEXP r_indigoVersion()
{
   SEXP result = PROTECT(allocVector(STRSXP, 1));
   _setStringToSTRSXP(&result, indigoVersion());
   UNPROTECT(1);
   return result;
}
   
REXPORT SEXP r_indigoGetLastError()
{
   SEXP result = PROTECT(allocVector(STRSXP, 1));
   _setStringToSTRSXP(&result, indigoGetLastError());
   UNPROTECT(1);
   return result;
}
   
REXPORT SEXP r_indigoAllocSessionId()
{
   SEXP result = PROTECT(allocVector(INTSXP, 1));
   INTEGER(result)[0] = indigoAllocSessionId();
   UNPROTECT(1);
   return result;
}
      
REXPORT void r_indigoSetSessionId(SEXP id)
{
   indigoSetSessionId(INTEGER(id)[0]);
}
   
REXPORT void r_indigoReleaseSessionId(SEXP id)
{
   indigoReleaseSessionId(INTEGER(id)[0]);
}
   
REXPORT void r_indigoFree(SEXP obj_id)
{
   indigoFree(INTEGER(obj_id)[0]);
}

REXPORT SEXP r_indigoLoadMolecule(SEXP data)
{
   SEXP result = PROTECT(allocVector(INTSXP, 1));
   INTEGER(result)[0] = indigoLoadMoleculeFromString(CHAR(STRING_ELT(data, 0)));
   UNPROTECT(1);
   return result;
}
   
REXPORT SEXP r_indigoLoadQueryMolecule(SEXP data)
{
   SEXP result = PROTECT(allocVector(INTSXP, 1));
   INTEGER(result)[0] = indigoLoadQueryMoleculeFromString(CHAR(STRING_ELT(data, 0)));
   UNPROTECT(1);
   return result;
}
   
REXPORT SEXP r_indigoCanonicalSmiles(SEXP mol)
{
   const char *con_smiles = indigoCanonicalSmiles(INTEGER(mol)[0]);
      
   SEXP result = PROTECT(allocVector(STRSXP, 1));
   _setStringToSTRSXP(&result, con_smiles);
         
   UNPROTECT(1);
   return result;
}
   
REXPORT SEXP r_indigoFingerprint(SEXP mol, SEXP mode)
{
   SEXP result = PROTECT(allocVector(INTSXP, 1));
   INTEGER(result)[0] = indigoFingerprint(INTEGER(mol)[0], CHAR(STRING_ELT(mode, 0)));
   UNPROTECT(1);
   return result;
}
   
REXPORT SEXP r_indigoMolecularWeight(SEXP mol)
{
   SEXP result = PROTECT(allocVector(REALSXP, 1));
   REAL(result)[0] = indigoMolecularWeight(INTEGER(mol)[0]);
   UNPROTECT(1);
   return result;
}
   
REXPORT SEXP r_indigoSetOption(SEXP option, SEXP value)
{
   SEXP result = PROTECT(allocVector(INTSXP, 1));
   SEXPTYPE val_type = TYPEOF(value);
      
   const char *opt_str = CHAR(STRING_ELT(option, 0));
   int set_result = -1;
      
   printf("val_type=%d\n", val_type);
      
   if (val_type == LGLSXP)
      set_result = indigoSetOptionBool(opt_str, LOGICAL(value)[0]);
   else if (val_type == INTSXP)
      set_result = indigoSetOptionInt(opt_str, INTEGER(value)[0]);
   else if (val_type == REALSXP)
      set_result = indigoSetOptionFloat(opt_str, REAL(value)[0]);
   else if (val_type == STRSXP)
      set_result = indigoSetOption(opt_str, CHAR(STRING_ELT(value, 0)));
      
   INTEGER(result)[0] = set_result;
   UNPROTECT(1);
   return result;
}

REXPORT SEXP r_indigoLayout(SEXP item)
{
   SEXP result = PROTECT(allocVector(INTSXP, 1));
   
   INTEGER(result)[0] = indigoLayout(INTEGER(item)[0]);
   UNPROTECT(1);

   return result;
}
   
REXPORT SEXP r_indigoAromatize(SEXP obj)
{
   SEXP result = PROTECT(allocVector(INTSXP, 1));
   INTEGER(result)[0] = indigoAromatize(INTEGER(obj)[0]);
   UNPROTECT(1);
   return result;
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

REXPORT SEXP canonicalSmiles(SEXP data)
{
   SEXP result = PROTECT(allocVector(STRSXP, 1));
      
   int m = indigoLoadMoleculeFromString(CHAR(STRING_ELT(data, 0)));
   _setStringToSTRSXP(&result, indigoCanonicalSmiles(m));
   indigoFree(m);
   UNPROTECT(1);
   return result;
}
   
REXPORT SEXP smiles(SEXP data)
{
   SEXP result = PROTECT(allocVector(STRSXP, 1));
      
   int m = indigoLoadMoleculeFromString(CHAR(STRING_ELT(data, 0)));
   _setStringToSTRSXP(&result, indigoSmiles(m));
   indigoFree(m);
   UNPROTECT(1);
   return result;
}
   
REXPORT void setFingerprintParams()
{
   indigoSetOptionInt("fp-ord-qwords", 25);
   indigoSetOptionInt("fp-sim-qwords", 8);
   indigoSetOptionInt("fp-any-qwords", 15);
   indigoSetOptionInt("fp-tau-qwords", 0);
   indigoSetOptionInt("fp-ext-enabled", 0);
}
   
REXPORT SEXP aromatize(SEXP mol)
{
   SEXP result = PROTECT(allocVector(STRSXP, 1));
      
   int m = indigoLoadMoleculeFromString(CHAR(STRING_ELT(mol, 0)));
   indigoAromatize(m);
   const char *ar_m =  indigoSmiles(m);
   _setStringToSTRSXP(&result, ar_m);
   indigoFree(m);
   UNPROTECT(1);
   return result;
}
   
REXPORT SEXP aromatizeQuery(SEXP mol)
{
   SEXP result = PROTECT(allocVector(STRSXP, 1));
      
   int m = indigoLoadQueryMoleculeFromString(CHAR(STRING_ELT(mol, 0)));
   indigoAromatize(m);
   const char *ar_m =  indigoSmiles(m);
   _setStringToSTRSXP(&result, ar_m);
   indigoFree(m);
   UNPROTECT(1);
   return result;
}

REXPORT SEXP checkSub(SEXP query, SEXP target, SEXP mode)
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

REXPORT SEXP checkSubBin(SEXP query, SEXP target, SEXP mode)
{
   int q = indigoLoadQueryMoleculeFromString(CHAR(STRING_ELT(query, 0)));
   int t = indigoUnserialize(RAW(VECTOR_ELT(target, 0)), GET_LENGTH(VECTOR_ELT(target, 0)));
   
   //printf("GET_LENGTH(VECTOR_ELT(target, 0)))=%d\n", GET_LENGTH(VECTOR_ELT(target, 0)));
   //printf("q=%d\n", q);
   //printf("t=%d\n", t);
   //printf("indigoSmiles(q)=%s\n", indigoSmiles(q));
   //printf("indigoSmiles(t)=%s\n", indigoSmiles(t));
   int matcher = indigoSubstructureMatcher(t, CHAR(STRING_ELT(mode, 0)));
   //printf("matcher=%d\n", matcher);
   SEXP smatch;
   PROTECT( smatch = NEW_INTEGER(1)) ;
      
   int match_res = indigoMatch(matcher, q);
   //printf("match_res=%d\n", match_res);
      
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

REXPORT SEXP fingerprint(SEXP item, SEXP mode)
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

REXPORT SEXP cmf(SEXP item)
{
   int i = indigoLoadMoleculeFromString(CHAR(STRING_ELT(item, 0)));

   byte* cmf_buf;
   int cmf_len;
   indigoSerialize(i, &cmf_buf, &cmf_len);
   SEXP result = PROTECT(allocVector(VECSXP, 1));
   SEXP result_el = PROTECT(allocVector(RAWSXP, cmf_len));
   if (cmf_buf != NULL) 
   {
      //RAW(result) = R_alloc(strlen(ones_list), sizeof(char));
      Rbyte *result_ptr = RAW(result_el);//(Rbyte *) DATAPTR(result);
      memcpy(result_ptr, cmf_buf, cmf_len);
      SET_VECTOR_ELT(result, 0, result_el);
   }

   indigoFree(i);  
   UNPROTECT(2);
      
   return result;
}
   
REXPORT SEXP fingerprintQuery(SEXP item, SEXP mode)
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

REXPORT SEXP molecularWeight(SEXP item)
{
   SEXP result = PROTECT(allocVector(REALSXP, 1));
   int i = indigoLoadMoleculeFromString(CHAR(STRING_ELT(item, 0)));
   REAL(result)[0] = indigoMolecularWeight(i);
   indigoFree(i);
   UNPROTECT(1);
   return result;
}

REXPORT SEXP render(SEXP item)
{
   SEXP result = PROTECT(allocVector(STRSXP, 1));
   
   char *raw_ptr;
   int size;
   
   indigoSetOption("render-output-format", "svg");
   //indigoSetOption("render-background-color", "255, 255, 255");
   
   int mol = indigoLoadMoleculeFromString(CHAR(STRING_ELT(item, 0)));
   indigoLayout(mol);
   int buffer_object = indigoWriteBuffer();
   int render_res = indigoRender(mol, buffer_object);
   const char *svg_str =indigoToString(buffer_object);
   
   SET_STRING_ELT(result, 0, mkChar(svg_str));
   indigoFree(buffer_object);
   indigoFree(mol);
   UNPROTECT(1);
   return result;
}

REXPORT SEXP renderBin(SEXP item)
{
   SEXP result = PROTECT(allocVector(STRSXP, 1));
   
   char *raw_ptr;
   int size;
   
   indigoSetOption("render-output-format", "svg");
   //indigoSetOption("render-background-color", "255, 255, 255");
   
   int mol = indigoUnserialize(RAW(VECTOR_ELT(item, 0)), GET_LENGTH(VECTOR_ELT(item, 0)));
   indigoLayout(mol);
   int buffer_object = indigoWriteBuffer();
   int render_res = indigoRender(mol, buffer_object);
   const char *svg_str =indigoToString(buffer_object);
   
   SET_STRING_ELT(result, 0, mkChar(svg_str));
   indigoFree(buffer_object);
   indigoFree(mol);
   UNPROTECT(1);
   return result;
}

REXPORT SEXP renderQuery(SEXP item)
{
   SEXP result = PROTECT(allocVector(STRSXP, 1));
   
   char *raw_ptr;
   int size;
   
   indigoSetOption("render-output-format", "svg");
   //indigoSetOption("render-background-color", "255, 255, 255");
   
   int mol = indigoLoadQueryMoleculeFromString(CHAR(STRING_ELT(item, 0)));
   indigoLayout(mol);
   int buffer_object = indigoWriteBuffer();
   int render_res = indigoRender(mol, buffer_object);
   const char *svg_str =indigoToString(buffer_object);
   
   SET_STRING_ELT(result, 0, mkChar(svg_str));
   indigoFree(buffer_object);
   indigoFree(mol);
   UNPROTECT(1);
   return result;
}

REXPORT SEXP renderHighlightedTarget(SEXP target, SEXP query) 
{
   SEXP result = PROTECT(allocVector(STRSXP, 1));
   
   char *raw_ptr;
   int size;
   
   indigoSetOption("render-output-format", "svg");
   //indigoSetOption("render-background-color", "255, 255, 255");
   
   int q = indigoLoadQueryMoleculeFromString(CHAR(STRING_ELT(query, 0)));
   int t = indigoLoadMoleculeFromString(CHAR(STRING_ELT(target, 0)));
   int m = indigoSubstructureMatcher(t, "");
   int res = 0;
   res = indigoMatch(m, q);

   int ht = indigoHighlightedTarget(res);
   
   indigoLayout(ht);
   int buffer_object = indigoWriteBuffer();
   int render_res = indigoRender(ht, buffer_object);
   const char *svg_str =indigoToString(buffer_object);
   
   SET_STRING_ELT(result, 0, mkChar(svg_str));
   indigoFree(buffer_object);
   indigoFree(ht);
   UNPROTECT(1);
   return result;
}

REXPORT SEXP renderHighlightedTargetBin(SEXP target, SEXP query) 
{
   SEXP result = PROTECT(allocVector(STRSXP, 1));
   
   char *raw_ptr;
   int size;
   
   indigoSetOption("render-output-format", "svg");
   //indigoSetOption("render-background-color", "255, 255, 255");
   
   int q = indigoLoadQueryMoleculeFromString(CHAR(STRING_ELT(query, 0)));
   int t = indigoUnserialize(RAW(VECTOR_ELT(target, 0)), GET_LENGTH(VECTOR_ELT(target, 0)));
   int m = indigoSubstructureMatcher(t, "");
   int res = 0;
   res = indigoMatch(m, q);

   int ht = indigoHighlightedTarget(res);
   
   indigoLayout(ht);
   int buffer_object = indigoWriteBuffer();
   int render_res = indigoRender(ht, buffer_object);
   const char *svg_str =indigoToString(buffer_object);
   
   SET_STRING_ELT(result, 0, mkChar(svg_str));
   indigoFree(buffer_object);
   indigoFree(ht);
   UNPROTECT(1);
   return result;
}

