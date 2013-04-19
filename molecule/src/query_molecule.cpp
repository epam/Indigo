/****************************************************************************
 * Copyright (C) 2009-2012 GGA Software Services LLC
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

#include "molecule/query_molecule.h"
#include "molecule/molecule_arom.h"
#include "base_cpp/output.h"
#include "molecule/elements.h"

using namespace indigo;

QueryMolecule::QueryMolecule ()
{
}

QueryMolecule::~QueryMolecule ()
{
}

QueryMolecule& QueryMolecule::asQueryMolecule ()
{
   return *this;
}

bool QueryMolecule::isQueryMolecule ()
{
   return true;
}

int QueryMolecule::getAtomNumber (int idx)
{
   int res;
   QueryMolecule::Atom *atom  = _atoms[idx];

   if (atom->sureValue(ATOM_NUMBER, res))
      return res;

   return -1;
}

int QueryMolecule::getAtomIsotope (int idx)
{
   int res;

   if (_atoms[idx]->sureValue(ATOM_ISOTOPE, res))
      return res;

   return -1;
}

int QueryMolecule::getAtomCharge (int idx)
{
   int res;

   if (_atoms[idx]->sureValue(ATOM_CHARGE, res))
      return res;

   return CHARGE_UNKNOWN;
}

int QueryMolecule::getAtomRadical (int idx)
{
   int res;

   if (_atoms[idx]->sureValue(ATOM_RADICAL, res))
      return res;

   return -1;
}

int QueryMolecule::getExplicitValence (int idx)
{
   int res;

   if (_atoms[idx]->sureValue(ATOM_VALENCE, res))
      return res;

   return -1;
}

int QueryMolecule::getAtomAromaticity (int idx)
{
   int res;

   if (_atoms[idx]->sureValue(ATOM_AROMATICITY, res))
      return res;

   return -1;
}

int QueryMolecule::getBondOrder (int idx)
{
   int res;

   if (_bonds[idx]->sureValue(BOND_ORDER, res))
      return res;

   return -1;
}

int QueryMolecule::getBondTopology (int idx)
{
   int res;

   if (getEdgeTopology(idx) == TOPOLOGY_RING)
      return TOPOLOGY_RING;

   if (_bonds[idx]->sureValue(BOND_TOPOLOGY, res))
      return res;

   return -1;
}

int QueryMolecule::getAtomValence (int idx)
{
   throw Error("not implemented");
}

int QueryMolecule::getAtomSubstCount (int idx)
{
   int res;

   if (_atoms[idx]->sureValue(ATOM_SUBSTITUENTS, res))
      return res;
   if (_atoms[idx]->sureValue(ATOM_SUBSTITUENTS_AS_DRAWN, res))
      return res;

   return -1;
}

int QueryMolecule::getAtomRingBondsCount (int idx)
{
   int res;

   if (_atoms[idx]->sureValue(ATOM_RING_BONDS, res))
      return res;
   if (_atoms[idx]->sureValue(ATOM_RING_BONDS_AS_DRAWN, res))
      return res;

   return -1;
}

bool QueryMolecule::atomNumberBelongs (int idx, const int *numbers, int count)
{
   return _atoms[idx]->sureValueBelongs(ATOM_NUMBER, numbers, count);
}

bool QueryMolecule::possibleAtomNumber (int idx, int number)
{
   QueryMolecule::Atom *atom  = _atoms[idx];

   if (!atom->possibleValue(ATOM_NUMBER, number))
      return false;

   return true;
}

bool QueryMolecule::possibleAtomNumberAndCharge (int idx, int number, int charge)
{
   return _atoms[idx]->possibleValuePair(ATOM_NUMBER, number, ATOM_CHARGE, charge);
}

bool QueryMolecule::possibleAtomNumberAndIsotope (int idx, int number, int isotope)
{
   return _atoms[idx]->possibleValuePair(ATOM_NUMBER, number, ATOM_ISOTOPE, isotope);
}

bool QueryMolecule::possibleAtomIsotope (int idx, int isotope)
{
   return _atoms[idx]->possibleValue(ATOM_ISOTOPE, isotope);
}

bool QueryMolecule::possibleAtomCharge (int idx, int charge)
{
   return _atoms[idx]->possibleValue(ATOM_CHARGE, charge);
}

bool QueryMolecule::possibleAtomRadical (int idx, int radical)
{
   return _atoms[idx]->possibleValue(ATOM_RADICAL, radical);
}

void QueryMolecule::getAtomDescription (int idx, Array<char> &description)
{
   ArrayOutput out(description);

   out.writeChar('[');
   _getAtomDescription(_atoms[idx], out, 0);
   out.writeChar(']');
   out.writeChar(0);
}

void QueryMolecule::_getAtomDescription (Atom *atom, Output &out, int depth)
{
   int i;
   
   switch (atom->type)
   {
      case OP_NONE:
         out.writeChar('*');
         return;
      case OP_AND:
      {
         if (depth > 0)
            out.writeChar('(');
         for (i = 0; i < atom->children.size(); i++)
         {
            if (i > 0)
               out.writeString(";");
            _getAtomDescription((Atom *)atom->children[i], out, depth+1);
         }
         if (depth > 0)
            out.writeChar(')');
         return;
      }
      case OP_OR:
      {
         if (depth > 0)
            out.writeChar('(');
         for (i = 0; i < atom->children.size(); i++)
         {
            if (i > 0)
               out.writeString(",");
            _getAtomDescription((Atom *)atom->children[i], out, depth+1);
         }
         if (depth > 0)
            out.writeChar(')');
         return;
      }
      case OP_NOT:
         out.writeString("!");
         _getAtomDescription((Atom *)atom->children[0], out,depth+1);
         return;
      case ATOM_NUMBER:
         out.writeString(Element::toString(atom->value_min));
         return;
      case ATOM_PSEUDO:
         out.writeString(atom->alias.ptr());
         return;
      case ATOM_CHARGE:
         out.printf("%+d", atom->value_min);
         return;
      case ATOM_ISOTOPE:
         out.printf("i%d", atom->value_min);
         return;
      case ATOM_AROMATICITY:
         if (atom->value_min == ATOM_AROMATIC)
            out.printf("a");
         else
            out.printf("A");
         return;
      case ATOM_RADICAL:
         out.printf("^%d", atom->value_min);
         return;
      case ATOM_FRAGMENT:
         out.printf("$(");
         if (atom->fragment->fragment_smarts.ptr() != 0 &&
             atom->fragment->fragment_smarts.size() > 0)
            out.printf("%s", atom->fragment->fragment_smarts.ptr());
         out.printf(")");
         return;
      case ATOM_TOTAL_H:
         out.printf("H%d", atom->value_min);
         return;
      case ATOM_CONNECTIVITY:
         out.printf("X%d", atom->value_min);
         return;
      case ATOM_SUBSTITUENTS:
         out.printf("s%d", atom->value_min);
         return;
      case ATOM_SUBSTITUENTS_AS_DRAWN:
         out.printf("s*");
         return;
      case ATOM_RING_BONDS:
         out.printf("rb%d", atom->value_min);
         return;
      case ATOM_RING_BONDS_AS_DRAWN:
         out.printf("rb*");
         return;
      case ATOM_UNSATURATION:
         out.printf("u");
         return;
      case ATOM_TOTAL_BOND_ORDER:
         out.printf("v%d", atom->value_min);
         return;
      case ATOM_SSSR_RINGS:
         out.printf("R%d", atom->value_min);
         return;
      case ATOM_SMALLEST_RING_SIZE:
         out.printf("r%d", atom->value_min);
         return;
      default:
         throw new Error("Unrecognized constraint type %d", atom->type);
   }
}

void QueryMolecule::getBondDescription (int idx, Array<char> &description)
{
   ArrayOutput out(description);

   _getBondDescription(_bonds[idx], out);
   out.writeChar(0);
}

void QueryMolecule::_getBondDescription (Bond *bond, Output &out)
{
   int i;
   
   switch (bond->type)
   {
      case OP_NONE:
         out.writeChar('~');
         return;
      case OP_AND:
      {
         out.writeChar('(');
         for (i = 0; i < bond->children.size(); i++)
         {
            if (i > 0)
               out.writeString(" & ");
            _getBondDescription((Bond *)bond->children[i], out);
         }
         out.writeChar(')');
         return;
      }
      case OP_OR:
      {
         out.writeChar('(');
         for (i = 0; i < bond->children.size(); i++)
         {
            if (i > 0)
               out.writeString(" | ");
            _getBondDescription((Bond  *)bond->children[i], out);
         }
         out.writeChar(')');
         return;
      }
      case OP_NOT:
         out.writeString("!(");
         _getBondDescription((Bond *)bond->children[0], out);
         out.writeChar(')');
         return;
      case BOND_ORDER:
         out.printf("order = %d", bond->value);
         return;
      case BOND_TOPOLOGY:
         out.printf("%s", bond->value == TOPOLOGY_RING ? "ring" : "chain");
         return;
      default:
         out.printf("<constraint of type %d>", bond->type);
   }
}


bool QueryMolecule::possibleBondOrder (int idx, int order)
{
   return _bonds[idx]->possibleValue(BOND_ORDER, order);
}

bool QueryMolecule::possibleNitrogenV5 (int idx)
{
   if (!possibleAtomNumber(idx, ELEM_N))
      return false;

   if (!possibleAtomCharge(idx, 0))
      return false;

   // Other conditions also can be checked as in Molecule::isNitrogenV5 but
   // two the meaning of this function can be different: check if nitrogen 
   // can valence 5 in the embedding or check if nitrogen can have valence 5
   // in the original query molecule as self-contained molecule

   return true;
}

bool QueryMolecule::isPseudoAtom (int idx)
{
   // This is dirty hack; however, it is legal here, as pseudo atoms
   // can not be present in deep query trees due to Molfile and SMILES
   // format limitations. If they could, we would have to implement
   // sureValue() for ATOM_PSEUDO

   if (_atoms[idx]->type == ATOM_PSEUDO)
      return true;

   if (_atoms[idx]->type == OP_AND)
   {
      int i;

      for (i = 0; i < _atoms[idx]->children.size(); i++)
         if (_atoms[idx]->children[i]->type == ATOM_PSEUDO)
            return true;
   }

   return false;
}

const char * QueryMolecule::getPseudoAtom (int idx)
{
   // see the comment above in isPseudoAtom()
   
   if (_atoms[idx]->type == ATOM_PSEUDO)
      return _atoms[idx]->alias.ptr();

   if (_atoms[idx]->type == OP_AND)
   {
      int i;

      for (i = 0; i < _atoms[idx]->children.size(); i++)
         if (_atoms[idx]->children[i]->type == ATOM_PSEUDO)
            return ((Atom *)_atoms[idx]->children[i])->alias.ptr();
   }

   throw Error("getPseudoAtom() applied to something that is not a pseudo-atom");
}


bool QueryMolecule::isSaturatedAtom (int idx)
{
   throw Error("not implemented");
}

QueryMolecule::Node::Node (int type_)
{
   type = (OpType)type_;
}

QueryMolecule::Node::~Node ()
{
}

IMPL_ERROR(QueryMolecule::Atom, "query atom");

QueryMolecule::Atom::Atom () : Node(OP_NONE)
{
   value_min = 0;
   value_max = 0;
}

QueryMolecule::Atom::Atom (int type_, int value) : Node(type_)
{
   if (type_ == ATOM_NUMBER  || type_ == ATOM_CHARGE ||
       type_ == ATOM_ISOTOPE || type_ == ATOM_RADICAL ||
       type_ == ATOM_AROMATICITY || type_ == ATOM_VALENCE ||
       type_ == ATOM_RING_BONDS || type_ == ATOM_RING_BONDS_AS_DRAWN ||
       type_ == ATOM_SUBSTITUENTS || type_ == ATOM_SUBSTITUENTS_AS_DRAWN ||
       type_ == ATOM_TOTAL_H || type_ == ATOM_CONNECTIVITY ||
       type_ == ATOM_TOTAL_BOND_ORDER ||
       type_ == ATOM_UNSATURATION || type == ATOM_SSSR_RINGS ||
       type == ATOM_SMALLEST_RING_SIZE || type == ATOM_RSITE || type == HIGHLIGHTING)
      value_min = value_max = value;
   else
      throw Error("bad type: %d", type_);
}

QueryMolecule::Atom::Atom (int type_, int val_min, int val_max) : Node(type_)
{
   value_min = val_min;
   value_max = val_max;
}

QueryMolecule::Atom::Atom (int type_, const char *value) : Node(type_)
{
   if (type_ == ATOM_PSEUDO)
      alias.readString(value, true);
   else
      throw Error("bad type: %d", type_);
}

QueryMolecule::Atom::Atom (int type_, QueryMolecule *value) : Node(type_)
{
   if (type_ == ATOM_FRAGMENT)
      fragment.reset(value);
   else
      throw Error("bad type: %d", type_);
}

QueryMolecule::Atom::~Atom ()
{
}

QueryMolecule::Bond::Bond () : Node(OP_NONE)
{
}

QueryMolecule::Bond::Bond (int type_, int value_) : Node(type_)
{
   value = value_;
}

QueryMolecule::Bond::~Bond ()
{
}


QueryMolecule::Node * QueryMolecule::Node::_und
                      (QueryMolecule::Node *node1, QueryMolecule::Node *node2)
{
   if (node1->type == QueryMolecule::OP_NONE)
   {
      delete node1;
      return node2;
   }

   if (node2->type == QueryMolecule::OP_NONE)
   {
      delete node2;
      return node1;
   }

   if (node1->type == QueryMolecule::OP_AND)
   {
      if (node2->type == QueryMolecule::OP_AND)
      {
         while (node2->children.size() != 0)
            node1->children.add(node2->children.pop());
      }
      else
         node1->children.add(node2);

      return node1;
   }

   if (node2->type == QueryMolecule::OP_AND)
   {
      node2->children.add(node1);
      return node2;
   }

   AutoPtr<QueryMolecule::Node> newnode(node1->_neu());

   newnode->type = QueryMolecule::OP_AND;
   newnode->children.add(node1);
   newnode->children.add(node2);
   return newnode.release();
}

QueryMolecule::Node * QueryMolecule::Node::_oder
                      (QueryMolecule::Node *node1, QueryMolecule::Node *node2)
{
   if (node1->type == QueryMolecule::OP_NONE)
   {
      delete node2;
      return node1;
   }

   if (node2->type == QueryMolecule::OP_NONE)
   {
      delete node1;
      return node2;
   }

   if (node1->type == QueryMolecule::OP_OR)
   {
      if (node2->type == QueryMolecule::OP_OR)
      {
         while (node2->children.size() != 0)
            node1->children.add(node2->children.pop());
      }
      else
         node1->children.add(node2);

      return node1;
   }

   if (node2->type == QueryMolecule::OP_OR)
   {
      node2->children.add(node1);
      return node2;
   }

   AutoPtr<QueryMolecule::Node> newnode(node1->_neu());

   newnode->type = QueryMolecule::OP_OR;
   newnode->children.add(node1);
   newnode->children.add(node2);
   return newnode.release();
}

QueryMolecule::Node * QueryMolecule::Node::_nicht (QueryMolecule::Node *node)
{
   if (node->type == QueryMolecule::OP_NOT)
   {
      Node * res = (Node *)node->children.pop();
      delete node;
      return res;
   }

   AutoPtr<QueryMolecule::Node> newnode(node->_neu());

   newnode->type = QueryMolecule::OP_NOT;
   newnode->children.add(node);
   return newnode.release();
}

QueryMolecule::Atom * QueryMolecule::Atom::und (Atom *atom1, Atom *atom2)
{
   return (Atom *)_und(atom1, atom2);
}

QueryMolecule::Atom * QueryMolecule::Atom::oder (Atom *atom1, Atom *atom2)
{
   return (Atom *)_oder(atom1, atom2);
}

QueryMolecule::Atom * QueryMolecule::Atom::nicht (Atom *atom)
{
   return (Atom *)_nicht(atom);
}

bool QueryMolecule::Atom::hasConstraintWithValue (int what_type, int what_value)
{
   if (type == what_type)
      return value_max == what_value && value_min == what_value;

   if (type == OP_NONE)
      return false;

   if (type == OP_AND || type == OP_OR || type == OP_NOT)
   {
      int i;

      for (i = 0; i < children.size(); i++)
         if (((Atom *)children[i])->hasConstraintWithValue(what_type, what_value))
            return true;
   }

   return false;
}

QueryMolecule::Bond * QueryMolecule::Bond::und (Bond *bond1, Bond *bond2)
{
   return (Bond *)_und(bond1, bond2);
}

QueryMolecule::Bond * QueryMolecule::Bond::oder (Bond *bond1, Bond *bond2)
{
   return (Bond *)_oder(bond1, bond2);
}

QueryMolecule::Bond * QueryMolecule::Bond::nicht (Bond *bond)
{
   return (Bond *)_nicht(bond);
}


void QueryMolecule::_flipBond (int atom_parent, int atom_from, int atom_to)
{
   int src_bond_idx = findEdgeIndex(atom_parent, atom_from);
   
   int new_bond_idx = addBond(atom_parent, atom_to, releaseBond(src_bond_idx));
   // Copy aromaticity information
   aromaticity.setCanBeAromatic(new_bond_idx, aromaticity.canBeAromatic(src_bond_idx));
   setBondStereoCare(new_bond_idx, bondStereoCare(src_bond_idx));
   updateEditRevision();
}

void QueryMolecule::_mergeWithSubmolecule (BaseMolecule &bmol, const Array<int> &vertices,
           const Array<int> *edges, const Array<int> &mapping, int skip_flags)
{
   QueryMolecule &mol = bmol.asQueryMolecule();
   int i;

   // atoms
   for (i = 0; i < vertices.size(); i++)
   {
      int newidx = mapping[vertices[i]];

      _atoms.expand(newidx + 1);
      _atoms.set(newidx, mol.getAtom(vertices[i]).clone());
   }
      
   // bonds
   if (edges != 0)
      for (i = 0; i < edges->size(); i++)
      {
         const Edge &edge = mol.getEdge(edges->at(i));
         int beg = mapping[edge.beg];
         int end = mapping[edge.end];

         if (beg == -1 || end == -1)
            // must have been thrown before in mergeWithSubgraph()
            throw Error("_mergeWithSubmolecule: internal");

         int idx = findEdgeIndex(beg, end);

         _bonds.expand(idx + 1);
         _bonds.set(idx, mol.getBond(edges->at(i)).clone());

         // Aromaticity
         if (!(skip_flags & SKIP_AROMATICITY))
            aromaticity.setCanBeAromatic(idx, mol.aromaticity.canBeAromatic(edges->at(i)));
         if (!(skip_flags & SKIP_CIS_TRANS) && mol.cis_trans.getParity(edges->at(i)) != 0)
            setBondStereoCare(idx, mol.bondStereoCare(edges->at(i)));
      }
   else
      for (i = mol.edgeBegin(); i < mol.edgeEnd(); i = mol.edgeNext(i))
      {
         const Edge &edge = mol.getEdge(i);
         int beg = mapping[edge.beg];
         int end = mapping[edge.end];

         if (beg == -1 || end == -1)
            continue;

         int idx = findEdgeIndex(beg, end);

         _bonds.expand(idx + 1);
         _bonds.set(idx, mol.getBond(i).clone());

         // Aromaticity
         if (!(skip_flags & SKIP_AROMATICITY))
            aromaticity.setCanBeAromatic(idx, mol.aromaticity.canBeAromatic(i));
         if (!(skip_flags & SKIP_CIS_TRANS) && mol.cis_trans.getParity(i) != 0)
            setBondStereoCare(idx, mol.bondStereoCare(i));
      }

   // 3D constraints
   if (!(skip_flags & SKIP_3D_CONSTRAINTS))
      spatial_constraints.buildOnSubmolecule(mol.spatial_constraints, mapping.ptr());

   // fixed atoms
   if (!(skip_flags & SKIP_FIXED_ATOMS))
   {
      for (i = 0; i < mol.fixed_atoms.size(); i++)
      {
         int idx = mapping[mol.fixed_atoms[i]];

         if (idx >= 0)
            fixed_atoms.push(idx);
      }
   }

   // components
   if (!(skip_flags & SKIP_COMPONENTS))
   {
      for (i = 0; i < vertices.size(); i++)
      {
         int v_idx = vertices[i];
         if (mol.components.size() > v_idx)
         {
            int newidx = mapping[v_idx];
            components.expandFill(newidx + 1, 0);
            components[newidx] = mol.components[v_idx];
         }
      }
   }

   updateEditRevision();
}

void QueryMolecule::_postMergeWithSubmolecule (BaseMolecule &bmol, const Array<int> &vertices,
                                    const Array<int> *edges, const Array<int> &mapping,
                                    int skip_flags)
{
   // Remove stereocare flags for bonds that are not cis-trans
   for (int i = edgeBegin(); i != edgeEnd(); i = edgeNext(i))
      if (cis_trans.getParity(i) == 0)
         setBondStereoCare(i, false);
}


void QueryMolecule::_removeAtoms (const Array<int> &indices, const int *mapping)
{
   spatial_constraints.removeAtoms(mapping);

   if (attachmentPointCount() > 0)
   {
      int i;
      
      for (i = 0; i < indices.size(); i++)
         this->removeAttachmentPointsFromAtom(indices[i]);

      bool empty = true;

      for (i = 1; i <= this->attachmentPointCount(); i++)
         if (this->getAttachmentPoint(i, 0) != -1)
         {
            empty = false;
            break;
         }

      if (empty)
         _attachment_index.clear();
   }

   for (int i = 0; i < indices.size(); i++)
   {
      int idx = indices[i];
      _atoms.reset(idx);
      if (idx < _rsite_attachment_points.size())
         _rsite_attachment_points[idx].clear();
   }

   // Collect bonds to remove
   QS_DEF(Array<int>, edges_to_remove);
   edges_to_remove.clear();
   for (int i = edgeBegin(); i != edgeEnd(); i = edgeNext(i))
   {
      const Edge &e = getEdge(i);
      if (mapping[e.beg] == -1 || mapping[e.end] == -1)
         edges_to_remove.push(i);
   }
   _removeBonds(edges_to_remove);
   updateEditRevision();
}

void QueryMolecule::_removeBonds (const Array<int> &indices)
{
   for (int i = 0; i < indices.size(); i++)
      _bonds.reset(indices[i]);

   _min_h.clear();
   updateEditRevision();
}

bool QueryMolecule::Node::sureValue (int what_type, int &value_out)
{
   int i;
   
   switch (type)
   {
      case OP_AND:
      {
         // of course one can fool this around, specifying something like
         // "(C || O) && (C || N)", but we do not claim to have perfect
         // logic here, and we do not need one.
         
         int  child_value = -1;
         bool child_value_valid = false;

         for (i = 0; i < children.size(); i++)
         {
            int child_value_tmp;
            if (children[i]->sureValue(what_type, child_value_tmp))
            {
               if (!child_value_valid)
               {
                  child_value_valid = true;
                  child_value = child_value_tmp;
               }
               else
               {
                  if (child_value_tmp != child_value)
                     return false; // self-contradictory query atom it is
               }
            }
         }

         if (child_value_valid)
         {
            value_out = child_value;
            return true;
         }
         return false;
      }
      case OP_OR:
      {
         int child_value = -1;

         for (i = 0; i < children.size(); i++)
         {
            int child_value_tmp;
            if (children[i]->sureValue(what_type, child_value_tmp))
            {
               if (i == 0)
                  child_value = child_value_tmp;
               else if (child_value_tmp != child_value)
                  return false;
            }
            else
               return false;
         }
         value_out = child_value;
         return true;
      }
      case OP_NOT:
         return children[0]->sureValueInv(what_type, value_out);
      case OP_NONE:
         return false;
      default:
         return _sureValue(what_type, value_out);
   }
}

bool QueryMolecule::Node::sureValueInv (int what_type, int &value_out)
{
   int i;

   switch (type)
   {
      case OP_OR:
      {
         int  child_value = -1;
         bool child_value_valid = false;

         for (i = 0; i < children.size(); i++)
         {
            int child_value_tmp;
            if (children[i]->sureValueInv(what_type, child_value_tmp))
            {
               if (!child_value_valid)
               {
                  child_value_valid = true;
                  child_value = child_value_tmp;
               }
               else
               {
                  if (child_value_tmp != child_value)
                     return false; // self-contradictory query atom it is
               }
            }
         }

         if (child_value_valid)
         {
            value_out = child_value;
            return true;
         }
         return false;
      }
      case OP_AND:
      {
         int child_value = -1;

         for (i = 0; i < children.size(); i++)
         {
            int child_value_tmp;
            if (children[i]->sureValueInv(what_type, child_value_tmp))
            {
               if (i == 0)
                  child_value = child_value_tmp;
               else if (child_value_tmp != child_value)
                  return false;
            }
            else
               return false;
         }
         value_out = child_value;
         return true;
      }
      case OP_NOT:
         return children[0]->sureValue(what_type, value_out);
      case OP_NONE:
         throw QueryMolecule::Error("sureValueInv(OP_NONE) not implemented");
      default:
         return false;
   }
}

bool QueryMolecule::Node::possibleValue (int what_type, int what_value)
{
   int i;

   switch (type)
   {
      case OP_AND:
      {
         for (i = 0; i < children.size(); i++)
            if (!children[i]->possibleValue(what_type, what_value))
               return false;

         return true;
      }
      case OP_OR:
      {
         for (i = 0; i < children.size(); i++)
            if (children[i]->possibleValue(what_type, what_value))
               return true;

         return false;
      }
      case OP_NOT:
         return children[0]->possibleValueInv(what_type, what_value);
      case OP_NONE:
         return true;
      default:
         return _possibleValue(what_type, what_value);
   }
}

bool QueryMolecule::Node::possibleValueInv (int what_type, int what_value)
{
   int i;

   switch (type)
   {
      case OP_AND:
      {
         for (i = 0; i < children.size(); i++)
            if (children[i]->possibleValueInv(what_type, what_value))
               return true;

         return false;
      }
      case OP_OR:
      {
         for (i = 0; i < children.size(); i++)
            if (!children[i]->possibleValueInv(what_type, what_value))
               return false;

         return true;
      }
      case OP_NOT:
         return children[0]->possibleValue(what_type, what_value);
      case OP_NONE:
         throw QueryMolecule::Error("possibleValueInv(OP_NONE) not implemented");
      default:
      {
         int val;

         if (!_sureValue(what_type, val))
            return true;
         
         return val != what_value;
      }
   }
}

bool QueryMolecule::Node::possibleValuePair (int what_type1, int what_value1,
                                             int what_type2, int what_value2)
{
   int i;

   switch (type)
   {
      case OP_AND:
      {
         for (i = 0; i < children.size(); i++)
            if (!children[i]->possibleValuePair(what_type1, what_value1,
                                                what_type2, what_value2))
               return false;

         return true;
      }
      case OP_OR:
      {
         for (i = 0; i < children.size(); i++)
            if (children[i]->possibleValuePair(what_type1, what_value1,
                                               what_type2, what_value2))
               return true;

         return false;
      }
      case OP_NOT:
         return children[0]->possibleValuePairInv(what_type1, what_value1,
                                                  what_type2, what_value2);
      case OP_NONE:
         return true;
      default:
         return _possibleValuePair(what_type1, what_value1, what_type2, what_value2);
   }
}

bool QueryMolecule::Node::possibleValuePairInv (int what_type1, int what_value1,
                                                int what_type2, int what_value2)
{
   int i;

   switch (type)
   {
      case OP_AND:
      {
         for (i = 0; i < children.size(); i++)
            if (children[i]->possibleValuePairInv(what_type1, what_value1,
                                                  what_type2, what_value2))
               return true;

         return false;
      }
      case OP_OR:
      {
         for (i = 0; i < children.size(); i++)
            if (!children[i]->possibleValuePairInv(what_type1, what_value1,
                                                   what_type2, what_value2))
               return false;

         return true;
      }
      case OP_NOT:
         return children[0]->possibleValuePair(what_type1, what_value1, what_type2, what_value2);
      case OP_NONE:
         throw QueryMolecule::Error("possibleValuePairInv(OP_NONE) not implemented");
      default:
      {
         int val1, val2;
         bool sure1, sure2;

         if ((sure1 = _sureValue(what_type1, val1)) &&
                 !hasConstraint(what_type2) && val1 == what_value1)
            return false;

         if ((sure2 = _sureValue(what_type2, val2)) &&
                 !hasConstraint(what_type1) && val2 == what_value2)
            return false;

         if (sure1 && sure2 && val1 == what_value1 && val2 == what_value2)
            return false;

         return true;
      }
   }
}


bool QueryMolecule::Node::sureValueBelongs (int what_type, const int *arr, int count)
{
   int i;

   switch (type)
   {
      case OP_AND:
      {
         // Of course one can fool this around;
         // see comment in sureValue()
         for (i = 0; i < children.size(); i++)
            if (children[i]->sureValueBelongs(what_type, arr, count))
               return true;
         
         return false;
      }
      case OP_OR:
      {
         for (i = 0; i < children.size(); i++)
            if (!children[i]->sureValueBelongs(what_type, arr, count))
               return false;
         
         return true;
      }
      case OP_NOT:
         return children[0]->sureValueBelongsInv(what_type, arr, count);
      case OP_NONE:
         return false;
      default:
         return _sureValueBelongs(what_type, arr, count);
   }
}

QueryMolecule::Atom* QueryMolecule::Atom::sureConstraint (int what_type)
{
   int count = 0;
   Atom *found = (Atom*)_findSureConstraint(what_type, count);
   if (count == 1)
      return found;
   return NULL;
}

QueryMolecule::Node* QueryMolecule::Node::_findSureConstraint (int what_type, int &count)
{
   switch (type)
   {
   case OP_AND:
   case OP_OR:
      {
         Node *subnode_found = NULL;
         for (int i = 0; i < children.size(); i++)
         {
            Node *subnode = children[i]->_findSureConstraint(what_type, count);
            if (subnode != NULL)
               subnode_found = subnode;
         }
         return subnode_found;
      }
   case OP_NOT:
      {
         Node *subnode = children[0]->_findSureConstraint(what_type, count);
         return NULL; // Do not return anything in this case but increase count if found
      }
   case OP_NONE:
      return NULL;
   default:
      if (type == what_type)
      {
         count++;
         return this;
      }
      return NULL;
   }
}

bool QueryMolecule::Node::sureValueBelongsInv (int what_type, const int *arr, int count)
{
   int i;

   switch (type)
   {
      case OP_OR:
      {
         for (i = 0; i < children.size(); i++)
            if (children[i]->sureValueBelongsInv(what_type, arr, count))
               return true;
         
         return false;
      }
      case OP_AND:
      {
         for (i = 0; i < children.size(); i++)
         {
            if (!children[i]->sureValueBelongsInv(what_type, arr, count))
               return false;
         }
         return true;
      }
      case OP_NOT:
         return children[0]->sureValueBelongs(what_type, arr, count);
      case OP_NONE:
         throw QueryMolecule::Error("sureValueBelongsInv(OP_NONE) not implemented");
      default:
         return false;
   }
}

void QueryMolecule::Node::optimize ()
{
   switch (type)
   {
   case OP_AND:
   case OP_OR:
   case OP_NOT:
      for (int i = 0; i < children.size(); i++)
         children[i]->optimize();
      break;
   case OP_NONE:
      return;
   }
   _optimize();
}

bool QueryMolecule::Atom::_sureValue (int what_type, int& value_out)
{
   if (type == what_type && value_max == value_min)
   {
      value_out = value_min;
      return true;
   }

   // common case for fragment 'atoms' in SMARTS queries
   if (type == ATOM_FRAGMENT && fragment->vertexCount() > 0)
   {
      if (fragment->getAtom(fragment->vertexBegin()).sureValue(what_type, value_out))
         return true;
   }

   return false;
}

bool QueryMolecule::Atom::_sureValueBelongs (int what_type, const int *arr, int count)
{
   int i;

   if (type == what_type)
   {
      for (i = 0; i < count; i++)
         if (arr[i] < value_min || arr[i] > value_max)
            return false;
      return true;
   }
   
   return false;
}

bool QueryMolecule::Atom::_possibleValue (int what_type, int what_value)
{
   if (type == what_type)
      return what_value >= value_min && what_value <= value_max;
   
   if (type == ATOM_FRAGMENT && fragment->vertexCount() > 0)
   {
      if (!fragment->getAtom(fragment->vertexBegin()).
           possibleValue(what_type, what_value))
         return false;
   }
   
   return true;
}

bool QueryMolecule::Atom::_possibleValuePair (int what_type1, int what_value1,
                                              int what_type2, int what_value2)
{
   if (type == what_type1)
      return what_value1 >= value_min && what_value1 <= value_max;

   if (type == what_type2)
      return what_value2 >= value_min && what_value2 <= value_max;

   if (type == ATOM_FRAGMENT && fragment->vertexCount() > 0)
   {
      if (!fragment->getAtom(fragment->vertexBegin()).
            possibleValuePair(what_type1, what_value1, what_type2, what_value2))
         return false;
   }
   return true;
}

void QueryMolecule::Atom::_optimize ()
{
   // Check if fragment has one atom
   if (type == ATOM_FRAGMENT && fragment->vertexCount() == 1)
   {
      AutoPtr<QueryMolecule> saved_fragment(fragment.release());
      copy(saved_fragment->getAtom(saved_fragment->vertexBegin()));
   }
}

bool QueryMolecule::Bond::_sureValue (int what_type, int& value_out)
{
   if (type == what_type)
   {
      value_out = value;
      return true;
   }
   
   return false;
}

bool QueryMolecule::Bond::_sureValueBelongs (int what_type, const int *arr, int count)
{
   throw QueryMolecule::Error("not implemented");
}

bool QueryMolecule::Bond::_possibleValue (int what_type, int what_value)
{
   if (type == what_type)
      return what_value == value;

   return true;
}

bool QueryMolecule::Bond::_possibleValuePair (int what_type1, int what_value1,
                                              int what_type2, int what_value2)
{
   if (type == what_type1)
      return what_value1 == value;
   if (type == what_type2)
      return what_value2 == value;
   return false;
}

bool QueryMolecule::Atom::valueWithinRange (int value)
{
   return value >= value_min && value <= value_max;
}

QueryMolecule::Atom * QueryMolecule::Atom::child (int idx)
{
   return (Atom *)children[idx];
}

QueryMolecule::Bond * QueryMolecule::Bond::child (int idx)
{
   return (Bond *)children[idx];
}

QueryMolecule::Node * QueryMolecule::Atom::_neu ()
{
   return new Atom();
}

QueryMolecule::Node * QueryMolecule::Bond::_neu ()
{
   return new Bond();
}

int QueryMolecule::addAtom (Atom *atom)
{
   int idx = _addBaseAtom();

   _atoms.expand(idx + 1);
   _atoms.set(idx, atom);

   updateEditRevision();
   return idx;
}

QueryMolecule::Atom & QueryMolecule::getAtom (int idx)
{
   return *_atoms[idx];
}

QueryMolecule::Atom * QueryMolecule::releaseAtom (int idx)
{
   updateEditRevision();
   return _atoms.release(idx);
}

void QueryMolecule::resetAtom (int idx, QueryMolecule::Atom *atom)
{
   _atoms.reset(idx);
   _atoms[idx] = atom;
   updateEditRevision();
}

int QueryMolecule::addBond (int beg, int end, QueryMolecule::Bond *bond)
{
   int idx = _addBaseBond(beg, end);

   _bonds.expand(idx + 1);
   _bonds.set(idx, bond);

   invalidateAtom(beg, CHANGED_CONNECTIVITY);
   invalidateAtom(end, CHANGED_CONNECTIVITY);

   aromaticity.setCanBeAromatic(idx, false);
   setBondStereoCare(idx, false);

   updateEditRevision();

   return idx;
}

QueryMolecule::Bond & QueryMolecule::getBond (int idx)
{
   return *_bonds[idx];
}

QueryMolecule::Bond * QueryMolecule::releaseBond (int idx)
{
   updateEditRevision();
   return _bonds.release(idx);
}

void QueryMolecule::resetBond (int idx, QueryMolecule::Bond *bond)
{
   _bonds.reset(idx);
   _bonds[idx] = bond;
   _min_h.clear();
   updateEditRevision();
}

void QueryMolecule::Atom::copy (Atom &other)
{
   type = other.type;
   value_max = other.value_max;
   value_min = other.value_min;

   fragment.reset(0);
   if (other.fragment.get() != 0)
   {
      fragment.reset(new QueryMolecule());
      fragment->clone(other.fragment.ref(), 0, 0);
      fragment->fragment_smarts.copy(other.fragment->fragment_smarts);
   }
   alias.copy(other.alias);

   children.clear();
   for (int i = 0; i < other.children.size(); i++)
      children.add(((Atom *)other.children[i])->clone());
}

QueryMolecule::Atom * QueryMolecule::Atom::clone ()
{
   AutoPtr<Atom> res(new Atom());
   res->copy(*this);
   return res.release();
}

QueryMolecule::Bond * QueryMolecule::Bond::clone ()
{
   AutoPtr<Bond> res(new Bond());
   int i;

   res->type = type;
   res->value = value;

   for (i = 0; i < children.size(); i++)
      res->children.add(((Bond *)children[i])->clone());

   return res.release();
}

bool QueryMolecule::Node::hasConstraint (int what_type)
{
   if (type == what_type)
      return true;

   if (type == OP_NONE)
      return false;

   if (type == OP_AND || type == OP_OR || type == OP_NOT)
   {
      for (int i = 0; i < children.size(); i++)
         if (children[i]->hasConstraint(what_type))
            return true;
   }

   return false;
}

bool QueryMolecule::Node::hasNoConstraintExcept (int what_type)
{
   return hasNoConstraintExcept(what_type, what_type);
}

bool QueryMolecule::Node::hasNoConstraintExcept (int what_type1, int what_type2)
{
   if (type == OP_NONE)
      return true;

   if (type == OP_AND || type == OP_OR || type == OP_NOT)
   {
      int i;

      for (i = 0; i < children.size(); i++)
         if (!children[i]->hasNoConstraintExcept(what_type1, what_type2))
            return false;

      return true;
   }

   return type == what_type1 || type == what_type2;
}

void QueryMolecule::Node::removeConstraints (int what_type)
{
   if (type == what_type)
   {
      type = OP_NONE;
      return;
   }

   if (type == OP_AND || type == OP_OR || type == OP_NOT)
   {
      int i;
      
      for (i = children.size() - 1; i >= 0; i--)
      {
         children[i]->removeConstraints(what_type);
         if (children[i]->type == OP_NONE)
            children.remove(i);
      }
      if (children.size() == 0)
         type = OP_NONE;
   }
}

void QueryMolecule::clear ()
{
   BaseMolecule::clear();

   _atoms.clear();
   _bonds.clear();
   spatial_constraints.clear();
   fixed_atoms.clear();
   _bond_stereo_care.clear();
   _min_h.clear();
   aromaticity.clear();
   components.clear();
   fragment_smarts.clear();
   updateEditRevision();
}

BaseMolecule * QueryMolecule::neu ()
{
   return new QueryMolecule();
}

bool QueryMolecule::bondStereoCare (int idx)
{
   if (idx >= _bond_stereo_care.size())
      return false;

   if (_bond_stereo_care[idx] && cis_trans.getParity(idx) == 0)
      throw Error("bond #%d has stereo-care flag, but is not cis-trans bond", idx); 

   return _bond_stereo_care[idx];
}

void QueryMolecule::setBondStereoCare (int idx, bool stereo_care)
{
   if (stereo_care == false && idx >= _bond_stereo_care.size())
      return;

   _bond_stereo_care.expandFill(idx + 1, false);
   _bond_stereo_care[idx] = stereo_care;
   updateEditRevision();
}

bool QueryMolecule::aromatize (const AromaticityOptions &options)
{
   updateEditRevision();
   return QueryMoleculeAromatizer::aromatizeBonds(*this, options);
}

bool QueryMolecule::dearomatize (const AromaticityOptions &options)
{
   throw Error("Dearomatization not implemented");
}

int QueryMolecule::getAtomMaxH (int idx)
{
   int total;

   if (_atoms[idx]->sureValue(ATOM_TOTAL_H, total))
      return total;

   int number = getAtomNumber(idx);

   if (number == -1)
      return -1;

   int conn = _calcAtomConnectivity(idx);

   if (conn == -1)
      return -1;

   int explicit_val = getExplicitValence(idx);

   int max_h = 0;

   int charge, radical;

   for (charge = -5; charge <= 8; charge++)
   {
      if (!possibleAtomCharge(idx, charge))
         continue;

      for (radical = 0; radical <= RADICAL_DOUBLET; radical++)
      {
         if (!possibleAtomRadical(idx, radical))
            continue;

         if (explicit_val != -1)
         {
            int h = explicit_val - Element::calcValenceMinusHyd(number, charge, radical, conn);
            if (h > max_h)
               max_h = h;
         }
         else
         {
            int h, val;
            if (Element::calcValence(number, charge, radical, conn, val, h, false))
            {
               if (h > max_h)
                  max_h = h;
            }
         }
      }
   }

   const Vertex &vertex = getVertex(idx);
   int i;

   for (i = vertex.neiBegin(); i != vertex.neiEnd(); i = vertex.neiNext(i))
   {
      if (possibleAtomNumber(vertex.neiVertex(i), ELEM_H))
         max_h++;
   }

   return max_h;
}

int QueryMolecule::getAtomMinH (int idx)
{
   if (_min_h.size() > idx && _min_h[idx] >= 0)
      return _min_h[idx];

   int i, min_h = _getAtomMinH(_atoms[idx]);

   if (min_h >= 0)
   {
      _min_h.expandFill(idx + 1, -1);
      _min_h[idx] = min_h;
      return min_h;
   }

   const Vertex &vertex = getVertex(idx);
   min_h = 0;

   for (i = vertex.neiBegin(); i != vertex.neiEnd(); i = vertex.neiNext(i))
   {
      if (getAtomNumber(vertex.neiVertex(i)) == ELEM_H)
         min_h++;
   }

   if (stereocenters.getType(idx) > 0)
   {
      if (vertex.degree() == 3)
      {
         int number = getAtomNumber(idx);
         int charge = getAtomCharge(idx);
         
         if (number == ELEM_C || number == ELEM_Si ||
            (number == ELEM_N && charge == 1))
            min_h++;
      }
   }
   
/* DP: this is an old piece of code. Not sure whether we need it.
   for (i = query.edgeBegin(); i < query.edgeEnd(); i = query.edgeNext(i))
   {
      const Edge &edge = query.getEdge(i);
      const QueryBond &query_bond = query.getQueryBond(i);

      if (query.cis_trans.getParity(i) != 0)
      {
         const Atom &beg = query.getAtom(edge.beg);
         const Atom &end = query.getAtom(edge.end);

         bool beg_lost_h = ((beg.label == ELEM_C || beg.label == ELEM_Si) && beg.charge == 0 && beg.radical == 0) ||
                            (beg.label == ELEM_N && beg.charge == 1 && beg.radical == 0);
         bool end_lost_h = ((end.label == ELEM_C || end.label == ELEM_Si) && end.charge == 0 && end.radical == 0) ||
                            (end.label == ELEM_N && end.charge == 1 && end.radical == 0);

         if (query.getVertex(edge.beg).degree() < 3 && beg_lost_h)
            counters[mapping == 0 ? edge.beg : mapping[edge.beg]]++;
         if (query.getVertex(edge.end).degree() < 3 && end_lost_h)
            counters[mapping == 0 ? edge.end : mapping[edge.end]]++;
      }
   } */

   _min_h.expandFill(idx + 1, -1);
   _min_h[idx] = min_h;
   return min_h;
}

