#ifndef _BINGO_PG_CONFIG_H__
#define	_BINGO_PG_CONFIG_H__

#include "bingo_postgres.h"
#include "base_cpp/red_black.h"
#include "base_cpp/exception.h"

namespace indigo {
   class Scanner;
   class Output;
}

class BingoPgConfig {
public:
   BingoPgConfig(){}
   ~BingoPgConfig(){}

   void readDefaultConfig(const char* schema_name);
   void updateByIndexConfig(PG_OBJECT index);
   void replaceInsertParameter(unsigned int name_datum, unsigned int value_datum);
   void setUpBingoConfiguration();

   void serialize(indigo::Array<char>& config_data);
   void deserialize(void* data, int data_len);
   
   DEF_ERROR("bingo postgres config");

private:
   BingoPgConfig(const BingoPgConfig&); //no implicit copy

   void _readTable(unsigned int id, bool tau);
   int _getNumericValue(int c_idx);

   void _replaceInsertTauParameter(unsigned int rule_datum, unsigned int beg_datum, unsigned int end_datum);
   void _toString(int value, indigo::Array<char>&);

   indigo::RedBlackStringObjMap< indigo::Array<char> > _rawConfig;
   
   class TauParameter{
   public:
      TauParameter(){};
      ~TauParameter(){};
      indigo::Array<char> beg;
      indigo::Array<char> end;
      void serialize(indigo::Scanner*, indigo::Output*);
   };

   indigo::RedBlackObjMap<int, TauParameter> _tauParameters;

};



#endif	/* BINGO_PG_CONFIG_H */

