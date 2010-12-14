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

#ifndef __molecule_arom_h__
#define __molecule_arom_h__

#include "base_cpp/tlscont.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4251)
#endif

namespace indigo {

class Graph;
class Molecule;
class QueryMolecule;
class BaseMolecule;

// Aromatization classes
class DLLEXPORT AromatizerBase
{
public:
   explicit AromatizerBase (BaseMolecule &molecule);
   virtual ~AromatizerBase ();

   void aromatize ();
   void reset     (void);

   bool isBondAromatic      (int e_idx);
   const byte* isBondAromaticArray (void);

   void addAromaticCycle    (int id, const int *cycle, int cycle_len);
   void removeAromaticCycle (int id, const int *cycle, int cycle_len);
   bool handleUnsureCycles  ();

   void setBondAromaticCount (int e_idx, int count);

   DEF_ERROR("aromatizer");
protected:
   // Functions for overloading
   virtual bool _checkVertex         (int v_idx);
   virtual bool _isCycleAromatic     (const int *cycle, int cycle_len) = 0;
   virtual void _handleAromaticCycle (const int *cycle, int cycle_len);

protected:

   enum { MAX_CYCLE_LEN = 22 };

   struct CycleDef
   {
      int   id;
      bool  is_empty;
      int   length;
      int   cycle[MAX_CYCLE_LEN];
   };

   BaseMolecule &_basemol;

   TL_CP_DECL(Array<byte>,      _bonds_arom);
   TL_CP_DECL(Array<int>,       _bonds_arom_count);
   TL_CP_DECL(Array<CycleDef>,  _unsure_cycles);

   bool _checkDoubleBonds     (const int *cycle, int cycle_len);
   void _aromatizeCycle       (const int *cycle, int cycle_len);
   void _handleCycle          (const Array<int> &vertices);

   static bool _cb_check_vertex (Graph &graph, int v_idx, void *context);
   static bool _cb_handle_cycle (Graph &graph, const Array<int> &vertices, const Array<int> &edges, void *context);

   int _cyclesHandled;
   int _unsureCyclesCount;
};

class DLLEXPORT MoleculeAromatizer : public AromatizerBase
{
public:
   // Interface function for aromatization
   static void aromatizeBonds (Molecule &mol);

   MoleculeAromatizer (Molecule &molecule);
   void precalculatePiLabels ();

   static void findAromaticAtoms (BaseMolecule &mol, Array<int> *atoms, Array<int> *bonds);

protected:
   virtual bool _checkVertex      (int v_idx);
   virtual bool _isCycleAromatic  (const int *cycle, int cycle_len);

   int _getPiLabel (int v_idx);
   TL_CP_DECL(Array<int>, _pi_labels);
};

class QueryMoleculeAromatizer : public AromatizerBase
{
public:
   // Interface function for query molecule aromatization
   static void aromatizeBonds (QueryMolecule &mol);

   enum { EXACT, FUZZY };

   explicit QueryMoleculeAromatizer (QueryMolecule &molecule);

   void setMode              (int mode);
   void precalculatePiLabels ();

protected:
   struct PiValue 
   {
      PiValue () {}
      PiValue (int min, int max) : min(min), max(max) {}

      bool canBeAromatic () { return min != -1; }

      int min, max; 
   };

   virtual bool _checkVertex         (int v_idx);
   virtual bool _isCycleAromatic     (const int *cycle, int cycle_len);
   virtual void _handleAromaticCycle (const int *cycle, int cycle_len);

   static void _aromatizeBondsExact (QueryMolecule &mol);
   static void _aromatizeBondsFuzzy (QueryMolecule &mol);

   static void _aromatizeBonds (QueryMolecule &mol, int additional_atom);

   static void _aromatizeRGroupFragment (QueryMolecule &fragment, bool add_single_bonds);

   PiValue _getPiLabel           (int v_idx);

   TL_CP_DECL(Array<PiValue>,   _pi_labels);
   TL_CP_DECL(Array<CycleDef>,  _aromatic_cycles);

   int _mode;
   bool _collecting;
};

// Structure that keeps query infromation abount bonds that 
// can be aromatic in the substructure search.
class DLLEXPORT QueryMoleculeAromaticity
{
public:
   bool canBeAromatic (int edge_index) const;
   void setCanBeAromatic (int edge_index, bool state);
   void clear();

private:
   Array<bool> can_bond_be_aromatic;
};

}

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
