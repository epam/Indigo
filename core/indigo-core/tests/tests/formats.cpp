/****************************************************************************
 * Copyright (C) from 2009 to Present EPAM Systems.
 *
 * This file is part of Indigo toolkit.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ***************************************************************************/

#include <gtest/gtest.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <base_cpp/output.h>
#include <base_cpp/scanner.h>
#include <molecule/cmf_loader.h>
#include <molecule/cmf_saver.h>
#include <molecule/cml_saver.h>
#include <molecule/molecule_cdxml_saver.h>
#include <molecule/molecule_json_loader.h>
#include <molecule/molecule_json_saver.h>
#include <molecule/molecule_mass.h>
#include <molecule/molecule_substructure_matcher.h>
#include <molecule/molfile_loader.h>
#include <molecule/molfile_saver.h>
#include <molecule/query_molecule.h>
#include <molecule/sdf_loader.h>
#include <molecule/smiles_loader.h>
#include <molecule/smiles_saver.h>

#include "common.h"

using namespace indigo;

class IndigoCoreFormatsTest : public IndigoCoreTest
{
};

TEST_F(IndigoCoreFormatsTest, load_targets_cmf)
{
    FileScanner sc(dataPath("molecules/resonance/resonance.sdf").c_str());

    SdfLoader sdf(sc);
    QueryMolecule qmol;

    Array<char> qbuf;
    qbuf.readString("N(#C)=C(C)C", false);
    BufferScanner sm_scanner(qbuf);
    SmilesLoader smiles_loader(sm_scanner);
    smiles_loader.loadQueryMolecule(qmol);

    sdf.readAt(138);
    try
    {
        BufferScanner bsc(sdf.data);
        MolfileLoader loader(bsc);
        Molecule mol;
        loader.loadMolecule(mol);
        Array<char> buf;
        ArrayOutput buf_out(buf);
        CmfSaver cmf_saver(buf_out);

        cmf_saver.saveMolecule(mol);

        Molecule mol2;
        BufferScanner buf_in(buf);
        CmfLoader cmf_loader(buf_in);
        cmf_loader.loadMolecule(mol2);

        MoleculeSubstructureMatcher matcher(mol2);
        matcher.use_pi_systems_matcher = true;
        matcher.setQuery(qmol);
        matcher.find();
    }
    catch (Exception& e)
    {
        ASSERT_STREQ("", e.message());
    }
}

TEST_F(IndigoCoreFormatsTest, save_cdxml)
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

TEST_F(IndigoCoreFormatsTest, save_cml)
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

TEST_F(IndigoCoreFormatsTest, smiles_data_sgroups)
{
    Molecule t_mol;

    loadMolecule("c1ccccc1N |SgD:3,2,1,0:name:data:like:unit:t:(-1)|", t_mol);
    ASSERT_EQ(t_mol.sgroups.getSGroupCount(), 1);
    SGroup& sg = t_mol.sgroups.getSGroup(0);
    ASSERT_EQ(sg.sgroup_type, SGroup::SG_TYPE_DAT);
    DataSGroup& dsg = static_cast<DataSGroup&>(sg);
    ASSERT_STREQ(dsg.name.ptr(), "name");
    ASSERT_STREQ(dsg.data.ptr(), "data");
    ASSERT_STREQ(dsg.queryoper.ptr(), "like");
    ASSERT_STREQ(dsg.description.ptr(), "unit");
    ASSERT_EQ(dsg.tag, 't');
    ASSERT_EQ(dsg.display_pos.x, 0.0f);
    ASSERT_EQ(dsg.display_pos.y, 0.0f);
    ASSERT_EQ(dsg.atoms.size(), 4);
    ASSERT_EQ(dsg.atoms.at(0), 3);
    ASSERT_EQ(dsg.atoms.at(1), 2);
    ASSERT_EQ(dsg.atoms.at(2), 1);
    ASSERT_EQ(dsg.atoms.at(3), 0);
    Array<char> out;
    ArrayOutput std_out(out);
    SmilesSaver saver(std_out);
    saver.saveMolecule(t_mol);
    ASSERT_EQ(out.size(), 48);
    std::string str{out.ptr(), static_cast<std::size_t>(out.size())};
    ASSERT_STREQ(str.c_str(), "c1c(N)cccc1 |SgD:3,2,1,0:name:data:like:unit:t:|");
}

