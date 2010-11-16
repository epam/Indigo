/****************************************************************************
 * Copyright (C) 2009-2010 GGA Software Services LLC
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

#include "base_c/defs.h"

#include "molecule/molecule.h"
#include "base_cpp/output.h"
#include "molecule/elements.h"
#include "molecule/molecule_arom.h"
#include "molecule/molecule_dearom.h"

using namespace indigo;

Molecule::Molecule ()
{
}

Molecule::~Molecule ()
{
}

Molecule& Molecule::asMolecule ()
{
   return *this;
}

void Molecule::clear ()
{
   BaseMolecule::clear();

   _pseudo_atom_values.clear();
   _atoms.clear();
   _bond_orders.clear();
   _connectivity.clear();
   _aromaticity.clear();
   _implicit_h.clear();
   _total_h.clear();
   _valence.clear();
   _radicals.clear();
}

void Molecule::_flipBond (int atom_parent, int atom_from, int atom_to)
{
   int src_bond_idx = findEdgeIndex(atom_parent, atom_from);
   int bond_order = getBondOrder(src_bond_idx);

   addBond(atom_parent, atom_to, bond_order);
}

void Molecule::_mergeWithSubmolecule (BaseMolecule &bmol, const Array<int> &vertices,
           const Array<int> *edges, const Array<int> &mapping, int skip_flags)
{
   Molecule &mol = bmol.asMolecule();
   int i;

   // atoms and pseudo-atoms and connectivities and implicit H counters
   for (i = 0; i < vertices.size(); i++)
   {
      int newidx = mapping[vertices[i]];

      _atoms.expand(newidx + 1);
      _atoms[newidx] = mol._atoms[vertices[i]];

      if (mol.isPseudoAtom(vertices[i]))
         setPseudoAtom(newidx, mol.getPseudoAtom(vertices[i]));

      // DPX: TODO: check that all the neighbors are mapped
      // prior to translating the connectivity, valence, radicals,
      // and implicit H counters
      bool nei_mapped = 
         (getVertex(newidx).degree() == mol.getVertex(vertices[i]).degree());

      if (mol._connectivity.size() > vertices[i])
      {
         _connectivity.expandFill(newidx + 1, -1);
         if (nei_mapped)
            _connectivity[newidx] = mol._connectivity[vertices[i]];
      }

      if (mol._atoms[vertices[i]].explicit_valence && mol._valence.size() > vertices[i])
      {
         _valence.expandFill(newidx + 1, -1);
         if (nei_mapped)
            _valence[newidx] = mol._valence[vertices[i]];
      }
      if (mol._implicit_h.size() > vertices[i])
      {
         _implicit_h.expandFill(newidx + 1, -1);
         if (nei_mapped)
            _implicit_h[newidx] = mol._implicit_h[vertices[i]];
      }
      if (mol._radicals.size() > vertices[i])
      {
         _radicals.expandFill(newidx + 1, -1);
         if (nei_mapped)
            _radicals[newidx] = mol._radicals[vertices[i]];
      }
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

         _bond_orders.expand(idx + 1);
         _bond_orders[idx] = mol._bond_orders[edges->at(i)];
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

         _bond_orders.expand(idx + 1);
         _bond_orders[idx] = mol._bond_orders[i];
      }
}

/*
void Molecule::setQueryBondAromatic (int idx)
{
   setBondAromatic(idx);
   _q_bonds->at(idx).type = 0;
   _q_bonds->at(idx).can_be_aromatic = false;
}

void Molecule::setQueryBondFuzzyAromatic (int idx)
{
   _q_bonds->at(idx).can_be_aromatic = true;
}

*/

void Molecule::_validateVertexConnectivity (int idx, bool validate)
{
   if (validate)
   {
      getAtomConnectivity_noImplH(idx);
      getImplicitH(idx);
      getAtomValence(idx);
   }
   else
   {
      if (_connectivity.size() > idx)
         _connectivity[idx] = -1;
      if (_implicit_h.size() > idx && !_atoms[idx].explicit_impl_h)
         _implicit_h[idx] = -1;
      if (_total_h.size() > idx)
         _total_h[idx] = -1;
      if (_valence.size() > idx && !_atoms[idx].explicit_valence)
         _valence[idx] = -1;
   }
}

