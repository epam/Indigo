/****************************************************************************
 * Copyright (C) 2010-2013 GGA Software Services LLC
 *
 * This file is part of Indigo toolkit.
 *
 * This file may be distributed and/or modified under the terms of the
 * GNU General Public License version 3 as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPL included in the
 * packaging of this file.
 *
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 ***************************************************************************/


#include "bingo.h"
#include "bingo_object.h"

#include "indigo_internal.h"
#include "indigo_molecule.h"
#include "indigo_reaction.h"
#include "indigo_cpp.h"
#include "bingo_internal.h"

#include "bingo_index.h"
#include "bingo_lock.h"

#include <stdio.h>
#include <string>

#include "base_cpp/profiling.h"
#include "base_cpp/ptr_array.h"
#include "base_cpp/auto_ptr.h"
#include "base_cpp/exception.h"
#include "base_cpp/os_sync_wrapper.h"

using namespace indigo;
using namespace bingo;

// TODO: warning C4273: 'indigo::BingoException::BingoException' : inconsistent dll linkage
IMPL_EXCEPTION(indigo, BingoException, "bingo");

static PtrPool<Index> _bingo_instances;
static OsLock _bingo_lock;
static PtrArray<DatabaseLockData> _lockers;
static PtrPool<Matcher> _searches;
static OsLock _searches_lock;
static Array<int> _searches_db;

static int _bingoCreateOrLoadDatabaseFile (const char *location, const char *options, bool create, const char *type = 0)
{
   MoleculeFingerprintParameters fp_params;

   fp_params.ext = 0;
   fp_params.any_qwords = 15;
   fp_params.ord_qwords = 25;
   fp_params.tau_qwords = 0;
   fp_params.sim_qwords = 8;

   AutoPtr<Index> context;
   std::string loc_dir(location);

   if (loc_dir.find_last_of('/') != loc_dir.length() - 1)
      loc_dir += '/';

   BaseIndex::IndexType ind_type = BaseIndex::UNKNOWN;
   if (!create)
      ind_type = BaseIndex::determineType(location);
   else if (type && strcmp(type, "molecule") == 0)
      ind_type = BaseIndex::MOLECULE;
   else if (type && strcmp(type, "reaction") == 0)
      ind_type = BaseIndex::REACTION;

   if (ind_type == BaseIndex::MOLECULE)
      context.reset(new MoleculeIndex());
   else if (ind_type == BaseIndex::REACTION)
      context.reset(new ReactionIndex());
   else
      throw BingoException("Unknown database type");

   _bingo_lock.Lock();
   int db_id = _bingo_instances.add(0);
   _bingo_lock.Unlock();

   if (create)
      context->create(loc_dir.c_str(), fp_params, options, db_id);
   else
      context->load(loc_dir.c_str(), options, db_id);

   _bingo_lock.Lock();
   _bingo_instances[db_id] = context.release();
   AutoPtr<DatabaseLockData> locker_ptr;
   locker_ptr.reset(new DatabaseLockData());
   _lockers.expand(db_id + 1);
   _lockers[db_id] = locker_ptr.release();
   _bingo_lock.Unlock();

   return db_id;
}


static int _insertObjectToDatabase (int db, Indigo &self, Index &bingo_index, IndigoObject &indigo_obj, int obj_id)
{
   profTimerStart(t, "_insertObjectToDatabase");
   if (bingo_index.getType() == Index::MOLECULE)
   {

      profTimerStart(t1, "_preadd");
      if (!IndigoMolecule::is(indigo_obj))
         throw BingoException("bingoInsertRecordObj: Only molecule objects can be added to molecule index");

      indigo_obj.getBaseMolecule().aromatize(self.arom_options);

      IndexMolecule ind_mol(indigo_obj.getMolecule());
      profTimerStop(t1);

      int id = bingo_index.add(ind_mol, obj_id, *_lockers[db]);
      return id;
   }
   else if (bingo_index.getType() == Index::REACTION)
   {
      if (!IndigoReaction::is(indigo_obj))
         throw BingoException("bingoInsertRecordObj: Only reaction objects can be added to reaction index");

      indigo_obj.getBaseReaction().aromatize(self.arom_options);

      IndexReaction ind_rxn(indigo_obj.getReaction());

      int id = bingo_index.add(ind_rxn, obj_id, *_lockers[db]);
      return id;
   }
   else
      throw BingoException("bingoInsertRecordObj: Incorrect database");

   return -1;
}

