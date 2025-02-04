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

#ifdef _MSC_VER
#pragma warning(push)
#endif

#include <gtest/gtest.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include "molecule/ket_document.h"
#include "molecule/ket_document_json_loader.h"
#include "molecule/ket_document_json_saver.h"
#include "molecule/monomers_template_library.h"
#include "molecule/sequence_loader.h"
#include "molecule/sequence_saver.h"
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
#include <reaction/reaction_cdxml_loader.h>

#include "common.h"

#include <algorithm>

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

TEST_F(IndigoCoreFormatsTest, mol_loader_issue_2732)
{
    Molecule t_mol;

    const char* mol = R"(
  ACCLDraw01172516572D

  5  4  0  0  0  0  0  0  0  0999 V2000
    4.4375   -4.3750    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    5.1538   -4.7885    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    5.8700   -4.3750    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    5.1538   -4.7885    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    5.1538   -4.7885    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
  2  3  1  0  0  0  0
  4  5  1  0  0  0  0
  2  5  1  0  0  0  0
  4  1  1  0  0  0  0
M  STY  1   1 MUL
M  SLB  1   1   1
M  SCN  1   1 HT
M  SAL   1  3   2   4   5
M  SBL   1  2   1   4
M  SPA   1  1   2
M  SDI   1  4    4.7956   -4.9953    4.7956   -4.1682
M  SDI   1  4    5.5119   -4.1682    5.5119   -4.9953
M  SMT   1 3
M  END
)";
    loadMolecule(mol, t_mol);
    ASSERT_EQ(t_mol.sgroups.getSGroupCount(), 1);
    SGroup& sg = t_mol.sgroups.getSGroup(0);
    ASSERT_EQ(sg.atoms.size(), 3);
    ASSERT_EQ(sg.atoms.at(0), 1);
    ASSERT_EQ(sg.atoms.at(1), 3);
    ASSERT_EQ(sg.atoms.at(2), 4);
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

    FileScanner sc(dataPath("molecules/basic/ket_with_query_properties.ket").c_str());
    std::string json;
    sc.readAll(json);
    rapidjson::Document data;
    if (!data.Parse(json.c_str()).HasParseError())
    {
        if (data.HasMember("root"))
        {
            MoleculeJsonLoader loader(data);
            loader.loadMolecule(q_mol);
        }
    }

    Array<char> out;
    ArrayOutput std_out(out);
    MoleculeJsonSaver saver(std_out);
    saver.pretty_json = true;
    saver.saveMolecule(q_mol);
    std::string json_out{out.ptr(), static_cast<std::size_t>(out.size())};
    // ASSERT_EQ(json, json_out);
}

TEST_F(IndigoCoreFormatsTest, idt_load)
{
    const char* idt = "ARAS";
    BufferScanner scanner(idt);
    MonomerTemplateLibrary library;
    FileScanner sc(dataPath("molecules/basic/monomer_library.ket").c_str());
    std::string json;
    sc.readAll(json);
    rapidjson::Document data;
    if (!data.Parse(json.c_str()).HasParseError())
    {
        if (data.HasMember("root"))
        {
            MoleculeJsonLoader loader(data);
            loader.loadMonomerLibrary(library);
        }
    }

    SequenceLoader loader(scanner, library);
    KetDocument document;
    loader.loadIdt(document);

    Array<char> out;
    ArrayOutput std_out(out);
    KetDocumentJsonSaver saver(std_out);
    saver.pretty_json = true;
    saver.saveKetDocument(document);
    std::string json_out{out.ptr(), static_cast<std::size_t>(out.size())};
    // printf("%s", json_out.c_str());
    Array<char> buf;
    ArrayOutput output(buf);
    SequenceSaver idt_saver(output, library);
    FileScanner ket(dataPath("molecules/basic/idt_mixed_std.ket").c_str());
    std::string json2;
    ket.readAll(json2);
    json2.erase(std::remove(json2.begin(), json2.end(), '\r'), json2.end());
    ASSERT_EQ(json2, json_out);
}

TEST_F(IndigoCoreFormatsTest, idt_save)
{
    MonomerTemplateLibrary library;
    FileScanner sc(dataPath("molecules/basic/monomer_library.ket").c_str());
    std::string json;
    sc.readAll(json);
    rapidjson::Document data;
    if (!data.Parse(json.c_str()).HasParseError())
    {
        if (data.HasMember("root"))
        {
            MoleculeJsonLoader loader(data);
            loader.loadMonomerLibrary(library);
        }
    }

    Array<char> buf;
    ArrayOutput output(buf);
    SequenceSaver idt_saver(output, library);
    FileScanner ket(dataPath("molecules/basic/idt_mixed_std.ket").c_str());
    std::string json2;
    ket.readAll(json2);
    KetDocument ket_document;
    KetDocumentJsonLoader kdloader;
    kdloader.parseJson(json2, ket_document);

    idt_saver.saveKetDocument(ket_document, SequenceSaver::SeqFormat::IDT);

    std::string json_out{buf.ptr(), static_cast<std::size_t>(buf.size())};
    // printf("%s", json_out.c_str());
    ASSERT_EQ("ARAS", json_out);
}

