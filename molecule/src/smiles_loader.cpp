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

#include <ctype.h>

#include "molecule/smiles_loader.h"

#include "base_cpp/scanner.h"
#include "molecule/molecule.h"
#include "molecule/query_molecule.h"
#include "molecule/molecule_stereocenters.h"
#include "graph/graph_highlighting.h"
#include "molecule/elements.h"
#include "graph/cycle_basis.h"

using namespace indigo;

SmilesLoader::SmilesLoader (Scanner &scanner) : _scanner(scanner),
TL_CP_GET(_neipool),
TL_CP_GET(_atoms),
TL_CP_GET(_bonds)
{
   reaction_atom_mapping = 0;
   highlighting = 0;
   inside_rsmiles = false;
   ignore_closing_bond_direction_mismatch = false;
   _mol = 0;
   _qmol = 0;
   _bmol = 0;
   smarts_mode = false;
   _balance = 0;
   _current_compno = 0;
   _inside_smarts_component = false;
}

SmilesLoader::~SmilesLoader ()
{
    // clear pool-dependent data in this thread to avoid data races
   _atoms.clear();
}

void SmilesLoader::loadMolecule (Molecule &mol)
{
   mol.clear();
   _bmol = &mol;
   _mol = &mol;
   _qmol = 0;
   _loadMolecule();
}

void SmilesLoader::loadQueryMolecule (QueryMolecule &mol)
{
   mol.clear();
   _bmol = &mol;
   _mol = 0;
   _qmol = &mol;
   _loadMolecule();
}

void SmilesLoader::_calcStereocenters ()
{
   int i, j, tmp;
   
   for (i = 0; i < _atoms.size(); i++)
   {
      if (_atoms[i].chirality > 0)
      {
         QS_DEF(Array<int>, saved);
         MoleculeStereocenters &stereocenters = _bmol->stereocenters;

         saved.clear_resize(_atoms.size());
         saved.zerofill();

         int pyramid[4] = {-1, -1, -1, -1};
         int counter = 0;
         int h_index = -1;

         if (_atoms[i].parent != -1)
            pyramid[counter++] = _atoms[i].parent;

         if (_atoms[i].neighbors.size() == 3)
         {
            h_index = counter;
            pyramid[counter++] = -1;
         }

         for (j = _atoms[i].neighbors.begin(); j != _atoms[i].neighbors.end();
              j = _atoms[i].neighbors.next(j))
         {
            int nei = _atoms[i].neighbors.at(j);

            if (counter >= 4)
               throw Error("too many bonds for chiral atom %d", i);

            if (nei != _atoms[i].parent)
               pyramid[counter++] = nei;
         }

         if (counter < 3)
            throw Error("only %d bonds for chiral atom %d", counter, i);

         if (counter == 4)
         {
            j = pyramid[0];
            pyramid[0] = pyramid[1];
            pyramid[1] = pyramid[2];
            pyramid[2] = pyramid[3];
            pyramid[3] = j;
            
            if (h_index == 0)
               h_index = 3;
            else if (h_index > 0)
               h_index--;
         }

         if (h_index >= 0)
         {
            if (counter != 4)
               throw Error("implicit hydrogen not allowed with %d neighbor atoms", counter - 1);
            
            bool parity = true;

            for (j = h_index; j < 3; j++)
            {
               __swap(pyramid[j], pyramid[j + 1], tmp);
               parity = !parity;
            }

            if (!parity)
               __swap(pyramid[0], pyramid[1], tmp);
         }

         if (_atoms[i].chirality == 2)
            __swap(pyramid[0], pyramid[1], j);

         stereocenters.add(i, MoleculeStereocenters::ATOM_ABS, 0, pyramid);
      }
   }
}

void SmilesLoader::_calcCisTrans ()
{
   QS_DEF(Array<int>, dirs);
   int i;

   dirs.clear();

   for (i = 0; i < _bonds.size(); i++)
      dirs.push(_bonds[i].dir);

   // there could be bonds added to stereocenters
   for (; i < _bmol->edgeEnd(); i++)
      dirs.push(0);

   _bmol->cis_trans.buildFromSmiles(dirs.ptr());
   if (_qmol != 0)
   {
      for (i = 0; i < _bonds.size(); i++)
         if (_bmol->cis_trans.getParity(i) != 0)
            _qmol->setBondStereoCare(i, true);
   }
}

