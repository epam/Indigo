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

#ifndef __base_molecule__
#define __base_molecule__

#include "graph/graph.h"
#include "base_cpp/red_black.h"
#include "math/algebra.h"
#include "molecule/molecule_stereocenters.h"
#include "molecule/molecule_cis_trans.h"
#include "molecule/molecule_allene_stereo.h"
#include "base_cpp/obj_array.h"
#include "molecule/molecule_rgroups.h"
#include "molecule/molecule_arom.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4251)
#endif

namespace indigo
{

enum
{
   CHARGE_UNKNOWN = -100
};

enum
{
   ATOM_AROMATIC = 1,
   ATOM_ALIPHATIC = 2
};

enum
{
   BOND_SINGLE = 1,
   BOND_DOUBLE = 2,
   BOND_TRIPLE = 3,
   BOND_AROMATIC = 4
};

enum
{
   BOND_UP = 1,
   BOND_DOWN = 2,
   BOND_EITHER = 3
};


// Flags that disables copying information in making submolecule,
// merging with molecule and cloning procedures
enum
{
   SKIP_ALL = -1,
   SKIP_CIS_TRANS = 0x01,
   SKIP_STEREOCENTERS = 0x02,
   SKIP_XYZ = 0x04,
   SKIP_RGROUP_FRAGMENTS = 0x08,
   SKIP_ATTACHMENT_POINTS = 0x16
};

class Molecule;
class QueryMolecule;

class DLLEXPORT BaseMolecule : public Graph
{
public:
   class DLLEXPORT SGroup
   {
   public:
      Array<int> atoms; // represented with SAL in Molfile format
      Array<int> bonds; // represented with SBL in Molfile format
      Array<Vec2f[2]> brackets;
      virtual ~SGroup ();
   };

   typedef RedBlackMap<int,int> Mapping;
   class DLLEXPORT DataSGroup : public SGroup
   {
   public:
      DataSGroup ();
      virtual ~DataSGroup ();

      Array<char> description; // SDT in Molfile format
      Array<char> data;        // SCD/SED in Molfile format
      Vec2f       display_pos; // SDD in Molfile format
      bool        detached;    // or attached
      bool        relative;    // or absolute
      bool        display_units;
      int         dasp_pos;
   };

   class DLLEXPORT Superatom : public SGroup
   {
   public:
      Superatom ();
      virtual ~Superatom ();

      Array<char> subscript; // SMT in Molfile format
      int   bond_idx;        // bond index (-1 if absent); SBV in Molfile format
      Vec2f bond_dir;        // bond direction
   };

   class DLLEXPORT RepeatingUnit : public SGroup
   {
   public:
      enum
      {
         HEAD_TO_HEAD = 1,
         HEAD_TO_TAIL,
         EITHER
      };

      RepeatingUnit ();
      virtual ~RepeatingUnit ();

      int connectivity;
      Array<char> subscript; // SMT in Molfile format
   };

   class DLLEXPORT MultipleGroup : public SGroup
   {
   public:
      MultipleGroup ();
      virtual ~MultipleGroup ();
      static void collapse (BaseMolecule& bm, int id, Mapping& mapAtom, Mapping& mapBondInv);
      static void collapse (BaseMolecule& bm, int id);
      static void collapse (BaseMolecule& bm);

      Array<int> parent_atoms;
      int multiplier;
   };


   BaseMolecule ();
   virtual ~BaseMolecule ();

   // Casting methods. Invalid casting throws exceptions.
   virtual Molecule& asMolecule ();
   virtual QueryMolecule& asQueryMolecule ();
   virtual bool isQueryMolecule ();

   virtual void clear ();

   // 'neu' means 'new' in German
   virtual BaseMolecule * neu () = 0;

   virtual int getAtomNumber         (int idx) = 0; // > 0 -- ELEM_***, 0 -- pseudo-atom, -1 -- not sure
   virtual int getAtomCharge         (int idx) = 0; // charge or CHARGE_UNKNOWN if not sure
   virtual int getAtomIsotope        (int idx) = 0; // > 0 -- isotope, -1 -- not sure
   virtual int getAtomRadical        (int idx) = 0; // > 0 -- RADICAL_***, -1 -- not sure
   virtual int getAtomAromaticity    (int idx) = 0; // ATOM_AROMATIC, ATOM_ALIPHATIC, or -1 -- not sure
   virtual int getExplicitValence    (int idx) = 0; // explicit valence or -1 if not set
   virtual int getAtomValence        (int idx) = 0; // >= 0 -- valence, -1 is not set explicitly
   virtual int getAtomSubstCount     (int idx) = 0;
   virtual int getAtomRingBondsCount (int idx) = 0; // >= 0 -- ring bonds count, -1 -- not sure

