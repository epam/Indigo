#ifndef _BINGO_PG_COMMON_H__
#define	_BINGO_PG_COMMON_H__

#include <math.h>
#include "bingo_postgres.h"
#include "base_cpp/array.h"
#include "base_c/bitarray.h"
#include "base_cpp/auto_ptr.h"
#include "base_cpp/obj_array.h"
#include "base_cpp/output.h"
#include "base_cpp/scanner.h"
#include "base_cpp/tlscont.h"

namespace indigo {
   struct MoleculeFingerprintParameters;
}

class BingoPgExternalBitset;
class BingoPgBuffer;

class BingoPgCommon {
public:

   enum {
      MOL_SUB = 1,
      MOL_EXACT = 2,
      MOL_SMARTS = 3,
      MOL_GROSS = 4,
      MOL_MASS_LESS = 5,
      MOL_MASS_GREAT = 6,
      MOL_SIM = 7,
      MOL_MASS = 100/*pseudo types*/
   };

   static void getSearchTypeString(int, indigo::Array<char>& result);

//   static float getBingoSim(char*, int, char*, int);
   
//   static void formIndexTuple(BingoPgBuffer& buffer, void* map_data, int size);
//   static void* getIndexData(BingoPgBuffer& pg_buffer, int& data_len);

   static void printBitset(const char* name, BingoPgExternalBitset& bitset);
   static void printFPBitset(const char* name, unsigned char* bitset, int size);


//   static char* getTextData(PG_OBJECT text_datum, int& size);

   static void setDefaultOptions();
//   static dword getFunctionOid(const char* name, indigo::Array<dword>& types);
//   static dword getFunctionOid1(const char* name, dword type1);
   
//   static dword callFunction(dword oid, indigo::Array<dword>& args);
//   static dword callFunction1(dword oid, dword arg1);

   

//   static void executeQuery(const char* query_str);
   static int executeQuery(indigo::Array<char>& query_str);
   static int executeQuery(const char *format, ...);
   static bool tableExists(const char* table_name);

   static void createDependency(const char* child_table, const char* parent_table);
   static void dropDependency(const char* table_name);



   /*
    * This class allows to avoid save-load sequence errors
    * You can save or load structures by calling one method
    * Numbers, arrays, double arrays etc are can be handled by this helper
    */
   class DataProcessing {
   public:

      template <typename T>
      static void handleNumber(T& number, indigo::Scanner* scanner, indigo::Output* output) {
         _handleNumber(number, scanner, output);
      }

      template <typename T>
      static void handleArray(indigo::Array<T>& data, indigo::Scanner* scanner, indigo::Output* output) {
         int size = data.size();
         handleNumber(size, scanner, output);
         if (scanner)
            data.resize(size);
         for (int i = 0; i < size; ++i) {
            handleNumber(data[i], scanner, output);
         }
      }

      template <typename T>
      static void handleDArray(indigo::ObjArray< indigo::Array<T> >& data, indigo::Scanner* scanner, indigo::Output* output) {
         int size = data.size();
         handleNumber(size, scanner, output);
         if (scanner)
            data.resize(size);
         for (int i = 0; i < size; ++i) {
            handleArray(data[i], scanner, output);
         }
      }

