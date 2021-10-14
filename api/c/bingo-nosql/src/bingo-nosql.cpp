/****************************************************************************
 * Copyright (C) from 2009 to Present EPAM Systems.
 *
 * This file is part of Indigo toolkit.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ***************************************************************************/

#include "bingo-nosql.h"

#include <cstdio>
#include <string>

#include "bingo_index.h"
#include "bingo_internal.h"
#include "indigo_internal.h"
#include "indigo_molecule.h"
#include "indigo_reaction.h"

//#define INDIGO_DEBUG

#ifdef INDIGO_DEBUG
#include <iostream>
#endif

using namespace indigo;
using namespace bingo;

// TODO: warning C4273: 'indigo::BingoException::BingoException' : inconsistent dll linkage
IMPL_EXCEPTION(indigo, BingoException, "bingo");

namespace
{
    struct SearchesData
    {
        PtrPool<Matcher> searches;
        Array<int> db;
    };

    using IndexHideObj = sf::safe_shared_hide_obj<std::unique_ptr<BaseIndex>>;

    class IndexPool
    {
    public:
        bool has(size_t id) const
        {
            return map.count(id) > 0;
        }

        IndexHideObj& at(size_t id)
        {
            return map.at(id);
        }

        const IndexHideObj& at(size_t id) const
        {
            return map.at(id);
        }

        size_t insert(IndexHideObj&& obj)
        {
            map[next_id] = std::move(obj);
            return next_id++;
        }

        void insert(size_t id, std::unique_ptr<BaseIndex>&& obj)
        {
            map[id] = std::move(IndexHideObj(std::move(obj)));
        }

        void remove(size_t id)
        {
            map.erase(id);
        }

        size_t getNextId()
        {
            return next_id++;
        }

    private:
        std::unordered_map<size_t, IndexHideObj> map;
        size_t next_id;
    };
}

static sf::safe_shared_hide_obj<IndexPool> _bingo_indexes;
static sf::safe_shared_hide_obj<SearchesData> _searches_data;

#include <iostream>

static int _bingoCreateOrLoadDatabaseFile(const char* location, const char* options, bool create, const char* type = 0)
{
    Indigo& self = indigoGetInstance();
    MoleculeFingerprintParameters fp_params(self.fp_params);

    /*
     * Since tautomer and resonance searches are not implemented yet,
     * tautomer and extended fingerprint parts are turned off
     *
     * Jira tickets:
     *    - https://jiraeu.epam.com/browse/IND-602
     *    - https://jiraeu.epam.com/browse/IND-603
     * */
    fp_params.ext = false;
    fp_params.tau_qwords = 0;

    std::string loc_dir(location);

    if (loc_dir.find_last_of('/') != loc_dir.length() - 1)
        loc_dir += '/';

    IndexType ind_type = IndexType::UNKNOWN;
    if (!create)
        ind_type = BaseIndex::determineType(location);
    else if (type && strcmp(type, "molecule") == 0)
        ind_type = IndexType::MOLECULE;
    else if (type && strcmp(type, "reaction") == 0)
        ind_type = IndexType::REACTION;

    std::unique_ptr<BaseIndex> context;
    if (ind_type == IndexType::MOLECULE)
        context = std::make_unique<MoleculeIndex>();
    else if (ind_type == IndexType::REACTION)
        context = std::make_unique<ReactionIndex>();
    else
        throw BingoException("Unknown database type");

    const auto db_id = sf::xlock_safe_ptr(_bingo_indexes)->getNextId();
    if (create)
    {
        context->create(loc_dir.c_str(), fp_params, options, db_id);
    }
    else
    {
        context->load(loc_dir.c_str(), options, db_id);
    }

    {
        auto bingo_indexes = sf::xlock_safe_ptr(_bingo_indexes);
        bingo_indexes->insert(db_id, std::move(context));
    }

#ifdef INDIGO_DEBUG
    std::stringstream ss;
    ss << "Bingo(" << db_id << ")";
    std::cout << ss.str() << std::endl;
#endif
    return db_id;
}