void Molecule::setBondOrder (int idx, int order, bool keep_connectivity)
{
   const Edge &edge = getEdge(idx);

   //if (_atoms[edge.beg].explicit_valence || _atoms[edge.end].explicit_valence)
   //   throw Error("setBondOrder(): explicit valence on atom");

   //if (_atoms[edge.beg].explicit_impl_h || _atoms[edge.end].explicit_impl_h)
   //   throw Error("setBondOrder(): explicit H count on atom");

   if (keep_connectivity && _bond_orders[idx] != BOND_AROMATIC && order != BOND_AROMATIC)
      throw Error("setBondOrder(): keep_connectivity must be used only with aromatic bonds");

   // If connectivity should be kept then calculate connectivity and 
   // all dependent constants (valence, implicit hydrogens) 
   _validateVertexConnectivity(edge.beg, keep_connectivity);
   _validateVertexConnectivity(edge.end, keep_connectivity);

   if (_bond_orders[idx] == BOND_AROMATIC || order == BOND_AROMATIC)
      _aromaticity.clear();

   _bond_orders[idx] = order;

   if (order != BOND_DOUBLE)
      cis_trans.setParity(idx, 0);
}

void Molecule::setBondOrder_Silent (int idx, int order)
{
   _bond_orders[idx] = order;
}

void Molecule::setAtomCharge (int idx, int charge)
{
   _atoms[idx].charge = charge;

   _validateVertexConnectivity(idx, false);
}

void Molecule::setAtomIsotope (int idx, int isotope)
{
   _atoms[idx].isotope = isotope;
}

void Molecule::setAtomRadical (int idx, int radical)
{
   _radicals.expandFill(idx + 1, -1);
   _radicals[idx] = radical;

   _validateVertexConnectivity(idx, false);
}

void Molecule::setExplicitValence (int idx, int valence)
{
   _valence.expandFill(idx + 1, -1);
   _valence[idx] = valence;
   _atoms[idx].explicit_valence = true;

   // _validateVertexConnectivity(idx, false);
}

void Molecule::setValence (int idx, int valence)
{
   _valence.expandFill(idx + 1, -1);
   _valence[idx] = valence;

   //_validateVertexConnectivity(idx, false);
}

void Molecule::setImplicitH (int idx, int impl_h)
{
   _implicit_h.expandFill(idx + 1, -1);
   _implicit_h[idx] = impl_h;
   _atoms[idx].explicit_impl_h = true;

   _validateVertexConnectivity(idx, false);
}

void Molecule::resetImplicitH (int idx)
{
   _atoms[idx].explicit_impl_h = false;
   _validateVertexConnectivity(idx, false);
}

void Molecule::setPseudoAtom (int idx, const char *text)
{
   _atoms[idx].number = ELEM_PSEUDO;
   _atoms[idx].pseudoatom_value_idx = _pseudo_atom_values.add(text);
   // TODO: take care of memory allocated here in _pseudo_atom_values
}

int Molecule::getVacantPiOrbitals (int atom_idx, int conn, int *lonepairs_out)
{
   int group = Element::group(getAtomNumber(atom_idx));
   int charge = getAtomCharge(atom_idx);
   int radical = getAtomRadical(atom_idx);

   return BaseMolecule::getVacantPiOrbitals(group, charge, radical, conn, lonepairs_out);
}

int Molecule::getVacantPiOrbitals (int atom_idx, int *lonepairs_out)
{
   return getVacantPiOrbitals(atom_idx, getAtomConnectivity(atom_idx), lonepairs_out);
}

int Molecule::getAtomConnectivity (int idx)
{
   int conn = getAtomConnectivity_noImplH(idx);

   if (conn < 0)
      return -1;

   int impl_h = getImplicitH(idx);

   if (impl_h < 0)
      return -1;

   return impl_h + conn;
}

