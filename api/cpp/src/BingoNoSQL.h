#include "IndigoQueryMolecule.h"
#include <string>

namespace indigo_cpp
{
    class IndigoChemicalEntity;
    class IndigoSession;
    class BingoObject;

    enum class BingoNoSqlDataBaseType
    {
        MOLECULE,
        REACTION
    };

    class BingoNoSQL
    {
    public:
        BingoNoSQL() = delete;
        BingoNoSQL(const BingoNoSQL&) = delete;
        BingoNoSQL& operator=(const BingoNoSQL&) = delete;
        BingoNoSQL(BingoNoSQL&&) = default;
        BingoNoSQL& operator=(BingoNoSQL&&) = delete;
        ~BingoNoSQL();

        static BingoNoSQL createDatabaseFile(IndigoSession& session, const std::string& path, const BingoNoSqlDataBaseType& type, const std::string& options);
        static BingoNoSQL loadDatabaseFile(IndigoSession& session, const std::string& path, const std::string& options);

        void close();

        int insertRecord(const IndigoChemicalEntity& entity) const;
        void deleteRecord(int recordId) const;

        BingoObject searchSub(const IndigoQueryMolecule& query, const std::string& options = "");

        IndigoSession& indigo;

    private:
        BingoNoSQL(IndigoSession& indigo, int e);

        int id;
    };

    class BingoObject
    {
        const Indigo&;
    };
}
