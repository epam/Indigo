/****************************************************************************
 * Copyright (C) 2009-2011 GGA Software Services LLC
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

#include "molecule/smiles_saver.h"
#include "molecule/molecule_rgroups.h"
#include "base_cpp/array.h"
#include "base_cpp/tlscont.h"
#include "base_cpp/output.h"
#include "molecule/molecule.h"
#include "molecule/query_molecule.h"
#include "molecule/molecule_stereocenters.h"
#include "graph/dfs_walk.h"
#include "molecule/elements.h"
#include "graph/graph_highlighting.h"
#include "molecule/molecule_arom_match.h"

using namespace indigo;

SmilesSaver::SmilesSaver (Output &output) : _output(output),
TL_CP_GET(_neipool),
TL_CP_GET(_atoms),
TL_CP_GET(_hcount),
TL_CP_GET(_dbonds),
TL_CP_GET(_written_atoms),
TL_CP_GET(_written_bonds)
{
   vertex_ranks = 0;
   atom_atom_mapping = 0;
   ignore_hydrogens = false;
   canonize_chiralities = false;
   write_extra_info = true;
   highlighting = 0;
   _mol = 0;
   smarts_mode = false;
   ignore_invalid_hcount = false;
}

SmilesSaver::~SmilesSaver ()
{
   _atoms.clear(); // to avoid data race when it is reused in another thread
}

void SmilesSaver::saveMolecule (Molecule &mol)
{
   _bmol = &mol;
   _qmol = 0;
   _mol = &mol;
   _saveMolecule();
}

void SmilesSaver::saveQueryMolecule (QueryMolecule &mol)
{
   _bmol = &mol;
   _qmol = &mol;
   _mol = 0;
   _saveMolecule();
}

void SmilesSaver::_saveMolecule ()
{
   QS_DEF(Array<int>, ignored_vertices);
   int i, j, k;

   _touched_cistransbonds = 0;
   _markCisTrans();

   _atoms.clear();
   while (_atoms.size() < _bmol->vertexEnd())
      _atoms.push(_neipool);

   ignored_vertices.clear_resize(_bmol->vertexEnd());
   ignored_vertices.zerofill();
   _written_atoms.clear();
   _written_bonds.clear();
   _written_components = 0;

   if (ignore_hydrogens)
   {
      if (_qmol != 0)
         throw Error("ignore_hydrogens does not make sense for query molecules");
      
      for (i = _bmol->vertexBegin(); i < _bmol->vertexEnd(); i = _bmol->vertexNext(i))
         if (_bmol->getAtomNumber(i) == ELEM_H && _bmol->getAtomIsotope(i) == 0 &&
             _bmol->getVertex(i).degree() == 1)
         {
            int nei = _bmol->getVertex(i).neiVertex(_bmol->getVertex(i).neiBegin());
            
            if (_bmol->getAtomNumber(nei) != ELEM_H)
            {
               if (_bmol->stereocenters.getType(nei) > 0)
                  if (_bmol->getVertex(nei).degree() == 3)
                     continue; // not ignoring hydrogens around stereocenters with lone pair
               
               ignored_vertices[i] = 1;
            }
         }
   }

   _hcount.clear_resize(_bmol->vertexEnd());

   for (i = _bmol->vertexBegin(); i < _bmol->vertexEnd(); i = _bmol->vertexNext(i))
   {
      _hcount[i] = 0;
      
      if (_mol != 0 && !_mol->isPseudoAtom(i) && !_mol->isRSite(i))
         _hcount[i] = _mol->getImplicitH(i);

      const Vertex &vertex = _bmol->getVertex(i);

      if (ignore_hydrogens)
      {
         if (_hcount[i] >= 0)
            for (j = vertex.neiBegin(); j != vertex.neiEnd(); j = vertex.neiNext(j))
            {
               int idx = vertex.neiVertex(j);
               if (_bmol->getAtomNumber(idx) == ELEM_H && _bmol->getAtomIsotope(idx) == 0)
                  if (ignored_vertices[idx])
                     _hcount[i]++;
            }
      }

      if (_bmol->getAtomAromaticity(i) == ATOM_AROMATIC)
      {
         _atoms[i].aromatic = true;
         static int allowed_lowercase[] = {ELEM_B, ELEM_C, ELEM_N, ELEM_O, ELEM_P, ELEM_S};
         if (_bmol->atomNumberBelongs(i, allowed_lowercase, NELEM(allowed_lowercase)))
            _atoms[i].lowercase = true;
      }
   }

   DfsWalk walk(*_bmol);
   
   walk.ignored_vertices = ignored_vertices.ptr();
   walk.vertex_ranks = vertex_ranks;

   for (i = _bmol->vertexBegin(); i < _bmol->vertexEnd(); i = _bmol->vertexNext(i))
   {
      if (_bmol->isRSite(i))
         // We break the DFS walk when going through R-sites. For details, see
         // http://blueobelisk.shapado.com/questions/how-r-group-atoms-should-be-represented-in-smiles
         walk.mustBeRootVertex(i);
   }

   walk.walk();

   const Array<DfsWalk::SeqElem> &v_seq = walk.getSequence();
   
   // fill up neighbor lists for the stereocenters calculation
   for (i = 0; i < v_seq.size(); i++)
   {
      int v_idx = v_seq[i].idx;
      int e_idx = v_seq[i].parent_edge;
      int v_prev_idx = v_seq[i].parent_vertex;

      _Atom &atom = _atoms[v_idx];

      if (e_idx >= 0)
      {
         if (walk.isClosure(e_idx))
         {
            int k;
            for (k = atom.neighbors.begin(); k != atom.neighbors.end(); k =
                     atom.neighbors.next(k))
            {
               if (atom.neighbors[k] == -1)
               {
                  atom.neighbors[k] = v_prev_idx;
                  break;
               }
            }
            if (k == atom.neighbors.end())
               throw Error("internal: can not put closing bond to its place");
         }
         else
         {
            atom.neighbors.add(v_prev_idx);
            atom.parent = v_prev_idx;
         }
         _atoms[v_prev_idx].neighbors.add(v_idx);
      }

      if (e_idx < 0 || !walk.isClosure(e_idx))
      {
         int openings = walk.numOpenings(v_idx);

         for (j = 0; j < openings; j++)
            atom.neighbors.add(-1);
      }
   }

   // detect chiral configurations
   MoleculeStereocenters &stereocenters = _bmol->stereocenters;

   for (i = stereocenters.begin(); i != stereocenters.end(); i = stereocenters.next(i))
   {
      int atom_idx, type, group, pyramid[4];

      stereocenters.get(i, atom_idx, type, group, pyramid);

      if (type < MoleculeStereocenters::ATOM_AND)
         continue;

      int implicit_h_idx = -1;

      if (pyramid[3] == -1)
         implicit_h_idx = 3;
      else for (j = 0; j < 4; j++)
         if (ignored_vertices[pyramid[j]])
         {
            implicit_h_idx = j;
            break;
         }

      int pyramid_mapping[4];
      int counter = 0;

      _Atom &atom = _atoms[atom_idx];

      if (atom.parent != -1)
         for (k = 0; k < 4; k++)
            if (pyramid[k] == atom.parent)
            {
               pyramid_mapping[counter++] = k;
               break;
            }

      if (implicit_h_idx != -1)
         pyramid_mapping[counter++] = implicit_h_idx;
         
      for (j = atom.neighbors.begin(); j != atom.neighbors.end(); j = atom.neighbors.next(j))
      {
         if (atom.neighbors[j] == atom.parent)
            continue;
         
         for (k = 0; k < 4; k++)
            if (atom.neighbors[j] == pyramid[k])
            {
               if (counter >= 4)
                  throw Error("internal: pyramid overflow");
               pyramid_mapping[counter++] = k;
               break;
            }
      }

      if (counter == 4)
      {
         // move the 'from' atom to the end
         counter = pyramid_mapping[0];
         pyramid_mapping[0] = pyramid_mapping[1];
         pyramid_mapping[1] = pyramid_mapping[2];
         pyramid_mapping[2] = pyramid_mapping[3];
         pyramid_mapping[3] = counter;
      }
      else if (counter != 3)
         throw Error("cannot calculate chirality");

      if (MoleculeStereocenters::isPyramidMappingRigid(pyramid_mapping))
         _atoms[atom_idx].chirality = 1;
      else
         _atoms[atom_idx].chirality = 2;
   }

   for (i = 0; i < v_seq.size(); i++)
   {
      int v_idx = v_seq[i].idx;
      int e_idx = v_seq[i].parent_edge;

      if (e_idx == -1 || !walk.isClosure(e_idx))
         _written_atoms.push(v_idx);

      if (e_idx != -1)
         _written_bonds.push(e_idx);
   }

   if (canonize_chiralities)
   {
      int i, j;
      QS_DEF(Array<int>, marked);
      QS_DEF(Array<int>, ids);
      const MoleculeStereocenters &stereocenters = _bmol->stereocenters;

      marked.clear_resize(_bmol->vertexEnd());
      marked.zerofill();

      for (i = 0; i < _written_atoms.size(); i++)
      {
         if (marked[i])
            continue;

         int idx = _written_atoms[i];
         
         if (_atoms[idx].chirality == 0)
            continue;

         int type = stereocenters.getType(idx);

         if (type != MoleculeStereocenters::ATOM_AND && type != MoleculeStereocenters::ATOM_OR)
            continue;

         ids.clear();
         ids.push(idx);

         int group = stereocenters.getGroup(idx);

         for (j = i + 1; j < _written_atoms.size(); j++)
         {
            if (marked[j])
               continue;

            int idx2 = _written_atoms[j];

            if (_atoms[idx2].chirality == 0)
               continue;

            int type2 = stereocenters.getType(idx2);
            int group2 = stereocenters.getGroup(idx2);

            if (type2 == type && group2 == group)
            {
               ids.push(idx2);
               marked[j] = 1;
            }
         }

         if (_atoms[ids[0]].chirality == 1)
            for (j = 0; j < ids.size(); j++)
               _atoms[ids[j]].chirality = 3 - _atoms[ids[j]].chirality;
      }
   }

   // write the SMILES itself
   
   // cycle_numbers[i] == -1 means that the number is available
   // cycle_numbers[i] == n means that the number is used by vertex n
   QS_DEF(Array<int>, cycle_numbers);

   int rsites_closures_starting_num = 91;

   cycle_numbers.clear();
   cycle_numbers.push(0); // never used
   
   bool first_component = true;
   
   for (i = 0; i < v_seq.size(); i++)
   {
      int v_idx = v_seq[i].idx;
      int e_idx = v_seq[i].parent_edge;
      int v_prev_idx = v_seq[i].parent_vertex;
      bool write_atom = true;

      if (v_prev_idx >= 0)
      {
         if (walk.numBranches(v_prev_idx) > 1)
            if (_atoms[v_prev_idx].branch_cnt > 0 && _atoms[v_prev_idx].paren_written)
               _output.writeChar(')');

         if (v_prev_idx >= 0)
         {
            int branches = walk.numBranches(v_prev_idx);
            
            if (branches > 1)
               if (_atoms[v_prev_idx].branch_cnt < branches - 1)
               {
                  if (walk.isClosure(e_idx))
                     _atoms[v_prev_idx].paren_written = false;
                  else
                  {
                     _output.writeChar('(');
                     _atoms[v_prev_idx].paren_written = true;
                  }
               }

            _atoms[v_prev_idx].branch_cnt++;

            if (_atoms[v_prev_idx].branch_cnt > branches)
               throw Error("unexpected branch");
         }

         const Edge &edge = _bmol->getEdge(e_idx);
         bool bond_written = true;

         int dir = 0;
         int bond_order = _bmol->getBondOrder(e_idx);

         if (bond_order == BOND_SINGLE)
            dir = _calcBondDirection(e_idx, v_prev_idx);

         if ((dir == 1 && v_idx == edge.end) || (dir == 2 && v_idx == edge.beg))
            _output.writeChar('/');
         else if ((dir == 2 && v_idx == edge.end) || (dir == 1 && v_idx == edge.beg))
            _output.writeChar('\\');
         else if (smarts_mode)
            _writeSmartsBond(e_idx, &_qmol->getBond(e_idx));
         else if (bond_order == BOND_DOUBLE)
            _output.writeChar('=');
         else if (bond_order == BOND_TRIPLE)
            _output.writeChar('#');
         else if (bond_order == BOND_AROMATIC &&
                 (!_atoms[edge.beg].lowercase || !_atoms[edge.end].lowercase ||
                  _bmol->getBondTopology(e_idx) != TOPOLOGY_RING))
            _output.writeChar(':');
         else if (bond_order == BOND_SINGLE && _atoms[edge.beg].aromatic && _atoms[edge.end].aromatic)
            _output.writeChar('-');
         else
            bond_written = false;

         if (walk.isClosure(e_idx))
         {
            for (j = 1; j < cycle_numbers.size(); j++)
               if (cycle_numbers[j] == v_idx)
                  break;

            if (j == cycle_numbers.size())
               throw Error("cycle number not found");

            _writeCycleNumber(j);

            cycle_numbers[j] = -1;
            write_atom = false;
         }
      }
      else
      {
         if (!first_component)
            _output.writeChar('.');
         first_component = false;
         _written_components++;
      }
      if (write_atom)
      {
         if (!smarts_mode)
            _writeAtom(v_idx, _atoms[v_idx].aromatic,
                       _atoms[v_idx].lowercase, _atoms[v_idx].chirality);
         else
            _writeSmartsAtom(v_idx, &_qmol->getAtom(v_idx), _atoms[v_idx].chirality, 0, false);

         QS_DEF(Array<int>, closing);

         walk.getNeighborsClosing(v_idx, closing);

         for (j = 0; j < closing.size(); j++)
         {
            if (_bmol->isRSite(closing[j]))
            {
               cycle_numbers.expandFill(rsites_closures_starting_num + 1, -1);
               for (k = rsites_closures_starting_num; k < cycle_numbers.size(); k++)
                  if (cycle_numbers[k] == -1)
                     break;
            }
            else
            {
               for (k = 1; k < cycle_numbers.size(); k++)
                  if (cycle_numbers[k] == -1)
                     break;
            }
            if (k == cycle_numbers.size())
               cycle_numbers.push(v_idx);
            else
               cycle_numbers[k] = v_idx;

            _writeCycleNumber(k);
         }
      }
   }

   if (write_extra_info)
   {
      _comma = false;
      _writeStereogroups();
      _writeRadicals();
      _writePseudoAtoms();
      _writeHighlighting();

      if (_comma)
         _output.writeChar('|');
   }
}

void SmilesSaver::_writeCycleNumber (int n) const
{
   if (n > 0 && n < 10)
      _output.printf("%d", n);
   else if (n >= 10 && n < 100)
      _output.printf("%%%2d", n);
   else if (n >= 100 && n < 1000)
      _output.printf("%%%%%3d", n);
   else 
      throw Error("bad cycle number: %d", n);
}

void SmilesSaver::_writeAtom (int idx, bool aromatic, bool lowercase, int chirality) const
{
   int i;
   bool need_brackets = false;
   int hydro = -1;
   int aam = 0;

   if (_bmol->isRSite(idx))
   {
      if (_bmol->getRSiteBits(idx) == 0)
         _output.printf("[*]");
      else
         _output.printf("[*:%d]",  _bmol->getSingleAllowedRGroup(idx));
      return;
   }

   int atom_number = _bmol->getAtomNumber(idx);

   if (_bmol->isPseudoAtom(idx)) // pseudo-atom
   {
      _output.printf("[*]");
      return;
   }

   if (atom_number < 1)
      throw Error("zero atom number");

   if (atom_atom_mapping != 0)
      aam = atom_atom_mapping[idx];

   if (atom_number != ELEM_C && atom_number != ELEM_P &&
       atom_number != ELEM_N && atom_number != ELEM_S &&
       atom_number != ELEM_O && atom_number != ELEM_Cl &&
       atom_number != ELEM_F && atom_number != ELEM_Br &&
       atom_number != ELEM_B && atom_number != ELEM_I)
      need_brackets = true;

   // Now we decide to write or not to write the hydrogen count to SMILES.
   // In a better world, we would have been checking that the hydrogens
   // 'make difference' by de-aromatizing the molecule and comparing
   // the hydrogen counts in the de-aromatized atoms with the atoms we
   // are writing now.
   // In the real world, de-aromatization is complicated and takes time,
   // so we write hydrogen counts if (i) there is a radical on the atom or
   // (ii) atom is aromatic (does not apply to C and O, for which we can
   // always tell the number of hydrogens by the charge, radical, and the
   // number of bonds).
   if (_bmol->getAtomRadical(idx) != 0 ||
       (aromatic && atom_number != ELEM_C && atom_number != ELEM_O))
   {
      hydro = _hcount[idx];
      if (hydro < 0 && !ignore_invalid_hcount)
         throw Error("unsure hydrogen count on atom #%d", idx);
   }

   int charge = _bmol->getAtomCharge(idx);
   int isotope = _bmol->getAtomIsotope(idx);

   if (charge == CHARGE_UNKNOWN)
      charge = 0;

   if (chirality > 0 || charge != 0 || isotope > 0 || hydro >= 0 || aam > 0)
      need_brackets = true;

   if (need_brackets)
   {
      if (hydro == -1)
      {
         hydro = _hcount[idx];
         if (hydro < 0 && !ignore_invalid_hcount)
            throw Error("unsure hydrogen count on atom #%d", idx);
      }
      _output.writeChar('[');
   }

   if (isotope > 0)
      _output.printf("%d", isotope);

   const char *elem = Element::toString(atom_number);

   if (lowercase)
   {
      for (i = 0; i < (int)strlen(elem); i++)
         _output.printf("%c", tolower(elem[i]));
   }
   else
      _output.printf("%s", elem);

   if (chirality > 0)
   {
      if (chirality == 1)
         _output.printf("@");
      else // chirality == 2
         _output.printf("@@");
   }

   if (hydro > 1)
      _output.printf("H%d", hydro);
   else if (hydro == 1)
      _output.printf("H");

   if (charge > 1)
      _output.printf("+%d", charge);
   else if (charge < -1)
      _output.printf("-%d", -charge);
   else if (charge == 1)
      _output.printf("+");
   else if (charge == -1)
      _output.printf("-");

   if (aam > 0)
      _output.printf(":%d", aam);

   if (need_brackets)
      _output.writeChar(']');

   /* DPX: take care of r-sites in SMILES
   if (_qmol != 0 && _qmol->isRGroupFragment())
   {
      for (i = 0; i < 2; i++)
      {
         int j;

         for (j = 0; _qmol->getRGroupFragment().getAttachmentPoint(i, j) != -1; j++)
            if (idx == _qmol->getRGroupFragment().getAttachmentPoint(i, j))
            {
               _output.printf("([*])");
               break;
            }

         if (_qmol->getRGroupFragment().getAttachmentPoint(i, j) != -1)
            break;
      }
   }*/
}

