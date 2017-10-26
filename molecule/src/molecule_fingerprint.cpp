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

#include "base_c/bitarray.h"
#include "base_cpp/output.h"

#include "graph/graph_subtree_enumerator.h"
#include "graph/cycle_enumerator.h"
#include "graph/subgraph_hash.h"

#include "molecule/molecule.h"
#include "molecule/molecule_fingerprint.h"
#include "molecule/molecule_tautomer.h"
#include "molecule/elements.h"
#include "molecule/query_molecule.h"

using namespace indigo;

IMPL_ERROR(MoleculeFingerprintBuilder, "fingerprint builder");

CP_DEF(MoleculeFingerprintBuilder);

MoleculeFingerprintBuilder::MoleculeFingerprintBuilder (BaseMolecule &mol,
                     const MoleculeFingerprintParameters &parameters):
cancellation(0),
_mol(mol),
_parameters(parameters), 
CP_INIT,
TL_CP_GET(_total_fingerprint),
TL_CP_GET(_atom_codes),
TL_CP_GET(_bond_codes),
TL_CP_GET(_atom_codes_empty),
TL_CP_GET(_bond_codes_empty),
TL_CP_GET(_atom_hydrogens),
TL_CP_GET(_atom_charges),
TL_CP_GET(_vertex_connectivity),
TL_CP_GET(_fragment_vertex_degree),
TL_CP_GET(_bond_orders),
TL_CP_GET(_ord_hashes)
{
   _total_fingerprint.resize(_parameters.fingerprintSize());
   cb_fragment = 0;

   query = false;
   
   skip_ord = false;
   skip_sim = false;
   skip_tau = false;
   skip_ext = false;
   skip_ext_charge = false;

   skip_any_atoms = false;
   skip_any_bonds = false;
   skip_any_atoms_bonds = false;

   _ord_hashes.clear();
}

void MoleculeFingerprintBuilder::_initHashCalculations (BaseMolecule &mol, const Filter &vfilter)
{
   subgraph_hash.create(mol);

   _atom_codes.clear_resize(mol.vertexEnd());
   _atom_codes_empty.clear_resize(mol.vertexEnd());
   _bond_codes.clear_resize(mol.edgeEnd());
   _bond_codes_empty.clear_resize(mol.edgeEnd());
   for (int i : mol.vertices())
   {
      _atom_codes[i] = mol.atomCode(i);
      _atom_codes_empty[i] = 0;
   }
   for (int i : mol.edges())
   {
      _bond_codes[i] = mol.bondCode(i);
      _bond_codes_empty[i] = 0;
   }

   // Count number of hydrogens and find non-carbon atoms
   _atom_hydrogens.clear_resize(mol.vertexEnd());
   _atom_charges.clear_resize(mol.vertexEnd());
   for (int i : mol.vertices())
   {
      try
      {
         _atom_hydrogens[i] = mol.getAtomMinH(i);
      }
      catch (Exception e)
      {
         // Set number of hydrogens to zero if anything is undefined
         _atom_hydrogens[i] = 0;
      }
      
      int charge = mol.getAtomCharge(i);
      if (charge == CHARGE_UNKNOWN)
         charge = 0;
      _atom_charges[i] = charge;
   }

   // Calculate vertex connectivity for the original structure (not tau)
   _vertex_connectivity.clear_resize(mol.vertexEnd());
   _vertex_connectivity.zerofill();
   for (int e : mol.edges())
   {
      if (_tau_super_structure)
         if (_tau_super_structure->isZeroedBond(e))
            continue;

      const Edge &edge = mol.getEdge(e);   
      if (!vfilter.valid(edge.beg) || !vfilter.valid(edge.end))
         continue;

      _vertex_connectivity[edge.beg]++;
      _vertex_connectivity[edge.end]++;
   }

   if (query)
   {
      QueryMolecule &q = mol.asQueryMolecule();
      // For the query structure increase _vertex_ord_degree so that only saturated atoms 
      // has original degree, i.e. to distinguish [CH4], [CH3] from [C]
      // For [CH4] vectex connectivity is 0
      // For [CH3] vectex connectivity is 1 meaning that one external bond is allowed
      for (int v : mol.vertices())
      {
         if (!vfilter.valid(v))
            continue;

         int ext_conn = q.getAtomMaxExteralConnectivity(v);
         if (ext_conn == -1)
            _vertex_connectivity[v] = INT_MAX;
         else
            _vertex_connectivity[v] += ext_conn;
      }
   }

   _fragment_vertex_degree.clear_resize(mol.vertexEnd());

   _bond_orders.clear_resize(mol.edgeEnd());
   _bond_orders.zerofill();
   for (int e : mol.edges())
   {
      if (_tau_super_structure)
         if (_tau_super_structure->isZeroedBond(e))
            continue;
      int order = mol.getBondOrder(e);

      if (order == BOND_SINGLE || order == BOND_DOUBLE || order == BOND_TRIPLE)
         _bond_orders[e] = order;
      else
         _bond_orders[e] = 1;
   }
}