int QueryMolecule::_getAtomMinH (QueryMolecule::Atom *atom)
{
   if (atom->type == ATOM_TOTAL_H)
      return atom->value_min;

   if (atom->type == OP_AND)
   {
      int i;

      for (i = 0; i < atom->children.size(); i++)
      {
         int h = _getAtomMinH((Atom *)atom->children[i]);

         if (h >= 0)
            return h;
      }
   }

   return -1;
}

int QueryMolecule::getAtomTotalH (int idx)
{
   int value;

   if (_atoms[idx]->sureValue(ATOM_TOTAL_H, value))
      return value;

   int minh = getAtomMinH(idx);
   int maxh = getAtomMaxH(idx);

   if (minh == maxh)
      return minh;

   return -1;
}

int QueryMolecule::_calcAtomConnectivity (int idx)
{
   const Vertex &vertex = getVertex(idx);
   int i, conn = 0;

   for (i = vertex.neiBegin(); i != vertex.neiEnd(); i = vertex.neiNext(i))
   {
      int order = getBondOrder(vertex.neiEdge(i));

      if (order == BOND_SINGLE || order == BOND_DOUBLE || order == BOND_TRIPLE)
         conn += order;
      else
         conn += 1;
   }

   return conn;
}

bool QueryMolecule::isRSite (int atom_idx)
{
   int bits;

   return _atoms[atom_idx]->sureValue(ATOM_RSITE, bits);
}