Matcher& getMatcher (int id)
{
   if (id < _searches.begin() || id >= _searches.end() || !_searches.hasElement(id))
      throw BingoException("Incorrect search object id=%d", id);
   return *_searches[id];
}

CEXPORT const char * bingoVersion ()
{
   return BINGO_VERSION;
}

CEXPORT int bingoCreateDatabaseFile (const char *location, const char *type, const char *options)
{
   INDIGO_BEGIN
   {
      return _bingoCreateOrLoadDatabaseFile(location, options, true, type);
   }
   INDIGO_END(-1);
}

CEXPORT int bingoLoadDatabaseFile (const char *location, const char *options)
{
   INDIGO_BEGIN
   {
      return _bingoCreateOrLoadDatabaseFile(location, options, false);
   }
   INDIGO_END(-1);
}

CEXPORT int bingoCloseDatabase (int db)
{
   BINGO_BEGIN_DB(db)
   {
      _bingo_instances.remove(db);
      return 1;
   }
   BINGO_END(-1);
}

CEXPORT int bingoInsertRecordObj (int db, int obj)
{
   BINGO_BEGIN_DB(db)
   {
      IndigoObject &indigo_obj = self.getObject(obj);
      Index &bingo_index = _bingo_instances.ref(db);

      long obj_id = -1;
      RedBlackStringObjMap< Array<char> > *properties = indigo_obj.getProperties();

      if (properties != 0)
      {
         const char *key_name = bingo_index.getIdPropertyName();

         if (key_name != 0 && properties->find(key_name))
         {
            Array<char> &key_str = properties->at(key_name);
            obj_id = strtol(key_str.ptr(), NULL, 10);
         }
      }

      return _insertObjectToDatabase (db, self, bingo_index, indigo_obj, obj_id);
   }
   BINGO_END(-1);
}

CEXPORT int bingoInsertRecordObjWithId (int db, int obj, int id)
{
   BINGO_BEGIN_DB(db)
   {
      IndigoObject &indigo_obj = self.getObject(obj);
      Index &bingo_index = _bingo_instances.ref(db);

      return _insertObjectToDatabase (db, self, bingo_index, indigo_obj, id);
   }
   BINGO_END(-1);
}

CEXPORT int bingoDeleteRecord (int db, int id)
{
   BINGO_BEGIN_DB(db)
   {
      Index &bingo_index = _bingo_instances.ref(db);

      WriteLock wlock(*_lockers[db]);
      bingo_index.remove(id);


      return id;
   }
   BINGO_END(-1);
}

CEXPORT int bingoGetRecordObj (int db, int id)
{
   BINGO_BEGIN_DB(db)
   {
      Index &bingo_index = _bingo_instances.ref(db);

      ReadLock rlock(*_lockers[db]);

      int cf_len;
      const byte * cf_buf = bingo_index.getObjectCf(id, cf_len);
      int indigo_obj_id = -1;

      BufferScanner buf_scn(cf_buf, cf_len);

      if (bingo_index.getType() == Index::MOLECULE)
      {
         AutoPtr<IndigoMolecule> molptr(new IndigoMolecule());

         Molecule &mol = molptr->mol;
         CmfLoader cmf_loader(buf_scn);
         cmf_loader.loadMolecule(mol);

         indigo_obj_id = self.addObject(molptr.release());
      }
      else if (bingo_index.getType() == Index::REACTION)
      {
         AutoPtr<IndigoReaction> rxnptr(new IndigoReaction());

         Reaction &rxn = rxnptr->rxn;
         CrfLoader crf_loader(buf_scn);
         crf_loader.loadReaction(rxn);

         indigo_obj_id = self.addObject(rxnptr.release());
      }
      else
         throw BingoException("bingoInsertRecordObj: Incorrect database");

      return indigo_obj_id;
   }
   BINGO_END(-1);
}

CEXPORT int bingoOptimize (int db)
{
   BINGO_BEGIN_DB(db)
   {
      Index &bingo_index = _bingo_instances.ref(db);

      WriteLock wlock(*_lockers[db]);
      bingo_index.optimize();


      return 0;
   }
   BINGO_END(-1);
}