MoleculeFingerprintBuilder::~MoleculeFingerprintBuilder ()
{
}

void MoleculeFingerprintBuilder::process ()
{
   _total_fingerprint.zerofill();
   _makeFingerprint(_mol);
}
/*
 * Accepted types: 'sim', 'sub', 'sub-res', 'sub-tau', 'full'
 */
void MoleculeFingerprintBuilder::parseFingerprintType(const char *type, bool query) {
   this->query = query;

   if (type == 0 || *type == 0 || strcasecmp(type, "sim") == 0)
   {
      // similarity
      this->skip_tau = true;
      this->skip_ext = true;
      this->skip_ord = true;
      this->skip_any_atoms = true;
      this->skip_any_bonds = true;
      this->skip_any_atoms_bonds = true;
   }
   else if (strcasecmp(type, "chem") == 0)
   {
      // chemical similarity
      this->skip_tau = true;
      this->skip_ext = true;
      this->skip_ord = true;
      this->skip_any_atoms = true;
      this->skip_any_bonds = true;
      this->skip_any_atoms_bonds = true;
   }
   else if (strcasecmp(type, "sub") == 0)
   {
      // substructure
      this->skip_sim = true;
      this->skip_tau = true;
   }
   else if (strcasecmp(type, "sub-res") == 0)
   {
      // resonance substructure
      this->skip_sim = true;
      this->skip_tau = true;
      this->skip_ord = true;
      this->skip_any_atoms = true;
      this->skip_ext_charge = true;
   }
   else if (strcasecmp(type, "sub-tau") == 0)
   {
      // tautomer
      this->skip_ord = true;
      this->skip_sim = true;

      // tautomer fingerprint part does already contain all necessary any-bits
      this->skip_any_atoms = true;
      this->skip_any_bonds = true;
      this->skip_any_atoms_bonds = true;
   }
   else if (strcasecmp(type, "full") == 0)
   {
      if (query)
         throw Error("there can not be 'full' fingerprint of a query molecule");
      // full (non-query) fingerprint, do not skip anything
   }
   else
      throw Error("unknown molecule fingerprint type: %s", type);
}

bool MoleculeFingerprintBuilder::_handleCycle (Graph &graph,
        const Array<int> &vertices, const Array<int> &edges, void *context)
{
   MoleculeFingerprintBuilder *self = (MoleculeFingerprintBuilder *)context;

   self->_handleSubgraph(graph, vertices, edges);
   return true;
}

void MoleculeFingerprintBuilder::_handleTree (Graph &graph,
        const Array<int> &vertices, const Array<int> &edges, void *context)
{
   MoleculeFingerprintBuilder *self = (MoleculeFingerprintBuilder *)context;

   self->_handleSubgraph(graph, vertices, edges);
}

int MoleculeFingerprintBuilder::_maximalSubgraphCriteriaValue (Graph &graph,
      const Array<int> &vertices, const Array<int> &edges, void *context)
{
   BaseMolecule &mol = (BaseMolecule &)graph;
   int ret = 0;
   int ni;
   MoleculeFingerprintBuilder *self = (MoleculeFingerprintBuilder *)context;

   // Check if fragment has query atoms or query bonds
   for (ni = 0; ni < vertices.size(); ni++)
   {
      int i = vertices[ni];
      if (mol.getAtomNumber(i) == -1)
         break;
   }

   bool has_query_atoms = (ni != vertices.size());

   for (ni = 0; ni < edges.size(); ni++)
   {
      int i = edges[ni];
      int bond_order = mol.getBondOrder(i);
      if (bond_order == -1 ||
            (self->query && mol.asQueryMolecule().aromaticity.canBeAromatic(i) && 
            bond_order != BOND_AROMATIC))
         break;
   }

   bool has_query_bonds = (ni != edges.size());

   if (has_query_atoms)
      ret |= 1;

   if (has_query_bonds)
      ret |= 2;

   return ret;
}