int Molecule::getAtomConnectivity_noImplH (int idx)
{
   if (_connectivity.size() > idx && _connectivity[idx] >= 0)
      return _connectivity[idx];

   int conn = calcAtomConnectivity_noImplH(idx);

   _connectivity.expandFill(idx + 1, -1);
   _connectivity[idx] = conn;
   return conn;
}

int Molecule::calcAtomConnectivity_noImplH (int idx)
{
   const Vertex &vertex = getVertex(idx);
   int i, conn = 0;

   for (i = vertex.neiBegin(); i != vertex.neiEnd(); i = vertex.neiNext(i))
   {
      int order = getBondOrder(vertex.neiEdge(i));

      if (order == BOND_AROMATIC)
         return -1;
      // not checking order == -1 because it is not QueryMolecule

      conn += order;
   }

   return conn;
}

int Molecule::totalHydrogensCount ()
{
   int i, total_h = 0;

   for (i = vertexBegin(); i < vertexEnd(); i = vertexNext(i))
   {
      if (getAtomNumber(i) == ELEM_H)
         total_h++;

      total_h += getImplicitH(i);
   }

   return total_h;
}

int Molecule::matchAtomsCmp (Graph &g1, Graph &g2,
                             int idx1, int idx2, void *userdata)
{
   Molecule &m1 = ((BaseMolecule &)g1).asMolecule();
   Molecule &m2 = ((BaseMolecule &)g2).asMolecule();

   if (m1.isPseudoAtom(idx1) && !m2.isPseudoAtom(idx2))
      return 1;

   if (!m1.isPseudoAtom(idx1) && m2.isPseudoAtom(idx2))
      return -1;

   bool pseudo = false;

   if (m1.isPseudoAtom(idx1) && m2.isPseudoAtom(idx2))
   {
      int res = strcmp(m1.getPseudoAtom(idx1), m2.getPseudoAtom(idx2));

      if (res != 0)
         return res;
      pseudo = true;
   }
   else
   {
      if (m1.getAtomNumber(idx1) > m2.getAtomNumber(idx2))
         return 1;
      if (m1.getAtomNumber(idx1) < m2.getAtomNumber(idx2))
         return -1;
   }

   if (m1.getAtomIsotope(idx1) > m2.getAtomIsotope(idx2))
      return 1;
   if (m1.getAtomIsotope(idx1) < m2.getAtomIsotope(idx2))
      return -1;

   if (m1.getAtomCharge(idx1) > m2.getAtomCharge(idx2))
      return 1;
   if (m1.getAtomCharge(idx1) < m2.getAtomCharge(idx2))
      return -1;

   if (!pseudo && m1.getAtomValence(idx1) > m2.getAtomValence(idx2))
      return 1;
   if (!pseudo && m1.getAtomValence(idx1) < m2.getAtomValence(idx2))
      return -1;

   if (!pseudo && m1.getAtomRadical(idx1) > m2.getAtomRadical(idx2))
      return 1;
   if (!pseudo && m1.getAtomRadical(idx1) < m2.getAtomRadical(idx2))
      return -1;

   return 0;
}

int Molecule::getAtomAromaticity (int idx)
{
   if (_aromaticity.size() > idx && _aromaticity[idx] >= 0)
      return _aromaticity[idx];

   const Vertex &vertex = getVertex(idx);
   int i;

   for (i = vertex.neiBegin(); i != vertex.neiEnd(); i = vertex.neiNext(i))
   {
      int order = getBondOrder(vertex.neiEdge(i));

      if (order == BOND_AROMATIC)
      {
         _aromaticity.expandFill(idx + 1, -1);
         _aromaticity[idx] = ATOM_AROMATIC;
         return ATOM_AROMATIC;
      }
         
      // not checking order == -1 because it is not QueryMolecule
   }
   _aromaticity.expandFill(idx + 1, -1);
   _aromaticity[idx] = ATOM_ALIPHATIC;
   return ATOM_ALIPHATIC;
}

