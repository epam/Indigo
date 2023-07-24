#include "molecule/structure_checker.h"
#include "common.h"
#include <base_cpp/scanner.h>
#include <molecule/molecule_auto_loader.h>

using namespace std;
using namespace indigo;

static char check_types[] = "radicals;pseudoatoms;stereo;query;overlap_atoms;overlap_bonds;rgroups;chiral;3d";
class IndigoCoreStructureCheckTest : public IndigoCoreTest
{
};

TEST_F(IndigoCoreStructureCheckTest, issue731)
{
    Molecule molecule;
    loadMolecule("|RG:_R23={C1CCCC[CH]1 |^1:5|}|", molecule);
    StructureChecker checker;
    StructureChecker::CheckResult result = checker.checkMolecule(molecule, check_types);
    ASSERT_EQ(result.messages.size(), 2);
    ASSERT_EQ(result.messages[0].code, StructureChecker::CheckMessageCode::CHECK_MSG_RGROUP);
    ASSERT_EQ(result.messages[1].code, StructureChecker::CheckMessageCode::CHECK_MSG_RADICAL);
    ASSERT_EQ(result.messages[1].ids.size(), 1);
    ASSERT_EQ(result.messages[1].ids[0], 5);
    ASSERT_STREQ(result.messages[1].prefix.c_str(), "R-Group R23");
}

TEST_F(IndigoCoreStructureCheckTest, no_msg)
{
    Molecule molecule;
    loadMolecule("C1CCCCC1", molecule);
    StructureChecker checker;
    StructureChecker::CheckResult result = checker.checkMolecule(molecule, check_types);
    ASSERT_EQ(result.messages.size(), 0);
}

TEST_F(IndigoCoreStructureCheckTest, radical)
{
    Molecule molecule;
    loadMolecule("C1CC[CH]CC1 |^1:3|", molecule);
    StructureChecker checker;
    StructureChecker::CheckResult result = checker.checkMolecule(molecule, check_types);
    ASSERT_EQ(result.messages.size(), 1);
    ASSERT_EQ(result.messages[0].code, StructureChecker::CheckMessageCode::CHECK_MSG_RADICAL);
    ASSERT_EQ(result.messages[0].ids.size(), 1);
    ASSERT_EQ(result.messages[0].ids[0], 3);
}

TEST_F(IndigoCoreStructureCheckTest, issue731_stereo)
{
    Molecule molecule;
    char* mol = R"({"root":{"nodes":[{"$ref":"rg2"}]},
"header":{"moleculeName":"null"},"rg2":{"rlogic":{"number":2},"type":"rgroup",
"atoms":[{"label":"C","location":[14.808632653403645,17.2218585385598,0]},
{"label":"C","location":[13.997755777985581,16.696221303012916,0]},
{"label":"C","location":[14.046217359380686,15.737848286709752,0]},
{"label":"C","location":[14.903343876794077,15.29948211475405,0]},
{"label":"N","location":[15.711606642012404,15.823410124401105,0],"isotope":14},
{"label":"C","location":[15.663547231417262,16.790027642103436,0]},
{"label":"N","location":[16.61478171602114,15.375391853246377,0],"isotope":15},
{"label":"C","location":[16.67329756741523,17.514739423630207,0]},
{"label":"O","location":[14.959044532588447,14.329647230652046,0]}],
"bonds":[{"type":1,"atoms":[0,1]},
{"type":1,"atoms":[1,2]},
{"type":1,"atoms":[2,3]},
{"type":1,"atoms":[3,4]},
{"type":1,"atoms":[4,5]},
{"type":1,"atoms":[0,5]},
{"type":1,"atoms":[4,6]},
{"type":1,"atoms":[5,7]},
{"type":1,"atoms":[3,8]}]}})";
    loadMolecule(mol, molecule);
    StructureChecker checker;
    StructureChecker::CheckResult result = checker.checkMolecule(molecule, check_types);
    ASSERT_EQ(result.messages.size(), 2);
    ASSERT_EQ(result.messages[0].code, StructureChecker::CheckMessageCode::CHECK_MSG_RGROUP);
    ASSERT_EQ(result.messages[1].code, StructureChecker::CheckMessageCode::CHECK_MSG_UNDEFINED_STEREO);
    ASSERT_EQ(result.messages[1].ids.size(), 3);
    ASSERT_EQ(result.messages[1].ids[0], 3);
    ASSERT_EQ(result.messages[1].ids[1], 4);
    ASSERT_EQ(result.messages[1].ids[2], 5);
    ASSERT_STREQ(result.messages[1].prefix.c_str(), "R-Group R2");
}
