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

#include <vector>
#include <string>
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
        std::vector<std::pair<int, std::string>> molecules;

        for(int i=0; i<mol.countComponents(); i++)
        {
            Molecule submol;
            Filter filter(mol.getDecomposition().ptr(), Filter::EQ, i);
            submol.makeSubmolecule(mol, filter, 0, 0);

            _processMolecule(submol);
            _smilesSaver->saveMolecule(submol);

            std::string smile(_buffer.ptr(), _buffer.size());
            molecules.push_back(std::make_pair(mol.getAtomNumber(i), smile));
            _buffer.clear();
        } 

        auto comparison = [](const std::pair<int, std::string>& p1, const std::pair<int, std::string>& p2)->bool
        {
            if(p1.first > p2.first) return true;
            if(p1.first == p2.first) return p1.second > p2.second;
            return false;
        };
        std::sort(molecules.begin(), molecules.end(), comparison);

        for(size_t i=0; i<molecules.size(); i++)
        {
            _output.writeString(molecules[i].second.c_str());
            if(i<molecules.size()-1)
                _output.writeChar('.');
        }
    }
    else
    {
        _processMolecule(mol);
        _smilesSaver->saveMolecule(mol);
        _output.write(_buffer.ptr(), _buffer.size());
    }
}

void CanonicalSmilesSaver::_processMolecule (Molecule &mol_)
{
   if (mol_.vertexCount() < 1)
      return;

   QS_DEF(Array<int>, ignored);
   QS_DEF(Array<int>, order);
   QS_DEF(Array<int>, ranks);
   QS_DEF(Molecule, mol);

   int i;

   if (mol_.sgroups.isPolimer())
      throw Error("can not canonicalize a polymer");

   // Detect hydrogens configuration if aromatic but not ambiguous
   // We can store this infromation in the original structure mol_.
   mol_.restoreAromaticHydrogens();

   mol.clone(mol_, 0, 0);

   // TODO: canonicalize allenes properly
   mol.allene_stereo.clear();

   ignored.clear_resize(mol.vertexEnd());
   ignored.zerofill();

   for (i = mol.vertexBegin(); i < mol.vertexEnd(); i = mol.vertexNext(i))
      if (mol.convertableToImplicitHydrogen(i))
         ignored[i] = 1;

   // Try to save into ordinary smiles and find what cis-trans bonds were used
   NullOutput null_output;
   SmilesSaver saver_cistrans(null_output);
   saver_cistrans.ignore_hydrogens = true;
   saver_cistrans.saveMolecule(mol);
   // Then reset cis-trans infromation that is not saved into SMILES
   const Array<int>& parities = saver_cistrans.getSavedCisTransParities();
   for (i = mol.edgeBegin(); i < mol.edgeEnd(); i = mol.edgeNext(i))
   {
      if (mol.cis_trans.getParity(i) != 0 && parities[i] == 0)
         mol.cis_trans.setParity(i, 0);
   }

   MoleculeAutomorphismSearch of;

   of.detect_invalid_cistrans_bonds = find_invalid_stereo;
   of.detect_invalid_stereocenters = find_invalid_stereo;
   of.find_canonical_ordering = true;
   of.ignored_vertices = ignored.ptr();
   of.process(mol);
   of.getCanonicalNumbering(order);

   for (i = mol.edgeBegin(); i != mol.edgeEnd(); i = mol.edgeNext(i))
      if (mol.cis_trans.getParity(i) != 0 && of.invalidCisTransBond(i))
         mol.cis_trans.setParity(i, 0);

   for (i = mol.vertexBegin(); i != mol.vertexEnd(); i = mol.vertexNext(i))
      if (mol.stereocenters.getType(i) > MoleculeStereocenters::ATOM_ANY && of.invalidStereocenter(i))
         mol.stereocenters.remove(i);

   ranks.clear_resize(mol.vertexEnd());

   for (i = 0; i < order.size(); i++)
      ranks[order[i]] = i;

   _smilesSaver->vertex_ranks = ranks.ptr();

   _actual_atom_atom_mapping.clear_resize(mol.vertexCount());
   _actual_atom_atom_mapping.zerofill();

   for (int i = 0; i < order.size(); ++i) {
      int aam = mol.reaction_atom_mapping[order[i]];
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
   mol.reaction_atom_mapping.copy(_actual_atom_atom_mapping);
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