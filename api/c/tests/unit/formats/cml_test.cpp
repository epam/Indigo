#include <fstream>

#include <gtest/gtest.h>

#include <base_cpp/exception.h>
#include <base_cpp/scanner.h>
#include <molecule/cml_saver.h>
#include <molecule/molecule.h>
#include <molecule/molecule_auto_loader.h>

#include <indigo_internal.h>

#include "common.h"

using namespace indigo;

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
