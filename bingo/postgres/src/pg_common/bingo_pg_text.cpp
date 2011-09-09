#include "bingo_pg_text.h"
#include "bingo_pg_index.h"

CEXPORT {
#include "postgres.h"
#include "fmgr.h"
#include "utils/builtins.h"
}

BingoPgText::BingoPgText():_text(0){
}
BingoPgText::BingoPgText(uintptr_t text_datum):_text(0) {
   init(text_datum);
}

BingoPgText::~BingoPgText() {
   clear();
}

void BingoPgText::init(uintptr_t text_datum) {
   clear();
   if (text_datum != 0) {
      _text = DatumGetTextPCopy(text_datum);
   }
}

void BingoPgText::initFromString(const char* str) {
   clear();
   _text = cstring_to_text(str);
}

void BingoPgText::initFromArray(indigo::Array<char>& str) {
   clear();
   _text = cstring_to_text_with_len(str.ptr(), str.sizeInBytes());
}

void BingoPgText::clear() {
   if(_text != 0)
      pfree(_text);
   _text = 0;
}
const char* BingoPgText::getText(int& size) {
   if(_text == 0) {
      size = 0;
      return 0;
   }
   text* t = (text*)_text;
   size = VARSIZE(t) - VARHDRSZ;
   return VARDATA(t);
}

const char* BingoPgText::getString() {
   if(_cstr.size() == 0) {
      int text_size;
      const char* text_data = getText(text_size);
      if(text_data == 0)
         return 0;
      _cstr.copy(text_data, text_size);
      _cstr.push(0);
   }
   return _cstr.ptr();
}

uintptr_t BingoPgText::getDatum() {
   return PointerGetDatum(_text);
}

PG_OBJECT BingoPgText::release() {
   PG_OBJECT res_text = _text;
   _text = 0;
   return res_text;
}