static int _insertObjectToDatabase(int db, Indigo& self, IndigoObject& indigo_obj, int obj_id)
{
    profTimerStart(t, "_insertObjectToDatabase");
    const IndexType index_type = [db]() {
        const auto bingo_indexes = sf::slock_safe_ptr(_bingo_indexes);
        auto bingo_index = sf::slock_safe_ptr(bingo_indexes->at(db));
        return (**bingo_index).getType();
    }();

    if (index_type == IndexType::MOLECULE)
    {
        profTimerStart(t1, "_preadd");

        if (!IndigoMolecule::is(indigo_obj))
        {
            throw BingoException("bingoInsertRecordObj: Only molecule objects can be added to molecule index");
        }
        // FIXME: MK: for some reason we need to aromatize input molecule. If we first clone and aromatize cloned, it won't work
        indigo_obj.getMolecule().aromatize(self.arom_options);
        IndexMolecule ind_mol(indigo_obj.getMolecule(), self.arom_options);
        profTimerStop(t1);

        const auto bingo_indexes = sf::slock_safe_ptr(_bingo_indexes);
        const auto obj_data = [&]() {
            const auto bingo_index_ptr = sf::slock_safe_ptr(bingo_indexes->at(db));
            return (*bingo_index_ptr)->prepareIndexData(ind_mol);
        }();
        {
            auto bingo_index_ptr = sf::xlock_safe_ptr(bingo_indexes->at(db));
            return (*bingo_index_ptr)->add(obj_id, obj_data);
        }
    }
    else if (index_type == IndexType::REACTION)
    {
        if (!IndigoReaction::is(indigo_obj))
        {
            throw BingoException("bingoInsertRecordObj: Only reaction objects can be added to reaction index");
        }

        indigo_obj.getReaction().aromatize(self.arom_options);
        IndexReaction ind_rxn(indigo_obj.getReaction(), self.arom_options);

        const auto bingo_indexes = sf::slock_safe_ptr(_bingo_indexes);
        const auto obj_data = [&]() {
            const auto bingo_index_ptr = sf::slock_safe_ptr(bingo_indexes->at(db));
            return (*bingo_index_ptr)->prepareIndexData(ind_rxn);
        }();
        {
            auto bingo_index_ptr = sf::xlock_safe_ptr(bingo_indexes->at(db));
            return (*bingo_index_ptr)->add(obj_id, obj_data);
        }
    }
    else
    {
        throw BingoException("bingoInsertRecordObj: Incorrect database");
    }
}

static int _insertObjectWithExtFPToDatabase(int db, Indigo& self, IndigoObject& indigo_obj, int obj_id, IndigoObject& fp)
{
    profTimerStart(t, "_insertObjectWithExtFPToDatabase");
    const IndexType index_type = [db]() {
        const auto bingo_indexes = sf::slock_safe_ptr(_bingo_indexes);
        const auto bingo_index_ptr = sf::slock_safe_ptr(bingo_indexes->at(db));
        return (*bingo_index_ptr)->getType();
    }();

    if (index_type == IndexType::MOLECULE)
    {
        profTimerStart(t1, "_preadd");
        if (!IndigoMolecule::is(indigo_obj))
        {
            throw BingoException("insertObjectWithExtFPToDatabase: Only molecule objects can be added to molecule index");
        }

        indigo_obj.getMolecule().aromatize(self.arom_options);
        IndexMolecule ind_mol(indigo_obj.getMolecule(), self.arom_options);
        profTimerStop(t1);

        const auto bingo_indexes = sf::slock_safe_ptr(_bingo_indexes);
        const auto obj_data = [&]() {
            const auto bingo_index_ptr = sf::slock_safe_ptr(bingo_indexes->at(db));
            return (*bingo_index_ptr)->prepareIndexDataWithExtFP(ind_mol, fp);
        }();
        {
            auto bingo_index_ptr = sf::xlock_safe_ptr(bingo_indexes->at(db));
            return (*bingo_index_ptr)->add(obj_id, obj_data);
        }
    }
    else if (index_type == IndexType::REACTION)
    {
        if (!IndigoReaction::is(indigo_obj))
        {
            throw BingoException("insertObjectWithExtFPToDatabase: Only reaction objects can be added to reaction index");
        }

        indigo_obj.getReaction().aromatize(self.arom_options);
        IndexReaction ind_rxn(indigo_obj.getReaction(), self.arom_options);

        const auto bingo_indexes = sf::slock_safe_ptr(_bingo_indexes);
        const auto obj_data = [&]() {
            const auto bingo_index_ptr = sf::slock_safe_ptr(bingo_indexes->at(db));
            return (*bingo_index_ptr)->prepareIndexDataWithExtFP(ind_rxn, fp);
        }();
        {
            auto bingo_index_ptr = sf::xlock_safe_ptr(bingo_indexes->at(db));
            return (*bingo_index_ptr)->add(obj_id, obj_data);
        }
    }
    else
    {
        throw BingoException("insertObjectWithExtFPToDatabase: Incorrect database");
    }
}