void Molecule::_removeAtoms (const Array<int> &indices, const int *mapping)
{
   int i;

   for (i = 0; i < indices.size(); i++)
   {
      int idx = indices[i];
      const Vertex &vertex = getVertex(idx);

      if (_atoms[idx].number == ELEM_H && _atoms[idx].isotope == 0 && vertex.degree() == 1)
      {
         int nidx = vertex.neiVertex(vertex.neiBegin());

         if (_implicit_h.size() > nidx && _implicit_h[nidx] >= 0)
            _implicit_h[nidx]++;
      }
   }
}

void Molecule::unfoldHydrogens (Array<int> *markers_out, int max_h_cnt )
{
   int i, j, v_end = vertexEnd();

   if (markers_out != 0)
   {
      markers_out->clear_resize(vertexEnd());
      markers_out->zerofill();
   }

   for (i = vertexBegin(); i < v_end; i = vertexNext(i))
   {
      if (isPseudoAtom(i) || isRSite(i))
         continue;

      int impl_h = getImplicitH(i);
      int h_cnt;
      if ((max_h_cnt == -1) || (max_h_cnt > impl_h))
         h_cnt = impl_h;
      else
         h_cnt = max_h_cnt;

      for (j = 0; j < h_cnt; j++)
      {
         int new_h_idx = addAtom(ELEM_H);

         addBond(i, new_h_idx, BOND_SINGLE);
         if (markers_out != 0)
         {
            markers_out->expandFill(new_h_idx + 1, 0);
            markers_out->at(new_h_idx) = 1;
         }

         stereocenters.registerUnfoldedHydrogen(i, new_h_idx);
      }

      _validateVertexConnectivity(i, false);
      _implicit_h[i] = impl_h - h_cnt;
   }
}

int Molecule::getImplicitH (int idx)
{
   if (_atoms[idx].number == ELEM_PSEUDO)
      throw Error("getImplicitH() does not work on pseudo-atoms");

   if (_atoms[idx].number == ELEM_RSITE)
      throw Error("getImplicitH() does not work on R-sites");

   if (_implicit_h.size() > idx && _implicit_h[idx] >= 0)
      return _implicit_h[idx];

   const _Atom &atom = _atoms[idx];

   int conn = getAtomConnectivity_noImplH(idx);
   int radical = 0;

   if (_radicals.size() > idx && _radicals[idx] >= 0)
      radical = _radicals[idx];
   
   if (conn < 0)
      return -1;

   int implicit_h;

   if (atom.explicit_valence)
   {
      // Explicit valence means that the molecule was converted from Molfile.
      // Conventions are that if we have explicit valence, we discard radical
      // and charge when calculating implicit hydgogens.
      implicit_h = _valence[idx] - Element::calcValenceMinusHyd(atom.number, 0, 0, conn);

      if (implicit_h < 0)
         throw Error("valence %d specified on %s, charge %d, radical %d, but %d bonds are drawn",
                    _valence[idx], Element::toString(atom.number), atom.charge, radical, conn);
   }
   else
   {
      int valence;
      Element::calcValence(atom.number, atom.charge, radical,
                           conn, valence, implicit_h, true);
      _valence.expandFill(idx + 1, -1);
      _valence[idx] = valence;
   }

   _implicit_h.expandFill(idx + 1, -1);
   _implicit_h[idx] = implicit_h;

   return implicit_h;
}

int Molecule::getAtomNumber (int idx)
{
   return _atoms[idx].number;
}

int Molecule::getAtomCharge (int idx)
{
   return _atoms[idx].charge;
}

int Molecule::getAtomIsotope (int idx)
{
   return _atoms[idx].isotope;
}

