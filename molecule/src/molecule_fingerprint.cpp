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

#include "base_cpp/crc32.h"
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

MoleculeFingerprintBuilder::MoleculeFingerprintBuilder (BaseMolecule &mol,
                     const MoleculeFingerprintParameters &parameters):
_mol(mol),
_parameters(parameters),
TL_CP_GET(_total_fingerprint)
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
      const int *v_mapping, const int *e_mapping, void *context)
{
   BaseMolecule &mol = (BaseMolecule &)graph;
   int ret = 0;
   int i;
   MoleculeFingerprintBuilder *self = (MoleculeFingerprintBuilder *)context;

   // Check if fragment has query atoms or query bonds
   for (i = mol.vertexBegin(); i != mol.vertexEnd(); i = mol.vertexNext(i))
   {
      if (v_mapping[i] < 0)
         continue;
      if (mol.getAtomNumber(i) == -1)
         break;
   }

   bool has_query_atoms = (i != mol.vertexEnd());

   for (i = mol.edgeBegin(); i !=  mol.edgeEnd(); i = mol.edgeNext(i))
   {
      if (e_mapping[i] < 0)
         continue;
      int bond_order = mol.getBondOrder(i);
      if (bond_order == -1 ||
            (self->query && mol.asQueryMolecule().aromaticity.canBeAromatic(i) && 
            bond_order != BOND_AROMATIC))
         break;
   }

   bool has_query_bonds = (i != mol.edgeEnd());

   if (has_query_atoms)
      ret |= 1;

   if (has_query_bonds)
      ret |= 2;

   return ret;
}

int MoleculeFingerprintBuilder::_vertex_code (Graph &graph, int vertex_idx, void *context)
{
   BaseMolecule &mol = (BaseMolecule &)graph;

   if (mol.isPseudoAtom(vertex_idx))
      return CRC32::get(mol.getPseudoAtom(vertex_idx));

   return mol.getAtomNumber(vertex_idx);
}

int MoleculeFingerprintBuilder::_edge_code (Graph &graph, int edge_idx, void *context)
{
   BaseMolecule &mol = (BaseMolecule &)graph;
   
   //MoleculeFingerprintBuilder *self = (MoleculeFingerprintBuilder *)context;
   //if (self->query && mol.asQueryMolecule().aromaticity.canBeAromatic(edge_idx))
   //  throw Error("internal: _edge_code for possibly aromatic bond");
   
   return mol.getBondOrder(edge_idx);
}

dword MoleculeFingerprintBuilder::_canonicalizeFragment (BaseMolecule &mol, const Array<int> &vertices,
         const Array<int> &edges, bool use_atoms, bool use_bonds, int *different_vertex_count)
{
   SubgraphHash subgraph_hash(mol);

   subgraph_hash.context = this;

   if (use_bonds)
      subgraph_hash.cb_edge_code = _edge_code; 
   if (use_atoms)
      subgraph_hash.cb_vertex_code = _vertex_code; 
   subgraph_hash.max_iterations = (edges.size() + 1) / 2;
   subgraph_hash.calc_different_codes_count = true;

   dword ret = subgraph_hash.getHash(vertices, edges);
   if (different_vertex_count != 0)
      *different_vertex_count = subgraph_hash.getDifferentCodesCount();
   return ret;
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
      _setBits(hash, getOrd(), _parameters.fingerprintSizeOrd(), bits_per_fragment);
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
   QS_DEF(Filter, vfilter);
   int i;

   vfilter.initAll(mol.vertexEnd());

   // remove (possible) hydrogens
   for (i = mol.vertexBegin(); i < mol.vertexEnd(); i = mol.vertexNext(i))
      if (mol.possibleAtomNumber(i, ELEM_H))
         vfilter.hide(i);

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
      se.callback2 = _handleTree;
      se.process();
   }
   
   if (!skip_ext && _parameters.ext)
      _calcExtraBits(mol, vfilter);
}

void MoleculeFingerprintBuilder::_calcExtraBits (BaseMolecule &mol, Filter &vfilter)
{
   int counters[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
   int i;

   for (i = mol.vertexBegin(); i != mol.vertexEnd(); i = mol.vertexNext(i))
   {
      if (!vfilter.valid(i))
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

byte * MoleculeFingerprintBuilder::getTau ()
{
   return _total_fingerprint.ptr() + _parameters.fingerprintSizeExt() + _parameters.fingerprintSizeOrd() +
          _parameters.fingerprintSizeSim();
}

byte * MoleculeFingerprintBuilder::getAny ()
{
   return _total_fingerprint.ptr() + _parameters.fingerprintSizeExt() + _parameters.fingerprintSizeOrd() +
        _parameters.fingerprintSizeSim() + _parameters.fingerprintSizeTau();
}

int MoleculeFingerprintBuilder::countBits_Sim ()
{
   return bitGetOnesCount(getSim(), _parameters.fingerprintSizeSim());
}