dword MoleculeFingerprintBuilder::_canonicalizeFragment (BaseMolecule &mol, const Array<int> &vertices,
         const Array<int> &edges, bool use_atoms, bool use_bonds, int *different_vertex_count)
{
   if (use_bonds)
      subgraph_hash->edge_codes = &_bond_codes; 
   else
      subgraph_hash->edge_codes = &_bond_codes_empty; 

   if (use_atoms)
      subgraph_hash->vertex_codes = &_atom_codes; 
   else
      subgraph_hash->vertex_codes = &_atom_codes_empty; 

   subgraph_hash->max_iterations = (edges.size() + 1) / 2;
   subgraph_hash->calc_different_codes_count = true;

   dword ret = subgraph_hash->getHash(vertices, edges);
   if (different_vertex_count != 0)
      *different_vertex_count = subgraph_hash->getDifferentCodesCount();

   return ret;
}

void MoleculeFingerprintBuilder::_addOrdHashBits (dword hash, int bits_per_fragment)
{
   HashBits hash_bits(hash, bits_per_fragment);

   auto it = _ord_hashes.find(hash_bits);
   if (it == _ord_hashes.end())
      _ord_hashes.emplace(hash_bits, 1);
   else
      _ord_hashes.at(hash_bits)++;
}

void MoleculeFingerprintBuilder::_calculateFragmentVertexDegree (BaseMolecule &mol, const Array<int> &vertices, const Array<int> &edges)
{
   for (int i = 0; i < vertices.size(); i++)
   {
      int v = vertices[i];
      _fragment_vertex_degree[v] = 0;
   }

   for (int i = 0; i < edges.size(); i++)
   {
      int e = edges[i];
      const Edge &edge = mol.getEdge(e);
      int order = _bond_orders[e];
      _fragment_vertex_degree[edge.beg] += order;
      _fragment_vertex_degree[edge.end] += order;
   }
}

int MoleculeFingerprintBuilder::_calculateFragmentExternalConn (BaseMolecule &mol, const Array<int> &vertices, const Array<int> &edges)
{
   _calculateFragmentVertexDegree(mol, vertices, edges);
   int sum = 0;
   for (int i = 0; i < vertices.size(); i++)
   {
      int v = vertices[i];
      sum += _vertex_connectivity[v] - _fragment_vertex_degree[v];
   }
   return sum;
}


