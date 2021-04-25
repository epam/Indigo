#include "base_cpp/exception.h"
#include "base_cpp/scanner.h"
#include "molecule/cml_saver.h"
#include "molecule/molecule.h"
#include "molecule/molecule_auto_loader.h"
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

TEST(IndigoCmlTest, cml_test1)
{
    Molecule t_mol;

    loadMolecule("c1ccccc1N", t_mol);

    Array<char> out;
    ArrayOutput std_out(out);
    CmlSaver saver(std_out);
    saver.saveMolecule(t_mol);
    loadMolecule("c1ccccc1", t_mol);
    saver.saveMolecule(t_mol);

    ASSERT_TRUE(out.size() > 1000);
}

// TEST(IndigoCmlTest, array_test1) {
//    Array<char> _message;
//    int _mseconds = 10;
//    ArrayOutput mes_out(_message);
//    mes_out.printf("The operation timed out: %d ms", _mseconds);
//    mes_out.writeChar(0);
// }