Matcher& getMatcher(int id)
{
    auto searches_data = sf::xlock_safe_ptr(_searches_data);
    if (id < searches_data->searches.begin() || id >= searches_data->searches.end() || !searches_data->searches.hasElement(id))
        throw BingoException("Incorrect search object id=%d", id);
    return *searches_data->searches[id];
}

CEXPORT const char* bingoVersion()
{
    return BINGO_VERSION;
}

CEXPORT int bingoCreateDatabaseFile(const char* location, const char* type, const char* options)
{
    INDIGO_BEGIN
    {
        return _bingoCreateOrLoadDatabaseFile(location, options, true, type);
    }
    INDIGO_END(-1);
}

CEXPORT int bingoLoadDatabaseFile(const char* location, const char* options)
{
    INDIGO_BEGIN
    {
        return _bingoCreateOrLoadDatabaseFile(location, options, false);
    }
    INDIGO_END(-1);
}

CEXPORT int bingoCloseDatabase(int db)
{
#ifdef INDIGO_DEBUG
    std::stringstream ss;
    ss << "~Bingo(" << db << ")";
    std::cout << ss.str() << std::endl;
#endif
    BINGO_BEGIN_DB_STATIC(db)
    {
        auto bingo_indexes = sf::xlock_safe_ptr(_bingo_indexes);
        bingo_indexes->remove(db);
        return 1;
    }
    BINGO_END(-1);
}

CEXPORT int bingoInsertRecordObj(int db, int obj)
{
    BINGO_BEGIN_DB(db)
    {
        IndigoObject& indigo_obj = self.getObject(obj);

        long obj_id = -1;
        auto& properties = indigo_obj.getProperties();

        const char* key_name = [db]() {
            const auto bingo_indexes = sf::slock_safe_ptr(_bingo_indexes);
            const auto bingo_index_ptr = sf::slock_safe_ptr(bingo_indexes->at(db));
            return (*bingo_index_ptr)->getIdPropertyName();
        }();

        if (key_name != nullptr && properties.contains(key_name))
        {
            obj_id = strtol(properties.at(key_name), NULL, 10);
        }

        return _insertObjectToDatabase(db, self, indigo_obj, obj_id);
    }
    BINGO_END(-1);
}

CEXPORT int bingoInsertRecordObjWithId(int db, int obj, int id)
{
    BINGO_BEGIN_DB(db)
    {
        IndigoObject& indigo_obj = self.getObject(obj);
        return _insertObjectToDatabase(db, self, indigo_obj, id);
    }
    BINGO_END(-1);
}

CEXPORT int bingoInsertRecordObjWithExtFP(int db, int obj, int fp)
{
    BINGO_BEGIN_DB(db)
    {
        IndigoObject& indigo_obj = self.getObject(obj);
        IndigoObject& ext_fp = self.getObject(fp);

        long obj_id = -1;
        auto& properties = indigo_obj.getProperties();

        const char* key_name = [db]() {
            const auto bingo_indexes = sf::slock_safe_ptr(_bingo_indexes);
            const auto bingo_index_ptr = sf::slock_safe_ptr(bingo_indexes->at(db));
            return (*bingo_index_ptr)->getIdPropertyName();
        }();

        if (key_name != 0 && properties.contains(key_name))
        {
            obj_id = strtol(properties.at(key_name), NULL, 10);
        }

        return _insertObjectWithExtFPToDatabase(db, self, indigo_obj, obj_id, ext_fp);
    }
    BINGO_END(-1);
}

CEXPORT int bingoInsertRecordObjWithIdAndExtFP(int db, int obj, int id, int fp)
{
    BINGO_BEGIN_DB(db)
    {
        IndigoObject& indigo_obj = self.getObject(obj);
        IndigoObject& ext_fp = self.getObject(fp);
        return _insertObjectWithExtFPToDatabase(db, self, indigo_obj, id, ext_fp);
    }
    BINGO_END(-1);
}