CEXPORT int bingoSearchSub (int db, int query_obj, const char *options)
{
   BINGO_BEGIN_DB(db)
   {
      IndigoObject &obj = *(self.getObject(query_obj).clone());

      if (IndigoQueryMolecule::is(obj))
      {
         obj.getBaseMolecule().aromatize(self.arom_options);

         AutoPtr<MoleculeSubstructureQueryData> query_data(new MoleculeSubstructureQueryData(obj.getQueryMolecule()));

         MoleculeIndex &bingo_index = dynamic_cast<MoleculeIndex &>(_bingo_instances.ref(db));
         MoleculeSubMatcher *matcher = dynamic_cast<MoleculeSubMatcher *>(bingo_index.createMatcher("sub", query_data.release(), options));

         _searches_lock.Lock();
         int search_id = _searches.add(matcher);
         _searches_db.expand(search_id + 1);
         _searches_db[search_id] = db;
         _searches_lock.Unlock();

         return search_id;
      }
      else if (IndigoQueryReaction::is(obj))
      {
         obj.getBaseReaction().aromatize(self.arom_options);

         AutoPtr<ReactionSubstructureQueryData> query_data(new ReactionSubstructureQueryData(obj.getQueryReaction()));

         ReactionIndex &bingo_index = dynamic_cast<ReactionIndex &>(_bingo_instances.ref(db));
         ReactionSubMatcher *matcher = dynamic_cast<ReactionSubMatcher *>(bingo_index.createMatcher("sub", query_data.release(), options));

         _searches_lock.Lock();
         int search_id = _searches.add(matcher);
         _searches_db.expand(search_id + 1);
         _searches_db[search_id] = db;
         _searches_lock.Unlock();

         return search_id;
      }
      else
         throw BingoException("bingoSearchSub: only query molecule and query reaction can be set as query object");
   }
   BINGO_END(-1);
}

CEXPORT int bingoSearchExact (int db, int query_obj, const char *options)
{
   BINGO_BEGIN_DB(db)
   {
      IndigoObject &obj = *(self.getObject(query_obj).clone());

      if (IndigoMolecule::is(obj))
      {
         obj.getBaseMolecule().aromatize(self.arom_options);

         AutoPtr<MoleculeExactQueryData> query_data(new MoleculeExactQueryData(obj.getMolecule()));

         MoleculeIndex &bingo_index = dynamic_cast<MoleculeIndex &>(_bingo_instances.ref(db));
         MolExactMatcher *matcher = dynamic_cast<MolExactMatcher *>(bingo_index.createMatcher("exact", query_data.release(), options));

         _searches_lock.Lock();
         int search_id = _searches.add(matcher);
         _searches_db.expand(search_id + 1);
         _searches_db[search_id] = db;
         _searches_lock.Unlock();

         return search_id;
      }
      else if (IndigoReaction::is(obj))
      {
         obj.getBaseReaction().aromatize(self.arom_options);

         AutoPtr<ReactionExactQueryData> query_data(new ReactionExactQueryData(obj.getReaction()));

         ReactionIndex &bingo_index = dynamic_cast<ReactionIndex &>(_bingo_instances.ref(db));
         RxnExactMatcher *matcher = dynamic_cast<RxnExactMatcher *>(bingo_index.createMatcher("exact", query_data.release(), options));

         _searches_lock.Lock();
         int search_id = _searches.add(matcher);
         _searches_db.expand(search_id + 1);
         _searches_db[search_id] = db;
         _searches_lock.Unlock();

         return search_id;
      }
      else
         throw BingoException("bingoSearchExact: only non-query molecules and reactions can be set as query object");
   }
   BINGO_END(-1);
}


CEXPORT int bingoSearchMolFormula (int db, const char *query, const char *options)
{
   BINGO_BEGIN_DB(db)
   {
      Array<char> gross_str;
      gross_str.copy(query, strlen(query) + 1);

      AutoPtr<GrossQueryData> query_data(new GrossQueryData(gross_str));

      BaseIndex &bingo_index = dynamic_cast<BaseIndex &>(_bingo_instances.ref(db));
      MolGrossMatcher *matcher = dynamic_cast<MolGrossMatcher *>(bingo_index.createMatcher("formula", query_data.release(), options));

      _searches_lock.Lock();
      int search_id = _searches.add(matcher);
      _searches_db.expand(search_id + 1);
      _searches_db[search_id] = db;
      _searches_lock.Unlock();

      return search_id;
   }
   BINGO_END(-1);
}