void SmilesLoader::_readOtherStuff ()
{
   MoleculeStereocenters &stereocenters = _bmol->stereocenters;
   
   while (1)
   {
      char c = _scanner.readChar();

      if (c == '|')
         break;

      if (c == 'w')
      {
         if (_scanner.readChar() != ':')
            throw Error("colon expected after 'w'");

         while (isdigit(_scanner.lookNext()))
         {
            int idx = _scanner.readUnsigned();

            stereocenters.add(idx, MoleculeStereocenters::ATOM_ANY, 0, false);

            if (_scanner.lookNext() == ',')
               _scanner.skip(1);
         }
      }
      else if (c == 'a')
      {
         if (_scanner.readChar() != ':')
            throw Error("colon expected after 'a'");

         while (isdigit(_scanner.lookNext()))
         {
            int idx = _scanner.readUnsigned();

            stereocenters.setType(idx, MoleculeStereocenters::ATOM_ABS, 0);

            if (_scanner.lookNext() == ',')
               _scanner.skip(1);
         }
      }
      else if (c == 'o')
      {
         int groupno = _scanner.readUnsigned();

         if (_scanner.readChar() != ':')
            throw Error("colon expected after 'o'");
         
         while (isdigit(_scanner.lookNext()))
         {
            int idx = _scanner.readUnsigned();

            stereocenters.setType(idx, MoleculeStereocenters::ATOM_OR, groupno);

            if (_scanner.lookNext() == ',')
               _scanner.skip(1);
         }
      }
      else if (c == '&')
      {
         int groupno = _scanner.readUnsigned();

         if (_scanner.readChar() != ':')
            throw Error("colon expected after '&'");

         while (isdigit(_scanner.lookNext()))
         {
            int idx = _scanner.readUnsigned();

            stereocenters.setType(idx, MoleculeStereocenters::ATOM_AND, groupno);

            if (_scanner.lookNext() == ',')
               _scanner.skip(1);
         }
      }
      else if (c == '^')
      {
         int rad = _scanner.readIntFix(1);
         int radical;

         if (rad == 1)
            radical = RADICAL_DOUPLET;
         else if (rad == 3)
            radical = RADICAL_SINGLET;
         else if (rad == 4)
            radical = RADICAL_TRIPLET;
         else
            throw Error("unsupported radical number: %d", rad);

         if (_scanner.readChar() != ':')
            throw Error("colon expected after radical number");

         while (isdigit(_scanner.lookNext()))
         {
            int idx = _scanner.readUnsigned();

            if (_mol != 0)
               _mol->setAtomRadical(idx, radical);
            else
               _qmol->resetAtom(idx, QueryMolecule::Atom::und(
                       _qmol->releaseAtom(idx),
                       new QueryMolecule::Atom(QueryMolecule::ATOM_RADICAL, radical)));

            if (_scanner.lookNext() == ',')
               _scanner.skip(1);
         }
      }
      else if (c == '$')
      {
         QS_DEF(Array<char>, label);

         for (int i = 0; i < _bmol->vertexCount(); i++)
         {
            label.clear();

            while (1)
            {
               if (_scanner.isEOF())
                  throw Error("end of input while reading $...$ block");
               c = _scanner.readChar();
               if (c == ';' || c == '$')
                  break;
               label.push(c);
            }
            if (c == '$' && i != _bmol->vertexCount() - 1)
               throw Error("only %d atoms found in pseudo-atoms $...$ block", i + 1);
            if (c == ';' && i == _bmol->vertexCount() - 1)
               throw Error("extra ';' in pseudo-atoms $...$ block");

            if (label.size() > 0)
            {
               label.push(0);
               int rnum;

               if (label.size() > 3 && label[0] == '_' && label[1] == 'R' &&
                   sscanf(label.ptr() + 2, "%d", &rnum) == 1)
               {
                  if (_qmol != 0)
                     _qmol->resetAtom(i, new QueryMolecule::Atom(QueryMolecule::ATOM_RSITE, 0));
                  // ChemAxon's Extended SMILES notation for R-sites
                  _bmol->allowRGroupOnRSite(i, rnum);
               }
               else
               {
                  if (_mol != 0)
                     _mol->setPseudoAtom(i, label.ptr());
                  else
                  {
                     QueryMolecule::Atom *atom = _qmol->releaseAtom(i);
                     atom->removeConstraints(QueryMolecule::ATOM_NUMBER);
                     _qmol->resetAtom(i, QueryMolecule::Atom::und(atom,
                          new QueryMolecule::Atom(QueryMolecule::ATOM_PSEUDO, label.ptr())));
                  }
               }
            }
         }
      }
      else if (c == 'h')
      {
         c = _scanner.readChar();

         int a = false;

         if (c == 'a')
            a = true;
         else if (c != 'b')
            throw Error("expected 'a' or 'b' after 'h', got '%c'", c);

         if (_scanner.readChar() != ':')
            throw Error("colon expected after 'h%c'", a ? 'a' : 'b');

         while (isdigit(_scanner.lookNext()))
         {
            int idx = _scanner.readUnsigned();

            if (highlighting != 0)
            {
               if (a)
                  highlighting->onVertex(idx);
               else
                  highlighting->onEdge(idx);
            }

            if (_scanner.lookNext() == ',')
               _scanner.skip(1);
         }
      }
   }
}

void SmilesLoader::loadSMARTS (QueryMolecule &mol)
{
   mol.clear();
   _bmol = &mol;
   _mol = 0;
   _qmol = &mol;
   smarts_mode = true;
   _loadMolecule();
}