CEXPORT int bingoDeleteRecord(int db, int id)
{
    BINGO_BEGIN_DB(db)
    {
        const auto bingo_indexes = sf::slock_safe_ptr(_bingo_indexes);
        auto bingo_index_ptr = sf::xlock_safe_ptr(bingo_indexes->at(db));
        (*bingo_index_ptr)->remove(id);
        return id;
    }
    BINGO_END(-1);
}

CEXPORT int bingoGetRecordObj(int db, int id)
{
    BINGO_BEGIN_DB(db)
    {
        const auto bingo_indexes = sf::slock_safe_ptr(_bingo_indexes);
        const auto bingo_index_ptr = sf::slock_safe_ptr(bingo_indexes->at(db));
        const auto& bingo_index = *bingo_index_ptr;

        int cf_len;
        const byte* cf_buf = bingo_index->getObjectCf(id, cf_len);
        int indigo_obj_id = -1;

        BufferScanner buf_scn(cf_buf, cf_len);

        if (bingo_index->getType() == IndexType::MOLECULE)
        {
            std::unique_ptr<IndigoMolecule> molptr = std::make_unique<IndigoMolecule>();
            Molecule& mol = molptr->mol;
            CmfLoader cmf_loader(buf_scn);
            cmf_loader.loadMolecule(mol);
            indigo_obj_id = self.addObject(molptr.release());
        }
        else if (bingo_index->getType() == IndexType::REACTION)
        {
            std::unique_ptr<IndigoReaction> rxnptr = std::make_unique<IndigoReaction>();

            Reaction& rxn = rxnptr->rxn;
            CrfLoader crf_loader(buf_scn);
            crf_loader.loadReaction(rxn);

            indigo_obj_id = self.addObject(rxnptr.release());
        }
        else
        {
            throw BingoException("bingoInsertRecordObj: Incorrect database");
        }

        return indigo_obj_id;
    }
    BINGO_END(-1);
}

CEXPORT int bingoOptimize(int db)
{
    BINGO_BEGIN_DB(db)
    {
        const auto bingo_indexes = sf::slock_safe_ptr(_bingo_indexes);
        auto bingo_index_ptr = sf::xlock_safe_ptr(bingo_indexes->at(db));
        (*bingo_index_ptr)->optimize();
        return 0;
    }
    BINGO_END(-1);
}