TEST_F(IndigoCoreFormatsTest, smiles_data_sgroups_coords)
{
    Molecule t_mol;

    loadMolecule("c1ccccc1 |SgD:1,2,0:::::s:(-1.5,7.8)|", t_mol);
    ASSERT_EQ(t_mol.sgroups.getSGroupCount(), 1);
    SGroup& sg = t_mol.sgroups.getSGroup(0);
    ASSERT_EQ(sg.sgroup_type, SGroup::SG_TYPE_DAT);
    DataSGroup& dsg = static_cast<DataSGroup&>(sg);
    ASSERT_STREQ(dsg.name.ptr(), "");
    ASSERT_STREQ(dsg.data.ptr(), "");
    ASSERT_STREQ(dsg.queryoper.ptr(), "");
    ASSERT_STREQ(dsg.description.ptr(), "");
    ASSERT_EQ(dsg.tag, 's');
    ASSERT_EQ(dsg.display_pos.x, -1.5f);
    ASSERT_EQ(dsg.display_pos.y, 7.8f);
    ASSERT_EQ(dsg.atoms.size(), 3);
    ASSERT_EQ(dsg.atoms.at(0), 1);
    ASSERT_EQ(dsg.atoms.at(1), 2);
    ASSERT_EQ(dsg.atoms.at(2), 0);
    Array<char> out;
    ArrayOutput std_out(out);
    SmilesSaver saver(std_out);
    saver.saveMolecule(t_mol);
    ASSERT_EQ(out.size(), 27);
    std::string str{out.ptr(), static_cast<std::size_t>(out.size())};
    ASSERT_STREQ(str.c_str(), "c1ccccc1 |SgD:1,2,0:::::s:|");
}

TEST_F(IndigoCoreFormatsTest, smiles_data_sgroups_short)
{
    Molecule t_mol;

    loadMolecule("c1ccccc1 |SgD:1,2,0:name|", t_mol);
    ASSERT_EQ(t_mol.sgroups.getSGroupCount(), 1);
    SGroup& sg = t_mol.sgroups.getSGroup(0);
    ASSERT_EQ(sg.sgroup_type, SGroup::SG_TYPE_DAT);
    DataSGroup& dsg = static_cast<DataSGroup&>(sg);
    ASSERT_STREQ(dsg.name.ptr(), "name");
    ASSERT_EQ(dsg.data.size(), 0);
    ASSERT_EQ(dsg.queryoper.size(), 0);
    ASSERT_EQ(dsg.description.size(), 0);
    ASSERT_EQ(dsg.tag, ' ');
    ASSERT_EQ(dsg.display_pos.x, 0.0f);
    ASSERT_EQ(dsg.display_pos.y, 0.0f);
    ASSERT_EQ(dsg.atoms.size(), 3);
    ASSERT_EQ(dsg.atoms.at(0), 1);
    ASSERT_EQ(dsg.atoms.at(1), 2);
    ASSERT_EQ(dsg.atoms.at(2), 0);
    Array<char> out;
    ArrayOutput std_out(out);
    SmilesSaver saver(std_out);
    saver.saveMolecule(t_mol);
    ASSERT_EQ(out.size(), 31);
    std::string str{out.ptr(), static_cast<std::size_t>(out.size())};
    ASSERT_STREQ(str.c_str(), "c1ccccc1 |SgD:1,2,0:name:::: :|");
}

TEST_F(IndigoCoreFormatsTest, smiles_pol_sgroups_conn_and_flip)
{
    Molecule t_mol;

    loadMolecule("*CC*C*N* |$star;;;star;;star;;star$,Sg:n:6,1,2,4::hh&#44;f:6,0,:4,2,|", t_mol);
    ASSERT_EQ(t_mol.sgroups.getSGroupCount(), 1);
    SGroup& sg = t_mol.sgroups.getSGroup(0);
    ASSERT_EQ(sg.sgroup_type, SGroup::SG_TYPE_SRU);
    RepeatingUnit& ru = static_cast<RepeatingUnit&>(sg);
    ASSERT_EQ(ru.atoms.size(), 4);
    ASSERT_EQ(ru.atoms.at(0), 6);
    ASSERT_EQ(ru.atoms.at(1), 1);
    ASSERT_EQ(ru.atoms.at(2), 2);
    ASSERT_EQ(ru.atoms.at(3), 4);
    ASSERT_EQ(ru.connectivity, RepeatingUnit::HEAD_TO_HEAD);
    Array<char> out;
    ArrayOutput std_out(out);
    SmilesSaver saver(std_out);
    saver.saveMolecule(t_mol);
    ASSERT_EQ(out.size(), 53);
    std::string str{out.ptr(), static_cast<std::size_t>(out.size())};
    ASSERT_STREQ(str.c_str(), "*CC*C*N* |$star;;;star;;star;;star$,Sg:n:6,1,2,4::hh|");
}

