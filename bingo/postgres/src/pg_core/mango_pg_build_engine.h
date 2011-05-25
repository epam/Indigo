/* 
 */

#ifndef _MANGO_PG_BUILD_ENGINE_H__
#define	_MANGO_PG_BUILD_ENGINE_H__

#include "base_cpp/array.h"
#include "base_cpp/auto_ptr.h"

#include "bingo_pg_build_engine.h"
#include "bingo_postgres.h"

class BingoPgText;
class BingoPgIndex;
class BingoPgConfig;
class BingoPgFpData;

/*
 * Class for procession molecule fingerprint data
 */
class MangoPgBuildEngine : public BingoPgBuildEngine {
public:
   MangoPgBuildEngine(BingoPgConfig& bingo_config, const char* rel_name);
   virtual ~MangoPgBuildEngine();

   virtual bool processStructure(BingoPgText& struct_text, indigo::AutoPtr<BingoPgFpData>&);

   virtual int getFpSize();
   virtual int getType() const {return BINGO_INDEX_TYPE_MOLECULE;}

   virtual void prepareShadowInfo();
   virtual void insertShadowInfo(BingoPgFpData&);
   virtual void finishShadowProcessing();

private:
   MangoPgBuildEngine(const MangoPgBuildEngine&); // no implicit copy

   static void _errorHandler(const char* message, void* context);

   indigo::Array<char> _relName;
   indigo::Array<char> _shadowRelName;
   indigo::Array<char> _shadowHashRelName;

   int _searchType;

};

#endif	/* MANGO_PG_BUILD_ENGINE_H */