void SmilesSaver::_writeSmartsAtom (int idx, QueryMolecule::Atom *atom, int chirality, int depth, bool has_or_parent) const
{
   int i;

   if (depth == 0)
      _output.printf("[");

   switch (atom->type)
   {
      case QueryMolecule::OP_NOT:
      {
         _output.writeChar('!');
         _writeSmartsAtom(idx, (QueryMolecule::Atom *)atom->children[0], chirality, depth + 1, has_or_parent);
         break;
      }
      case QueryMolecule::OP_AND:
      {
         for (i = 0; i < atom->children.size(); i++)
         {
            if (i > 0)
               _output.writeChar(has_or_parent ? '&' : ';');
            _writeSmartsAtom(idx, (QueryMolecule::Atom *)atom->children[i], 0, depth + 1, has_or_parent);
         }
         break;
      }
      case QueryMolecule::OP_OR:
      {
         for (i = 0; i < atom->children.size(); i++)
         {
            if (i > 0)
               _output.printf(",");
            _writeSmartsAtom(idx, (QueryMolecule::Atom *)atom->children[i], 0, depth + 1, true);
         }
         break;
      }
      case QueryMolecule::ATOM_ISOTOPE:
         _output.printf("%d", atom->value_max);
         break;
      case QueryMolecule::ATOM_NUMBER:
      {
         _output.printf("#%d", atom->value_max);
         if (chirality == 1)
            _output.printf("@");
         else if (chirality == 2)
            _output.printf("@@");

         if (chirality > 0 || _bmol->getAtomRadical(idx) != 0)
         {
            int hydro = _bmol->getAtomTotalH(idx);

            if (hydro > 1)
               _output.printf("H%d", hydro);
            else if (hydro == 1)
               _output.printf("H");
         }
         if (atom_atom_mapping != 0)
         {
            int aam = atom_atom_mapping[idx];

            if (aam > 0)
               _output.printf(":%d", aam);
         }
         break;
      }
      case QueryMolecule::ATOM_CHARGE:
      {
         int charge = atom->value_max;

         if (charge > 1)
            _output.printf("+%d", charge);
         else if (charge < -1)
            _output.printf("-%d", -charge);
         else if (charge == 1)
            _output.printf("+");
         else if (charge == -1)
            _output.printf("-");
         break;
      }
      case QueryMolecule::ATOM_FRAGMENT:
      {
         if (atom->fragment->fragment_smarts.ptr() == 0)
            throw Error("fragment_smarts has unexpectedly gone");
         _output.printf("$(%s)", atom->fragment->fragment_smarts.ptr());
         break;
      }
      case QueryMolecule::ATOM_AROMATICITY:
      {
         if (atom->value_min == ATOM_AROMATIC)
            _output.printf("a");
         else
            _output.printf("A");
         break;
      }
      case QueryMolecule::OP_NONE:
         _output.writeChar('*');
         break;
      default:
         ;
   }

   if (depth == 0)
      _output.writeChar(']');
}

