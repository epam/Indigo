#include "BingoNoSQL.h"

#include <bingo-nosql.h>

#include "IndigoChemicalEntity.h"
#include "IndigoSession.h"

using namespace indigo_cpp;

namespace
{
    const char* bingoNoSqlDataBaseTypeToCharArray(const BingoNoSqlDataBaseType& type)
    {
        switch (type)
        {
        case BingoNoSqlDataBaseType::MOLECULE:
            return "molecule";
        case BingoNoSqlDataBaseType::REACTION:
            return "reaction";
        default:
            return "";
        }
    }
}

BingoNoSQL::BingoNoSQL(IndigoSession& indigo, const int id) : indigo(indigo), id(id)
{
}

BingoNoSQL::~BingoNoSQL()
{
    close();
}

BingoNoSQL BingoNoSQL::createDatabaseFile(IndigoSession& session, const std::string& path, const BingoNoSqlDataBaseType& type, const std::string& options)
{
    session.setSessionId();
    return {session, session._checkResult(bingoCreateDatabaseFile(path.c_str(), bingoNoSqlDataBaseTypeToCharArray(type), options.c_str()))};
}

BingoNoSQL BingoNoSQL::loadDatabaseFile(IndigoSession& session, const std::string& path, const std::string& options)
{
    session.setSessionId();
    int id = bingoLoadDatabaseFile(path.c_str(), options.c_str());
    return {session, id};
}

void BingoNoSQL::close()
{
    if (id >= 0)
    {
        indigo.setSessionId();
        indigo._checkResult(bingoCloseDatabase(id));
        id = -1;
    }
}

int BingoNoSQL::insertRecord(const IndigoChemicalEntity& entity) const
{
    indigo.setSessionId();
    return indigo._checkResult(bingoInsertRecordObj(id, entity.id));
}

void BingoNoSQL::deleteRecord(const int recordId) const
{
    indigo.setSessionId();
    indigo._checkResult(bingoDeleteRecord(id, recordId));
}