TEST_F(IndigoCoreFormatsTest, smiles_pol_sgroups_bracket)
{
    Molecule t_mol;

    loadMolecule("C1CCCCC1 |Sg:n:0,5,4,3,2,1:::::(d,s,-7.03,2.12,-2.21,2.12,-2.21,-3.11,-7.03,-3.11,)|", t_mol);
    ASSERT_EQ(t_mol.sgroups.getSGroupCount(), 1);
    SGroup& sg = t_mol.sgroups.getSGroup(0);
    ASSERT_EQ(sg.sgroup_type, SGroup::SG_TYPE_SRU);
    RepeatingUnit& ru = static_cast<RepeatingUnit&>(sg);
    ASSERT_EQ(ru.atoms.size(), 6);
    ASSERT_EQ(ru.atoms.at(0), 0);
    ASSERT_EQ(ru.atoms.at(1), 5);
    ASSERT_EQ(ru.atoms.at(2), 4);
    ASSERT_EQ(ru.atoms.at(3), 3);
    ASSERT_EQ(ru.atoms.at(4), 2);
    ASSERT_EQ(ru.atoms.at(5), 1);
    Array<char> out;
    ArrayOutput std_out(out);
    SmilesSaver saver(std_out);
    saver.saveMolecule(t_mol);
    ASSERT_EQ(out.size(), 31);
    std::string str{out.ptr(), static_cast<std::size_t>(out.size())};
    ASSERT_STREQ(str.c_str(), "C1CCCCC1 |Sg:n:0,5,4,3,2,1::eu|");
}

TEST_F(IndigoCoreFormatsTest, smiles_pol_sgroups_gen)
{
    Molecule t_mol;

    loadMolecule("CCCC |Sg:gen:0,1,2:|", t_mol);
    ASSERT_EQ(t_mol.sgroups.getSGroupCount(), 1);
    SGroup& sg = t_mol.sgroups.getSGroup(0);
    ASSERT_EQ(sg.sgroup_type, SGroup::SG_TYPE_GEN);
    Array<char> out;
    ArrayOutput std_out(out);
    SmilesSaver saver(std_out);
    saver.saveMolecule(t_mol);
    ASSERT_EQ(out.size(), 20);
    std::string str{out.ptr(), static_cast<std::size_t>(out.size())};
    ASSERT_STREQ(str.c_str(), "CCCC |Sg:gen:0,1,2:|");
}

TEST_F(IndigoCoreFormatsTest, mol_saver_issue_1200)
{
    Molecule t_mol;

    const char* mol = R"(
  -INDIGO-07262316452D

  6  6  0  0  0  0  0  0  0  0999 V2000
   -1.4617   -0.6508    0.0000 N   0  0  0  0  0  0  0  0  0  0  0  0
    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    1.3856   -0.8000    0.0000 N   0  0  0  0  0  0  0  0  0  0  0  0
    2.5747    0.2706    0.0000 N   0  0  0  0  0  0  0  0  0  0  0  0
    1.9239    1.7323    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.3327    1.5650    0.0000 N   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  2  0  0  0  0
  2  3  4  0  0  0  0
  3  4  4  0  0  0  0
  4  5  4  0  0  0  0
  5  6  4  0  0  0  0
  6  2  4  0  0  0  0
M  STY  4   1 DAT   2 DAT   3 DAT   4 DAT
M  SLB  4   1   1   2   2   3   3   4   4
M  SAL   1  1   3
M  SDT   1 MRV_IMPLICIT_H                                                       
M  SDD   1     0.0000    0.0000    DA    ALL  1       1  
M  SED   1 IMPL_H1
M  SAL   2  1   4
M  SDT   2 MRV_IMPLICIT_H                                                       
M  SDD   2     0.0000    0.0000    DA    ALL  1       1  
M  SED   2 IMPL_H1
M  SAL   3  1   3
M  SDT   3 MRV_IMPLICIT_H                                                       
M  SDD   3     0.0000    0.0000    DA    ALL  1       1  
M  SED   3 IMPL_H1
M  SAL   4  1   4
M  SDT   4 MRV_IMPLICIT_H                                                       
M  SDD   4     0.0000    0.0000    DA    ALL  1       1  
M  SED   4 IMPL_H1
M  END
)";
    loadMolecule(mol, t_mol);
    ASSERT_EQ(t_mol.sgroups.getSGroupCount(), 0);
    Array<char> out;
    ArrayOutput std_out(out);
    MolfileSaver saver(std_out);
    saver.saveMolecule(t_mol);
    ASSERT_EQ(t_mol.sgroups.getSGroupCount(), 0);
    saver.mode = MolfileSaver::MODE_2000;
    saver.saveMolecule(t_mol);
    ASSERT_EQ(t_mol.sgroups.getSGroupCount(), 0);
    saver.mode = MolfileSaver::MODE_3000;
    saver.saveMolecule(t_mol);
    ASSERT_EQ(t_mol.sgroups.getSGroupCount(), 0);
}