CEXPORT int bingoSearchSub(int db, int query_obj, const char* options)
{
    BINGO_BEGIN_DB(db)
    {
        IndigoObject& obj = *(self.getObject(query_obj).clone());

        if (IndigoQueryMolecule::is(obj))
        {
            obj.getBaseMolecule().aromatize(self.arom_options);

            std::unique_ptr<MoleculeSubstructureQueryData> query_data = std::make_unique<MoleculeSubstructureQueryData>(obj.getQueryMolecule());

            MoleculeSubMatcher* matcher = [&]() {
                const auto bingo_indexes = sf::slock_safe_ptr(_bingo_indexes);
                const auto bingo_index_ptr = sf::slock_safe_ptr(bingo_indexes->at(db));
                return dynamic_cast<MoleculeSubMatcher*>((*bingo_index_ptr)->createMatcher("sub", query_data.release(), options));
            }();

            {
                auto searches_data = sf::xlock_safe_ptr(_searches_data);
                auto search_id = searches_data->searches.add(matcher);
                searches_data->db.expand(search_id + 1);
                searches_data->db[search_id] = db;
                return search_id;
            }
        }
        else if (IndigoQueryReaction::is(obj))
        {
            obj.getBaseReaction().aromatize(self.arom_options);

            std::unique_ptr<ReactionSubstructureQueryData> query_data = std::make_unique<ReactionSubstructureQueryData>(obj.getQueryReaction());

            ReactionSubMatcher* matcher = [&]() {
                const auto bingo_indexes = sf::slock_safe_ptr(_bingo_indexes);
                const auto bingo_index_ptr = sf::slock_safe_ptr(bingo_indexes->at(db));
                return dynamic_cast<ReactionSubMatcher*>((*bingo_index_ptr)->createMatcher("sub", query_data.release(), options));
            }();

            {
                auto searches_data = sf::xlock_safe_ptr(_searches_data);
                auto search_id = searches_data->searches.add(matcher);
                searches_data->db.expand(search_id + 1);
                searches_data->db[search_id] = db;
                return search_id;
            }
        }
        else
            throw BingoException("bingoSearchSub: only query molecule and query reaction can be set as query object");
    }
    BINGO_END(-1);
}
//
// CEXPORT int bingoSearchExact(int db, int query_obj, const char* options)
//{
//    BINGO_BEGIN_DB(db)
//    {
//        IndigoObject& obj = *(self.getObject(query_obj).clone());
//
//        if (IndigoMolecule::is(obj))
//        {
//            obj.getBaseMolecule().aromatize(self.arom_options);
//
//            std::unique_ptr<MoleculeExactQueryData> query_data = std::make_unique<MoleculeExactQueryData>(obj.getMolecule());
//
//            MoleculeIndex& bingo_index = dynamic_cast<MoleculeIndex&>(sf::xlock_safe_ptr(_bingo_indexes)->ref(db));
//            MolExactMatcher* matcher = dynamic_cast<MolExactMatcher*>(bingo_index.createMatcher("exact", query_data.release(), options));
//
//            int search_id;
//            {
//                auto searches_data = sf::xlock_safe_ptr(_searches_data);
//                search_id = searches_data->searches.add(matcher);
//                searches_data->db.expand(search_id + 1);
//                searches_data->db[search_id] = db;
//            }
//
//            return search_id;
//        }
//        else if (IndigoReaction::is(obj))
//        {
//            obj.getBaseReaction().aromatize(self.arom_options);
//
//            std::unique_ptr<ReactionExactQueryData> query_data = std::make_unique<ReactionExactQueryData>(obj.getReaction());
//
//            ReactionIndex& bingo_index = dynamic_cast<ReactionIndex&>(sf::xlock_safe_ptr(_bingo_indexes)->ref(db));
//            RxnExactMatcher* matcher = dynamic_cast<RxnExactMatcher*>(bingo_index.createMatcher("exact", query_data.release(), options));
//
//            int search_id;
//            {
//                auto searches_data = sf::xlock_safe_ptr(_searches_data);
//                search_id = searches_data->searches.add(matcher);
//                searches_data->db.expand(search_id + 1);
//                searches_data->db[search_id] = db;
//            }
//
//            return search_id;
//        }
//        else
//            throw BingoException("bingoSearchExact: only non-query molecules and reactions can be set as query object");
//    }
//    BINGO_END(-1);
//}
//
// CEXPORT int bingoSearchMolFormula(int db, const char* query, const char* options)
//{
//    BINGO_BEGIN_DB(db)
//    {
//        Array<char> gross_str;
//        gross_str.copy(query, (int)(strlen(query) + 1));
//
//        std::unique_ptr<GrossQueryData> query_data = std::make_unique<GrossQueryData>(gross_str);
//
//        BaseIndex& bingo_index = dynamic_cast<BaseIndex&>(sf::xlock_safe_ptr(_bingo_indexes)->ref(db));
//        MolGrossMatcher* matcher = dynamic_cast<MolGrossMatcher*>(bingo_index.createMatcher("formula", query_data.release(), options));
//
//        int search_id;
//        {
//            auto searches_data = sf::xlock_safe_ptr(_searches_data);
//            search_id = searches_data->searches.add(matcher);
//            searches_data->db.expand(search_id + 1);
//            searches_data->db[search_id] = db;
//        }
//
//        return search_id;
//    }
//    BINGO_END(-1);
//}
//
CEXPORT int bingoSearchSim(int db, int query_obj, float min, float max, const char* options)
{
    BINGO_BEGIN_DB(db)
    {
        IndigoObject& obj = *(self.getObject(query_obj).clone());

        if (IndigoMolecule::is(obj))
        {
            obj.getBaseMolecule().aromatize(self.arom_options);

            std::unique_ptr<MoleculeSimilarityQueryData> query_data = std::make_unique<MoleculeSimilarityQueryData>(obj.getMolecule(), min, max);

            MoleculeSimMatcher* matcher = [&]() {
                const auto bingo_indexes = sf::slock_safe_ptr(_bingo_indexes);
                const auto bingo_index_ptr = sf::slock_safe_ptr(bingo_indexes->at(db));
                return dynamic_cast<MoleculeSimMatcher*>((*bingo_index_ptr)->createMatcher("sim", query_data.release(), options));
            }();

            {
                auto searches_data = sf::xlock_safe_ptr(_searches_data);
                auto search_id = searches_data->searches.add(matcher);
                searches_data->db.expand(search_id + 1);
                searches_data->db[search_id] = db;
                return search_id;
            }
        }
        else if (IndigoReaction::is(obj))
        {
            obj.getBaseReaction().aromatize(self.arom_options);

            std::unique_ptr<ReactionSimilarityQueryData> query_data = std::make_unique<ReactionSimilarityQueryData>(obj.getReaction(), min, max);

            ReactionSimMatcher* matcher = [&]() {
                const auto bingo_indexes = sf::slock_safe_ptr(_bingo_indexes);
                const auto bingo_index_ptr = sf::slock_safe_ptr(bingo_indexes->at(db));
                return dynamic_cast<ReactionSimMatcher*>((*bingo_index_ptr)->createMatcher("sim", query_data.release(), options));
            }();

            {
                auto searches_data = sf::xlock_safe_ptr(_searches_data);
                auto search_id = searches_data->searches.add(matcher);
                searches_data->db.expand(search_id + 1);
                searches_data->db[search_id] = db;
                return search_id;
            }
        }
        else
            throw BingoException("bingoSearchSim: only query molecule and query reaction can be set as query object");
    }
    BINGO_END(-1);
}

