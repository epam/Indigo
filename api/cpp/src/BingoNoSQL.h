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

        static BingoNoSQL createDatabaseFile(IndigoSessionPtr session, const std::string& path, const BingoNoSqlDataBaseType& type, const std::string& options);
        static BingoNoSQL loadDatabaseFile(IndigoSessionPtr session, const std::string& path, const std::string& options);

        void close();

        int insertRecord(const IndigoChemicalEntity& entity) const;
        void deleteRecord(int recordId) const;

        BingoObject searchSub(const IndigoQueryMolecule& query, const std::string& options = "");

        IndigoSessionPtr session;

    private:
        BingoNoSQL(IndigoSessionPtr indigo, int e);

        int id;
    };

    class BingoObject
    {
    public:
        BingoObject(int id, IndigoSessionPtr session);
    private:
        int id;
        IndigoSessionPtr session;
    };
}