   int getAtomRadical_NoThrow (int idx, int fallback);
   int getAtomValence_NoThrow (int idx, int fallback);

   virtual int getAtomMaxH   (int idx) = 0;
   virtual int getAtomMinH   (int idx) = 0;
   virtual int getAtomTotalH (int idx) = 0;

   int possibleAtomTotalH (int idx, int hcount);

   virtual bool isPseudoAtom (int idx) = 0;
   virtual const char * getPseudoAtom (int idx) = 0;

   int countRSites ();
   int countSGroups ();

   virtual bool  isRSite (int atom_idx) = 0;
   virtual dword getRSiteBits (int atom_idx) = 0;
   virtual void  allowRGroupOnRSite (int atom_idx, int rg_idx) = 0;

   void getAllowedRGroups (int atom_idx, Array<int> &rgroup_list);
   int  getSingleAllowedRGroup (int atom_idx);
   int  getRSiteAttachmentPointByOrder (int idx, int order) const;
   void setRSiteAttachmentOrder (int atom_idx, int att_atom_idx, int order);

   void addAttachmentPoint (int order, int atom_index);
   int  getAttachmentPoint (int order, int index) const;
   void removeAttachmentPointsFromAtom (int atom_index);
   int  attachmentPointCount () const;
   void removeAttachmentPoints ();

   void getAttachmentIndicesForAtom (int atom_idx, Array<int> &res);

   virtual bool isSaturatedAtom    (int idx) = 0;

   virtual int  getBondOrder      (int idx) = 0; // > 0 -- BOND_***, -1 -- not sure
   virtual int  getBondTopology   (int idx) = 0; // > 0 -- TOPOLOGY_***, -1 -- not sure

   // true if the atom number belongs to the given list, false otherwise
   virtual bool atomNumberBelongs (int idx, const int *numbers, int count) = 0;

   // true if the atom can have that number, false otherwise
   virtual bool possibleAtomNumber (int idx, int number) = 0;

   // true if the atom can have that number and that charge, false otherwise
   virtual bool possibleAtomNumberAndCharge (int idx, int number, int charge) = 0;

   // true if the atom can have that number and that charge, false otherwise
   virtual bool possibleAtomNumberAndIsotope (int idx, int number, int isotope) = 0;

   // true if the atom can have that isotope index, false otherwise
   virtual bool possibleAtomIsotope (int idx, int isotope) = 0;

   // true if the atom can have that isotope index, false otherwise
   virtual bool possibleAtomCharge (int idx, int charge) = 0;

   // human-readable atom and bond desciptions for diagnostic purposes
   virtual void getAtomDescription (int idx, Array<char> &description) = 0;
   virtual void getBondDescription (int idx, Array<char> &description) = 0;

   // true if the bond can be that order, false otherwise
   virtual bool possibleBondOrder (int idx, int order) = 0;

   // true if bond stereoconfiguration is important
   virtual bool bondStereoCare (int idx) = 0;

   // Returns true if some bonds were changed
   virtual bool aromatize (const AromaticityOptions &options) = 0;
   // Returns true if all bonds were dearomatized
   virtual bool dearomatize (const AromaticityOptions &options) = 0;

   enum
   {
      CHANGED_ATOM_NUMBER = 0x01,
      CHANGED_CONNECTIVITY = 0x02,
      CHANGED_ALL = 0xFF,
   };
   virtual void invalidateAtom (int index, int mask);

   Vec3f & getAtomXyz (int idx);
   void setAtomXyz (int idx, float x, float y, float z);
   void setAtomXyz (int idx, const Vec3f& v);

   void clearXyz ();

   MoleculeStereocenters stereocenters;
   MoleculeCisTrans cis_trans;
   MoleculeAlleneStereo allene_stereo;

   bool have_xyz;

   bool isChrial ();

   // TODO: use a single array
   ObjPool<DataSGroup> data_sgroups;
   ObjPool<Superatom>  superatoms;
   ObjPool<RepeatingUnit> repeating_units;
   ObjPool<MultipleGroup> multiple_groups;
   ObjPool<SGroup> generic_sgroups;

   MoleculeRGroups rgroups;
   
   Array<char> name;

   static bool hasCoord (BaseMolecule &mol);
   static bool hasZCoord (BaseMolecule &mol);

   void mergeWithSubmolecule (BaseMolecule &mol, const Array<int> &vertices,
                              const Array<int> *edges, Array<int> *mapping_out,
                              int skip_flags = 0);

   int mergeAtoms (int atom1, int atom2);