void SmilesLoader::_loadMolecule ()
{
   QS_DEF(StringPool, pending_bonds_pool);
   QS_DEF(Array<_CycleDesc>, cycles);
   QS_DEF(Array<int>, atom_stack);

   _atoms.clear();
   _bonds.clear();
   cycles.clear();
   pending_bonds_pool.clear();
   atom_stack.clear();
   if (highlighting != 0)
      highlighting->clear();

   bool first_atom = true;

   while (!_scanner.isEOF())
   {
      int next = _scanner.lookNext();

      if (isspace(next))
         break;

      _BondDesc *bond = 0;

      if (!first_atom)
      {
         bool added_bond = false;

         while (isdigit(next) || next == '%')
         {
            int number;

            _scanner.skip(1);
            if (next == '%')
               number = _scanner.readIntFix(2);
            else
               number = next - '0';

            while (cycles.size() <= number)
               cycles.push().clear();

            // closing some previous numbered atom, like the last '1' in c1ccccc1
            if (cycles[number].beg >= 0)
            {
               bond = &_bonds.push();
               bond->dir = 0;
               bond->topology = 0;
               bond->beg = atom_stack.top();
               bond->end = cycles[number].beg;
               bond->type = -1; // will later become single or aromatic bond
               cycles[number].clear();
               added_bond = true;

               if (_qmol != 0)
               {
                  if (smarts_mode)
                     _qmol->addBond(bond->beg, bond->end, QueryMolecule::Bond::oder(
                          new QueryMolecule::Bond(QueryMolecule::BOND_ORDER, BOND_SINGLE),
                          new QueryMolecule::Bond(QueryMolecule::BOND_ORDER, BOND_AROMATIC)));
                  else
                     _qmol->addBond(bond->beg, bond->end, new QueryMolecule::Bond());
               }

               _atoms[bond->beg].neighbors.add(bond->end);
               _atoms[bond->end].closure(number, bond->beg);

               break;
            }
            // closing some previous pending bond, like the last '1' in C-1=CC=CC=C1'
            else if (cycles[number].pending_bond >= 0)
            {
               bond = &_bonds[cycles[number].pending_bond];
               bond->end = atom_stack.top();
               added_bond = true;
               _atoms[bond->end].neighbors.add(bond->beg);
               _atoms[bond->beg].closure(number, bond->end);

               if (_qmol != 0)
               {
                  QS_DEF(Array<char>, bond_str);
                  AutoPtr<QueryMolecule::Bond> qbond(new QueryMolecule::Bond());

                  bond_str.readString(pending_bonds_pool.at(cycles[number].pending_bond_str), false);
                  _readBond(bond_str, *bond, qbond);
                  _qmol->addBond(bond->beg, bond->end, qbond.release());
               }
               
               cycles[number].clear();

               break;
            }
            // opening new cycle, like the first '1' in c1ccccc1
            else
            {
               cycles[number].beg = atom_stack.top();
               cycles[number].pending_bond = -1;
               _atoms[cycles[number].beg].pending(number);
            }
            next = _scanner.lookNext();
         }
         if (added_bond)
            continue;
      }

      if (next == '.')
      {
         _scanner.skip(1);

         if (smarts_mode && _balance == 0)
         {
            _inside_smarts_component = false;
            atom_stack.clear(); // needed to detect errors like "C.(C)(C)"
         }
         else
         {
            if (atom_stack.size() < 1)
               ; // we allow misplaced dots because we are so kind
            else
               atom_stack.pop();
         }
         first_atom = true;
         continue;
      }

      if (next == '(')
      {
         _scanner.skip(1);

         if (smarts_mode && first_atom)
         {
            if (_balance > 0)
               throw Error("hierarchical component-level grouping is not allowed");
            _current_compno++;
            _inside_smarts_component = true;
         }
         else
         {
            if (atom_stack.size() < 1)
               throw Error("probably misplaced '('");
            atom_stack.push(atom_stack.top());
         }

         _balance++;
         continue;
      }

      if (next == ')')
      {
         _scanner.skip(1);

         if (_balance <= 0)
            throw Error("unexpected ')'");

         _balance--;

         //if (smarts_mode && _balance == 0)
         //   ;
         //else
            atom_stack.pop();
         continue;
      }

      if (!first_atom)
      {
         bond = &_bonds.push();
         bond->beg = atom_stack.top();
         bond->end = -1;
         bond->type = -1;
         bond->dir = 0;
         bond->topology = 0;
      }

      AutoPtr<QueryMolecule::Bond> qbond;

      if (bond != 0)
      {
         QS_DEF(Array<char>, bond_str);

         bond_str.clear();
         while (strchr("-=#:@!;,&~?/\\", next) != NULL)
         {
            bond_str.push(_scanner.readChar());
            next = _scanner.lookNext();
         }

         if (_qmol != 0)
            qbond.reset(new QueryMolecule::Bond());

         // empty bond designator?
         if (bond_str.size() < 1)
         {
            // 1) SMARTS mode
            //   A missing bond symbol is interpreted as "single or aromatic".
            //   (http://www.daylight.com/dayhtml/doc/theory/theory.smarts.html)
            if (smarts_mode)
               qbond.reset(QueryMolecule::Bond::oder(
                    new QueryMolecule::Bond(QueryMolecule::BOND_ORDER, BOND_SINGLE),
                    new QueryMolecule::Bond(QueryMolecule::BOND_ORDER, BOND_AROMATIC)));

            // 2) query or non-query SMILES mode:
            //    this is either single or aromatic bond, which
            //    is detected after the scanning cycle (see below)
         }
         else
            _readBond(bond_str, *bond, qbond);

         // The bond "directions" are already saved in _BondDesc::dir,
         // so we can safely discard them. We are doing that to succeed
         // the later check 'pending bond vs. closing bond'.
         {
            int i;
            
            for (i = 0; i < bond_str.size(); i++)
               if (bond_str[i] == '/' || bond_str[i] == '\\')
                  bond_str[i] = '-';
         }

         if (bond_str.size() > 0)
         {
            if (isdigit(next) || next == '%')
            {
               int number;
               _scanner.skip(1);

               if (next == '%')
                  number = _scanner.readIntFix(2);
               else
                  number = next - '0';

               // closing some previous numbered atom, like the last '1' in C1C=CC=CC=1
               if (number >= 0 && number < cycles.size() && cycles[number].beg >= 0)
               {
                  bond->end = cycles[number].beg;

                  if (_qmol != 0)
                     _qmol->addBond(bond->beg, bond->end, qbond.release());

                  _atoms[bond->end].closure(number, bond->beg);
                  _atoms[bond->beg].neighbors.add(bond->end);

                  cycles[number].clear();
                  continue;
               }
               // closing some previous pending cycle bond, like the last '1' in C=1C=CC=CC=1
               else if (number >= 0 && number < cycles.size() && cycles[number].pending_bond >= 0)
               {
                  _BondDesc &pending_bond = _bonds[cycles[number].pending_bond];

                  // transfer direction from closing bond to pending bond
                  if (bond->dir > 0)
                  {
                     if (bond->dir == pending_bond.dir)
                     {
                        if (!ignore_closing_bond_direction_mismatch)
                           throw Error("cycle %d: closing bond direction does not match pending bond direction",
                              number);
                     }
                     else
                        pending_bond.dir = 3 - bond->dir;
                  }

                  // apart from the direction, check that the closing bond matches the pending bond
                  const char *str = pending_bonds_pool.at(cycles[number].pending_bond_str);

                  if (bond_str.size() > 0)
                  {
                     if ((int)strlen(str) != bond_str.size() || memcmp(str, bond_str.ptr(), strlen(str)) != 0)
                        throw Error("cycle %d: closing bond description %.*s does not match pending bond description %s",
                             number, bond_str.size(), bond_str.ptr(), str);
                  }
                  else
                  {
                     bond_str.readString(str, false);
                     _readBond(bond_str, *bond, qbond);
                  }

                  if (_qmol != 0)
                     _qmol->addBond(pending_bond.beg, bond->beg, qbond.release());

                  pending_bond.end = bond->beg;
                  _atoms[pending_bond.end].neighbors.add(pending_bond.beg);
                  _atoms[pending_bond.beg].closure(number, pending_bond.end);

                  // forget the closing bond
                  _bonds.pop();
                  cycles[number].clear();
                  continue;
               }
               // opening some pending cycle bond, like the first '1' in C=1C=CC=CC=1
               else
               {
                  while (cycles.size() <= number)
                     cycles.push().clear();
                  cycles[number].pending_bond = _bonds.size() - 1;
                  cycles[number].pending_bond_str = pending_bonds_pool.add(bond_str);
                  cycles[number].beg = -1; // have it already in the bond
                  _atoms[bond->beg].pending(number);

                  continue;
               }
            }
         }
      }

      _AtomDesc &atom = _atoms.push(_neipool);

      AutoPtr<QueryMolecule::Atom> qatom;

      if (_qmol != 0)
         qatom.reset(new QueryMolecule::Atom());

      if (!first_atom)
         atom_stack.pop();
      atom_stack.push(_atoms.size() - 1);
      first_atom = false;

      if (bond != 0)
         bond->end = _atoms.size() - 1;

      QS_DEF(Array<char>, atom_str);

      atom_str.clear();

      bool brackets = false;

      if (next == '[')
      {
         _scanner.skip(1);
         int cnt = 1;

         while (1)
         {
            if (_scanner.isEOF())
               throw Error("'[' without a ']'");
            char c = _scanner.readChar();
            if (c == '[')
               cnt++;
            else if (c == ']')
            {
               cnt--;
               if (cnt == 0)
                  break;
            }
            atom_str.push(c);
         }
         brackets = true;
      }
      else if (next == -1)
         throw Error("unexpected end of input");
      else
      {
         _scanner.skip(1);
         atom_str.push(next);
         if (next == 'B' && _scanner.lookNext() == 'r')
            atom_str.push(_scanner.readChar());
         else if (next == 'C' && _scanner.lookNext() == 'l')
            atom_str.push(_scanner.readChar());
      }

      _readAtom(atom_str, brackets, atom, qatom);
      atom.brackets = brackets;

      if (_qmol != 0)
      {
         _qmol->addAtom(qatom.release());
         if (bond != 0)
            _qmol->addBond(bond->beg, bond->end, qbond.release());
      }

      if (bond != 0)
      {
         _atoms[bond->beg].neighbors.add(bond->end);
         _atoms[bond->end].neighbors.add(bond->beg);
         _atoms[bond->end].parent = bond->beg;
      }

      if (_inside_smarts_component)
      {
         _qmol->components.expandFill(_atoms.size(), 0);
         _qmol->components[_atoms.size() - 1] = _current_compno;
      }
   }

   int i;

   for (i = 0; i < cycles.size(); i++)
   {
      if (cycles[i].beg >= 0)
         throw Error("cycle %d not closed", i);
   }

   if (_mol != 0)
   {
      for (i = 0; i < _atoms.size(); i++)
      {
         if (_atoms[i].label == 0)
            throw Error("atom without a label");
         int idx = _mol->addAtom(_atoms[i].label);

         _mol->setAtomCharge(idx, _atoms[i].charge);
         _mol->setAtomIsotope(idx, _atoms[i].isotope);
      }

      for (i = 0; i < _bonds.size(); i++)
      {
         int beg = _bonds[i].beg;
         int end = _bonds[i].end;

         if (end == -1)
            throw Error("probably pending bond %d not closed", i);
         _mol->addBond(beg, end, _bonds[i].type);
      }
   }

   if (!smarts_mode)
   {
      CycleBasis basis;
      basis.create(*_bmol);

      // mark all 'empty' bonds in "aromatic" rings as aromatic
      for (i = 0; i < basis.getCyclesCount(); i++)
      {
         const Array<int> &cycle = basis.getCycle(i);
         int j;
         bool needs_modification = false;

         for (j = 0; j < cycle.size(); j++)
         {
            int idx = cycle[j];
            const Edge &edge = _bmol->getEdge(idx);
            if (!_atoms[edge.beg].aromatic || !_atoms[edge.end].aromatic)
               break;
            if (_bonds[idx].type == BOND_SINGLE || _bonds[idx].type == BOND_DOUBLE || _bonds[idx].type == BOND_TRIPLE)
               break;
            if (_qmol != 0 && !_qmol->possibleBondOrder(idx, BOND_AROMATIC))
               break;
            if (_bonds[idx].type == -1)
               needs_modification = true;
         }

         if (j != cycle.size())
            continue;

         if (needs_modification)
         {
            for (j = 0; j < cycle.size(); j++)
            {
               int idx = cycle[j];
               if (_bonds[idx].type == -1)
               {
                  _bonds[idx].type = BOND_AROMATIC;
                  if (_mol != 0)
                     _mol->setBondOrder_Silent(idx, BOND_AROMATIC);
                  if (_qmol != 0)
                     _qmol->resetBond(idx, QueryMolecule::Bond::und(_qmol->releaseBond(idx),
                             new QueryMolecule::Bond(QueryMolecule::BOND_ORDER, BOND_AROMATIC)));
               }
            }
         }
      }

      // mark the rest 'empty' bonds as single
      for (i = 0; i < _bonds.size(); i++)
      {
         if (_bonds[i].type == -1)
         {
            if (_mol != 0)
               _mol->setBondOrder_Silent(i, BOND_SINGLE);
            if (_qmol != 0)
               _qmol->resetBond(i, QueryMolecule::Bond::und(_qmol->releaseBond(i),
                       new QueryMolecule::Bond(QueryMolecule::BOND_ORDER, BOND_SINGLE)));
         }
      }

   }

   if (_mol != 0)
   {
      for (i = 0; i < _atoms.size(); i++)
      {
         int idx = i;

         if (_atoms[i].hydrogens >= 0)
            _mol->setImplicitH(idx, _atoms[i].hydrogens);
         else if (_atoms[i].brackets) // no hydrogens in brackets?
            _mol->setImplicitH(idx, 0); // no implicit hydrogens on atom then
         else if (_atoms[i].aromatic)
         {
            if (_atoms[i].label == ELEM_C)
            {
               // here we are basing on the fact that
               // aromatic uncharged carbon always has a double bond
               if (_mol->getVertex(i).degree() < 3)
                  // 2-connected aromatic carbon must have 1 single bond and 1 double bond,
                  // so we have one implicit hydrogen left
                  _mol->setImplicitH(idx, 1);
               else
                  _mol->setImplicitH(idx, 0);
            }
            else
               // it is probably not fair to set it to zero at
               // this point, but other choices seem to be worse:
               //   1) raise an exception (too ugly)
               //   2) try to de-aromatize the molecule to know the implicit hydrogens (too complicated)
               _mol->setImplicitH(idx, 0);
         }
      }
   }

   if (smarts_mode)
   {
      // Forbid matching SMARTS atoms to hydrogens
      for (i = 0; i < _atoms.size(); i++)
      {
         // not needed if it is a sure atom or a list without a hydrogen
         if (_qmol->getAtomNumber(i) == -1 && _qmol->possibleAtomNumber(i, ELEM_H))
         {
            // not desired if it is a list with hydrogen
            if (!_qmol->getAtom(i).hasConstraintWithValue(QueryMolecule::ATOM_NUMBER, ELEM_H))
            {
               AutoPtr<QueryMolecule::Atom> newatom;
               AutoPtr<QueryMolecule::Atom> oldatom(_qmol->releaseAtom(i));

               newatom.reset(QueryMolecule::Atom::und(
                     QueryMolecule::Atom::nicht(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_H)),
                       oldatom.release()));

               _qmol->resetAtom(i, newatom.release());
            }
         }
      }
   }

   if (!inside_rsmiles)
      for (i = 0; i < _atoms.size(); i++)
         if (_atoms[i].star_atom && _atoms[i].aam != 0)
         {
            if (_qmol != 0)
               _qmol->resetAtom(i, new QueryMolecule::Atom(QueryMolecule::ATOM_RSITE, 0));
            _bmol->allowRGroupOnRSite(i, _atoms[i].aam);
         }

   _calcStereocenters();
   _calcCisTrans();

   if (highlighting != 0)
      highlighting->init(*_bmol);

   _scanner.skipSpace();

   if (_scanner.lookNext() == '|')
   {
      _scanner.skip(1);
      _readOtherStuff();
   }

   // Update attachment orders for rsites
   for (i = _bmol->vertexBegin(); i < _bmol->vertexEnd(); i = _bmol->vertexNext(i))
   {
      if (!_bmol->isRSite(i))
         continue;

      const Vertex &vertex = _bmol->getVertex(i);

      int j, k = 0;
      for (j = vertex.neiBegin(); j < vertex.neiEnd(); j = vertex.neiNext(j))
         _bmol->setRSiteAttachmentOrder(i, vertex.neiVertex(j), k++);
   }

   if (!inside_rsmiles)
   {
     _scanner.skipSpace();
     if (!_scanner.isEOF())
        _scanner.readString(_bmol->name, true);
   }

   if (reaction_atom_mapping != 0)
   {
      reaction_atom_mapping->clear_resize(_bmol->vertexCount());
      reaction_atom_mapping->zerofill();
      for (i = 0; i < _atoms.size(); i++)
         reaction_atom_mapping->at(i) = _atoms[i].aam;
   }
}