void SmilesSaver::_writeSmartsBond (int idx, QueryMolecule::Bond *bond) const
{
   int i;

   switch (bond->type)
   {
      case QueryMolecule::OP_NONE:
         _output.writeChar('~');
         break;
      case QueryMolecule::OP_NOT:
      {
         _output.writeChar('!');
         _writeSmartsBond(idx, (QueryMolecule::Bond *)bond->children[0]);
         break;
      }
      case QueryMolecule::OP_OR:
      {
         for (i = 0; i < bond->children.size(); i++)
         {
            if (i > 0)
               _output.printf(",");
            _writeSmartsBond(idx, (QueryMolecule::Bond *)bond->children[i]);
         }
         break;
      }
      case QueryMolecule::BOND_ORDER:
      {
         int bond_order = bond->value;

         if (bond_order == BOND_SINGLE)
            _output.writeChar('-');
         if (bond_order == BOND_DOUBLE)
            _output.writeChar('=');
         else if (bond_order == BOND_TRIPLE)
            _output.writeChar('#');
         else if (bond_order == BOND_AROMATIC)
            _output.writeChar(':');
         break;
      }
      default:
         ;
   }
}


void SmilesSaver::_markCisTrans ()
{
   BaseMolecule &mol = *_bmol;
   int i, j;

   _dbonds.clear_resize(mol.edgeEnd());

   for (i = mol.edgeBegin(); i != mol.edgeEnd(); i = mol.edgeNext(i))
   {
      _dbonds[i].ctbond_beg = -1;
      _dbonds[i].ctbond_end = -1;
      _dbonds[i].saved = 0;
   }

   if (!mol.cis_trans.exists())
      return;

   for (i = mol.edgeBegin(); i != mol.edgeEnd(); i = mol.edgeNext(i))
      if (mol.cis_trans.getParity(i) != 0 && mol.getBondTopology(i) != TOPOLOGY_RING)
      {
         const Edge &edge = mol.getEdge(i);
         const Vertex &beg = mol.getVertex(edge.beg);
         const Vertex &end = mol.getVertex(edge.end);
         bool arom_fail_beg = true, arom_fail_end = true;

         for (j = beg.neiBegin(); j != beg.neiEnd(); j = beg.neiNext(j))
         {
            int idx = beg.neiEdge(j);

            if (idx != i && mol.getBondOrder(idx) == BOND_SINGLE)
               arom_fail_beg = false;
         }

         for (j = end.neiBegin(); j != end.neiEnd(); j = end.neiNext(j))
         {
            int idx = end.neiEdge(j);

            if (idx != i && mol.getBondOrder(idx) == BOND_SINGLE)
               arom_fail_end = false;
         }

         if (arom_fail_beg || arom_fail_end)
            continue;


         for (j = beg.neiBegin(); j != beg.neiEnd(); j = beg.neiNext(j))
         {
            int idx = beg.neiEdge(j);

            if (idx != i)
            {
               if (mol.getEdge(idx).beg == edge.beg)
                  _dbonds[idx].ctbond_beg = i;
               else
                  _dbonds[idx].ctbond_end = i;
            }
         }

         for (j = end.neiBegin(); j != end.neiEnd(); j = end.neiNext(j))
         {
            int idx = end.neiEdge(j);

            if (idx != i)
            {
               if (mol.getEdge(idx).beg == edge.end)
                  _dbonds[idx].ctbond_beg = i;
               else
                  _dbonds[idx].ctbond_end = i;
            }
         }
      }
}

