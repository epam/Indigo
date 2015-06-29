/****************************************************************************
 * Copyright (C) 2009-2015 EPAM Systems
 * 
 * This file is part of Indigo toolkit.
 * 
 * This file may be distributed and/or modified under the terms of the
 * GNU General Public License version 3 as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPL included in the
 * packaging of this file.
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 ***************************************************************************/

#include "molecule/molecule_ionize.h"
#include "molecule/base_molecule.h"
#include "molecule/molecule.h"
#include "molecule/smiles_loader.h"
#include "base_cpp/scanner.h"
#include "molecule/query_molecule.h"
#include "molecule/molecule_substructure_matcher.h"

using namespace indigo;

IMPL_ERROR(MoleculeIonizer, "Molecule Ionizer");

CP_DEF(MoleculeIonizer);
MoleculeIonizer::MoleculeIonizer():
CP_INIT{
}

bool MoleculeIonizer::ionize (Molecule &mol, float ph, float ph_toll, const IonizeOptions &options)
{
   QS_DEF(Array<int>, acid_sites);
   QS_DEF(Array<int>, basic_sites);
   QS_DEF(Array<float>, acid_pkas);
   QS_DEF(Array<float>, basic_pkas);

   acid_sites.clear();
   basic_sites.clear();
   acid_pkas.clear();
   basic_pkas.clear();

   _estimate_pKa(mol, options, acid_sites, basic_sites, acid_pkas, basic_pkas);

   if (acid_sites.size() > 0 || basic_sites.size() > 0)
      _setCharges(mol, ph, ph_toll, options, acid_sites, basic_sites, acid_pkas, basic_pkas);

   return true;
}

void MoleculeIonizer::_loadPkaModel(const IonizeOptions &options,
                      ObjArray<QueryMolecule> &acids, Array<float> &a_pkas,
                      ObjArray<QueryMolecule> &basics, Array<float> &b_pkas)
{
   struct PkaDef
   {
      const char *acid;
      float pka;
      const char *basic;
   };

   static PkaDef simple_pka_model[] = 
   {
      {"[F;!H0]", 3.18, "[F-]"},
      {"[Cl;!H0]", -6.50, "[Cl-]"},
      {"[Br;!H0]", -8.50, "[Br-]"},
      {"[I;!H0]", -9.00, "[I-]"},
      {"[c;!H0]", 43.00, "[c-]"},
      {"[$([C]#N);!H0]", 9.30, "[$([C-]#N)]"},
      {"[C;!H0]", 50.00, "[C-]"},
      {"[nH;!H0]", 16.50, "[n-]"},
      {"[$([N]=N=*);!H0]", -99.99, "[$([N-]=N=*)]"},
      {"[$([N]C=O);!H0]", 22.00, "[$([N-]C=O)]"},
      {"[$([N]S(=O)=O);!H0]", 10.10, "[$([N-]S(=O)=O)]"},
      {"[N;!H0]", 32.50, "[N-]"},
      {"[nH2+;!H0]", -3.80, "[nH]"}, 
      {"[nH+;!H0]", 5.23, "[n]"},
      {"[$([NH+]#*);!H0]", -12.00, "[$([N]#*)]"},
      {"[$([NH+]=C(N)N);!H0]", 14.4, "[$([N]=C(N)N)]"},
      {"[$([NH+]=C(N)a);!H0]", 11.6, "[$([N]=C(N)a)]"},
      {"[$([NH+]=CN);!H0]", 12.4, "[$([N]=CN)]"},
      {"[$([NH+]=*);!H0]", -99.99, "[$([N]=*)]"},
      {"[$([NH+]a);!H0]", 4.69, "[$([N]a)]"},
      {"[$([NH+]C=O);!H0]", 4.74, "[$([N]C=O)]"},
      {"[$([NH+]C=N);!H0]", -99.99, "[$([N]C=N)]"},
      {"[$([NH+]S(=O)=O);!H0]", -99.99, "[$([N]S(=O)=O)]"},
      {"[NH+;!H0]", 10.5, "[N]"},
      {"[NH2+;!H0]", 11.1, "[$([N](C)C)]"},
      {"[NH3+;!H0]", 10.6, "[NH2]"},
      {"[NH4+;!H0]", 9.25, "[NH3]"},
      {"[OH2;!H0]", 15.70, "[OH-]"},
      {"[$([O]c);!H0]", 10.00, "[O-]a"},
      {"[$([O]C(=O)[O-]);!H0]", 10.33, "[$([O-]C(=O)[O-])]"},
      {"[$([O]C(=O)a);!H0]", 4.20, "[$([O-]C(=O)a)]"},
      {"[$([O]C=O);!H0]", 4.80, "[$([O-]C=O)]"},
      {"[$([O]C);!H0]", 15.50, "[$([O-]C)]"},
      {"[$([O]N(=O)=O);!H0]", -1.40, "[$([O-]N(=O)=O)]"},
      {"[$([O][N+]=O);!H0]", -12.00, "[$([O-][N+]=O)]"},
      {"[$([O]NC=O);!H0]", 9.40, "[$([O-]NC=O)]"},
      {"[$([O]N=*);!H0]", 12.34, "[$([O-]N=*)]"},
      {"[$([O]N(*)*);!H0]", 5.2, "[$([O-]N(*)*)]"},
      {"[$([O]N);!H0]", 5.96, "[$([O-]N)]"},
      {"[$([O]P([O-])([O-]));!H0]", 12.50, "[$([O-]P([O-])([O-]))]"},
      {"[$([O]P([O-])=O);!H0]", 6.70, "[$([O-]P([O-])=O)]"},
      {"[$([O]P=O);!H0]", 2.00, "[$([O-]P=O)]"},
      {"[$([O]P[O-]);!H0]", 99.99, "[$([O-]P[O-])]"},
      {"[$([O]Pa);!H0]", 2.10, "[$([O-]Pa)]"},
      {"[$([O]P);!H0]", 3.08, "[$([O-]P)]"},
      {"[$([O]S(=O)(=O)[O-]);!H0]", 2.0, "[$([O-]S(=O)(=O)[O-])]"},
      {"[$([O]S(=O)(=O));!H0]", -99.99, "[$([O-]S(=O)(=O))]"},
      {"[$([O]S(=O)[O-]);!H0]", 7.20, "[$([O-]S(=O)[O-])]"},
      {"[$([O]S(=O));!H0]", 1.80, "[$([O-]S(=O))]"},
      {"[O;!H0]", 99.99, "[O-]"},
      {"[OH+;!H0]", -1.74, "[O]"},
      {"[P;!H0]", 29.00, "[P-]"},
      {"[P+;!H0]", -13.00, "[P]"},
      {"[$([S]*=O);!H0]", 3.52, "[$([S-]*=O)]"},
      {"[$([S]a);!H0]", 6.52, "[$([S-]a)]"},
      {"[SH2;!H0]", 7.00, "[SH-]"},
      {"[S;!H0]", 12.00, "[S-]"},
      {"[SH+;!H0]", -7.00, "[S]"},
   };

   acids.clear();
   basics.clear();
   a_pkas.clear();
   b_pkas.clear();

   for (auto i = 0; i < NELEM(simple_pka_model); i++)
   {
      BufferScanner scanner(simple_pka_model[i].acid);
      SmilesLoader loader(scanner);
      QueryMolecule &acid = acids.push();
      loader.loadSMARTS(acid);
      a_pkas.push(simple_pka_model[i].pka);
   }

   for (auto i = 0; i < NELEM(simple_pka_model); i++)
   {
      BufferScanner scanner(simple_pka_model[i].basic);
      SmilesLoader loader(scanner);
      QueryMolecule &basic = basics.push();
      loader.loadSMARTS(basic);
      b_pkas.push(simple_pka_model[i].pka);
   }
}