dword QueryMolecule::getRSiteBits (int atom_idx)
{
   int bits;

   if (!_atoms[atom_idx]->sureValue(ATOM_RSITE, bits))
      throw Error("getRSiteBits(): atom #%d is not an r-site", atom_idx);

   return (dword)bits;
}

void QueryMolecule::allowRGroupOnRSite (int atom_idx, int rg_idx)
{
   if (rg_idx < 1 || rg_idx > 32)
      throw Error("allowRGroupOnRSite(): rgroup number %d is invalid", rg_idx);

   rg_idx--;

   // This is dirty hack; however, it is legal here, as rgroups
   // can not be present in deep query trees due to Molfile 
   // format limitations. 

   if (_atoms[atom_idx]->type == ATOM_RSITE)
   {
      _atoms[atom_idx]->value_max |= (1 << rg_idx);
      _atoms[atom_idx]->value_min |= (1 << rg_idx);
      return;
   }

   if (_atoms[atom_idx]->type == OP_AND)
   {
      int i;

      for (i = 0; i < _atoms[atom_idx]->children.size(); i++)
         if (_atoms[atom_idx]->children[i]->type == ATOM_RSITE)
         {
            ((Atom *)_atoms[atom_idx]->children[i])->value_max |= (1 << rg_idx);
            ((Atom *)_atoms[atom_idx]->children[i])->value_min |= (1 << rg_idx);
         }
   }

   throw Error("allowRGroupOnRSite(): atom #%d does not seem to be an r-site", atom_idx);
}