int Molecule::getAtomValence (int idx)
{
   if (_valence.size() > idx && _valence[idx] >= 0)
      return _valence[idx];

   const _Atom &atom = _atoms[idx];

   {
      int val = Element::calcValenceByCharge(atom.number, atom.charge);

      if (val > 0)
      {
         _valence.expandFill(idx + 1, -1);
         _valence[idx] = val;
         return val;
      }
   }

   int impl_h = getImplicitH(idx);
   int radical = 0;
   int conn = getAtomConnectivity_noImplH(idx);

   if (conn < 0)
      return -1;

   if (_radicals.size() > idx && _radicals[idx] >= 0)
      radical = _radicals[idx];

   int normal_val, normal_hyd;

   Element::calcValence(atom.number, atom.charge, radical, conn, normal_val, normal_hyd, true);

   if (impl_h != normal_hyd && radical == 0)
   {
      // try to put additional radicals

      // first try a singlet radical
      if (Element::calcValence(atom.number, atom.charge, RADICAL_SINGLET,
                               conn, normal_val, normal_hyd, false) &&
                               impl_h == normal_hyd)
      {
         _radicals.expandFill(idx + 1, -1);
         _radicals[idx] = RADICAL_SINGLET;
      }
      else
      {
         // try a douplet radical
         if (Element::calcValence(atom.number, atom.charge, RADICAL_DOUPLET,
                                  conn, normal_val, normal_hyd, false) &&
                                  impl_h == normal_hyd)
         {
            _radicals.expandFill(idx + 1, -1);
            _radicals[idx] = RADICAL_DOUPLET;
         }
      }
   }

   if (impl_h == normal_hyd)
   {
      _valence.expandFill(idx + 1, -1);
      _valence[idx] = normal_val;
      return normal_val;
   }
   else
   {
      // radicals have not helped, calculate 'explicit' valence
      int val = Element::calcValenceMinusHyd(atom.number, atom.charge, radical, conn) + impl_h;

      _valence.expandFill(idx + 1, -1);
      _valence[idx] = val;
      return val;
   }
}

int Molecule::getAtomRadical (int idx)
{
   if (_radicals.size() > idx && _radicals[idx] >= 0)
      return _radicals[idx];

   const _Atom &atom = _atoms[idx];
   int conn = getAtomConnectivity_noImplH(idx);
   int impl_h = getImplicitH(idx);
   int normal_val, normal_hyd;

   Element::calcValence(atom.number, atom.charge, 0, conn, normal_val, normal_hyd, true);

   if (impl_h != normal_hyd)
   {
      // first try a singlet radical
      if (Element::calcValence(atom.number, atom.charge, RADICAL_SINGLET,
                               conn, normal_val, normal_hyd, false) &&
                               impl_h == normal_hyd)
      {
         _radicals.expandFill(idx + 1, -1);
         _radicals[idx] = RADICAL_SINGLET;
      }
      else
      {
         // try a douplet radical
         if (Element::calcValence(atom.number, atom.charge, RADICAL_DOUPLET,
                                  conn, normal_val, normal_hyd, false) &&
                                  impl_h == normal_hyd)
         {
            _radicals.expandFill(idx + 1, -1);
            _radicals[idx] = RADICAL_DOUPLET;
         }
      }
   }

   if (_radicals.size() > idx && _radicals[idx] >= 0)
      return _radicals[idx];

   return 0;
}

void Molecule::saveBondOrders (Molecule &mol, Array<int> &orders)
{
   orders.copy(mol._bond_orders);
}

void Molecule::loadBondOrders (Molecule &mol, Array<int> &orders)
{
   mol._bond_orders.copy(orders);
}

int Molecule::getAtomSubstCount (int idx)
{
   int i, res = 0;
   const Vertex &vertex = getVertex(idx);

   for (i = vertex.neiBegin(); i != vertex.neiEnd(); i = vertex.neiNext(i))
   {
      if (_atoms[vertex.neiVertex(i)].number != ELEM_H)
         res++;
   }

   return res;
}

int Molecule::getAtomRingBondsCount (int idx)
{
   int i, res = 0;
   const Vertex &vertex = getVertex(idx);

   for (i = vertex.neiBegin(); i != vertex.neiEnd(); i = vertex.neiNext(i))
   {
      if (getEdgeTopology(vertex.neiEdge(i)) == TOPOLOGY_RING)
         res++;
   }

   return res;
}