bool SmilesSaver::_updateSideBonds (int bond_idx)
{
   BaseMolecule &mol = *_bmol;
   const Edge &edge = mol.getEdge(bond_idx);
   int subst[4];

   mol.cis_trans.getSubstituents_All(bond_idx, subst);
   int parity = mol.cis_trans.getParity(bond_idx);

   int sidebonds[4] = {-1, -1, -1, -1};

   sidebonds[0] = mol.findEdgeIndex(subst[0], edge.beg);
   if (subst[1] != -1)
      sidebonds[1] = mol.findEdgeIndex(subst[1], edge.beg);

   sidebonds[2] = mol.findEdgeIndex(subst[2], edge.end);
   if (subst[3] != -1)
      sidebonds[3] = mol.findEdgeIndex(subst[3], edge.end);

   int n1 = 0, n2 = 0, n3 = 0, n4 = 0;

   if (_dbonds[sidebonds[0]].saved != 0)
   {
      if ((_dbonds[sidebonds[0]].saved == 1 && mol.getEdge(sidebonds[0]).beg == edge.beg) ||
          (_dbonds[sidebonds[0]].saved == 2 && mol.getEdge(sidebonds[0]).end == edge.beg))
         n1++;
      else
         n2++;
   }
   if (sidebonds[1] != -1 && _dbonds[sidebonds[1]].saved != 0)
   {
      if ((_dbonds[sidebonds[1]].saved == 2 && mol.getEdge(sidebonds[1]).beg == edge.beg) ||
          (_dbonds[sidebonds[1]].saved == 1 && mol.getEdge(sidebonds[1]).end == edge.beg))
         n1++;
      else
         n2++;
   }
   if (_dbonds[sidebonds[2]].saved != 0)
   {
      if ((_dbonds[sidebonds[2]].saved == 1 && mol.getEdge(sidebonds[2]).beg == edge.end) ||
          (_dbonds[sidebonds[2]].saved == 2 && mol.getEdge(sidebonds[2]).end == edge.end))
         n3++;
      else
         n4++;
   }
   if (sidebonds[3] != -1 && _dbonds[sidebonds[3]].saved != 0)
   {
      if ((_dbonds[sidebonds[3]].saved == 2 && mol.getEdge(sidebonds[3]).beg == edge.end) ||
          (_dbonds[sidebonds[3]].saved == 1 && mol.getEdge(sidebonds[3]).end == edge.end))
         n3++;
      else
         n4++;
   }

   if (parity == MoleculeCisTrans::CIS)
   {
      n1 += n3;
      n2 += n4;
   }
   else
   {
      n1 += n4;
      n2 += n3;
   }

   if (n1 > 0 && n2 > 0)
      throw Error("incompatible cis-trans configuration");

   if (n1 == 0 && n2 == 0)
      return false;

   if (n1 > 0)
   {
      _dbonds[sidebonds[0]].saved =
         (mol.getEdge(sidebonds[0]).beg == edge.beg) ? 1 : 2;
      if (sidebonds[1] != -1)
         _dbonds[sidebonds[1]].saved =
            (mol.getEdge(sidebonds[1]).beg == edge.beg) ? 2 : 1;

      _dbonds[sidebonds[2]].saved =
         ((mol.getEdge(sidebonds[2]).beg == edge.end) == (parity == MoleculeCisTrans::CIS)) ? 1 : 2;
      if (sidebonds[3] != -1)
         _dbonds[sidebonds[3]].saved =
            ((mol.getEdge(sidebonds[3]).beg == edge.end) == (parity == MoleculeCisTrans::CIS)) ? 2 : 1;
   }
   if (n2 > 0)
   {
      _dbonds[sidebonds[0]].saved =
         (mol.getEdge(sidebonds[0]).beg == edge.beg) ? 2 : 1;
      if (sidebonds[1] != -1)
         _dbonds[sidebonds[1]].saved =
            (mol.getEdge(sidebonds[1]).beg == edge.beg) ? 1 : 2;

      _dbonds[sidebonds[2]].saved =
         ((mol.getEdge(sidebonds[2]).beg == edge.end) == (parity == MoleculeCisTrans::CIS)) ? 2 : 1;
      if (sidebonds[3] != -1)
         _dbonds[sidebonds[3]].saved =
            ((mol.getEdge(sidebonds[3]).beg == edge.end) == (parity == MoleculeCisTrans::CIS)) ? 1 : 2;
   }

   return true;
}

