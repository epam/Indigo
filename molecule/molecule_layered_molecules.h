/****************************************************************************
 * Copyright (C) 2015 GGA Software Services LLC
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

#ifndef __molecule_layered_molecules_h__
#define __molecule_layered_molecules_h__

#include "common/base_cpp/d_bitset.h"
#include "molecule/base_molecule.h"
#include "molecule/molecule.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4251)
#endif

namespace indigo {

class DLLEXPORT LayeredMolecules : public BaseMolecule
{      
public:
   static const int BOND_TYPES_NUMBER = 5;

   LayeredMolecules(BaseMolecule& molecule);
   virtual ~LayeredMolecules();

   // This method returns a bitmask of all layers that contain a bond idx of a specific order:
   const Dbitset &getBondMask(int idx, int order) const;

   // These methods are used for tracking if the atom is a possible position for a mobile hydrogen:
   bool isMobilePosition(int idx) const;
   void setMobilePosition(int idx, bool value);

   // These methods are used for tracking if the mobile position is occupied already.
   // The bitmask is the layers where the position is occupied.
   const Dbitset &getMobilePositionOccupiedMask(int idx) const;
   void setMobilePositionOccupiedMask(int idx, Dbitset &mask, bool value);

   // mask: the mask of layers used as prototypes;
   // edgesPath: the path of single-double bonds to be inverted
   // beg, end: the mobile positions of hydrogen to swap
   // forward: the direction to move the hydrogen
   void addLayersWithInvertedPath(const Dbitset &mask, const Array<int> &path, int beg, int end, bool forward);

   bool aromatize (int layerFrom, int layerTo, const AromaticityOptions &options);

   // construct a molecule that is represented as a layer
   void constructMolecule(Molecule &molecule, int layer, bool aromatized) const;

   unsigned getHash(int layer, bool aromatized)
   {
      if(aromatized)
         return _hashsAromatized[layer];
      return _hashs[layer];
   }


   virtual void clear ();

   virtual BaseMolecule * neu ();

   virtual int getAtomNumber  (int idx);
   virtual int getAtomCharge  (int idx);
   virtual int getAtomIsotope (int idx);
   virtual int getAtomRadical (int idx);
   virtual int getAtomAromaticity (int idx);
   virtual int getExplicitValence (int idx);
   virtual int getAtomValence (int idx);
   virtual int getAtomSubstCount (int idx);
   virtual int getAtomRingBondsCount (int idx);

   virtual int getAtomMaxH   (int idx);
   virtual int getAtomMinH   (int idx);
   virtual int getAtomTotalH (int idx);

   virtual bool isPseudoAtom (int idx);
   virtual const char * getPseudoAtom (int idx);

   virtual bool  isRSite (int atom_idx);
   virtual dword getRSiteBits (int atom_idx);
   virtual void  allowRGroupOnRSite (int atom_idx, int rg_idx);

   virtual int getBondOrder      (int idx);
   virtual int getBondTopology   (int idx);

   virtual bool atomNumberBelongs (int idx, const int *numbers, int count);
   virtual bool possibleAtomNumber (int idx, int number);
   virtual bool possibleAtomNumberAndCharge (int idx, int number, int charge);
   virtual bool possibleAtomNumberAndIsotope (int idx, int number, int isotope);
   virtual bool possibleAtomIsotope (int idx, int isotope);
   virtual bool possibleAtomCharge (int idx, int charge);
   virtual void getAtomDescription (int idx, Array<char> &description);
   virtual void getBondDescription (int idx, Array<char> &description);
   virtual bool possibleBondOrder (int idx, int order);

   virtual bool isSaturatedAtom (int idx);

   virtual bool bondStereoCare (int idx);

   virtual bool aromatize (const AromaticityOptions &options);
   virtual bool dearomatize (const AromaticityOptions &options);

   int layers;

protected:
   struct AromatizationContext
   {
      LayeredMolecules *self;
      int layerFrom;
      int layerTo;
      bool result;
   };


   Molecule _proto;
   ObjArray<Dbitset> _bond_masks[BOND_TYPES_NUMBER];
   Array<bool> _mobilePositions;
   ObjArray<Dbitset> _mobilePositionsOccupied;

   virtual void _mergeWithSubmolecule (BaseMolecule &bmol, const Array<int> &vertices,
                                       const Array<int> *edges, const Array<int> &mapping, 
                                       int skip_flags);

   static bool _cb_handle_cycle(Graph &graph, const Array<int> &vertices, const Array<int> &edges, void *context);

   void _resizeLayers(int newSize);
   void _calcConnectivity(int layerFrom, int layerTo);
   void _calcPiLabels(int layerFrom, int layerTo);
   bool _handleCycle(int layerFrom, int layerTo, const Array<int> &path);
   bool _isCycleAromaticInLayer(const int *cycle, int cycle_len, int layer);
   void _aromatizeCycle(const Array<int> &cycle, const Dbitset &mask);
   void _registerAromatizedLayers(int layerFrom, int layerTo);

private:
   LayeredMolecules(const LayeredMolecules &); // no implicit copy
   ObjArray<Array<int>> _piLabels;
   ObjArray<Array<int>> _connectivity;
   int _layersAromatized;


   struct TrieNode
   {
      static const int ALPHABET_SIZE = 5;

      TrieNode()
      {
         for(auto &n : next)
            n = -1;
      }
      unsigned next[ALPHABET_SIZE];
   };

   class Trie
   {
   public:
      Trie()
      {
         _nodes = new ObjPool<TrieNode>();
         // Adding root (index == 0)
         _nodes->add();
      }
      unsigned getRoot() { return 0; }
      unsigned follow(unsigned nodeInd, unsigned key) { return _nodes->at(nodeInd).next[key]; }
      unsigned add(unsigned nodeInd, unsigned key, bool &newlyAdded)
      {
         if(_nodes->at(nodeInd).next[key] != -1)
         {
            newlyAdded = false;
            return _nodes->at(nodeInd).next[key];
         }
         newlyAdded = true;
         int ind = _nodes->add();
         _nodes->at(nodeInd).next[key] = ind;
         return ind;
      }

   private:
      AutoPtr<ObjPool<TrieNode>> _nodes;
   };

   Trie _trie;
   Array<unsigned> _hashs;
   Array<unsigned> _hashsAromatized;

};

}

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
