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

#include "molecule/canonical_smiles_saver.h"

#include "base_cpp/output.h"
#include "base_cpp/tlscont.h"
#include "molecule/molecule.h"
#include "molecule/smiles_saver.h"
#include "molecule/molecule_automorphism_search.h"
#include "molecule/elements.h"
#include "molecule/molecule_dearom.h"

#include <algorithm>

using namespace indigo;

IMPL_ERROR(CanonicalSmilesSaver, "canonical SMILES saver");

CP_DEF(CanonicalSmilesSaver);

CanonicalSmilesSaver::CanonicalSmilesSaver (Output &output) : CP_INIT,
TL_CP_GET(_actual_atom_atom_mapping),
TL_CP_GET(_initial_to_actual),
_output(output)
{
   find_invalid_stereo = true;   
   _initial_to_actual.clear();
   _initial_to_actual.insert(0, 0);
   _aam_counter = 0;
   
   _arrayOutput = std::unique_ptr<ArrayOutput>(new ArrayOutput(_buffer));
   _smilesSaver = std::unique_ptr<SmilesSaver>(new SmilesSaver(*_arrayOutput));
   _smilesSaver->ignore_invalid_hcount = false;
   _smilesSaver->ignore_hydrogens = true;
   _smilesSaver->canonize_chiralities = true;
}

CanonicalSmilesSaver::~CanonicalSmilesSaver ()
{
}

void CanonicalSmilesSaver::saveMolecule (Molecule &mol)
{
    if(mol.countComponents() > MAX_NUMBER_OF_COMPONENTS)
    {
        typedef std::unique_ptr<Molecule> MoleculePtr;
        typedef std::pair<MoleculePtr, std::vector<int>> ProcMol;
        std::vector<ProcMol> molecules(mol.countComponents());
        for(int i=0; i<mol.countComponents(); i++)
        {
            Molecule submol;
            molecules[i].first = MoleculePtr(new Molecule());
            Filter filter(mol.getDecomposition().ptr(), Filter::EQ, i);
            submol.makeSubmolecule(mol, filter, 0, 0);

            _processMolecule(submol, *molecules[i].first, molecules[i].second);
        }
        
        auto comporator = [&](ProcMol& m1, ProcMol& m2)
        {                    
            if(m1.first->vertexCount() > m2.first->vertexCount()) return true;
            if(m1.first->vertexCount() == m2.first->vertexCount())
            {
                _smilesSaver->vertex_ranks = &m1.second[0];
                _smilesSaver->saveMolecule(*m1.first);
                std::string smile1(_buffer.ptr(), _buffer.size());
                _buffer.clear();
                _smilesSaver->vertex_ranks = &m2.second[0];
                _smilesSaver->saveMolecule(*m2.first);
                std::string smile2(_buffer.ptr(), _buffer.size());
                _buffer.clear();
                return smile1<smile2;   
            }
            return false;
        };
        
        std::sort(molecules.begin(), molecules.end(), comporator);
        
        Molecule allMolecules;
        std::vector<int> allRanks;
        int shift=0;
        for(auto& mol : molecules)
        {
            auto& ranks = mol.second;
            for(auto& rank : ranks) 
                rank+=shift;
            shift += ranks.size();
            allRanks.insert(allRanks.end(), ranks.begin(), ranks.end());
            allMolecules.mergeWithMolecule(*mol.first, 0);
        }       
                
        _smilesSaver->vertex_ranks = &allRanks[0];
        _smilesSaver->saveMolecule(allMolecules);
    }
    else
    {
        Molecule prcmol;
        std::vector<int> ranks;
        _processMolecule(mol, prcmol, ranks);
        _smilesSaver->vertex_ranks = &ranks[0];
        _smilesSaver->saveMolecule(prcmol);
    }
    _output.write(_buffer.ptr(), _buffer.size());
    _buffer.clear();
}

