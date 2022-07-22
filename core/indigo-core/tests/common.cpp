#include "common.h"

#include <base_cpp/scanner.h>
#include <molecule/molecule_auto_loader.h>
#include <molecule/molecule_substructure_matcher.h>

using namespace indigo;

namespace
{
    const std::string dataPathPrefix = DATA_PATH;
}

void IndigoCoreTest::loadMolecule(const char* buf, Molecule& molecule)
{
    BufferScanner scanner(buf);
    MoleculeAutoLoader loader(scanner);
    loader.loadMolecule(molecule);
}

void IndigoCoreTest::loadQueryMolecule(const char* buf, QueryMolecule& queryMolecule)
{
    BufferScanner scanner(buf);
    MoleculeAutoLoader loader(scanner);
    loader.loadQueryMolecule(queryMolecule);
}

bool IndigoCoreTest::substructureMatch(const char* targetString, const char* queryString)
{
    Molecule target;
    QueryMolecule query;
    loadMolecule(targetString, target);
    loadQueryMolecule(queryString, query);
    MoleculeSubstructureMatcher matcher(target);
    matcher.setQuery(query);
    return matcher.find();
}

std::string IndigoCoreTest::dataPath(const char* dataPathSuffix)
{
    return dataPathPrefix + "/" + dataPathSuffix;
}