bool Molecule::isSaturatedAtom (int idx)
{
   int i;
   const Vertex &vertex = getVertex(idx);

   for (i = vertex.neiBegin(); i != vertex.neiEnd(); i = vertex.neiNext(i))
      if (getBondOrder(vertex.neiEdge(i)) != BOND_SINGLE)
         return false;

   return true;
}

int Molecule::getBondOrder (int idx)
{
   return _bond_orders[idx];
}

int Molecule::getBondTopology (int idx)
{
   return getEdgeTopology(idx);
}

int Molecule::getExplicitValence (int idx)
{
   if (_atoms[idx].explicit_valence)
      return _valence[idx];
   return -1;
}

bool Molecule::atomNumberBelongs (int idx, const int *numbers, int count)
{
   int number = _atoms[idx].number;
   int i;

   for (i = 0; i < count; i++)
      if (number == numbers[i])
         return true;

   return false;
}

bool Molecule::possibleAtomNumber (int idx, int number)
{
   return _atoms[idx].number == number;
}

bool Molecule::possibleAtomNumberAndCharge (int idx, int number, int charge)
{
   return _atoms[idx].number == number && _atoms[idx].charge == charge;
}

bool Molecule::possibleAtomNumberAndIsotope (int idx, int number, int isotope)
{
   return _atoms[idx].number == number && _atoms[idx].isotope == isotope;
}

bool Molecule::possibleAtomIsotope (int idx, int isotope)
{
   return _atoms[idx].isotope == isotope;
}

bool Molecule::possibleAtomCharge (int idx, int charge)
{
   return _atoms[idx].charge == charge;
}

void Molecule::getAtomDescription (int idx, Array<char> &description)
{
   _Atom &atom = _atoms[idx];
   ArrayOutput output(description);

   if (atom.isotope != 0)
      output.printf("%d", atom.isotope);

   if (isPseudoAtom(idx))
      output.printf("%s", getPseudoAtom(idx));
   else
      output.printf("%s", Element::toString(atom.number));

   if (atom.charge == -1)
      output.printf("-");
   else if (atom.charge == 1)
      output.printf("+");
   else if (atom.charge > 0)
      output.printf("+%d", atom.charge);
   else if (atom.charge < 0)
      output.printf("-%d", -atom.charge);

   output.writeChar(0);
}

void Molecule::getBondDescription (int idx, Array<char> &description)
{
   ArrayOutput output(description);

   switch (_bond_orders[idx])
   {
      case BOND_SINGLE: output.printf("single"); return;
      case BOND_DOUBLE: output.printf("double"); return;
      case BOND_TRIPLE: output.printf("triple"); return;
      case BOND_AROMATIC: output.printf("aromatic"); return;
   }
}

bool Molecule::possibleBondOrder (int idx, int order)
{
   return _bond_orders[idx] == order;
}

int Molecule::addAtom (int number)
{
   int idx = _addBaseAtom();

   _atoms.expand(idx + 1);
   memset(&_atoms[idx], 0, sizeof(_Atom));
   _atoms[idx].number = number;
   return idx;
}

int Molecule::addBond (int beg, int end, int order)
{
   int idx = _addBaseBond(beg, end);

   _bond_orders.expand(idx + 1);
   _bond_orders[idx] = order;

   _aromaticity.clear();

   return idx;
}

bool Molecule::isPseudoAtom (int idx)
{
   return _atoms[idx].number == ELEM_PSEUDO;
}

const char * Molecule::getPseudoAtom (int idx)
{
   const _Atom &atom = _atoms[idx];
   
   if (atom.number != ELEM_PSEUDO)
      throw Error("getPseudoAtom(): atom #%d is not a pseudoatom", idx);

   const char *res = _pseudo_atom_values.at(atom.pseudoatom_value_idx);

   if (res == 0)
      throw Error("pseudoatom string is zero");

   return res;
}

BaseMolecule * Molecule::neu ()
{
   return new Molecule();
}

