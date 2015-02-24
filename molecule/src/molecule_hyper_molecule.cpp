/****************************************************************************
 * Copyright (C) 2009-2015 GGA Software Services LLC
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

#include "molecule/molecule_hyper_molecule.h"

#include "base_c/defs.h"
#include "base_cpp/output.h"
#include "molecule/elements.h"
#include "molecule/molecule_arom.h"
#include "molecule/molecule_dearom.h"
#include "molecule/molecule_standardize.h"

using namespace indigo;

HyperMolecule::HyperMolecule(BaseMolecule& molecule)
{
   int i;

   _proto.clone(molecule.asMolecule(), 0, 0);
   _proto.dearomatize(AromaticityOptions());

   cloneGraph(_proto, 0);

   layers = 1;
   _wordsNeeded = 1;
   for (i = 0; i != _proto.edgeCount(); i++)
   {
      const Edge &edge = _proto.getEdge(i);

      _bond_masks[0].push();
      _bond_masks[1].push();
      _bond_masks[2].push();
      _bond_masks[3].push();

      if (_proto.getBondOrder(i) == 1)
      {
         _bond_masks[1].top().set(0);
         _bond_masks[2].top().reset(0);
         _bond_masks[3].top().reset(0);
      }
      else
      {
         _bond_masks[1].top().reset(0);
         _bond_masks[2].top().set(0);
         _bond_masks[3].top().reset(0);
      }
   }

   _mobilePositions.expandFill(_proto.vertexCount(), false);
   _mobilePositionsOccupied.expand(_proto.vertexCount());

   layers = 1;
}

HyperMolecule::~HyperMolecule()
{
}

void HyperMolecule::constructMolecule(Molecule &molecule, int layer)
{
   molecule.clone(_proto, NULL, NULL);
   for (int i = 0; i != _proto.edgeCount(); ++i)
   {
      int order = (_bond_masks[1][i].get(layer) ? 1 : 2);
      molecule.setBondOrder(i, order);
   }
}

void HyperMolecule::clear()
{
   BaseMolecule::clear();

   // ...
}

Dbitset &HyperMolecule::getBondMaskIND(int idx, int order)
{
   return _bond_masks[order][idx];
}

bool HyperMolecule::isMobilePosition(int idx)
{
   return _mobilePositions[idx];
}

void HyperMolecule::setMobilePosition(int idx, bool value)
{
   _mobilePositions[idx] = value;
}

Dbitset &HyperMolecule::getMobilePositionOccupiedMask(int idx)
{
   return _mobilePositionsOccupied[idx];
}

void HyperMolecule::setMobilePositionOccupiedMask(int idx, Dbitset &mask, bool value)
{
   if (value)
      _mobilePositionsOccupied[idx].orWith(mask);
   else
      _mobilePositionsOccupied[idx].andNotWith(mask);
}

void HyperMolecule::addTautomer(Dbitset &mask1, Array<int> &path, int beg, int end, bool forward)
{
   Array<bool> onPath;
   onPath.expandFill(edgeCount(), false);
   Dbitset onPathMask(layers);
   onPathMask.flip();
   int order = forward ? 2 : 1;
   for (auto i = 0; i < path.size(); ++i)
   {
      onPath[path[i]] = true;
      onPathMask.andWith(_bond_masks[order][path[i]]);
      order = order == 1 ? 2 : 1;
   }

   Dbitset mask_;
   mask_.copy(mask1);

   Dbitset newTautomersMask(_wordsNeeded * 64);
   while (!mask_.isEmpty())
   {
      unsigned newTautomerIndex = layers;
      if (_wordsNeeded * 64 < newTautomerIndex + 1)
      {
         // resize all masks
         for (auto i = 0; i < edgeCount(); ++i)
         {
            _bond_masks[1][i].resize(newTautomerIndex+1);
            _bond_masks[2][i].resize(newTautomerIndex+1);
            _bond_masks[3][i].resize(newTautomerIndex+1);
         }
         for (auto i = 0; i < vertexCount(); ++i)
         {
            _mobilePositionsOccupied[i].resize(newTautomerIndex+1);
         }
         newTautomersMask.resize(newTautomerIndex + 1);
         _wordsNeeded = (newTautomerIndex + 1) / 64 + 1;
      }


      Dbitset protoMask; protoMask.copy(mask_);
      Dbitset antiProtoMask; antiProtoMask.copy(onPathMask);
      int leaderIndex = mask_.nextSetBit(0);

      for (auto i = 0; i < onPath.size(); ++i)
      {
         if (onPath[i])
            continue;
         if (_bond_masks[1][i].get(leaderIndex))
         {
            _bond_masks[1][i].set(newTautomerIndex);
            protoMask.andWith(_bond_masks[1][i]);
            antiProtoMask.andWith(_bond_masks[1][i]);
         }
         else
         {
            _bond_masks[2][i].set(newTautomerIndex);
            protoMask.andWith(_bond_masks[2][i]);
            antiProtoMask.andWith(_bond_masks[2][i]);
         }
      }

      if (!antiProtoMask.isEmpty())
      {
         mask_.andNotWith(protoMask);
         for (auto i = 0; i < onPath.size(); ++i)
         {
            if (onPath[i])
               continue;
            _bond_masks[1][i].reset(newTautomerIndex);
            _bond_masks[2][i].reset(newTautomerIndex);
         }
         continue;
      }

      for (auto i = 0; i < _mobilePositionsOccupied.size(); ++i)
      {
         if (_mobilePositionsOccupied[i].intersects(protoMask))
            _mobilePositionsOccupied[i].set(newTautomerIndex);
      }

      ++layers;
      mask_.andNotWith(protoMask);
      newTautomersMask.set(newTautomerIndex);
   }

   order = forward? 2: 1;
   for (auto i = 0; i < path.size(); ++i)
   {
      _bond_masks[order][path[i]].orWith(newTautomersMask);
      order = order == 1 ? 2 : 1;
   }

   _mobilePositionsOccupied[forward ? beg : end].andNotWith(newTautomersMask);
   _mobilePositionsOccupied[forward ? end : beg].orWith(newTautomersMask);
}

int HyperMolecule::getAtomNumber(int idx)
{
   return _proto.getAtomNumber(idx);
}

int HyperMolecule::getAtomCharge(int idx)
{
   return _proto.getAtomCharge(idx);
}

int HyperMolecule::getAtomIsotope(int idx)
{
   return _proto.getAtomIsotope(idx);
}

int HyperMolecule::getAtomRadical(int idx)
{
   return _proto.getAtomRadical(idx);
}

int HyperMolecule::getAtomAromaticity(int idx)
{
   return _proto.getAtomAromaticity(idx);
}

int HyperMolecule::getExplicitValence(int idx)
{
   return _proto.getExplicitValence(idx);
}

int HyperMolecule::getAtomValence(int idx)
{
   return _proto.getAtomValence(idx);
}

int HyperMolecule::getAtomSubstCount(int idx)
{
   return _proto.getAtomSubstCount(idx);
}

int HyperMolecule::getAtomRingBondsCount(int idx)
{
   return _proto.getAtomSubstCount(idx);
}

int HyperMolecule::getAtomMaxH(int idx)
{
   return getAtomTotalH(idx);
}

int HyperMolecule::getAtomMinH(int idx)
{
   return getAtomTotalH(idx);
}

int HyperMolecule::getAtomTotalH(int idx)
{
   throw Error("getAtomTotalH method has no sense for LayeredMolecules");
}

bool HyperMolecule::isPseudoAtom(int idx)
{
   return _proto.isPseudoAtom(idx);
}

const char * HyperMolecule::getPseudoAtom(int idx)
{
   return _proto.getPseudoAtom(idx);
}

bool HyperMolecule::isRSite(int idx)
{
   return _proto.isRSite(idx);
}

dword HyperMolecule::getRSiteBits(int idx)
{
   return _proto.getRSiteBits(idx);
}

void HyperMolecule::allowRGroupOnRSite(int atom_idx, int rg_idx)
{
   throw Error("allowRGroupOnRSite method is not implemented in LayeredMolecules class");
}

int HyperMolecule::getBondOrder(int idx)
{
   throw Error("getBondOrder method has no sense for LayeredMolecules");
}

int HyperMolecule::getBondTopology(int idx)
{
   throw Error("getBondTopology method is not implemented in LayeredMolecules class");
}

bool HyperMolecule::atomNumberBelongs(int idx, const int *numbers, int count)
{
   return _proto.atomNumberBelongs(idx, numbers, count);
}

bool HyperMolecule::possibleAtomNumber(int idx, int number)
{
   return _proto.possibleAtomNumber(idx, number);
}

bool HyperMolecule::possibleAtomNumberAndCharge(int idx, int number, int charge)
{
   return _proto.possibleAtomNumberAndCharge(idx, number, charge);
}

bool HyperMolecule::possibleAtomNumberAndIsotope(int idx, int number, int isotope)
{
   return _proto.possibleAtomNumberAndIsotope(idx, number, isotope);
}

bool HyperMolecule::possibleAtomIsotope(int idx, int isotope)
{
   return _proto.possibleAtomIsotope(idx, isotope);
}

bool HyperMolecule::possibleAtomCharge(int idx, int charge)
{
   return _proto.possibleAtomCharge(idx, charge);
}

void HyperMolecule::getAtomDescription(int idx, Array<char> &description)
{
   return _proto.getAtomDescription(idx, description);
}

void HyperMolecule::getBondDescription(int idx, Array<char> &description)
{
   throw Error("getBondDescription method is not implemented in LayeredMolecules class");
}

bool HyperMolecule::possibleBondOrder(int idx, int order)
{
   throw Error("possibleBondOrder method has no sense for LayeredMolecules");
}

bool HyperMolecule::isSaturatedAtom(int idx)
{
   throw Error("isSaturatedAtom method is not implemented in LayeredMolecules class");
}

bool HyperMolecule::bondStereoCare(int idx)
{
   throw Error("bondStereoCare method is not implemented in LayeredMolecules class");
}

bool HyperMolecule::aromatize(const AromaticityOptions &options)
{
   return _proto.aromatize(options);
}

bool HyperMolecule::dearomatize(const AromaticityOptions &options)
{
   return _proto.dearomatize(options);
}

void HyperMolecule::_mergeWithSubmolecule(BaseMolecule &bmol, const Array<int> &vertices,
   const Array<int> *edges, const Array<int> &mapping, int skip_flags)
{
   throw Error("_mergeWithSubmolecule method is not implemented in LayeredMolecules class");
}

BaseMolecule * HyperMolecule::neu ()
{
   throw Error("neu method is not implemented in LayeredMolecules class");
}