bool QueryMolecule::isKnownAttr (QueryMolecule::Atom& qa)
{
   return
      (qa.type == QueryMolecule::ATOM_CHARGE || 
      qa.type == QueryMolecule::ATOM_ISOTOPE || 
      qa.type == QueryMolecule::ATOM_RADICAL ||
      qa.type == QueryMolecule::ATOM_VALENCE ||
      qa.type == QueryMolecule::ATOM_TOTAL_H ||
      qa.type == QueryMolecule::ATOM_SUBSTITUENTS ||
      qa.type == QueryMolecule::ATOM_SUBSTITUENTS_AS_DRAWN ||
      qa.type == QueryMolecule::ATOM_RING_BONDS || 
      qa.type == QueryMolecule::ATOM_RING_BONDS_AS_DRAWN ||
      qa.type == QueryMolecule::ATOM_UNSATURATION) && 
      qa.value_max == qa.value_min;
}

bool QueryMolecule::isNotAtom (QueryMolecule::Atom& qa, int elem) {
   if (qa.type == QueryMolecule::OP_NOT) {
      QueryMolecule::Atom& c = *qa.child(0);
      if (c.type == QueryMolecule::ATOM_NUMBER && c.value_min == elem && c.value_max == elem)
         return true;
   }
   return false;
}