   void flipBond (int atom_parent, int atom_from, int atom_to);

   void makeSubmolecule (BaseMolecule &mol, const Array<int> &vertices,
                         Array<int> *mapping_out, int skip_flags = 0);
   void makeSubmolecule (BaseMolecule &other, const Filter &filter,
                         Array<int> *mapping_out, Array<int> *inv_mapping,
                         int skip_flags = 0);
   void makeEdgeSubmolecule (BaseMolecule &mol, const Array<int> &vertices,
                             const Array<int> &edges, Array<int> *v_mapping,
                             int skip_flags = 0);

   void clone (BaseMolecule &other, Array<int> *mapping, Array<int> *inv_mapping, int skip_flags = 0);

   // This is a bad hack for those who are too lazy to handle the mappings.
   // NEVER USE IT.
   void clone_KeepIndices (BaseMolecule &other, int skip_flags = 0);

   void mergeWithMolecule (BaseMolecule &other, Array<int> *mapping, int skip_flags = 0);

   void removeAtoms (const Array<int> &indices);
   void removeAtoms (const Filter &filter);
   void removeAtom  (int idx);
   void removeBonds (const Array<int> &indices);
   void removeBond  (int idx);

   void unhighlightAll ();
   void highlightAtom (int idx);
   void highlightBond (int idx);
   void highlightAtoms (const Filter &filter);
   void highlightBonds (const Filter &filter);
   void unhighlightAtom (int idx);
   void unhighlightBond (int idx);
   int countHighlightedAtoms ();
   int countHighlightedBonds ();
   bool hasHighlighting ();
   bool isAtomHighlighted (int idx);
   bool isBondHighlighted (int idx);
   void highlightSubmolecule (BaseMolecule &sub, const int *mapping, bool entire);

   static int getVacantPiOrbitals (int group, int charge, int radical, int conn, int *lonepairs_out);

   // Returns edit revision for this molecule.
   // Each time molecule is changed revision number is increased.
   // If revision number is the same then molecule hasn't been changed.
   int getEditRevision ();
   // Manually update edit revision. This is required when molecule is changed
   // directly without calling molecule methods (for example mol.cis_trans.clear() and etc.)
   void updateEditRevision ();

   void clearBondDirections ();
   int  getBondDirection (int idx) const;
   void setBondDirection (int idx, int dir);

   int  getBondDirection2 (int center_idx, int nei_idx);

   void mergeSGroupsWithSubmolecule (BaseMolecule &mol, Array<int> &mapping);

   void mergeSGroupsWithSubmolecule (BaseMolecule &mol, Array<int> &mapping, Array<int> &edge_mapping);
   void clearSGroups();

   void getSGroupAtomsCenterPoint (SGroup &sgroup, Vec2f &res);

   void getAtomSymbol (int v, Array<char> &output);

   DECL_ERROR;
protected:

   void _mergeWithSubmolecule_Sub (BaseMolecule &mol, const Array<int> &vertices,
                                   const Array<int> *edges, Array<int> &mapping,
                                   Array<int> &edge_mapping, int skip_flags);

   void _flipSGroupBond(SGroup &sgroup, int src_bond_idx, int new_bond_idx);
   
   virtual void _mergeWithSubmolecule (BaseMolecule &mol, const Array<int> &vertices,
           const Array<int> *edges, const Array<int> &mapping, int skip_flags) = 0;

   virtual void _postMergeWithSubmolecule (BaseMolecule &mol, const Array<int> &vertices,
           const Array<int> *edges, const Array<int> &mapping, int skip_flags);

   virtual void _flipBond (int atom_parent, int atom_from, int atom_to);

   virtual void _removeAtoms (const Array<int> &indices, const int *mapping);
   virtual void _removeBonds (const Array<int> &indices);

   int _addBaseAtom ();
   int _addBaseBond (int beg, int end);

   void _removeAtomsFromSGroup (SGroup &sgroup, Array<int> &indices);
   void _removeAtomsFromMultipleGroup (MultipleGroup &mg, Array<int> &mapping);
   bool _mergeSGroupWithSubmolecule (SGroup &sgroup, SGroup &super, BaseMolecule &supermol,
        Array<int> &mapping, Array<int> &edge_mapping);

   Array<int> _hl_atoms;
   Array<int> _hl_bonds;
   Array<int> _bond_directions;

   Array<Vec3f> _xyz;
   ObjArray< Array<int> > _rsite_attachment_points;
   bool _rGroupFragment;

   ObjArray< Array<int> > _attachment_index;

   // When molecule gets edited then edit revision is increased.
   // If edit revision is the same then molecule wasn't edited
   int _edit_revision;
};

}

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
