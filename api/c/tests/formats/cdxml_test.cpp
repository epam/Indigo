#include "base_cpp/exception.h"
#include "base_cpp/output.h"
#include "base_cpp/scanner.h"
#include "indigo-inchi.h"
#include "indigo.h"
#include "molecule/molecule.h"
#include "molecule/molecule_auto_loader.h"
#include "molecule/molecule_cdxml_saver.h"
#include "molecule/molecule_mass.h"
#include "src/indigo_internal.h"
#include "gtest/gtest.h"
#include <fstream>

using namespace indigo;

static void printMap(Array<int>& map)
{
    for (int i = 0; i < map.size(); ++i)
    {
        printf("%d %d\n", i, map[i]);
    }
}

static void loadMolecule(const char* buf, Molecule& mol)
{
    BufferScanner scanner(buf);
    MoleculeAutoLoader loader(scanner);
    loader.loadMolecule(mol);
}

static void errorHandling(const char* message, void* context)
{
    throw Exception(message);
}

TEST(IndigoCdxmlTest, cdxml_test1)
{
    Molecule t_mol;

    loadMolecule("c1ccccc1N", t_mol);

    Array<char> out;
    ArrayOutput std_out(out);
    MoleculeCdxmlSaver saver(std_out);
    saver.saveMolecule(t_mol);
    loadMolecule("c1ccccc1", t_mol);
    saver.saveMolecule(t_mol);

    ASSERT_TRUE(out.size() > 2000);
}