void MoleculeFingerprintBuilder::_canonicalizeFragmentAndSetBits (BaseMolecule &mol, const Array<int> &vertices,
         const Array<int> &edges, bool use_atoms, bool use_bonds, int subgraph_type, dword &bits_set)
{
   bool set_sim = false, set_ord = false, set_any = false, set_tau = false;

   if (subgraph_type == TautomerSuperStructure::ORIGINAL)
   {
      // SIM is made of: rings of size up to 6, trees of size up to 4 edges
      if (use_atoms && use_bonds && !skip_sim && _parameters.sim_qwords > 0)
      {
         set_sim = true;
         if (vertices.size() > 6)
            set_sim = false;
         else if (edges.size() == vertices.size() - 1 && edges.size() > 4)
            set_sim = false;
      }
      
      // ORD and ANY are made of all fragments having more than 2 vertices
      if (use_atoms && use_bonds)
      {
         if (!skip_ord && _parameters.ord_qwords > 0)
            set_ord = true;
      }
      else if (_parameters.any_qwords > 0)
      {
         if (use_atoms)
         {
            if (!skip_any_bonds)
               set_any = true;
         }
         else if (use_bonds)
         {
            if (!skip_any_atoms)
               set_any = true;
         }
         else if (!skip_any_atoms_bonds)
            set_any = true;
      }
   }

   // TAU is made of fragments without bond types
   if (!use_bonds && !skip_tau && _parameters.tau_qwords > 0)
      set_tau = true;

   if (!set_any && !set_ord && !set_sim && !set_tau)
      return; 

   // different_vertex_count is equal to the number of orbits 
   // if codes have no collisions
   int different_vertex_count;
   dword hash = _canonicalizeFragment(mol, vertices, edges, 
      use_atoms, use_bonds, &different_vertex_count);

   // Calculate bits count factor based on different_vertex_count
   int bits_per_fragment;
   if (2 * vertices.size() > 3 * different_vertex_count)
      bits_per_fragment = 5;
   else if (vertices.size() <= 3)
      bits_per_fragment = 2;
   else if (vertices.size() >= 5 && vertices.size() != edges.size())
      bits_per_fragment = 1;
   else
      bits_per_fragment = 2;

   if (cb_fragment != 0)
      (*cb_fragment)(mol, vertices, edges, use_atoms, use_bonds, hash);

   // Set bits only if bits_set doesn't have such bits
   dword bits_set_src = bits_set;
   if (!query)
      bits_set_src = 0;

   if (set_sim && !(bits_set_src & 0x01))
   {
      _setBits(hash, getSim(), _parameters.fingerprintSizeSim(), 1);
      bits_set |= 0x01;
   }
   
   
   
   
   if (set_ord && !(bits_set_src & 0x02))
   {
      _addOrdHashBits(hash, bits_per_fragment);

      // Add extra bits for charged fragments
      int charged = 0;
      for (int i = 0; i < vertices.size(); i++)
      {
         int v = vertices[i];
         if (_atom_charges[v] != 0)
            charged++;
      }

      if (charged != 0)
      {
         const dword CHARGE_MASK = 0x526e7e24; // random
         _addOrdHashBits(hash ^ CHARGE_MASK, bits_per_fragment);
      }
   /*
    * IND-692 disable incorrect fingerprint part (some tests are failed)
    */
      // Add extra bits for fragments with a lot of terminal atoms
//      const dword CONN_MASK = 0x7e24526e; // random
//      int ext_conn = _calculateFragmentExternalConn(mol, vertices, edges);
//      if (ext_conn == 0)
//         // Increase bits per fragment because this is maximal fragment and they are rare
//         _addOrdHashBits(hash ^ CONN_MASK, bits_per_fragment + 4); 
//      if (ext_conn <= 1 && vertices.size() > 3)
//         _addOrdHashBits(hash ^ (CONN_MASK << 1), bits_per_fragment);
//      if (ext_conn <= 2 && vertices.size() > 4)
//         _addOrdHashBits(hash ^ (CONN_MASK << 2), bits_per_fragment);

      bits_set |= 0x02;
   }

   // Any part is used only if 'ord' bit wasn't set - 0x02 bit mask is checked
   if (set_any && !(bits_set_src & 0x04) && !(bits_set_src & 0x02)) 
   {
      _setBits(hash, getAny(), _parameters.fingerprintSizeAny(), bits_per_fragment);
      bits_set |= 0x04;
   }

   if (set_tau && !(bits_set_src & 0x08))
   {
      _setBits(hash, getTau(), _parameters.fingerprintSizeTau(), 2);
      bits_set |= 0x08;
   }
}

void MoleculeFingerprintBuilder::_handleSubgraph (Graph &graph,
        const Array<int> &vertices, const Array<int> &edges)
{

   if(cancellation && cancellation->isCancelled())
      throw Error("Fingerprint calculation has been cancelled: %s", cancellation->cancelledRequestMessage());

   BaseMolecule &mol = (BaseMolecule &)graph;
   int i;

   int subgraph_type;
   if (_tau_super_structure != 0)
      subgraph_type = _tau_super_structure->getSubgraphType(vertices, edges);
   else
      subgraph_type = TautomerSuperStructure::ORIGINAL;
   
   if (subgraph_type == TautomerSuperStructure::NONE)
      return;

   // Check if fragment has query atoms or query bonds
   for (i = 0; i < vertices.size(); i++)
      if (mol.getAtomNumber(vertices[i]) == -1)
         break;

   bool has_query_atoms = (i != vertices.size());

   for (i = 0; i < edges.size(); i++)
   {
      int e = edges[i];
      int bond_order = mol.getBondOrder(e);
      if (bond_order == -1 || 
            (query && mol.asQueryMolecule().aromaticity.canBeAromatic(e) && 
            bond_order != BOND_AROMATIC))
         break;
   }

   bool has_query_bonds = (i != edges.size());

   dword bits_set = 0;
   if (!has_query_atoms && !has_query_bonds)
      _canonicalizeFragmentAndSetBits(mol, vertices, edges, true, true, subgraph_type, bits_set);

   dword bits_set_a = bits_set;
   if (!query || !has_query_atoms)
      _canonicalizeFragmentAndSetBits(mol, vertices, edges, true, false, subgraph_type, bits_set_a);

   dword bits_set_b = bits_set;
   if (!query || !has_query_bonds)
      _canonicalizeFragmentAndSetBits(mol, vertices, edges, false, true, subgraph_type, bits_set_b);

   dword bits_set_ab = (bits_set_a | bits_set_b);
   _canonicalizeFragmentAndSetBits(mol, vertices, edges, false, false, subgraph_type, bits_set_ab);
}

