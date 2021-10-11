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

BingoNoSQL::BingoNoSQL(IndigoSessionPtr session, const int id) : session(std::move(session)), id(id)
{
}

BingoNoSQL::~BingoNoSQL()
{
    close();
}

BingoNoSQL BingoNoSQL::createDatabaseFile(IndigoSessionPtr session, const std::string& path, const BingoNoSqlDataBaseType& type, const std::string& options)
{
    session->setSessionId();
    int id = session->_checkResult(bingoCreateDatabaseFile(path.c_str(), bingoNoSqlDataBaseTypeToCharArray(type), options.c_str()));
    return {std::move(session), id};
}

BingoNoSQL BingoNoSQL::loadDatabaseFile(IndigoSessionPtr session, const std::string& path, const std::string& options)
{
    session->setSessionId();
    int id = session->_checkResult(bingoLoadDatabaseFile(path.c_str(), options.c_str()));
    return {std::move(session), id};
}

void BingoNoSQL::close()
{
    if (id >= 0)
    {
        session->setSessionId();
        session->_checkResult(bingoCloseDatabase(id));
        id = -1;
    }
}

int BingoNoSQL::insertRecord(const IndigoChemicalEntity& entity) const
{
    session->setSessionId();
    return session->_checkResult(bingoInsertRecordObj(id, entity.id));
}

void BingoNoSQL::deleteRecord(const int recordId) const
{
    session->setSessionId();
    session->_checkResult(bingoDeleteRecord(id, recordId));
}

BingoObject BingoNoSQL::searchSub(const IndigoQueryMolecule& query, const std::string& options)
{
    session->setSessionId();
    return {session->_checkResult(bingoSearchSub(id, query.id, options.c_str())), session};
}

BingoObject::BingoObject(int id, IndigoSessionPtr session) : id(id), session(std::move(session))
{
}
