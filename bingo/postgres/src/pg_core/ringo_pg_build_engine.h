#ifndef _RINGO_PG_BUILD_ENGINE_H__
#define	_RINGO_PG_BUILD_ENGINE_H__

#include "base_cpp/array.h"
#include "base_cpp/auto_ptr.h"

#include "bingo_pg_build_engine.h"
#include "bingo_postgres.h"

class BingoPgText;
class BingoPgIndex;
class BingoPgConfig;
class BingoPgFpData;

/*
 * Class for procession reaction fingerprint data
 */
class RingoPgBuildEngine : public BingoPgBuildEngine {
public:
   RingoPgBuildEngine(BingoPgConfig& bingo_config, const char* rel_name);
   virtual ~RingoPgBuildEngine();

   virtual bool processStructure(BingoPgText& struct_text, indigo::AutoPtr<BingoPgFpData>&);

   virtual int getFpSize();
   virtual int getType() const {return BINGO_INDEX_TYPE_REACTION;}

   virtual void prepareShadowInfo(const char* schema_name);
   virtual void insertShadowInfo(BingoPgFpData&);
   virtual void finishShadowProcessing();

private:
   RingoPgBuildEngine(const RingoPgBuildEngine&); // no implicit copy

   static void _errorHandler(const char* message, void* context);

   indigo::Array<char> _relName;
   indigo::Array<char> _shadowRelName;

   int _searchType;

};

#endif	/* RINGO_PG_BUILD_ENGINE_H */

