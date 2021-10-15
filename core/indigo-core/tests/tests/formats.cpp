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

#include <base_cpp/output.h>
#include <base_cpp/scanner.h>
#include <molecule/cmf_loader.h>
#include <molecule/cmf_saver.h>
#include <molecule/cml_saver.h>
#include <molecule/molecule_cdxml_saver.h>
#include <molecule/molecule_mass.h>
#include <molecule/molecule_substructure_matcher.h>
#include <molecule/molfile_loader.h>
#include <molecule/query_molecule.h>
#include <molecule/sdf_loader.h>
#include <molecule/smiles_loader.h>

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
