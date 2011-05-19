/* 
 */

#ifndef _MANGO_PG_SEARCH_ENGINE_H__
#define	_MANGO_PG_SEARCH_ENGINE_H__

#include "bingo_pg_search_engine.h"

#include "base_cpp/array.h"
#include "base_cpp/auto_ptr.h"
#include "base_cpp/red_black.h"

#include "pg_bingo_context.h"
#include "bingo_postgres.h"
#include "bingo_pg_cursor.h"

class BingoPgText;
class BingoPgIndex;
class BingoPgConfig;

class MangoPgFpData : public BingoPgFpData {
public:

   MangoPgFpData():_mass(0), _fragments(0) {}
   virtual ~MangoPgFpData() {}

   void setMass(float mass) {_mass = mass;}
   float getMass() const {return _mass;}

   void insertHash(dword hash, int c_cnt);
   const indigo::RedBlackMap<dword, int>& getHashes() const {return _hashes;}

   void setGrossStr(const char* gross_str, const char* counter_str);
   const char* getGrossStr() const {return _gross.ptr();}

   int getFragmentsCount() const {return _fragments;}
   void setFragmentsCount(int fr) {_fragments = fr;}
private:
   MangoPgFpData(const MangoPgFpData&); //no implicit copy

   float _mass;
   int _fragments;
   /*
    * Map: hash - components count
    */
   indigo::RedBlackMap<dword, int> _hashes;
   indigo::Array<char> _gross;

};

/*
 * Class for procession molecule fingerprint data
 */
class MangoPgSearchEngine : public BingoPgSearchEngine {
public:
   MangoPgSearchEngine(BingoPgConfig& bingo_config, const char* rel_name);
   virtual ~MangoPgSearchEngine();

   virtual bool matchTarget(int section_idx, int structure_idx);
   virtual bool matchTarget(BingoItemData& item_data) {return BingoPgSearchEngine::matchTarget(item_data);}

   virtual int getFpSize();
   virtual int getType() const {return BINGO_INDEX_TYPE_MOLECULE;}

   virtual void prepareQuerySearch(BingoPgIndex&, PG_OBJECT scan_desc);
   virtual bool searchNext(PG_OBJECT result_ptr);

   virtual void loadDictionary(BingoPgIndex&);
   virtual const char* getDictionary(int& size);

private:
   MangoPgSearchEngine(const MangoPgSearchEngine&); // no implicit copy

   void _prepareExactQueryStrings(indigo::Array<char>& what_clause, indigo::Array<char>& from_clause, indigo::Array<char>& where_clause);
   void _prepareExactTauStrings(indigo::Array<char>& what_clause, indigo::Array<char>& from_clause, indigo::Array<char>& where_clause);

   void _prepareSubSearch(PG_OBJECT scan_desc);
   void _prepareExactSearch(PG_OBJECT scan_desc);
   void _prepareGrossSearch(PG_OBJECT scan_desc);
   void _prepareSmartsSearch(PG_OBJECT scan_desc);
   void _prepareMassSearch(PG_OBJECT scan_desc);
   void _prepareSimSearch(PG_OBJECT scan_desc);
   void _getScanQueries(dword arg_datum, BingoPgText& str1, BingoPgText& str2);
   void _getScanQueries(dword arg_datum, float& min_bound, float& max_bound, BingoPgText& str1, BingoPgText& str2);

   static void _errorHandler(const char* message, void* context);

   indigo::Array<char> _relName;
   indigo::Array<char> _shadowRelName;
   indigo::Array<char> _shadowHashRelName;

   int _searchType;

};
#endif	/* MANGO_PG_SEARCH_ENGINE_H */