TEST_F(IndigoCoreFormatsTest, smarts_load_save)
{
    QueryMolecule q_mol;

    std::string smarts_in{"([#8].[#6]).([#6].[#8])"};
    BufferScanner scanner(smarts_in.c_str());
    SmilesLoader loader(scanner);
    loader.smarts_mode = true;
    loader.loadQueryMolecule(q_mol);
    Array<char> out;
    ArrayOutput std_out(out);
    SmilesSaver saver(std_out);
    saver.smarts_mode = true;
    saver.saveQueryMolecule(q_mol);
    std::string smarts_out{out.ptr(), static_cast<std::size_t>(out.size())};
    ASSERT_EQ(smarts_in, smarts_out);
}

TEST_F(IndigoCoreFormatsTest, json_load_save)
{
    QueryMolecule q_mol;

    char* ket = R"({"root":{"nodes":[{"$ref":"mol0"},{"$ref":"mol1"}]},"mol0":{"type":"molecule","atoms":[
{"label":"C","location":[6.872400427225807,-8.203302184026775,0]},{"label":"C","location":[8.183693207880431,-9.188796758369131,0]},
{"label":"C","location":[7.549396700010967,-8.661999658659791,0]},{"label":"C","location":[6.355203274663791,-8.661999658659791,0]},
{"label":"C","location":[6.183604219405395,-7.66270516029316,0]},{"label":"C","location":[8.01389414271216,-8.184302288631033,0]},
{"label":"C","location":[5.716306792119568,-9.537294839706838,0]},{"label":"C","location":[6.478602595286669,-9.074097389848514,0]}],
"bonds":[{"type":1,"atoms":[1,2]},{"type":1,"atoms":[3,4]},{"type":1,"atoms":[4,0]},{"type":1,"atoms":[0,5]},{"type":1,"atoms":[5,1]},
{"type":1,"atoms":[3,6]},{"type":1,"atoms":[0,7]},{"type":1,"atoms":[6,7]},{"type":1,"atoms":[3,2]}]},
"mol1":{"type":"molecule","atoms":[{"label":"C","location":[4.759849152128566,-4.125074417174607,0]},
{"label":"C","location":[6.490150847871433,-4.124589229177203,0]},{"label":"C","location":[5.626637509491239,-3.6249668888501874,0]},
{"label":"C","location":[6.490150847871433,-5.125532067822148,0]},{"label":"C","location":[4.759849152128566,-5.130020056798137,0]},
{"label":"C","location":[5.6288208554795585,-5.625033111149812,0]}],"bonds":[{"type":2,"atoms":[2,0]},{"type":2,"atoms":[3,1]},
{"type":1,"atoms":[0,4]},{"type":1,"atoms":[1,2]},{"type":2,"atoms":[4,5]},{"type":1,"atoms":[5,3]}],
"sgroups":[{"type":"DAT","atoms":[0,1,2,3,4,5],"context":"Fragment","fieldName":"2323fc","fieldData":"22","bonds":[0,1,2,3,4,5]}]}})";

    BufferScanner scanner(ket);
    rapidjson::Document data;
    if (!data.Parse(ket).HasParseError())
    {
        if (data.HasMember("root"))
        {
            MoleculeJsonLoader loader(data);
            /**
            loader.stereochemistry_options = stereochemistry_options;
            loader.ignore_noncritical_query_features = ignore_noncritical_query_features;
            loader.treat_x_as_pseudoatom = treat_x_as_pseudoatom;
            loader.skip_3d_chirality = skip_3d_chirality;
            loader.ignore_no_chiral_flag = ignore_no_chiral_flag;
            loader.treat_stereo_as = treat_stereo_as;
            //*/
            loader.loadMolecule(q_mol);
            return;
        }
    }

    Array<char> out;
    ArrayOutput std_out(out);
    MoleculeJsonSaver saver(std_out);
    saver.saveMolecule(q_mol);
    std::string smarts_out{out.ptr(), static_cast<std::size_t>(out.size())};
    // ASSERT_EQ(smarts_in, smarts_out);
    printf(smarts_out.c_str());
}