bool QueryMolecule::collectAtomList (QueryMolecule::Atom& qa, Array<int>& list, bool& notList) {
   list.clear();
   if (qa.type == QueryMolecule::OP_OR || qa.type == QueryMolecule::OP_NOT) {
      notList = (qa.type == QueryMolecule::OP_NOT);
      QueryMolecule::Atom* qap = &qa;
      if (notList) {
         qap = qa.child(0);
         if (qap->type != QueryMolecule::OP_OR)
            return false;
      }
      for (int i = 0; i < qap->children.size(); ++i) {
         QueryMolecule::Atom& qc = *qap->child(i);
         if (qc.type != QueryMolecule::ATOM_NUMBER || qc.value_min != qc.value_max)
            return false;
         list.push(qc.value_min);
      }
   } else if (qa.type == QueryMolecule::OP_AND) {
      notList = true;
      for (int i = 0; i < qa.children.size(); ++i) {
         QueryMolecule::Atom& qc = *qa.child(i);
         if (qc.type != QueryMolecule::OP_NOT)
            return false;
         QueryMolecule::Atom& qd = *qc.child(0);
         if (qd.type != QueryMolecule::ATOM_NUMBER || qd.value_min != qd.value_max)
            return false;
         list.push(qd.value_min);
      }
   }
   return true;
}