bool Molecule::bondStereoCare (int idx)
{
   if (!cis_trans.exists())
      return false;
   // In ordinary molecule all bond's stereoconfigurations are important
   return cis_trans.getParity(idx) != 0;
}

void Molecule::aromatize ()
{
   MoleculeAromatizer::aromatizeBonds(*this);
}

void Molecule::dearomatize ()
{
   MoleculeDearomatizer::dearomatizeMolecule(*this);
}

int Molecule::getAtomMaxH (int idx)
{
   return getAtomTotalH(idx);
}

int Molecule::getAtomMinH (int idx)
{
   return getAtomTotalH(idx);
}

int Molecule::getAtomTotalH (int idx)
{
   if (_total_h.size() > idx && _total_h[idx] >= 0)
      return _total_h[idx];

   int i, h = getImplicitH(idx);

   if (h == -1)
      throw Error("can not calculate implicit hydrogens on atom #%d", idx);

   const Vertex &vertex = getVertex(idx);

   for (i = vertex.neiBegin(); i != vertex.neiEnd(); i = vertex.neiNext(i))
      if (getAtomNumber(vertex.neiVertex(i)) == ELEM_H)
         h++;

   _total_h.expandFill(idx + 1, -1);
   _total_h[idx] = h;

   return h;
}

bool Molecule::isRSite (int atom_idx)
{
   return _atoms[atom_idx].number == ELEM_RSITE;
}

int Molecule::getRSiteBits (int atom_idx)
{
   if (_atoms[atom_idx].number != ELEM_RSITE)
      throw Error("getRSiteBits(): atom #%d is not an r-site", atom_idx);
   return _atoms[atom_idx].rgroup_bits;
}

void Molecule::setRSiteBits (int atom_idx, int bits)
{
   if (_atoms[atom_idx].number != ELEM_RSITE)
      throw Error("setRSiteBits(): atom #%d is not an r-site", atom_idx);
   _atoms[atom_idx].rgroup_bits = bits;
}


void Molecule::allowRGroupOnRSite (int atom_idx, int rg_idx)
{
   if (_atoms[atom_idx].number != ELEM_RSITE)
      throw Error("allowRGroupOnRSite(): atom #%d is not an r-site", atom_idx);

   if (rg_idx < 1 || rg_idx > 32)
      throw Error("allowRGroupOnRSite(): rgroup number %d is invalid", rg_idx);

   rg_idx--;

   _atoms[atom_idx].rgroup_bits |= (1 << rg_idx);
}

bool Molecule::convertableToImplicitHydrogen (int idx)
{
   if (getAtomNumber(idx) == ELEM_H &&
       getAtomIsotope(idx) == 0 && getVertex(idx).degree() == 1)
   {
      int nei = getVertex(idx).neiVertex(getVertex(idx).neiBegin());
      if (getAtomNumber(nei) != ELEM_H || getAtomIsotope(nei) != 0)
      {
         if (stereocenters.getType(nei) > 0)
            if (getVertex(nei).degree() == 3)
               return false; // not ignoring hydrogens around stereocenters with lone pair

         return true;
      }
   }
   return false;
}

void Molecule::invalidateHCounters ()
{
   _implicit_h.clear();
   _total_h.clear();
   _connectivity.clear();
}

 void Molecule::checkForConsistency (Molecule &mol)
 {
    int i;

   for (i = mol.vertexBegin(); i < mol.vertexEnd(); i = mol.vertexNext(i))
   {
      const Vertex &vertex = mol.getVertex(i);

      if (mol.isPseudoAtom(i) || mol.isRSite(i))
         continue;

      // check that all explicit hydrogens are lone or 1-connected
      if (mol.getAtomNumber(i) == ELEM_H && vertex.degree() > 1)
         throw Error("%d-connected hydrogen atom", vertex.degree());

      // check if molecule was drawn with inconsistent aromatic bonds (or query bonds)
      if (mol.getImplicitH(i) == -1)
         throw Error("can not calculate implicit hydrogens on atom %d", i);
   }
}