void MoleculeFingerprintBuilder::_makeFingerprint (BaseMolecule &mol)
{
   Obj<TautomerSuperStructure> tau_super_structure;
   BaseMolecule *mol_for_enumeration = &mol;
   
   if (!query && _parameters.tau_qwords > 0 && !skip_tau)
   {
      tau_super_structure.create(mol.asMolecule());

      _tau_super_structure = tau_super_structure.get();
      mol_for_enumeration = tau_super_structure.get();
   }
   else
      _tau_super_structure = 0;
   
   if (!skip_ord || !skip_any_atoms || !skip_any_atoms_bonds ||
       !skip_any_bonds || !skip_tau || !skip_sim)
   {
      QS_DEF(Filter, vfilter);
      vfilter.initAll(mol_for_enumeration->vertexEnd());

      // remove (possible) hydrogens
      for (auto v : mol_for_enumeration->vertices())
         if (mol_for_enumeration->possibleAtomNumber(v, ELEM_H))
            vfilter.hide(v);

      _initHashCalculations(*mol_for_enumeration, vfilter);

      CycleEnumerator ce(*mol_for_enumeration);
      GraphSubtreeEnumerator se(*mol_for_enumeration);

      ce.vfilter = &vfilter;
      se.vfilter = &vfilter;

      bool sim_only = skip_ord && skip_tau && skip_any_atoms &&
                      skip_any_atoms_bonds && skip_any_bonds;

      _is_cycle = true;
      ce.context = this;
      ce.max_length = sim_only ? 6 : 8;
      ce.cb_handle_cycle = _handleCycle;
      ce.process();
   
      _is_cycle = false;
      se.context = this;
      se.min_vertices = 1;
      se.max_vertices = sim_only ? 5 : 7;
      se.handle_maximal = false;
      se.maximal_critera_value_callback = _maximalSubgraphCriteriaValue;
      se.callback = _handleTree;
      se.process();

      // Set hash bits
      for (auto it : _ord_hashes)
      {
         int bits_per_fragment = it.first.bits_per_fragment + (bitLog2Dword(it.second) - 1);
         // Heuristic: if a fragment has high frequency and they are close to each other (like in substructure query)
         // then there should be larger fragment with lower frequency
         if (bits_per_fragment > 8)
            bits_per_fragment = 8;
         _setBits(it.first.hash, getOrd(), _parameters.fingerprintSizeOrd(), bits_per_fragment);
      }
   }
   
   if (!skip_ext && _parameters.ext)
      _calcExtraBits(mol);

   if (!skip_chem)
   {
      QS_DEF(Array<int>, feature_set);
      for (int vi = mol.vertexBegin(); vi != mol.vertexEnd(); vi = mol.vertexNext(vi))
      {
         // No exception should ever be thrown since `vi` iterates only through
         // valid atoms (not template, R, pseudo atoms)
         // Added the try-catch just to be sure
         try
         {
            MoleculePkaModel::getAtomLocalFeatureSet(mol, vi, feature_set);
         } catch (indigo::Exception &e) {
            // `vi` correspond to an invalid atom, nothing to add to the fingerprint
            continue;
         }

         const dword salt = 1024;  // 100 < salt < DWORD_MAX / 30
         for (int i = 0; i < feature_set.size(); i++)
         {
            int bits_per_fragment = 1;  // By analogy with "sim"
            dword hash = (dword) feature_set[i] + salt * i;
            _setBits(hash, getChem(), _parameters.fingerprintSizeChem(), bits_per_fragment);
         }
      }
   }
}