void CanonicalSmilesSaver::_processMolecule (Molecule &mol, Molecule &prcmol, std::vector<int>& ranks)
{
   if (mol.vertexCount() < 1)
      return;

   QS_DEF(Array<int>, ignored);
   QS_DEF(Array<int>, order);

   int i;

   if (mol.sgroups.isPolimer())
      throw Error("can not canonicalize a polymer");

   // Detect hydrogens configuration if aromatic but not ambiguous
   // We can store this infromation in the original structure mol.
   mol.restoreAromaticHydrogens();

   prcmol.clone(mol, 0, 0);

   // TODO: canonicalize allenes properly
   prcmol.allene_stereo.clear();

   ignored.clear_resize(prcmol.vertexEnd());
   ignored.zerofill();

   for (i = prcmol.vertexBegin(); i < prcmol.vertexEnd(); i = prcmol.vertexNext(i))
      if (prcmol.convertableToImplicitHydrogen(i))
         ignored[i] = 1;

   // Try to save into ordinary smiles and find what cis-trans bonds were used
   NullOutput null_output;
   SmilesSaver saver_cistrans(null_output);
   saver_cistrans.ignore_hydrogens = true;
   saver_cistrans.saveMolecule(prcmol);
   // Then reset cis-trans infromation that is not saved into SMILES
   const Array<int>& parities = saver_cistrans.getSavedCisTransParities();
   for (i = prcmol.edgeBegin(); i < prcmol.edgeEnd(); i = prcmol.edgeNext(i))
   {
      if (prcmol.cis_trans.getParity(i) != 0 && parities[i] == 0)
         prcmol.cis_trans.setParity(i, 0);
   }

   MoleculeAutomorphismSearch of;

   of.detect_invalid_cistrans_bonds = find_invalid_stereo;
   of.detect_invalid_stereocenters = find_invalid_stereo;
   of.find_canonical_ordering = true;
   of.ignored_vertices = ignored.ptr();
   of.process(prcmol);
   of.getCanonicalNumbering(order);

   for (i = prcmol.edgeBegin(); i != prcmol.edgeEnd(); i = prcmol.edgeNext(i))
      if (prcmol.cis_trans.getParity(i) != 0 && of.invalidCisTransBond(i))
         prcmol.cis_trans.setParity(i, 0);

   for (i = prcmol.vertexBegin(); i != prcmol.vertexEnd(); i = prcmol.vertexNext(i))
      if (prcmol.stereocenters.getType(i) > MoleculeStereocenters::ATOM_ANY && of.invalidStereocenter(i))
         prcmol.stereocenters.remove(i);

   ranks.clear();
   ranks.resize(prcmol.vertexEnd());

   for (i = 0; i < order.size(); i++)
      ranks[order[i]] = i;

   _actual_atom_atom_mapping.clear_resize(prcmol.vertexCount());
   _actual_atom_atom_mapping.zerofill();

   for (int i = 0; i < order.size(); ++i) {
      int aam = prcmol.reaction_atom_mapping[order[i]];
      if (aam) {
         if (!_initial_to_actual.find(aam)) {
            _initial_to_actual.insert(aam, ++_aam_counter);
            _actual_atom_atom_mapping[order[i]] = _aam_counter;
         }
         else {
            _actual_atom_atom_mapping[order[i]] = _initial_to_actual.at(aam);
         }
      }
   }
   prcmol.reaction_atom_mapping.copy(_actual_atom_atom_mapping);
}

void CanonicalSmilesSaver::setSmartsMode(bool smarts_mode)
{
    _smilesSaver->smarts_mode = smarts_mode;
}

void CanonicalSmilesSaver::saveQueryMolecule (QueryMolecule &mol)
{
    _smilesSaver->saveQueryMolecule(mol);
}

void CanonicalSmilesSaver::setInsideRsmiles(bool inside_rsmiles)
{
    _smilesSaver->inside_rsmiles = inside_rsmiles;
}

int CanonicalSmilesSaver::writtenComponents ()
{
   return _smilesSaver->writtenComponents();
}

const Array<int> & CanonicalSmilesSaver::writtenAtoms ()
{
   return _smilesSaver->writtenAtoms();
}

const Array<int> & CanonicalSmilesSaver::writtenBonds ()
{
    return _smilesSaver->writtenBonds();
}