int SmilesSaver::_calcBondDirection (int idx, int vprev)
{
   BaseMolecule &mol = *_bmol;
   int i, ntouched;

   if (_dbonds[idx].ctbond_beg == -1 && _dbonds[idx].ctbond_end == -1)
      return 0;

   if (mol.getBondOrder(idx) != BOND_SINGLE)
      throw Error("internal: directed bond type %d", mol.getBondOrder(idx));

   while (1)
   {
      ntouched = 0;
      for (i = mol.edgeBegin(); i != mol.edgeEnd(); i = mol.edgeNext(i))
         if (mol.cis_trans.getParity(i) != 0 && mol.getBondTopology(i) != TOPOLOGY_RING)
         {
            if (_updateSideBonds(i))
               ntouched++;
         }
      if (ntouched == _touched_cistransbonds)
         break;
      _touched_cistransbonds = ntouched;
   }

   if (_dbonds[idx].saved == 0)
   {
      if (vprev == mol.getEdge(idx).beg)
         _dbonds[idx].saved = 1;
      else
         _dbonds[idx].saved = 2;
   }

   return _dbonds[idx].saved;
}

void SmilesSaver::_writeStereogroups ()
{
   BaseMolecule &mol = *_bmol;
   MoleculeStereocenters &stereocenters = mol.stereocenters;
   int i, j;

   for (i = stereocenters.begin(); i != stereocenters.end(); i = stereocenters.next(i))
   {
      int atom, type, group;
      stereocenters.get(i, atom, type, group, 0);

      if (type != MoleculeStereocenters::ATOM_ABS)
         break;
   }

   if (i == stereocenters.end())
      return;

   int and_group_idx = 1;
   int or_group_idx = 1;
   
   QS_DEF(Array<int>, marked);

   marked.clear_resize(_written_atoms.size());
   marked.zerofill();

   for (i = 0; i < _written_atoms.size(); i++)
   {
      if (marked[i])
         continue;

      int type = stereocenters.getType(_written_atoms[i]);

      if (type > 0)
      {
         if (_comma)
            _output.writeChar(',');
         else
         {
            _output.writeString(" |");
            _comma = true;
         }
      }

      if (type == MoleculeStereocenters::ATOM_ANY)
      {
         _output.printf("w:%d", i);

         for (j = i + 1; j < _written_atoms.size(); j++)
            if (stereocenters.getType(_written_atoms[j]) == MoleculeStereocenters::ATOM_ANY)
            {
               marked[j] = 1;
               _output.printf(",%d", j);
            }
      }
      else if (type == MoleculeStereocenters::ATOM_ABS)
      {
         _output.printf("a:%d", i);

         for (j = i + 1; j < _written_atoms.size(); j++)
            if (stereocenters.getType(_written_atoms[j]) == MoleculeStereocenters::ATOM_ABS)
            {
               marked[j] = 1;
               _output.printf(",%d", j);
            }
      }
      else if (type == MoleculeStereocenters::ATOM_AND)
      {
         int group = stereocenters.getGroup(_written_atoms[i]);
         
         _output.printf("&%d:%d", and_group_idx++, i);
         for (j = i + 1; j < _written_atoms.size(); j++)
            if (stereocenters.getType(_written_atoms[j]) == MoleculeStereocenters::ATOM_AND &&
                stereocenters.getGroup(_written_atoms[j]) == group)
            {
               marked[j] = 1;
               _output.printf(",%d", j);
            }
      }
      else if (type == MoleculeStereocenters::ATOM_OR)
      {
         int group = stereocenters.getGroup(_written_atoms[i]);

         _output.printf("o%d:%d", or_group_idx++, i);
         for (j = i + 1; j < _written_atoms.size(); j++)
            if (stereocenters.getType(_written_atoms[j]) == MoleculeStereocenters::ATOM_OR &&
                stereocenters.getGroup(_written_atoms[j]) == group)
            {
               marked[j] = 1;
               _output.printf(",%d", j);
            }
      }
   }
}