QueryMolecule::Atom* QueryMolecule::stripKnownAttrs (QueryMolecule::Atom& qa) {
   QueryMolecule::Atom* qd = NULL;
   if (qa.type == QueryMolecule::OP_AND) {
      for (int i = 0; i < qa.children.size(); ++i) {
         QueryMolecule::Atom* qc = qa.child(i);
         if (!isKnownAttr(*qc)) {
            if (qd != NULL) 
               return NULL;
            qd = qc;
         }
      }
   }
   return qd == NULL ? &qa : qd;
}

int QueryMolecule::parseQueryAtom (QueryMolecule& qm, int aid, Array<int>& list) {
   QueryMolecule::Atom& qa = qm.getAtom(aid);
   QueryMolecule::Atom* qc = stripKnownAttrs(qa);
   if (qc != NULL && isNotAtom(*qc, ELEM_H))
      return QUERY_ATOM_A;
   bool notList = false;
   if (collectAtomList(qa, list, notList) || 
      (qa.type == QueryMolecule::OP_NOT && collectAtomList(*qa.child(0), list, notList) && !notList)) { // !notList is to check there's no double negation
      if (list.size() == 0)
         return -1;
      notList = notList || qa.type == QueryMolecule::OP_NOT;
      if (!notList && list.size() == 5 && list[0] == ELEM_F && list[1] == ELEM_Cl && list[2] == ELEM_Br && list[3] == ELEM_I && list[4] == ELEM_At)
         return QUERY_ATOM_X;
      if (notList && list.size() == 2 && ((list[0] == ELEM_C && list[1] == ELEM_H) || (list[0] == ELEM_H && list[1] == ELEM_C)))
         return QUERY_ATOM_Q;
      return notList ? QUERY_ATOM_NOTLIST : QUERY_ATOM_LIST;
   }
   return -1;
}

