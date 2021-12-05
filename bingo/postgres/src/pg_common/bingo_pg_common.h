#ifndef _BINGO_PG_COMMON_H__
#define	_BINGO_PG_COMMON_H__
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

#include <math.h>
#include "bingo_postgres.h"
#include "base_cpp/array.h"
#include "base_c/bitarray.h"
#include "bingo_core_c_internal.h"
#include <memory>
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
        REACT_SUB = 1,
        REACT_EXACT = 2,
        REACT_SMARTS = 3,
        MOL_MASS = 100/*pseudo types*/
    };

    static void getSearchTypeString(int, indigo::Array<char>& result, bool molecule);

//   static float getBingoSim(char*, int, char*, int);

//   static void formIndexTuple(BingoPgBuffer& buffer, void* map_data, int size);
//   static void* getIndexData(BingoPgBuffer& pg_buffer, int& data_len);

    static void printBitset(const char* name, BingoPgExternalBitset& bitset);
    static void printFPBitset(const char* name, unsigned char* bitset, int size);


//   static char* getTextData(PG_OBJECT text_datum, int& size);

    // static void setDefaultOptions(bingo_core::BingoCore& bingoCore);
//   static dword getFunctionOid(const char* name, indigo::Array<dword>& types);
//   static dword getFunctionOid1(const char* name, dword type1);

//   static dword callFunction(dword oid, indigo::Array<dword>& args);
//   static dword callFunction1(dword oid, dword arg1);



//   static void executeQuery(const char* query_str);
    static int executeQuery(indigo::Array<char>& query_str);
    static int executeQuery(const char *format, ...);
    static bool tableExists(const char* schema_name,const char* table_name);

    static void createDependency(const char* schema_name,const char* index_schema, const char* child_table, const char* parent_table);
    static void dropDependency(const char* schema_name, const char* index_schema, const char* table_name);
    static void appendPath(const char* schema_name);

    static char* releaseString(const char* str);



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
    class BingoSessionHandler {
    public:
        BingoSessionHandler(unsigned int func_id);
        virtual ~BingoSessionHandler();
        bingo_core::BingoCore bingoCore;

        const char* getFunctionName() const {
            return _functionName.size() ? _functionName.ptr() : 0;
        }

        void setFunctionName(const char* name) {
            _functionName.readString(name, true);
        }

        // void refresh();
    private:
        BingoSessionHandler(const BingoSessionHandler&); //no implicit copy
        // qword _sessionId;
        indigo::Array<char> _functionName;
        
        std::unique_ptr<BingoContext> _bingoContext;
        std::unique_ptr<MangoContext> _mangoContext;
        std::unique_ptr<RingoContext> _ringoContext;
    };

    DECL_ERROR;
private:
    BingoPgCommon();
    BingoPgCommon(const BingoPgCommon&); //no implicit copy


};
/*
 * Class for handling PG objects
 */
class BingoPgWrapper {
public:
    BingoPgWrapper();
    ~BingoPgWrapper();

    void clear();

    const char* getFuncNameSpace(dword oid_func);
    const char* getRelNameSpace(dword oid_rel);
    const char* getFuncName(dword oid_func);
    const char* getRelName(dword oid_rel);

private:
    BingoPgWrapper(const BingoPgWrapper&); //no implicit copy

    PG_OBJECT _ptr;
};

//class CancelException {
//public:
//   explicit CancelException(){}
//   virtual ~CancelException(){}
//   CancelException(const CancelException&){}
//private:
//};


#define BINGO_PG_TRY {\
   bool pg_error_raised = false; \
   QS_DEF(Array<char>, pg_message); \
   PG_TRY();

#define BINGO_PG_HANDLE(handle_statement) \
   PG_CATCH(); { \
      ErrorData *err = CopyErrorData(); \
      pg_message.readString(err->message, true); \
      FreeErrorData(err); \
      FlushErrorState(); \
      pg_error_raised = true; \
   } PG_END_TRY(); \
   if(pg_error_raised) { \
      const char* message = pg_message.ptr(); \
      handle_statement;\
   }}


#define PG_BINGO_BEGIN { \
   int pg_err_mess = 0; \
   bool pg_raise_error = false; \
   try

#define PG_BINGO_END \
   catch (indigo::Exception& e) { \
      pg_raise_error = true; \
      errstart(ERROR, __FILE__, __LINE__, PG_FUNCNAME_MACRO, TEXTDOMAIN); \
      pg_err_mess = errmsg("error: %s", e.message()); \
   } catch (...) { \
      pg_raise_error = true; \
      errstart(ERROR, __FILE__, __LINE__, PG_FUNCNAME_MACRO, TEXTDOMAIN); \
      pg_err_mess = errmsg("bingo unknown error"); \
   } \
   if (pg_raise_error) { \
      errfinish((errcode(ERRCODE_INTERNAL_ERROR), pg_err_mess)); \
   }}

#define PG_BINGO_HANDLE(statement) \
   catch (indigo::Exception& e) { \
      pg_raise_error = true; \
      errstart(ERROR, __FILE__, __LINE__, PG_FUNCNAME_MACRO, TEXTDOMAIN); \
      pg_err_mess = errmsg("error: %s", e.message()); \
   } catch (...) { \
      pg_raise_error = true; \
      errstart(ERROR, __FILE__, __LINE__, PG_FUNCNAME_MACRO, TEXTDOMAIN); \
      pg_err_mess = errmsg("bingo unknown error"); \
   } \
   if (pg_raise_error) { \
      statement; \
      errfinish((errcode(ERRCODE_INTERNAL_ERROR), pg_err_mess)); \
   }}

