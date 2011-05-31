#ifndef _BINGO_PG_TEXT_H__
#define	_BINGO_PG_TEXT_H__

#include <stdint.h>
#include "bingo_postgres.h"
#include "base_cpp/array.h"

class BingoPgText {
public:
   BingoPgText();
   BingoPgText(uintptr_t text_datum);
   ~BingoPgText();

   void clear();
   void init(uintptr_t text_datum);
   void initFromString(const char* str);
   void initFromArray(indigo::Array<char>& str);

   const char* getText(int& size);
   const char* getString();

   uintptr_t getDatum();
private:
   BingoPgText(const BingoPgText&); //no implicit copy

   PG_OBJECT _text;
   indigo::Array<char> _cstr;
};

#endif	/* BINGO_PG_TEXT_H */

