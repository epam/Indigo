#include "common.h"

#include <base_cpp/output.h>
#include <base_cpp/scanner.h>
#include <molecule/canonical_smiles_saver.h>
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
    loader.loadMolecule(queryMolecule);
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

std::string IndigoCoreTest::smartsLoadSaveLoad(const std::string& queryString)
{
    QueryMolecule query;
    loadQueryMolecule(queryString.c_str(), query);
    Array<char> buffer;
    ArrayOutput output(buffer);
    CanonicalSmilesSaver smilesSaver(output);
    smilesSaver.smarts_mode = true;
    smilesSaver.ignore_hydrogens = false;
    smilesSaver.saveQueryMolecule(query);
    output.writeChar(0);
    std::string result(buffer.ptr());
    return result;
}

std::string IndigoCoreTest::dataPath(const char* dataPathSuffix)
{
    return dataPathPrefix + "/" + dataPathSuffix;
}
