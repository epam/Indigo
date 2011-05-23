/* 
 */

#ifndef _BINGO_PG_BUILD_H__
#define	_BINGO_PG_BUILD_H__


#include "base_c/defs.h"
#include "base_cpp/auto_ptr.h"
#include "base_cpp/exception.h"

#include "bingo_postgres.h"
#include "bingo_pg_index.h"
#include "bingo_pg_build_engine.h"

class BingoPgText;
class BingoPgConfig;

/*
 * Class for building and updating the bingo index
 */
class BingoPgBuild {
public:
   BingoPgBuild(PG_OBJECT index, const char* schema_name, bool new_index);
   ~BingoPgBuild();

   /*
    * Inserts a new structure into the index
    * Returns true if insertion was successfull
    */
   bool insertStructure(PG_OBJECT item_ptr, BingoPgText& struct_text);

   DEF_ERROR("BuildEngine");

private:
   BingoPgBuild(const BingoPgBuild&); //no implicit copy

   static void _errorHandler(const char* message, void* context);

   void _prepareBuilding(const char* schema_name);
   void _prepareUpdating();

   /*
    * Index relation
    */
   PG_OBJECT _index;

   /*
    * Buffers section handler
    */
   BingoPgIndex _bufferIndex;

   indigo::AutoPtr<BingoPgBuildEngine> fp_engine;

   /*
    * There are two possible uses - build(true) and update(false)
    */
   bool _buildingState;

};

#endif	/* BINGO_PG_BUILD_H */