void MoleculeIonizer::_estimate_pKa (Molecule &mol, const IonizeOptions &options, Array<int> &acid_sites,
                      Array<int> &basic_sites, Array<float> &acid_pkas, Array<float> &basic_pkas)
{
   QS_DEF(Array<int>, ignore_atoms);
   QS_DEF(Array<int>, mapping);
   QS_DEF(ObjArray<QueryMolecule>, acids);
   QS_DEF(ObjArray<QueryMolecule>, basics);
   QS_DEF(Array<float>, a_pkas);
   QS_DEF(Array<float>, b_pkas);
   AromaticityOptions opts;
   bool _dearomatize = false;

   _loadPkaModel(options, acids, a_pkas, basics, b_pkas);

   if (!mol.isAromatized())
   {
      if (mol.aromatize(opts))
         _dearomatize = true;
   }
   else
      _dearomatize = false;

   MoleculeSubstructureMatcher matcher(mol);
   matcher.fmcache = new MoleculeSubstructureMatcher::FragmentMatchCache;
   matcher.use_aromaticity_matcher = true;
   ignore_atoms.clear();
   for (auto i = 0; i < acids.size(); i++)
   {
      matcher.setQuery(acids[i]);

      for (int j = 0; j < ignore_atoms.size(); j++)
         matcher.ignoreTargetAtom(ignore_atoms[j]);

      if (!matcher.find())
         continue;

      for (;;)
      {
         mapping.clear();
         mapping.copy(matcher.getQueryMapping(), acids[i].vertexEnd());
         for (int j = 0; j < mapping.size(); j++)
         {
            if (mapping[j] > -1)
            {
               acid_sites.push(mapping[j]);
               acid_pkas.push(a_pkas[i]);
               ignore_atoms.push(mapping[j]);
//               printf("atom with index %d pKa = %f\n", mapping[j], a_pkas[i]);
            }  
         }
         if (!matcher.findNext())
            break;
      }
   }

   ignore_atoms.clear();
   for (auto i = 0; i < basics.size(); i++)
   {
      matcher.setQuery(basics[i]);

      for (int j = 0; j < ignore_atoms.size(); j++)
         matcher.ignoreTargetAtom(ignore_atoms[j]);

      if (!matcher.find())
         continue;

      for (;;)
      {
         mapping.clear();
         mapping.copy(matcher.getQueryMapping(), basics[i].vertexEnd());
         for (int j = 0; j < mapping.size(); j++)
         {
            if (mapping[j] > -1)
            {
               basic_sites.push(mapping[j]);
               basic_pkas.push(b_pkas[i]);
               ignore_atoms.push(mapping[j]);
//               printf("atom with index %d pKa = %f\n", mapping[j], b_pkas[i]);
            }  
         }
         if (!matcher.findNext())
            break;
      }
   }

   if (_dearomatize)
      mol.dearomatize(opts);
}

void MoleculeIonizer::_setCharges (Molecule &mol, float pH, float pH_toll, const IonizeOptions &options, Array<int> &acid_sites,
                      Array<int> &basic_sites, Array<float> &acid_pkas, Array<float> &basic_pkas)
{
   for (auto i = 0; i < acid_sites.size(); i++)
   {
      if ((acid_pkas[i] - pH) < pH_toll)
         mol.setAtomCharge(acid_sites[i], mol.getAtomCharge(acid_sites[i]) - 1);
   }

   for (auto i = 0; i < basic_sites.size(); i++)
   {
      if ((basic_pkas[i] - pH) > -pH_toll)
         mol.setAtomCharge(basic_sites[i], mol.getAtomCharge(basic_sites[i]) + 1);
   }
}
