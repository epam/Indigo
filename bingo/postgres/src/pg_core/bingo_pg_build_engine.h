#ifndef _BINGO_PG_BUILD_ENGINE_H__
#define	_BINGO_PG_BUILD_ENGINE_H__

extern "C" {
   #include "c.h"
   #include "storage/itemptr.h"
}

#ifdef qsort
#undef qsort
#endif
/*
 * Interface class for procession fingerprint data
 */

#include "bingo_postgres.h"
#include "base_cpp/auto_ptr.h"
#include "base_cpp/obj_array.h"

#include "bingo_pg_text.h"
#include "bingo_pg_search_engine.h"

//class BingoPgText;
class BingoPgIndex;
class BingoPgConfig;
//class BingoPgFpData;


class BingoPgBuildEngine {
public:
   class StructCache {
   public:
      StructCache(){}
      ~StructCache(){}
      indigo::AutoPtr<BingoPgText> text;
      indigo::AutoPtr<ItemPointerData> ptr;
      indigo::AutoPtr<BingoPgFpData> data;
   private:
      StructCache(const StructCache&); //no implicit copy
   };
   
   BingoPgBuildEngine();
   virtual ~BingoPgBuildEngine();

   virtual bool processStructure(BingoPgText& struct_text, indigo::AutoPtr<BingoPgFpData>&){return true;}
   virtual void processStructures(indigo::ObjArray<StructCache>& struct_cache){}

   virtual int getType() const {return 0;}
   virtual int getFpSize() {return 0;}

   virtual void prepareShadowInfo(const char* schema_name, const char* index_schema){}
   virtual void insertShadowInfo(BingoPgFpData&){}
   virtual void finishShadowProcessing(){}

   void loadDictionary(BingoPgIndex&);
   const char* getDictionary(int& size);

private:
   BingoPgBuildEngine(const BingoPgBuildEngine&); //no implicit copy
protected:
   void _setBingoContext();
   qword _bingoSession;
   BingoPgIndex* _bufferIndexPtr;
};



#endif	/* BINGO_PG_BUILD_ENGINE_H */