bool QueryMolecule::queryAtomIsRegular (QueryMolecule& qm, int aid) {
   QueryMolecule::Atom& qa = qm.getAtom(aid);
   QueryMolecule::Atom* qc = stripKnownAttrs(qa);
   return qc && qc->type == QueryMolecule::ATOM_NUMBER;
}

QueryMolecule::Bond* QueryMolecule::getBondOrderTerm (QueryMolecule::Bond& qb, bool& complex)
{
   if (qb.type == QueryMolecule::OP_AND) {
      QueryMolecule::Bond* r = NULL;
      for (int i = 0; i < qb.children.size(); ++i) {
         QueryMolecule::Bond* c = getBondOrderTerm(*qb.child(i), complex);
         if (complex)
            return NULL;
         if (c != NULL) {
            if (r != NULL) {
               complex = true;
               return NULL;
            }
            r = c;
         }
      }
      return r;
   }

   if (qb.type == QueryMolecule::BOND_TOPOLOGY)
      return NULL;
   return &qb;
}

bool QueryMolecule::isOrBond (QueryMolecule::Bond& qb, int type1, int type2)
{
   if ((qb.type == OP_AND || qb.type == OP_OR) && qb.children.size() == 1)
      return isOrBond(*qb.child(0), type1, type2);

   if (qb.type != QueryMolecule::OP_OR || qb.children.size() != 2)
      return false;
   QueryMolecule::Bond& b1 = *qb.child(0);
   QueryMolecule::Bond& b2 = *qb.child(1);
   if (b1.type != QueryMolecule::BOND_ORDER || b2.type != QueryMolecule::BOND_ORDER)
      return false;
   if ((b1.value == type1 && b2.value == type2) || (b1.value == type2 && b2.value == type1))
      return true;
   return false;
}