class DLLEXPORT BingoPgError : public indigo::Exception {
public:
    explicit BingoPgError (const char *format, ...) : indigo::Exception("bingo-postgres: ") {
        va_list args;
        va_start(args, format);
        const size_t len = strlen(_message);
        vsnprintf(_message + len, sizeof(_message) - len, format, args);
        va_end(args);
    }
};

#define CORE_CATCH_ERROR(suffix)\
   catch (indigo::Exception& e) { \
      throw BingoPgError("%s: %s", suffix, e.message());\
   } catch (...) { \
      throw BingoPgError("%s: bingo unknown error", suffix);\
   }

#define CORE_CATCH_ERROR_TID_NO_INDEX(suffix, block, offset)\
   catch (indigo::Exception& e) { \
      throw BingoPgError("%s with ctid='(%d,%d)'::tid: %s", suffix, block, offset, e.message());\
   } catch (...) { \
      throw BingoPgError("%s with ctid='(%d,%d)'::tid: bingo unknown error", suffix, block, offset);\
   }

#define CORE_CATCH_ERROR_TID(suffix, section_idx, structure_idx)\
   catch (indigo::Exception& e) { \
      ItemPointerData target_item;\
      _bufferIndexPtr->readTidItem(section_idx, structure_idx, &target_item);\
      int block_number = ItemPointerGetBlockNumber(&target_item);\
      int offset_number = ItemPointerGetOffsetNumber(&target_item);\
      throw BingoPgError("%s with ctid='(%d,%d)'::tid: %s", suffix, block_number, offset_number, e.message());\
   } catch (...) { \
      ItemPointerData target_item;\
      _bufferIndexPtr->readTidItem(section_idx, structure_idx, &target_item);\
      int block_number = ItemPointerGetBlockNumber(&target_item);\
      int offset_number = ItemPointerGetOffsetNumber(&target_item);\
      throw BingoPgError("%s with ctid='(%d,%d)'::tid: bingo unknown error", suffix, block_number, offset_number);\
   }

#define CORE_CATCH_WARNING_RETURN(suffix, return_value)\
   catch (indigo::Exception& e) { \
      elog(WARNING, "%s: %s", suffix, e.message());\
      return_value; \
   } catch (...) { \
      elog(WARNING, "%s: bingo unknown error", suffix);\
      return_value; \
   }

#define CORE_CATCH_WARNING(suffix)\
   catch (indigo::Exception& e) { \
      elog(WARNING, "%s: %s", suffix, e.message());\
   } catch (...) { \
      elog(WARNING, "%s: bingo unknown error", suffix);\
   }

#define CORE_CATCH_REJECT_WARNING(suffix, return_exp) \
   catch (indigo::Exception& e) { \
      int val = 0; \
      bingoCore,bingoGetConfigInt("reject_invalid_structures", &val); \
      if (val > 0) { \
         throw BingoPgError("%s: %s", suffix, e.message()); \
      } else { \
          elog(WARNING, "%s: %s", suffix, e.message());\
          return_exp; \
      } \
   } catch (...) { \
      int val = 0; \
      bingoCore,bingoGetConfigInt("reject_invalid_structures", &val); \
      if (val > 0) { \
         throw BingoPgError("%s: bingo unknown error", suffix); \
      } else { \
          elog(WARNING, "%s: bingo unknown error", suffix);\
          return_exp; \
      } \
   }

#define CORE_HANDLE_ERROR(res, success_res, suffix, message)\
   if (res < success_res) {\
      throw BingoPgError("%s: %s", suffix, message);\
   }

#define CORE_HANDLE_ERROR_TID_NO_INDEX(res, success_res, suffix, block, offset, message)\
   if (res < success_res) {\
      throw BingoPgError("%s with ctid='(%d,%d)'::tid: %s", suffix, block, offset, message);\
   }

#define CORE_HANDLE_ERROR_TID(res, success_res, suffix, section_idx, structure_idx, message)\
   if (res < success_res) {\
      ItemPointerData target_item;\
      _bufferIndexPtr->readTidItem(section_idx, structure_idx, &target_item);\
      int block_number = ItemPointerGetBlockNumber(&target_item);\
      int offset_number = ItemPointerGetOffsetNumber(&target_item);\
      throw BingoPgError("%s with ctid='(%d,%d)'::tid: %s", suffix, block_number, offset_number, message);\
   }


#define CORE_HANDLE_WARNING(res, success_res, suffix, message)\
   if (res < success_res) {\
      elog(WARNING, "%s: %s", suffix, message);\
   }

#define CORE_HANDLE_WARNING_TID_NO_INDEX(res, success_res, suffix, block, offset, message)\
   if (res < success_res) {\
      elog(WARNING, "%s with ctid='(%d,%d)'::tid: %s", suffix, block, offset, message);\
   }

#define CORE_RETURN_WARNING(res, success_res, suffix, message)\
   if (res < success_res) {\
      elog(WARNING, "%s: %s", suffix, message);\
      return false;\
   }

#define CORE_RETURN_WARNING_TID(res, success_res, suffix, section_idx, structure_idx, message)\
   if (res < success_res) {\
      ItemPointerData target_item;\
      _bufferIndexPtr->readTidItem(section_idx, structure_idx, &target_item);\
      int block_number = ItemPointerGetBlockNumber(&target_item);\
      int offset_number = ItemPointerGetOffsetNumber(&target_item);\
      elog(WARNING, "%s with ctid='(%d,%d)'::tid: %s", suffix, block_number, offset_number, message);\
      return false;\
   }


#endif	/* BINGO_PG_COMMON_H */
