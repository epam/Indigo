#include "common.h"

#include <base_cpp/scanner.h>
#include <molecule/molecule_auto_loader.h>

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

std::string IndigoCoreTest::dataPath(const char* dataPathSuffix)
{
    return dataPathPrefix + "/" + dataPathSuffix;
}