void SmilesSaver::_writeRadicals ()
{
   BaseMolecule &mol = *_bmol;
   QS_DEF(Array<int>, marked);
   int i, j;

   marked.clear_resize(_written_atoms.size());
   marked.zerofill();

   for (i = 0; i < _written_atoms.size(); i++)
   {
      if (marked[i] || mol.isRSite(_written_atoms[i]) || mol.isPseudoAtom(_written_atoms[i]))
         continue;

      int radical = mol.getAtomRadical(_written_atoms[i]);

      if (radical <= 0)
         continue;

      if (_comma)
         _output.writeChar(',');
      else
      {
         _output.writeString(" |");
         _comma = true;
      }
    
      if (radical == RADICAL_SINGLET)
         _output.writeString("^3:");
      else if (radical == RADICAL_DOUPLET)
         _output.writeString("^1:");
      else // RADICAL_TRIPLET
         _output.writeString("^4:");

      _output.printf("%d", i);
      
      for (j = i + 1; j < _written_atoms.size(); j++)
         if (mol.getAtomRadical(_written_atoms[j]) == radical)
         {
            marked[j] = 1;
            _output.printf(",%d", j);
         }
   }
}

void SmilesSaver::_writePseudoAtoms ()
{
   BaseMolecule &mol = *_bmol;
   int i;
   
   for (i = 0; i < _written_atoms.size(); i++)
   {
      if (mol.isPseudoAtom(_written_atoms[i]) ||
          (mol.isRSite(_written_atoms[i]) && mol.getRSiteBits(_written_atoms[i]) != 0))
         break;
   }

   if (i == _written_atoms.size())
      return;

   if (_comma)
      _output.writeChar(',');
   else
   {
      _output.writeString(" |");
      _comma = true;
   }

   _output.writeChar('$');

   for (i = 0; i < _written_atoms.size(); i++)
   {
      if (i > 0)
         _output.writeChar(';');

      if (mol.isPseudoAtom(_written_atoms[i]))
         writePseudoAtom(mol.getPseudoAtom(_written_atoms[i]), _output);
      else if (mol.isRSite(_written_atoms[i]) && mol.getRSiteBits(_written_atoms[i]) != 0)
         // ChemAxon's Extended SMILES notation for R-sites
         _output.printf("_R%d", mol.getSingleAllowedRGroup(_written_atoms[i]));
   }

   _output.writeChar('$');
}

