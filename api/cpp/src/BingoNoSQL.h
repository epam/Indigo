#pragma once

#include "BingoResultIterator.h"
#include "IndigoMolecule.h"
#include "IndigoQueryMolecule.h"
#include "IndigoSession.h"
#include "IndigoSimilarityMetric.h"

#include <string>

namespace indigo_cpp
{
    class IndigoChemicalEntity;
    class IndigoSession;

    template <typename target_t, typename query_t> class BingoNoSQL
    {
    public:
        BingoNoSQL() = delete;
        BingoNoSQL(const BingoNoSQL&) = delete;
        BingoNoSQL& operator=(const BingoNoSQL&) = delete;
        BingoNoSQL(BingoNoSQL&&) noexcept = default;
        BingoNoSQL& operator=(BingoNoSQL&&) = delete;
        ~BingoNoSQL();

        static BingoNoSQL createDatabaseFile(IndigoSessionPtr session, const std::string& path, const std::string& options = "");

        static BingoNoSQL loadDatabaseFile(IndigoSessionPtr session, const std::string& path, const std::string& options = "");

        void close();

        int insertRecord(const target_t& entity);

        void deleteRecord(int recordId);

        BingoResultIterator<target_t> searchSub(const query_t& query, const std::string& options = "") const;
        BingoResultIterator<target_t> searchSim(const target_t& query, double min, double max = 1.0,
                                                IndigoSimilarityMetric metric = IndigoSimilarityMetric::TANIMOTO) const;

        IndigoSessionPtr session;

    private:
        BingoNoSQL(IndigoSessionPtr indigo, int e);

        int id;
    };

    using BingoMolecule = BingoNoSQL<IndigoMolecule, IndigoQueryMolecule>;
}