bool QueryMolecule::isSingleOrDouble (QueryMolecule::Bond& qb)
{
   if ((qb.type == OP_AND || qb.type == OP_OR) && qb.children.size() == 1)
      return isSingleOrDouble(*qb.child(0));

   if (qb.type != QueryMolecule::OP_AND || qb.children.size() != 2)
      return false;
   QueryMolecule::Bond& b1 = *qb.child(0);
   QueryMolecule::Bond& b2 = *qb.child(1);
   if (!isOrBond(b2, BOND_SINGLE, BOND_DOUBLE))
      return false;
   if (b1.type != QueryMolecule::OP_NOT)
      return false;
   QueryMolecule::Bond& b11 = *b1.child(0);
   if (b11.type != QueryMolecule::BOND_ORDER || b11.value != BOND_AROMATIC)
      return false;
   return true;
}

int QueryMolecule::getQueryBondType (QueryMolecule::Bond& qb)
{
   if (!qb.hasConstraint(QueryMolecule::BOND_ORDER))
      return QUERY_BOND_ANY;

   QueryMolecule::Bond* qb2 = &qb;
   AutoPtr<QueryMolecule::Bond> qb_modified;
   int topology;
   if (qb.sureValue(QueryMolecule::BOND_TOPOLOGY, topology))
   {
      qb_modified.reset(qb.clone());
      qb_modified->removeConstraints(QueryMolecule::BOND_TOPOLOGY);
      qb2 = qb_modified.get();
   }

   if (isSingleOrDouble(*qb2) || isOrBond(*qb2, BOND_SINGLE, BOND_DOUBLE))
         return QUERY_BOND_SINGLE_OR_DOUBLE;
   if (isOrBond(*qb2, BOND_SINGLE, BOND_AROMATIC))
         return QUERY_BOND_SINGLE_OR_AROMATIC;
   if (isOrBond(*qb2, BOND_DOUBLE, BOND_AROMATIC))
         return QUERY_BOND_DOUBLE_OR_AROMATIC;
   return -1;
}

void QueryMolecule::invalidateAtom (int index, int mask)
{
   BaseMolecule::invalidateAtom(index, mask);
   if (_min_h.size() > index)
      _min_h[index] = -1;
}

void QueryMolecule::optimize ()
{
   for (int i = vertexBegin(); i != vertexEnd(); i = vertexNext(i))
      getAtom(i).optimize();
   updateEditRevision();
}