void SmilesSaver::writePseudoAtom (const char *label, Output &out)
{
   if (*label == 0)
      throw Error("empty pseudo-atom");

   do
   {
      if (*label == '\n' || *label == '\r' || *label == '\t')
         throw Error("character 0x%x is not allowed inside pseudo-atom", *label);
      if (*label == '$' || *label == ';')
         throw Error("'%c' not allowed inside pseudo-atom", *label);

      out.writeChar(*label);
   } while (*(++label) != 0);
}

void SmilesSaver::_writeHighlighting ()
{
   if (highlighting == 0)
      return;

   int i;

   bool ha = false;

   for (i = 0; i < _written_atoms.size(); i++)
   {
      if (highlighting->hasVertex(_written_atoms[i]))
      {
         if (ha)
            _output.writeChar(',');
         else
         {
            if (_comma)
               _output.writeChar(',');
            else
            {
               _output.writeString(" |");
               _comma = true;
            }
            _output.writeString("ha:");
            ha = true;
         }

         _output.printf("%d", i);
      }
   }

   bool hb = false;

   for (i = 0; i < _written_bonds.size(); i++)
   {
      if (highlighting->hasEdge(_written_bonds[i]))
      {
         if (hb)
            _output.writeChar(',');
         else
         {
            if (_comma)
               _output.writeChar(',');
            else
            {
               _output.writeString(" |");
               _comma = true;
            }
            _output.writeString("hb:");
            hb = true;
         }

         _output.printf("%d", i);
      }
   }
}


int SmilesSaver::writtenComponents ()
{
   return _written_components;
}

const Array<int> & SmilesSaver::writtenAtoms ()
{
   return _written_atoms;
}

const Array<int> & SmilesSaver::writtenBonds ()
{
   return _written_bonds;
}

SmilesSaver::_Atom::_Atom (Pool<List<int>::Elem> &neipool) :
               neighbors(neipool)
{
   aromatic = 0;
   lowercase = false;
   chirality = 0;
   branch_cnt = 0;
   paren_written = false;
   
   parent = -1;
}

SmilesSaver::_Atom::~_Atom ()
{
}
