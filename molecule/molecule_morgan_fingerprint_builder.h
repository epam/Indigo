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


#ifndef PROJECT_MOLECULE_MORGAN_FINGERPRINT_H
#define PROJECT_MOLECULE_MORGAN_FINGERPRINT_H

#include <set>
#include <vector>
#include <base_cpp/array.h>
#include "base_c/defs.h"
#include "base_molecule.h"

namespace indigo {

class DLLEXPORT MoleculeMorganFingerprintBuilder : public NonCopyable {
public:
   explicit MoleculeMorganFingerprintBuilder(BaseMolecule &mol);

   void calculateDescriptorsECFP(int fp_depth, Array<dword> &res);
   void calculateDescriptorsFCFP(int fp_depth, Array<dword> &res);

   void packFingerprintECFP(int fp_depth, Array <byte> &res);
   void packFingerprintFCFP(int fp_depth, Array <byte> &res);

private:
   enum {MAGIC_HASH_NUMBER = 37};

   static void setBits(dword hash, byte *fp, int size);

   typedef dword (*InitialStateCallback)(BaseMolecule &mol, int idx);

   void initDescriptors(MoleculeMorganFingerprintBuilder::InitialStateCallback initialStateCallback);
   void buildDescriptors(int fp_depth);
   void calculateNewAtomDescriptors(int iterationNumber);

   /**
    * ECFP: (hash of 7 ints)
    *  - number of  non-hydrogen neighbors;
    *  - valence minus the number of hydrogens;
    *  - the atomic number;
    *  - the atomic mass;
    *  - the atomic charge;
    *  - the number of attached hydrogens (both implicit and explicit)
    *  - whether the atom is contained in at least one ring
    *  */
   static dword initialStateCallback_ECFP(BaseMolecule &mol, int idx);

   /**
    * FCFP: (6 bits)
    *  - hydrogen-bond acceptor
    *  - hydrogen-bond donor
    *  - negatively ionizable
    *  - positively ionizable
    *  - aromatic
    *  - halogen
    *  */
   static dword initialStateCallback_FCFP(BaseMolecule &mol, int idx);

   typedef struct BondDescriptor {
      int bond_type;
      int vertex_idx;
      int edge_idx;
   };

   typedef struct FeatureDescriptor {
      dword hash;
      std::set<int> bond_set;

      bool operator==(const FeatureDescriptor &rhs) const;

      bool operator<(const FeatureDescriptor &rhs) const;
   };

   typedef struct AtomDescriptor {
      FeatureDescriptor descr;
      FeatureDescriptor new_descr;
      std::vector<BondDescriptor> bond_descriptors;
   };

   int bondDescriptorCmp(const BondDescriptor & bd1, const BondDescriptor & bd2);

   BaseMolecule& mol;
   std::vector<FeatureDescriptor> features;
   std::vector<AtomDescriptor> atom_descriptors;
};

};

#endif //PROJECT_MOLECULE_MORGAN_FINGERPRINT_H