void MoleculeFingerprintBuilder::_calcExtraBits (BaseMolecule &mol)
{
   int counters[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};

   for (auto i : mol.vertices())
   {
      if (mol.possibleAtomNumber(i, ELEM_H))
         continue;

      int an = mol.getAtomNumber(i);

      if (an == ELEM_C)
         counters[0]++;
      else if (an == ELEM_N)
         counters[1]++;
      else if (an == ELEM_O)
         counters[2]++;
      else if (an == ELEM_P)
         counters[3]++;
      else if (an == ELEM_S)
         counters[4]++;
      else if (Element::isHalogen(an))
         counters[5]++;
      else if (an > ELEM_H)
         counters[6]++;

      if (!skip_ext_charge && mol.getAtomCharge(i) != 0 && mol.getAtomCharge(i) != CHARGE_UNKNOWN)
         counters[7]++;
      if (mol.getAtomIsotope(i) > 0)
         counters[8]++;
   }
   
   byte *fp = _total_fingerprint.ptr();

   if (counters[0] > 13) // > 13 C
      fp[0] |= 1;
   if (counters[0] > 16) // > 16 C
      fp[0] |= 2;
   if (counters[0] > 19) // > 19 C
      fp[0] |= 4;
   if (counters[1] > 1) // > 1 N
      fp[0] |= 8;
   if (counters[1] > 2) // > 2 N
      fp[0] |= 16;
   if (counters[2] > 3) // > 3 O
      fp[0] |= 32;
   if (counters[2] > 4) // > 4 O
      fp[0] |= 64;
   if (counters[3] > 0) // have P
      fp[0] |= 128;
   if (counters[4] > 0) // have S
      fp[1] |= 1;
   if (counters[4] > 1) // > 1 S
      fp[1] |= 2;
   if (counters[5] > 1) // > 1 halogen
      fp[1] |= 4;
   if (counters[5] > 2) // > 2 halogen
      fp[1] |= 8;
   if (counters[6] > 0) // have rare atoms
      fp[1] |= 16;
   if (counters[6] > 1) // > 1 rare atom
      fp[1] |= 32;
   if (counters[7] > 0) // have charged atoms
      fp[1] |= 64;
   if (counters[8] > 1) // have isotopes
      fp[1] |= 128;
}

void MoleculeFingerprintBuilder::_setBits (dword hash, byte *fp, int size, int nbits)
{
   unsigned seed = hash;

   // fill random bits
   while (nbits-- > 0)
   {
      seed = seed * 0x8088405 + 1;

      // Uniformly distributed bits
      unsigned k = (unsigned)(((qword)(size * 8) * seed) / (unsigned)(-1));
      //unsigned k = seed % (size * 8);
      unsigned nbyte = k / 8;
      unsigned nbit = k - nbyte * 8;

      fp[nbyte] = fp[nbyte] | (1 << nbit);
   }
}

const byte * MoleculeFingerprintBuilder::get ()
{
   return _total_fingerprint.ptr();
}

byte * MoleculeFingerprintBuilder::getOrd ()
{
   return _total_fingerprint.ptr() + _parameters.fingerprintSizeExt();
}

byte * MoleculeFingerprintBuilder::getSim ()
{
   return _total_fingerprint.ptr() + _parameters.fingerprintSizeExt() + _parameters.fingerprintSizeOrd();
}

byte * MoleculeFingerprintBuilder::getChem ()
{
   return _total_fingerprint.ptr() + _parameters.fingerprintSizeExt() + _parameters.fingerprintSizeOrd() +
          _parameters.fingerprintSizeSim();
}

byte * MoleculeFingerprintBuilder::getTau ()
{
   return _total_fingerprint.ptr() + _parameters.fingerprintSizeExt() + _parameters.fingerprintSizeOrd() +
          _parameters.fingerprintSizeSim() + _parameters.fingerprintSizeChem();
}

byte * MoleculeFingerprintBuilder::getAny ()
{
   return _total_fingerprint.ptr() + _parameters.fingerprintSizeExt() + _parameters.fingerprintSizeOrd() +
        _parameters.fingerprintSizeSim() + _parameters.fingerprintSizeChem() + _parameters.fingerprintSizeTau();
}

int MoleculeFingerprintBuilder::countBits_Sim ()
{
   return bitGetOnesCount(getSim(), _parameters.fingerprintSizeSim());
}

//
// MoleculeFingerprintBuilder::HashBits
//

MoleculeFingerprintBuilder::HashBits::HashBits (dword hash, int bits_per_fragment) : 
   hash(hash), bits_per_fragment(bits_per_fragment)
{
}

bool MoleculeFingerprintBuilder::HashBits::operator== (const HashBits &right) const
{
   return right.bits_per_fragment == bits_per_fragment && right.hash == hash;
}

//
// MoleculeFingerprintBuilder::Hasher
//
size_t MoleculeFingerprintBuilder::Hasher::operator () (const HashBits &input) const
{
   return (input.bits_per_fragment << 10) + input.hash;
}