//
// CEXPORT int bingoSearchSimWithExtFP(int db, int query_obj, float min, float max, int fp, const char* options)
//{
//    BINGO_BEGIN_DB(db)
//    {
//        IndigoObject& obj = *(self.getObject(query_obj).clone());
//        IndigoObject& ext_fp = self.getObject(fp);
//
//        if (IndigoMolecule::is(obj))
//        {
//            obj.getBaseMolecule().aromatize(self.arom_options);
//
//            std::unique_ptr<MoleculeSimilarityQueryData> query_data = std::make_unique<MoleculeSimilarityQueryData>(obj.getMolecule(), min, max);
//
//            MoleculeIndex& bingo_index = dynamic_cast<MoleculeIndex&>(sf::xlock_safe_ptr(_bingo_indexes)->ref(db));
//            MoleculeSimMatcher* matcher = dynamic_cast<MoleculeSimMatcher*>(bingo_index.createMatcherWithExtFP("sim", query_data.release(), options, ext_fp));
//
//            int search_id;
//            {
//                auto searches_data = sf::xlock_safe_ptr(_searches_data);
//                search_id = searches_data->searches.add(matcher);
//                searches_data->db.expand(search_id + 1);
//                searches_data->db[search_id] = db;
//            }
//
//            return search_id;
//        }
//        else if (IndigoReaction::is(obj))
//        {
//            obj.getBaseReaction().aromatize(self.arom_options);
//
//            std::unique_ptr<ReactionSimilarityQueryData> query_data = std::make_unique<ReactionSimilarityQueryData>(obj.getReaction(), min, max);
//
//            ReactionIndex& bingo_index = dynamic_cast<ReactionIndex&>(sf::xlock_safe_ptr(_bingo_indexes)->ref(db));
//            ReactionSimMatcher* matcher = dynamic_cast<ReactionSimMatcher*>(bingo_index.createMatcherWithExtFP("sim", query_data.release(), options, ext_fp));
//
//            int search_id;
//            {
//                auto searches_data = sf::xlock_safe_ptr(_searches_data);
//                search_id = searches_data->searches.add(matcher);
//                searches_data->db.expand(search_id + 1);
//                searches_data->db[search_id] = db;
//            }
//
//            return search_id;
//        }
//        else
//            throw BingoException("bingoSearchSim: only query molecule and query reaction can be set as query object");
//    }
//    BINGO_END(-1);
//}
//
// CEXPORT int bingoSearchSimTopN(int db, int query_obj, int limit, float min, const char* options)
//{
//    BINGO_BEGIN_DB(db)
//    {
//        IndigoObject& obj = *(self.getObject(query_obj).clone());
//
//        if (IndigoMolecule::is(obj))
//        {
//            obj.getBaseMolecule().aromatize(self.arom_options);
//
//            std::unique_ptr<MoleculeSimilarityQueryData> query_data = std::make_unique<MoleculeSimilarityQueryData>(obj.getMolecule(), min, 1.0);
//
//            MoleculeIndex& bingo_index = dynamic_cast<MoleculeIndex&>(sf::xlock_safe_ptr(_bingo_indexes)->ref(db));
//            MoleculeTopNSimMatcher* matcher = dynamic_cast<MoleculeTopNSimMatcher*>(bingo_index.createMatcherTopN("sim", query_data.release(), options,
//            limit));
//
//            int search_id;
//            {
//                auto searches_data = sf::xlock_safe_ptr(_searches_data);
//                search_id = searches_data->searches.add(matcher);
//                searches_data->db.expand(search_id + 1);
//                searches_data->db[search_id] = db;
//            }
//
//            return search_id;
//        }
//        else if (IndigoReaction::is(obj))
//        {
//            obj.getBaseReaction().aromatize(self.arom_options);
//
//            std::unique_ptr<ReactionSimilarityQueryData> query_data = std::make_unique<ReactionSimilarityQueryData>(obj.getReaction(), min, 1.0);
//
//            ReactionIndex& bingo_index = dynamic_cast<ReactionIndex&>(sf::xlock_safe_ptr(_bingo_indexes)->ref(db));
//            ReactionTopNSimMatcher* matcher = dynamic_cast<ReactionTopNSimMatcher*>(bingo_index.createMatcherTopN("sim", query_data.release(), options,
//            limit));
//
//            int search_id;
//            {
//                auto searches_data = sf::xlock_safe_ptr(_searches_data);
//                search_id = searches_data->searches.add(matcher);
//                searches_data->db.expand(search_id + 1);
//                searches_data->db[search_id] = db;
//            }
//
//            return search_id;
//        }
//        else
//            throw BingoException("bingoSearchSimTopN: only query molecule and query reaction can be set as query object");
//    }
//    BINGO_END(-1);
//}
//
// CEXPORT int bingoSearchSimTopNWithExtFP(int db, int query_obj, int limit, float min, int fp, const char* options)
//{
//    BINGO_BEGIN_DB(db)
//    {
//        IndigoObject& obj = *(self.getObject(query_obj).clone());
//        IndigoObject& ext_fp = self.getObject(fp);
//
//        if (IndigoMolecule::is(obj))
//        {
//            obj.getBaseMolecule().aromatize(self.arom_options);
//
//            std::unique_ptr<MoleculeSimilarityQueryData> query_data = std::make_unique<MoleculeSimilarityQueryData>(obj.getMolecule(), min, 1.0);
//
//            MoleculeIndex& bingo_index = dynamic_cast<MoleculeIndex&>(sf::xlock_safe_ptr(_bingo_indexes)->ref(db));
//            MoleculeTopNSimMatcher* matcher =
//                dynamic_cast<MoleculeTopNSimMatcher*>(bingo_index.createMatcherTopNWithExtFP("sim", query_data.release(), options, limit, ext_fp));
//
//            int search_id;
//            {
//                auto searches_data = sf::xlock_safe_ptr(_searches_data);
//                search_id = searches_data->searches.add(matcher);
//                searches_data->db.expand(search_id + 1);
//                searches_data->db[search_id] = db;
//            }
//
//            return search_id;
//        }
//        else if (IndigoReaction::is(obj))
//        {
//            obj.getBaseReaction().aromatize(self.arom_options);
//
//            std::unique_ptr<ReactionSimilarityQueryData> query_data = std::make_unique<ReactionSimilarityQueryData>(obj.getReaction(), min, 1.0);
//
//            ReactionIndex& bingo_index = dynamic_cast<ReactionIndex&>(sf::xlock_safe_ptr(_bingo_indexes)->ref(db));
//            ReactionTopNSimMatcher* matcher =
//                dynamic_cast<ReactionTopNSimMatcher*>(bingo_index.createMatcherTopNWithExtFP("sim", query_data.release(), options, limit, ext_fp));
//
//            int search_id;
//            {
//                auto searches_data = sf::xlock_safe_ptr(_searches_data);
//                search_id = searches_data->searches.add(matcher);
//                searches_data->db.expand(search_id + 1);
//                searches_data->db[search_id] = db;
//            }
//
//            return search_id;
//        }
//        else
//            throw BingoException("bingoSearchSimTopN: only query molecule and query reaction can be set as query object");
//    }
//    BINGO_END(-1);
//}
//
// CEXPORT int bingoEnumerateId(int db)
//{
//    BINGO_BEGIN_DB(db)
//    {
//        Index& index = sf::xlock_safe_ptr(_bingo_indexes)->ref(db);
//        EnumeratorMatcher* matcher = dynamic_cast<EnumeratorMatcher*>(index.createMatcher("enum", nullptr, nullptr));
//
//        int search_id;
//        {
//            auto searches_data = sf::xlock_safe_ptr(_searches_data);
//            search_id = searches_data->searches.add(matcher);
//            searches_data->db.expand(search_id + 1);
//            searches_data->db[search_id] = db;
//        }
//
//        return search_id;
//    }
//    BINGO_END(-1);
//}
//
CEXPORT int bingoEndSearch(int search_obj)
{
#ifdef INDIGO_DEBUG
    std::stringstream ss;
    ss << "~BingoObject(" << search_obj << ")";
    std::cout << ss.str() << std::endl;
#endif
    BINGO_BEGIN_SEARCH_STATIC(search_obj)
    {
        auto searches_data = sf::xlock_safe_ptr(_searches_data);
        // getMatcher(search_obj);
        searches_data->searches.remove(search_obj);
        return 1;
    }
    BINGO_END(-1);
}