TEST_F(IndigoCoreFormatsTest, mol_to_document)
{
    MonomerTemplateLibrary library;
    FileScanner sc(dataPath("molecules/basic/chem_peptide.mol").c_str());
    MolfileLoader loader(sc);
    Molecule mol;
    loader.loadMolecule(mol);

    KetDocument& document = mol.getKetDocument();
    std::vector<std::deque<std::string>> sequences;
    // parse connections between monomers and return backbone sequences
    // non-backbone connection stored in document.nonSequenceConnections()
    document.parseSimplePolymers(sequences);
    int idx = 1;
    for (auto& sequence : sequences)
    {
        printf("Backbone %d\n", idx++);
        for (auto monomer_id : sequence)
        {

            const std::unique_ptr<KetBaseMonomer>& monomer = document.monomers().at(monomer_id);
            MonomerClass monomer_class = document.getMonomerClass(*monomer);
            KetBaseMonomer::MonomerType monomer_type = monomer->monomerType();
            const std::optional<Vec2f>& position = monomer->position();
            Vec2f pos = position.has_value() ? position.value() : Vec2f{0, 0};
            printf("monomer %s\tclass=%s\tposition: %f,%f\n", monomer->alias().c_str(), MonomerTemplate::MonomerClassToStr(monomer_class).c_str(), pos.x,
                   pos.y);
            if (monomer_type == KetBaseMonomer::MonomerType::AmbiguousMonomer)
            {
                const KetAmbiguousMonomerTemplate& templ = document.ambiguousTemplates().at(monomer->templateId());
                printf("ambigous monomer, templates: ");
                for (const KetAmbiguousMonomerOption& option : templ.options())
                {
                    printf("%s ", option.templateId().c_str());
                }
                printf("\n");
            }
            else if (monomer_type == KetBaseMonomer::MonomerType::Monomer)
            {
                const MonomerTemplate& templ = document.templates().at(monomer->templateId());
                printf("monomer, template: %s\n", templ.getStringProp("alias").c_str());
                int atom_idx = 0;
                printf("Atoms:\n");
                for (const std::shared_ptr<KetBaseAtomType>& batom : templ.atoms())
                {
                    printf("%d: ", atom_idx++);
                    if (batom->getType() == KetBaseAtom::atype::atom)
                    {
                        const KetAtom& atom = static_cast<KetAtom&>(*batom);
                        printf("atom %s", atom.label().c_str());
                    }
                    else if (batom->getType() == KetBaseAtom::atype::atom_list)
                    {
                        const KetAtomList& atom_list = static_cast<KetAtomList&>(*batom);
                        printf("atom list: ");
                        for (std::string atom : atom_list.atomList())
                        {
                            printf("%s ", atom.c_str());
                        }
                    }
                    const std::optional<Vec3f>& location = batom->location();
                    if (location.has_value())
                        printf(" (%f, %f, %f)", location->x, location->y, location->z);
                    printf("\n");
                }
                printf("Bonds:\n");
                for (const KetBond& bond : templ.bonds())
                {
                    const std::pair<int, int>& atoms = bond.atoms();
                    printf("bond: type=%d, atoms=(%d, %d)\n", bond.getType(), atoms.first, atoms.second);
                }
            }
        }
    }
    if (document.nonSequenceConnections().size() > 0)
        printf("Non-standard connections:\n");
    for (const KetConnection& connection : document.nonSequenceConnections())
    {
        auto fill_conn_info = [](const KetConnectionEndPoint& ep, std::string& info) {
            if (ep.hasStringProp("monomerId"))
            {
                info += "monomer " + ep.getStringProp("monomerId");
                if (ep.hasStringProp("attachmentPointId"))
                {
                    info += " attachment point " + ep.getStringProp("attachmentPointId");
                }
            }
            else if (ep.hasStringProp("moleculeId"))
            {
                info += "molecule " + ep.getStringProp("moleculeId");
                if (ep.hasStringProp("atomId"))
                {
                    info += " atom " + ep.getStringProp("atomId");
                }
            }
        };
        std::string ep1_info, ep2_info;
        fill_conn_info(connection.ep1(), ep1_info);
        fill_conn_info(connection.ep2(), ep2_info);
        printf("%s connection from %s to %s\n", connection.connectionType().c_str(), ep1_info.c_str(), ep2_info.c_str());
    }
}

TEST_F(IndigoCoreFormatsTest, wrong_stereochemistry_2739)
{
    FileScanner scanner(dataPath("reactions/basic/wrong_stereochemistry_2739.cdxml").c_str());
    Reaction reaction;
    ReactionCdxmlLoader loader(scanner);
    loader.loadReaction(reaction);

    std::vector<std::pair<int, int>> testData = {{1, 1}, {1, 2}, {2, 1}};

    std::vector<std::pair<int, int>> bondDirections;
    for (int i = reaction.begin(); i != reaction.end(); i = reaction.next(i))
    {
        const Molecule& mol = reaction.getMolecule(i);
        int bondUp = 0;
        int bondDown = 0;
        for (int j = mol.edgeBegin(); j != mol.edgeEnd(); j = mol.edgeNext(j))
        {
            int direction = mol.getBondDirection(j);
            if (direction == BOND_UP)
            {
                ++bondUp;
            }
            else if (direction == BOND_DOWN)
            {
                ++bondDown;
            }
        }
        bondDirections.push_back({bondUp, bondDown});
    }

    ASSERT_EQ(testData, bondDirections);
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif
