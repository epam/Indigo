#include <string>

namespace indigo_cpp
{
    class IndigoChemicalEntity;
    class IndigoSession;

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

        static BingoNoSQL createDatabaseFile(const IndigoSession& session, const std::string& path, const BingoNoSqlDataBaseType& type,
                                             const std::string& options);
        static BingoNoSQL loadDatabaseFile(const IndigoSession& session, const std::string& path, const std::string& options);

        void close();


        int insertRecord(const IndigoChemicalEntity& entity) const;
        void deleteRecord(int recordId) const;

        const IndigoSession& indigo;

    private:
        BingoNoSQL(const IndigoSession& indigo, int e);

        int id;
    };
}