void SmilesLoader::_readBond (Array<char> &bond_str, _BondDesc &bond,
                              AutoPtr<QueryMolecule::Bond> &qbond)
{
   if (bond_str.find(';') != -1)
   {
      QS_DEF(Array<char>, substring);
      AutoPtr<QueryMolecule::Bond> subqbond;
      int i;

      if (_qmol == 0)
         throw Error("';' is allowed only within queries");

      substring.clear();
      for (i = 0; i <= bond_str.size(); i++)
      {
         if (i == bond_str.size() || bond_str[i] == ';')
         {
            subqbond.reset(new QueryMolecule::Bond);
            _readBond(substring, bond, subqbond);
            qbond.reset(QueryMolecule::Bond::und(qbond.release(), subqbond.release()));
            substring.clear();
         }
         else
            substring.push(bond_str[i]);
      }
      return;
   }
   if (bond_str.find(',') != -1)
   {
      QS_DEF(Array<char>, substring);
      AutoPtr<QueryMolecule::Bond> subqbond;
      int i;

      if (_qmol == 0)
         throw Error("',' is allowed only within queries");

      substring.clear();
      for (i = 0; i <= bond_str.size(); i++)
      {
         if (i == bond_str.size() || bond_str[i] == ',')
         {
            subqbond.reset(new QueryMolecule::Bond);
            _readBond(substring, bond, subqbond);
            if (qbond->type == 0)
               qbond.reset(subqbond.release());
            else
               qbond.reset(QueryMolecule::Bond::oder(qbond.release(), subqbond.release()));
            substring.clear();
         }
         else
            substring.push(bond_str[i]);
      }
      return;
   }
   if (bond_str.find('&') != -1)
   {
      QS_DEF(Array<char>, substring);
      AutoPtr<QueryMolecule::Bond> subqbond;
      int i;

      if (_qmol == 0)
         throw Error("'&' is allowed only within queries");

      substring.clear();
      for (i = 0; i <= bond_str.size(); i++)
      {
         if (i == bond_str.size() || bond_str[i] == '&')
         {
            subqbond.reset(new QueryMolecule::Bond);
            _readBond(substring, bond, subqbond);
            qbond.reset(QueryMolecule::Bond::und(qbond.release(), subqbond.release()));
            substring.clear();
         }
         else
            substring.push(bond_str[i]);
      }
      return;
   }

   BufferScanner scanner(bond_str);

   bool neg = false;
   
   while (!scanner.isEOF())
   {
      int next = scanner.lookNext();
      int order = -1;
      int topology = -1;

      if (next == '!')
      {
         scanner.skip(1);
         neg = !neg;
         if (qbond.get() == 0)
            throw Error("'!' is allowed only within queries");
         continue;
      }
      if (next == '-')
      {
         scanner.skip(1);
         order = BOND_SINGLE;
      }
      else if (next == '=')
      {
         scanner.skip(1);
         order = BOND_DOUBLE;
      }
      else if (next == '#')
      {
         scanner.skip(1);
         order = BOND_TRIPLE;
      }
      else if (next == ':')
      {
         scanner.skip(1);
         order = BOND_AROMATIC;
      }
      else if (next == '/')
      {
         scanner.skip(1);
         order = BOND_SINGLE;
         bond.dir = 1;
      }
      else if (next == '\\')
      {
         scanner.skip(1);
         order = BOND_SINGLE;
         bond.dir = 2;
      }
      else if (next == '~')
      {
         scanner.skip(1);
         order = _ANY_BOND;
         if (qbond.get() == 0)
            throw Error("'~' any bond is allowed only for queries");
      }
      else if (next == '@')
      {
         scanner.skip(1);
         if (qbond.get() == 0)
            throw Error("'@' ring bond is allowed only for queries");
         topology = TOPOLOGY_RING;
      }

      AutoPtr<QueryMolecule::Bond> subqbond;

      if (order > 0)
      {
         bond.type = order;
         if (qbond.get() != 0)
         {
            if (subqbond.get() == 0)
               subqbond.reset(new QueryMolecule::Bond(QueryMolecule::BOND_ORDER, order));
            else
               subqbond.reset(QueryMolecule::Bond::und(subqbond.release(),
                  new QueryMolecule::Bond(QueryMolecule::BOND_ORDER, order)));
         }
      }
      else if (order == _ANY_BOND)
         bond.type = order;

      if (topology > 0)
      {
         if (subqbond.get() == 0)
            subqbond.reset(new QueryMolecule::Bond(QueryMolecule::BOND_TOPOLOGY, topology));
         else
            subqbond.reset(QueryMolecule::Bond::und(subqbond.release(),
               new QueryMolecule::Bond(QueryMolecule::BOND_TOPOLOGY, topology)));
      }

      if (subqbond.get() != 0)
      {
         if (neg)
         {
            subqbond.reset(QueryMolecule::Bond::nicht(subqbond.release()));
            neg = false;
         }
         qbond.reset(QueryMolecule::Bond::und(qbond.release(), subqbond.release()));
      }
   }
}

