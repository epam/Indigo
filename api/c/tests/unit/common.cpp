#include "common.h"

#include "base_cpp/scanner.h"
#include "molecule/molecule_auto_loader.h"

namespace
{
    const std::string dataPathPrefix = DATA_PATH;
}

std::string indigo::dataPath(const char* dataPathSuffix)
{
    return dataPathPrefix + "/" + dataPathSuffix;
}

void indigo::errorHandling(const char* message, void* context)
{
    throw indigo::Exception(message);
}

void indigo::printMap(Array<int>& map)
{
    for (int i = 0; i < map.size(); ++i)
    {
        printf("%d %d\n", i, map[i]);
    }
}

void indigo::loadMolecule(const char* buf, indigo::Molecule& mol)
{
    indigo::BufferScanner scanner(buf);
    indigo::MoleculeAutoLoader loader(scanner);
    loader.loadMolecule(mol);
}