CEXPORT int bingoNext(int search_obj)
{
    BINGO_BEGIN_SEARCH(search_obj)
    {

        return getMatcher(search_obj).next();
    }
    BINGO_END(-1);
}

CEXPORT int bingoGetCurrentId(int search_obj)
{
    BINGO_BEGIN_SEARCH(search_obj)
    {
        return getMatcher(search_obj).currentId();
    }
    BINGO_END(-1);
}

CEXPORT float bingoGetCurrentSimilarityValue(int search_obj)
{
    BINGO_BEGIN_SEARCH(search_obj)
    {
        return getMatcher(search_obj).currentSimValue();
    }
    BINGO_END(-1);
}
//
// CEXPORT int bingoEstimateRemainingResultsCount(int search_obj)
//{
//    BINGO_BEGIN_SEARCH(search_obj)
//    {
//        int delta;
//        return getMatcher(search_obj).esimateRemainingResultsCount(delta);
//    }
//    BINGO_END(-1);
//}
//
// CEXPORT int bingoContainersCount(int search_obj)
//{
//    BINGO_BEGIN_SEARCH(search_obj)
//    {
//        return getMatcher(search_obj).containersCount();
//    }
//    BINGO_END(-1);
//}
//
// CEXPORT int bingoCellsCount(int search_obj)
//{
//    BINGO_BEGIN_SEARCH(search_obj)
//    {
//        return getMatcher(search_obj).cellsCount();
//    }
//    BINGO_END(-1);
//}
//
// CEXPORT int bingoCurrentCell(int search_obj)
//{
//    BINGO_BEGIN_SEARCH(search_obj)
//    {
//        return getMatcher(search_obj).currentCell();
//    }
//    BINGO_END(-1);
//}
//
// CEXPORT int bingoMinCell(int search_obj)
//{
//    BINGO_BEGIN_SEARCH(search_obj)
//    {
//        return getMatcher(search_obj).minCell();
//    }
//    BINGO_END(-1);
//}
//
// CEXPORT int bingoMaxCell(int search_obj)
//{
//    BINGO_BEGIN_SEARCH(search_obj)
//    {
//        return getMatcher(search_obj).maxCell();
//    }
//    BINGO_END(-1);
//}
//
// CEXPORT int bingoEstimateRemainingResultsCountError(int search_obj)
//{
//    BINGO_BEGIN_SEARCH(search_obj)
//    {
//        int delta;
//        getMatcher(search_obj).esimateRemainingResultsCount(delta);
//        return delta;
//    }
//    BINGO_END(-1);
//}
//
// CEXPORT int bingoEstimateRemainingTime(int search_obj, float* time_sec)
//{
//    BINGO_BEGIN_SEARCH(search_obj)
//    {
//        float delta;
//        *time_sec = getMatcher(search_obj).esimateRemainingTime(delta);
//        return 1;
//    }
//    BINGO_END(-1);
//}
//
CEXPORT int bingoGetObject(int search_obj)
{
    BINGO_BEGIN_SEARCH(search_obj)
    {
        Matcher& matcher = getMatcher(search_obj);
        const auto& bingo_index = matcher.getIndex();

        return self.addObject(matcher.currentObject());
    }
    BINGO_END(-1);
}

CEXPORT const char* bingoProfilingGetStatistics(int for_session)
{
    INDIGO_BEGIN
    {
        auto& tmp = self.getThreadTmpData();
        ArrayOutput output(tmp.string);
        profGetStatistics(output, for_session);
        output.writeByte(0);
        return tmp.string.ptr();
    }
    INDIGO_END(nullptr);
}