void SmilesLoader::_readAtom (Array<char> &atom_str, bool first_in_brackets,
                              _AtomDesc &atom, AutoPtr<QueryMolecule::Atom> &qatom)
{
   QS_DEF(Array<char>, atom_str_copy);
   if (atom_str.size() < 1)
      throw Error("empty atom?");

   atom_str_copy.copy(atom_str);
   int i, k;

   while ((k = atom_str_copy.find('$')) != -1)
   {
      // fill the "$(...) part of atom_str_copy with '^'"
      int cnt = 1;
      atom_str_copy[k] = '^';
      for (i = k + 2; i < atom_str_copy.size(); i++)
      {
         if (atom_str_copy[i] == '(')
            cnt++;
         else if (atom_str_copy[i] == ')')
            cnt--;
         
         if (cnt == 0)
            break;

         atom_str_copy[i] = '^';
      }
   }

   if (atom_str_copy.find(';') != -1)
   {
      QS_DEF(Array<char>, substring);
      AutoPtr<QueryMolecule::Atom> subqatom;
      int i, k = 0;
      
      if (qatom.get() == 0)
         throw Error("';' is allowed only for query molecules");

      substring.clear();
      for (i = 0; i <= atom_str_copy.size(); i++)
      {
         if (i == atom_str.size() || atom_str_copy[i] == ';')
         {
            subqatom.reset(new QueryMolecule::Atom);
            _readAtom(substring, first_in_brackets && (k == 0), atom, subqatom);
            qatom.reset(QueryMolecule::Atom::und(qatom.release(), subqatom.release()));
            substring.clear();
            k++;
         }
         else
            substring.push(atom_str[i]);
      }
      return;
   }

   if (atom_str_copy.find(',') != -1)
   {
      QS_DEF(Array<char>, substring);
      AutoPtr<QueryMolecule::Atom> subqatom;
      int i, k = 0;
      
      if (qatom.get() == 0)
         throw Error("',' is allowed only for query molecules");

      substring.clear();
      for (i = 0; i <= atom_str.size(); i++)
      {
         if (i == atom_str.size() || atom_str_copy[i] == ',')
         {
            subqatom.reset(new QueryMolecule::Atom);
            _readAtom(substring, first_in_brackets && (k == 0), atom, subqatom);
            if (qatom->type == 0)
               qatom.reset(subqatom.release());
            else
               qatom.reset(QueryMolecule::Atom::oder(qatom.release(), subqatom.release()));
            substring.clear();
            k++;
         }
         else
            substring.push(atom_str[i]);
      }
      return;
   }

   if (atom_str_copy.find('&') != -1)
   {
      QS_DEF(Array<char>, substring);
      AutoPtr<QueryMolecule::Atom> subqatom;
      int i, k = 0;
      
      if (qatom.get() == 0)
         throw Error("'&' is allowed only for query molecules");

      substring.clear();
      for (i = 0; i <= atom_str.size(); i++)
      {
         if (i == atom_str.size() || atom_str_copy[i] == '&')
         {
            subqatom.reset(new QueryMolecule::Atom);
            _readAtom(substring, first_in_brackets && (k == 0), atom, subqatom);
            qatom.reset(QueryMolecule::Atom::und(qatom.release(), subqatom.release()));
            substring.clear();
            k++;
         }
         else
            substring.push(atom_str[i]);
      }
      return;
   }

   BufferScanner scanner(atom_str);

   bool element_assigned = false;
   bool neg = false;
   while (!scanner.isEOF())
   {
      bool isotope_set = false;
      int element = -1;
      int aromatic = 0;
      int next = scanner.lookNext();
      AutoPtr<QueryMolecule::Atom> subatom;

      if (next == '!')
      {
         if (qatom.get() == 0)
            throw Error("'!' is allowed only within queries");

         scanner.skip(1);
         neg = !neg;
         first_in_brackets = false;
         continue;
      }
      else if (next == '$')
      {
         scanner.skip(1);
         if (scanner.readChar() != '(')
            throw Error("'$' must be followed by '('");

         if (!smarts_mode)
            throw Error("'$' fragments are allowed only in SMARTS queries");

         QS_DEF(Array<char>, subexp);

         subexp.clear();
         int cnt = 1;

         while (1)
         {
            char c = scanner.readChar();
            if (c == '(')
               cnt++;
            else if (c == ')')
            {
               cnt--;
               if (cnt == 0)
                  break;
            }
            subexp.push(c);
         }

         BufferScanner subscanner(subexp);
         SmilesLoader subloader(subscanner);
         AutoPtr<QueryMolecule> fragment(new QueryMolecule());

         subloader.loadSMARTS(fragment.ref());
         fragment->fragment_smarts.copy(subexp);
         fragment->fragment_smarts.push(0);

         if (subatom.get() == 0)
            subatom.reset(new QueryMolecule::Atom(QueryMolecule::ATOM_FRAGMENT, fragment.release()));
         else
            subatom.reset(QueryMolecule::Atom::und(subatom.release(),
                          new QueryMolecule::Atom(QueryMolecule::ATOM_FRAGMENT, fragment.release())));


      }
      else if (isdigit(next))
      {
         int isotope = scanner.readUnsigned();

         if (qatom.get() != 0)
            subatom.reset(new QueryMolecule::Atom(QueryMolecule::ATOM_ISOTOPE, isotope));
         else
            atom.isotope = isotope;
         isotope_set = true;
      }
      else if (next == 'H')
      {
         scanner.skip(1);

         // Now comes the trouble with the 'H' symbol.
         // As the manual says
         // (see http://www.daylight.com/dayhtml/doc/theory/theory.smarts.html):
         //    [H] means hydrogen atom.
         //    [*H2] means any atom with exactly two hydrogens attached.
         // Yet in the combined expressions like [n;H1] 'H' means the hydrogen
         // count, not the element. To distinguish these things, we use
         // the 'first in brackets' flag, which is true only for the very
         // first sub-expression in the brackets.
         // Also, the following elements begin with H: He, Hs, Hf, Ho, Hg
         if (strchr("esfog", scanner.lookNext()) == NULL)
         {
            if (first_in_brackets)
               element = ELEM_H;
            else
            {
               atom.hydrogens = 1;
               if (isdigit(scanner.lookNext()))
                  atom.hydrogens = scanner.readUnsigned();
               if (qatom.get() != 0)
                  subatom.reset(new QueryMolecule::Atom(QueryMolecule::ATOM_TOTAL_H, atom.hydrogens));
            }
         }
         else
            element = Element::fromTwoChars('H', scanner.readChar());
      }
      // The 'A' symbol is weird too. It can be the 'aliphatic' atomic primitive,
      // and can also be Al, Ar, As, Ag, Au, At, Ac, or Am.
      else if (next == 'A')
      {
         scanner.skip(1);

         if (strchr("lrsgutcm", scanner.lookNext()) == NULL)
         {
            if (qatom.get() == 0)
               throw Error("'A' specifier is allowed only for query molecules");

            subatom.reset(new QueryMolecule::Atom(QueryMolecule::ATOM_AROMATICITY, ATOM_ALIPHATIC));
         }
         else
            element = Element::fromTwoChars('A', scanner.readChar());
      }
      // Similarly, 'R' can start Rb, Ru, Rh, Re, Rn, Ra, Rf, Rg
      else if (next == 'R')
      {
         scanner.skip(1);

         if (strchr("buhenafg", scanner.lookNext()) == NULL)
         {
            if (qatom.get() == 0)
               throw Error("'R' specifier is allowed only for query molecules");

            if (isdigit(scanner.lookNext()))
            {
               int rc = scanner.readUnsigned();

               if (rc == 0)
                  subatom.reset(new QueryMolecule::Atom(QueryMolecule::ATOM_RING_BONDS, 0));
               else
                  subatom.reset(new QueryMolecule::Atom(QueryMolecule::ATOM_SSSR_RINGS, rc));
            }
            else
               subatom.reset(new QueryMolecule::Atom(QueryMolecule::ATOM_RING_BONDS, 1, 100));
         }
         else
            element = Element::fromTwoChars('R', scanner.readChar());
      }
      // Yet 'D' can start Db, Ds, Dy
      else if (next == 'D')
      {
         scanner.skip(1);

         if (strchr("bsy", scanner.lookNext()) == NULL)
         {
            if (qatom.get() == 0)
               throw Error("'D' specifier is allowed only for query molecules");

            int degree = 1;

            if (isdigit(scanner.lookNext()))
               degree = scanner.readUnsigned();

            subatom.reset(new QueryMolecule::Atom(QueryMolecule::ATOM_SUBSTITUENTS, degree));
         }
         else
            element = Element::fromTwoChars('D', scanner.readChar());
      }
      // ... and 'X' can start Xe
      else if (next == 'X')
      {
         scanner.skip(1);

         if (scanner.lookNext() != 'e')
         {
            if (qatom.get() == 0)
               throw Error("'X' specifier is allowed only for query molecules");

            int conn = 1;

            if (isdigit(scanner.lookNext()))
               conn = scanner.readUnsigned();

            subatom.reset(new QueryMolecule::Atom(QueryMolecule::ATOM_CONNECTIVITY, conn));
         }
         else
            element = Element::fromTwoChars('X', scanner.readChar());
      }
      else if (next == '*')
      {
         atom.star_atom = true;
         if (qatom.get() == 0)
            atom.label = ELEM_RSITE;
         else
            subatom.reset(QueryMolecule::Atom::nicht(new QueryMolecule::Atom
               (QueryMolecule::ATOM_NUMBER, ELEM_H)));
         scanner.skip(1);
      }
      else if (next == '#')
      {
         if (!smarts_mode)
            throw Error("'#' is allowed only within SMARTS queries");
         scanner.skip(1);
         element = scanner.readUnsigned();
      }
      // Now we check that we have here an element from the periodic table.
      // We assume that this must be an alphabetic character and also
      // something not from the alphabetic SMARTS 'atomic primitives'
      // (see http://www.daylight.com/dayhtml/doc/theory/theory.smarts.html).
      else if (isalpha(next) && strchr("hrvxa", next) == NULL)
      {
         scanner.skip(1);

         if (next == 'b')
         {
            element = ELEM_B;
            aromatic = ATOM_AROMATIC;
         }
         else if (next == 'c')
         {
            element = ELEM_C;
            aromatic = ATOM_AROMATIC;
         }
         else if (next == 'n')
         {
            element = ELEM_N;
            aromatic = ATOM_AROMATIC;
         }
         else if (next == 'o')
         {
            element = ELEM_O;
            aromatic = ATOM_AROMATIC;
         }
         else if (next == 'p')
         {
            element = ELEM_P;
            aromatic = ATOM_AROMATIC;
         }
         else if (next == 's')
         {
            element = ELEM_S;
            aromatic = ATOM_AROMATIC;
         }
         else if (islower(next))
            throw Error("unrecognized lowercase symbol: %c", next);

         // Now we are sure that 'next' is a capital letter

         // Check if we have a lowercase letter right after...
         else if (isalpha(scanner.lookNext()) && islower(scanner.lookNext()) &&
                  // If a lowercase letter is following the uppercase letter,
                  // we should consider reading them as a single element.
                  // They can possibly not form an element: for example,
                  // [Nr] is formally a nitrogen in a ring (although nobody would
                  // write it that way: [N;r] is much more clear).
                  (element = Element::fromTwoChars2(next, scanner.lookNext())) > 0)
            scanner.skip(1);
         else
         {
            // It is a single-char uppercase element identifier then
            element = Element::fromChar(next);

            if (smarts_mode)
               if (element == ELEM_B || element == ELEM_C || element == ELEM_N ||
                   element == ELEM_O || element == ELEM_P || element == ELEM_S)
                  aromatic = ATOM_ALIPHATIC;
         }
      }
      else if (next == '@')
      {
         atom.chirality = 1;
         scanner.skip(1);
         if (scanner.lookNext() == '@')
         {
            atom.chirality = 2;
            scanner.skip(1);
         }
      }
      else if (next == '+' || next == '-')
      {
         char c = scanner.readChar();
         if (c == '+')
            atom.charge = 1;
         else
            atom.charge = -1;

         if (isdigit(scanner.lookNext()))
            atom.charge *= scanner.readUnsigned();
         else while (scanner.lookNext() == c)
         {
            scanner.skip(1);
            if (c == '+')
               atom.charge++;
            else
               atom.charge--;
         }

         if (qatom.get() != 0)
            subatom.reset(new QueryMolecule::Atom(QueryMolecule::ATOM_CHARGE, atom.charge));
      }
      else if (next == 'a')
      {
         scanner.skip(1);
         if (qatom.get() == 0)
            throw Error("'a' specifier is allowed only for query molecules");

         subatom.reset(new QueryMolecule::Atom(QueryMolecule::ATOM_AROMATICITY, ATOM_AROMATIC));
      }
      else if (next == 'h')
         // Why would anybody ever need 'implicit hydrogen'
         // count rather than total hydrogen count?
         throw Error("'h' specifier is not supported");
      else if (next == 'r')
      {
         scanner.skip(1);
         if (qatom.get() == 0)
            throw Error("'r' specifier is allowed only for query molecules");

         if (isdigit(scanner.lookNext()))
            subatom.reset(new QueryMolecule::Atom(QueryMolecule::ATOM_SMALLEST_RING_SIZE,
                                            scanner.readUnsigned()));
         else
            subatom.reset(new QueryMolecule::Atom(QueryMolecule::ATOM_RING_BONDS, 1, 100));
      }
      else if (next == 'v')
      {
         scanner.skip(1);
         if (qatom.get() == 0)
            throw Error("'v' specifier is allowed only for query molecules");

         int val = 1;

         if (isdigit(scanner.lookNext()))
            val = scanner.readUnsigned();

         subatom.reset(new QueryMolecule::Atom(QueryMolecule::ATOM_TOTAL_BOND_ORDER, val));
      }
      else if (next == 'x')
      {
         scanner.skip(1);
         if (qatom.get() == 0)
            throw Error("'x' specifier is allowed only for query molecules");

         if (isdigit(scanner.lookNext()))
            subatom.reset(new QueryMolecule::Atom(QueryMolecule::ATOM_RING_BONDS, scanner.readUnsigned()));
         else
            subatom.reset(new QueryMolecule::Atom(QueryMolecule::ATOM_RING_BONDS, 1, 100));
      }
      else if (next == ':')
      {
         scanner.skip(1);
         atom.aam = scanner.readUnsigned();
      }
      else
         throw Error("invalid character within atom description: '%c'", next);

      if (element > 0)
      {
         if (element_assigned)
            throw Error("two element labels for one atom");
         if (qatom.get() != 0)
            subatom.reset(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, element));
         else
            atom.label = element;
         element_assigned = true;
      }

      if (aromatic != 0)
      {
         if (aromatic == ATOM_AROMATIC)
            atom.aromatic = true;

         if (qatom.get() != 0)
         {
            if (subatom.get() == 0)
               subatom.reset(new QueryMolecule::Atom(QueryMolecule::ATOM_AROMATICITY, aromatic));
            else
               subatom.reset(QueryMolecule::Atom::und(subatom.release(),
                             new QueryMolecule::Atom(QueryMolecule::ATOM_AROMATICITY, aromatic)));
         }
      }

      if (subatom.get() != 0)
      {
         if (neg)
         {
            subatom.reset(QueryMolecule::Atom::nicht(subatom.release()));
            neg = false;
         }
         qatom.reset(QueryMolecule::Atom::und(qatom.release(), subatom.release()));
      }

      // we check for isotope_set here to treat [2H] as deuterium atom,
      // not like something with isotope number 2 and h-count 1
      if (!isotope_set)
         first_in_brackets = false;
   }
}

SmilesLoader::_AtomDesc::_AtomDesc (Pool<List<int>::Elem> &neipool) :
               neighbors(neipool)
{
   label = 0;
   isotope = 0;
   charge = 0;
   hydrogens = -1;
   chirality = 0;
   aromatic = 0;
   aam = 0;
   brackets = false;
   star_atom = false;

   parent = -1;
}

SmilesLoader::_AtomDesc::~_AtomDesc ()
{
}

void SmilesLoader::_AtomDesc::pending (int cycle)
{
   if (cycle < 1)
      throw Error("cycle number %d is not allowed", cycle);
   neighbors.add(-cycle);
}

void SmilesLoader::_AtomDesc::closure (int cycle, int end)
{
   int i;

   if (cycle < 1)
      throw Error("cycle number %d is not allowed", cycle);
   
   for (i = neighbors.begin(); i != neighbors.end(); i = neighbors.next(i))
   {
      if (neighbors.at(i) == -cycle)
      {
         neighbors.at(i) = end;
         break;
      }
   }
}