      template <typename T>
      static void handleRedBlackString(indigo::RedBlackStringMap<T>& data, indigo::Scanner* scanner, indigo::Output* output) {
         indigo::Array<char> key_tmp;
         int size = data.size();
         handleNumber(size, scanner, output);
         if (scanner) {
            data.clear();
            for (int i = 0; i < size; ++i) {
               handleArray(key_tmp, scanner, output);
               T data_value;
               handleNumber(data_value, scanner, output);
               data.insert(key_tmp.ptr(), data_value);
            }
         }
         if (output) {
            for (int i = data.begin(); i != data.end(); i = data.next(i)) {
               key_tmp.readString(data.key(i), true);
               handleArray(key_tmp, scanner, output);
               handleNumber(data.value(i), scanner, output);
            }
         }
      }
      template <typename T, typename R>
      static void handleRedBlackArr(indigo::RedBlackObjMap<T, indigo::Array<R> >& data, indigo::Scanner* scanner, indigo::Output* output) {
         int size = data.size();
         handleNumber(size, scanner, output);
         if (scanner) {
            T key;
            data.clear();
            for (int i = 0; i < size; ++i) {
               handleNumber(key, scanner, output);
               handleArray(data.insert(key), scanner, output);
            }
         }
         if (output) {
            for (int i = data.begin(); i != data.end(); i = data.next(i)) {
               handleNumber(data.key(i), scanner, output);
               handleArray(data.value(i), scanner, output);
            }
         }
      }
      template <typename T>
      static void handleRedBlackStringArr(indigo::RedBlackStringObjMap< indigo::Array<T> >& data, indigo::Scanner* scanner, indigo::Output* output) {
         indigo::Array<char> key_tmp;
         int size = data.size();
         handleNumber(size, scanner, output);
         if (scanner) {
            data.clear();
            for (int i = 0; i < size; ++i) {
               handleArray(key_tmp, scanner, output);
               int key_idx = data.insert(key_tmp.ptr());
               handleArray(data.value(key_idx), scanner, output);
            }
         }
         if (output) {
            for (int i = data.begin(); i != data.end(); i = data.next(i)) {
               key_tmp.readString(data.key(i), true);
               handleArray(key_tmp, scanner, output);
               handleArray(data.value(i), scanner, output);
            }
         }
      }
      template <typename T, typename R>
      static void handleRedBlackObject(indigo::RedBlackObjMap<T, R>& data, indigo::Scanner* scanner, indigo::Output* output) {
         int size = data.size();
         handleNumber(size, scanner, output);
         if (scanner) {
            T key;
            data.clear();
            for (int i = 0; i < size; ++i) {
               handleNumber(key, scanner, output);
               data.insert(key).serialize(scanner, output);
            }
         }
         if (output) {
            for (int i = data.begin(); i != data.end(); i = data.next(i)) {
               handleNumber(data.key(i), scanner, output);
               data.value(i).serialize(scanner, output);
            }
         }
      }

   private:
      DataProcessing();
      DataProcessing(const DataProcessing&);

      static void _handleNumber(bool& number, indigo::Scanner* scanner, indigo::Output* output) {
         char b;
         if (scanner) {
            b = scanner->readChar();
            number = (b == 1);
         }
         if (output) {
            b = number ? 1 : 0;
            output->writeChar(b);
         }

      }
      static void _handleNumber(char& number, indigo::Scanner* scanner, indigo::Output* output) {
         if (scanner) {
            number = scanner->readChar();
         }
         if (output) {
            output->writeChar(number);
         }
      }
      static void _handleNumber(byte& number, indigo::Scanner* scanner, indigo::Output* output) {
         if (scanner) {
            number = scanner->readByte();
         }
         if (output) {
            output->writeByte(number);
         }
      }
      static void _handleNumber(int& number, indigo::Scanner* scanner, indigo::Output* output) {
         if (scanner) {
            number = scanner->readBinaryInt();
         }
         if (output) {
            output->writeBinaryInt(number);
      }
         }

      static void _handleNumber(float& number, indigo::Scanner* scanner, indigo::Output* output) {
         if (scanner) {
            number = scanner->readBinaryFloat();
         }
         if (output) {
            output->writeBinaryFloat(number);
         }
      }

      static void _handleNumber(double& number, indigo::Scanner* scanner, indigo::Output* output) {
         /*
          * Double workaround with string saving
          */
         indigo::Array<char> d_string;
         d_string.clear();
         if (scanner) {
            handleArray(d_string, scanner, output);
            number = atof(d_string.ptr());
         }
         if (output) {
            indigo::bprintf(d_string, "%.16e", number);
            handleArray(d_string, scanner, output);
         }
      }
   };

   static void convertTo(const indigo::Array<char>& value_str, float& val) {
      indigo::BufferScanner scanner(value_str);
      if (!scanner.tryReadFloat(val))
         throw Error("can not read float value in string %s\n", value_str.ptr());
   }

   static void convertTo(const indigo::Array<char>& value_str, bool& val) {
      if (strcasecmp("true", value_str.ptr()) == 0) {
         val = true;
      } else if (strcasecmp("false", value_str.ptr()) == 0) {
         val = false;
      } else {
         throw Error("unknown value '%s' expected boolean 'true' or 'false' ", value_str.ptr());
      }
   }

   static void convertTo(const indigo::Array<char>& value_str, int& val) {
      indigo::BufferScanner scanner(value_str);
      val = scanner.readInt();
   }

   static void convertTo(const indigo::Array<char>& value_str, indigo::Array<char>& val) {
      val.copy(value_str);
   }

   static void setDefaultValue(int& param) {
      param = -1;
   }

   static void setDefaultValue(bool& param) {
      param = false;
   }

   DEF_ERROR("common helpers");
private:
   BingoPgCommon();
   BingoPgCommon(const BingoPgCommon&); //no implicit copy


};

#endif	/* BINGO_PG_COMMON_H */