CEXPORT int bingoSearchSim (int db, int query_obj, float min, float max, const char *options)
{
   BINGO_BEGIN_DB(db)
   {
      IndigoObject &obj = *(self.getObject(query_obj).clone());

      if (IndigoMolecule::is(obj))
      {
         obj.getBaseMolecule().aromatize(self.arom_options);

         AutoPtr<MoleculeSimilarityQueryData> query_data(new MoleculeSimilarityQueryData(obj.getMolecule(), min, max));

         MoleculeIndex &bingo_index = dynamic_cast<MoleculeIndex &>(_bingo_instances.ref(db));
         MoleculeSimMatcher *matcher = dynamic_cast<MoleculeSimMatcher *>(bingo_index.createMatcher("sim", query_data.release(), options));

         _searches_lock.Lock();
         int search_id = _searches.add(matcher);
         _searches_db.expand(search_id + 1);
         _searches_db[search_id] = db;
         _searches_lock.Unlock();

         return search_id;
      }
      else if (IndigoReaction::is(obj))
      {
         obj.getBaseReaction().aromatize(self.arom_options);

         AutoPtr<ReactionSimilarityQueryData> query_data(new ReactionSimilarityQueryData(obj.getReaction(), min, max));

         ReactionIndex &bingo_index = dynamic_cast<ReactionIndex &>(_bingo_instances.ref(db));
         ReactionSimMatcher *matcher = dynamic_cast<ReactionSimMatcher *>(bingo_index.createMatcher("sim", query_data.release(), options));

         _searches_lock.Lock();
         int search_id = _searches.add(matcher);
         _searches_db.expand(search_id + 1);
         _searches_db[search_id] = db;
         _searches_lock.Unlock();

         return search_id;
      }
      else
         throw BingoException("bingoSearchSub: only query molecule and query reaction can be set as query object");
   }
   BINGO_END(-1);
}

CEXPORT int bingoEndSearch (int search_obj)
{
   BINGO_BEGIN_SEARCH(search_obj)
   {
      // Ensure that such matcher exists
      _searches_lock.Lock();

      getMatcher(search_obj);

      _searches.remove(search_obj);

      _searches_lock.Unlock();
      return 1;
   }
   BINGO_END(-1);
}

CEXPORT int bingoNext (int search_obj)
{
   BINGO_BEGIN_SEARCH(search_obj)
   {
      ReadLock rlock(*_lockers[ _searches_db[search_obj] ]);
      return getMatcher(search_obj).next();
   }
   BINGO_END(-1);
}

CEXPORT int bingoGetCurrentId (int search_obj)
{
   BINGO_BEGIN_SEARCH(search_obj)
   {
      return getMatcher(search_obj).currentId();
   }
   BINGO_END(-1);
}

CEXPORT float bingoGetCurrentSimilarityValue (int search_obj)
{
   BINGO_BEGIN_SEARCH(search_obj)
   {
      return getMatcher(search_obj).currentSimValue();
   }
   BINGO_END(-1);
}

CEXPORT int bingoEstimateRemainingResultsCount (int search_obj)
{
   BINGO_BEGIN_SEARCH(search_obj)
   {
      int delta;
      return getMatcher(search_obj).esimateRemainingResultsCount(delta);
   }
   BINGO_END(-1);
}

CEXPORT int bingoEstimateRemainingResultsCountError (int search_obj)
{
   BINGO_BEGIN_SEARCH(search_obj)
   {
      int delta;
      getMatcher(search_obj).esimateRemainingResultsCount(delta);
      return delta;
   }
   BINGO_END(-1);
}

CEXPORT int bingoEstimateRemainingTime (int search_obj, float *time_sec)
{
   BINGO_BEGIN_SEARCH(search_obj)
   {
      float delta;
      *time_sec = getMatcher(search_obj).esimateRemainingTime(delta);
      return 1;
   }
   BINGO_END(-1);
}


CEXPORT int bingoGetObject (int search_obj)
{
   BINGO_BEGIN_SEARCH(search_obj)
   {
      Matcher &matcher = getMatcher(search_obj);
      const Index &bingo_index = matcher.getIndex();

      return self.addObject(matcher.currentObject());
   }
   BINGO_END(-1);
}
