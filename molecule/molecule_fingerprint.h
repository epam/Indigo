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

#ifndef __molecule_fingerprint__
#define __molecule_fingerprint__

#include "base_cpp/tlscont.h"
#include "base_cpp/obj.h"
#include "molecule/base_molecule.h"
#include "base_cpp/cancellation_handler.h"
#include "graph/subgraph_hash.h"

#include <unordered_map>
#include <limits.h>

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4251)
#endif

namespace indigo {

class TautomerSuperStructure;

// Fingerprint consists of 5 parts: EXT + ORD + ANY + TAU + SIM.
// EXT is always 3 bytes long, other parts' sizes are configured.
// ORD, ANY, and SIM parts are build up from fragments.
// Each fragments goes to:
//    SIM -- as long as it has no query atoms/bonds and is small enough
//    ORD -- as long as it has no query atoms/bonds
//    ANY (with bond types discarded) -- as long as it has no query atoms
//    ANY (with atom types discarded) -- as long as it has no query bonds
//    ANY (with atom and bond types discarded) -- always
// TAU part is build up from a 'supermolecule' having some added bonds,
//     and with all bond types discarded
// EXT part is build up from some element, isotope, and charge counters

enum SimilarityType
{
   SIM = 0,
   CHEM = 1,
   ECFP2, ECFP4, ECFP6,
   FCFP2, FCFP4, FCFP6,
};

struct MoleculeFingerprintParameters
{
   bool ext;
   SimilarityType similarity_type;
   int ord_qwords, any_qwords, tau_qwords, sim_qwords;

   int fingerprintSize    () const { return (ext ? 3 : 0) + (ord_qwords + any_qwords + tau_qwords + sim_qwords) * 8; }
   int fingerprintSizeExt () const { return (ext ? 3 : 0); }
   int fingerprintSizeOrd () const { return ord_qwords * 8; }
   int fingerprintSizeSim () const { return sim_qwords * 8; }
   int fingerprintSizeTau () const { return tau_qwords * 8; }
   int fingerprintSizeAny () const { return any_qwords * 8; }

   int fingerprintSizeExtOrd () const { return (ext ? 3 : 0) + ord_qwords * 8;}
   int fingerprintSizeExtOrdSim () const { return (ext ? 3 : 0) + ord_qwords * 8 + sim_qwords * 8;}

};

class DLLEXPORT MoleculeFingerprintBuilder
{
public:
   MoleculeFingerprintBuilder (BaseMolecule &mol, const MoleculeFingerprintParameters &parameters);
   ~MoleculeFingerprintBuilder ();

   bool query;

   bool skip_ord; // don't build 'ordinary' part of the fingerprint
   bool skip_sim; // don't build 'similarity' part of the fingerprint
   bool skip_tau; // don't build 'tautomer' part of the fingerprint
   bool skip_ext; // don't build 'extra' part of the fingerprint
   bool skip_ext_charge; // don't store information about charges in 'extra' part

   bool skip_any_atoms; // don't build 'any atoms' part of the fingerprint
   bool skip_any_bonds; // don't build 'any bonds' part of the fingerprint
   bool skip_any_atoms_bonds; // don't build 'any atoms, any bonds' part of the fingerprint

   void process ();

   const byte * get ();
   byte * getOrd ();
   byte * getSim ();
   byte * getTau ();
   byte * getAny ();
   
   int countBits_Sim ();

   void (*cb_fragment) (BaseMolecule &mol, const Array<int> &vertices, const Array<int> &edges,
                        bool use_atoms, bool use_bonds, dword hash);

   void parseFingerprintType(const char *type, bool query);

   static SimilarityType parseSimilarityType(const char * type);
   static const char * printSimilarityType(SimilarityType type);

   CancellationHandler* cancellation;

   DECL_ERROR;

protected:
   void _initHashCalculations (BaseMolecule &mol, const Filter &vfilter);

   static void _handleTree     (Graph &graph, const Array<int> &vertices, const Array<int> &edges, void *context);
   static bool _handleCycle    (Graph &graph, const Array<int> &vertices, const Array<int> &edges, void *context);

   static int _maximalSubgraphCriteriaValue (Graph &graph, const Array<int> &vertices, const Array<int> &edges, void *context);

   void _handleSubgraph (Graph &graph, const Array<int> &vertices, const Array<int> &edges);

   dword _canonicalizeFragment (BaseMolecule &mol, const Array<int> &vertices, const Array<int> &edges,
      bool use_atoms, bool use_bonds, int *different_vertex_count);

   void _canonicalizeFragmentAndSetBits (BaseMolecule &mol, const Array<int> &vertices, const Array<int> &edges,
      bool use_atoms, bool use_bonds, int subgraph_type, dword &bits_to_set);

   void _makeFingerprint (BaseMolecule &mol);
   void _makeFingerprint_calcOrdSim(BaseMolecule &mol);
   void _makeFingerprint_calcChem(BaseMolecule &mol);
   void _calcExtraBits (BaseMolecule &mol);

   void _setTauBits (const char *str, int nbits);
   void _setOrdBits (const char *str, int nbits);

   static void _setBits (dword hash, byte *fp, int size, int nbits);
   
   void _calculateFragmentVertexDegree (BaseMolecule &mol, const Array<int> &vertices, const Array<int> &edges);
   int _calculateFragmentExternalConn (BaseMolecule &mol, const Array<int> &vertices, const Array<int> &edges);

   BaseMolecule &_mol;
   const MoleculeFingerprintParameters &_parameters;

   // these parameters are indirectly passed to the callbacks
   TautomerSuperStructure *_tau_super_structure;
   bool _is_cycle;

   struct HashBits
   {
      HashBits (dword hash, int bits_per_fragment);
      bool operator== (const HashBits &right) const;

      dword hash;
      int bits_per_fragment;
   };
   struct Hasher
   {
      size_t operator () (const HashBits &input) const;
   };

   void _addOrdHashBits (dword hash, int bits_per_fragment);

   Obj<SubgraphHash> subgraph_hash;

   CP_DECL;
   TL_CP_DECL(Array<byte>, _total_fingerprint);
   TL_CP_DECL(Array<int>, _atom_codes);
   TL_CP_DECL(Array<int>, _bond_codes);
   TL_CP_DECL(Array<int>, _atom_codes_empty);
   TL_CP_DECL(Array<int>, _bond_codes_empty);
   TL_CP_DECL(Array<int>, _atom_hydrogens);
   TL_CP_DECL(Array<int>, _atom_charges);
   TL_CP_DECL(Array<int>, _vertex_connectivity);
   TL_CP_DECL(Array<int>, _fragment_vertex_degree);
   TL_CP_DECL(Array<int>, _bond_orders);

   typedef std::unordered_map<HashBits, int, Hasher> HashesMap;
   TL_CP_DECL(HashesMap, _ord_hashes);

private:
   MoleculeFingerprintBuilder (const MoleculeFingerprintBuilder &); // no implicit copy
};

}

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
