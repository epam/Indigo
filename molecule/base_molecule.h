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

#ifndef __base_molecule__
#define __base_molecule__

#include "graph/graph.h"
#include "base_cpp/red_black.h"
#include "molecule/molecule_stereocenters.h"
#include "math/algebra.h"
#include "molecule/molecule_cis_trans.h"
#include "base_cpp/obj_array.h"

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

// Flags that disables copying information in making submolecule, 
// merging with molecule and cloning procedures
enum
{
   SKIP_ALL = -1,
   SKIP_CIS_TRANS = 0x01,
   SKIP_STEREOCENTERS = 0x02,
   SKIP_XYZ = 0x04,
};

class Molecule;
class QueryMolecule;

class BaseMolecule : public Graph
{
public:
   DLLEXPORT BaseMolecule ();
   DLLEXPORT virtual ~BaseMolecule ();

   // Casting methods. Invalid casting throws exceptions.
   DLLEXPORT virtual Molecule& asMolecule ();
   DLLEXPORT virtual QueryMolecule& asQueryMolecule ();
   DLLEXPORT virtual bool isQueryMolecule ();

   DLLEXPORT virtual void clear ();

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

   virtual int getAtomMaxH   (int idx) = 0;
   virtual int getAtomMinH   (int idx) = 0;
   virtual int getAtomTotalH (int idx) = 0;

   int possibleAtomTotalH (int idx, int hcount);

   virtual bool isPseudoAtom (int idx) = 0;
   virtual const char * getPseudoAtom (int idx) = 0;

   int countRSites ();

   virtual bool isRSite (int atom_idx) = 0;
   virtual int  getRSiteBits (int atom_idx) = 0;
   virtual void allowRGroupOnRSite (int atom_idx, int rg_idx) = 0;

   DLLEXPORT void getAllowedRGroups (int atom_idx, Array<int> &rgroup_list);
   DLLEXPORT int  getSingleAllowedRGroup (int atom_idx);
   DLLEXPORT int  getRSiteAttachmentPointByOrder (int idx, int order) const;
   DLLEXPORT void setRSiteAttachmentOrder (int atom_idx, int att_atom_idx, int order);

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

   virtual void aromatize () = 0;
   virtual void dearomatize () = 0;

   DLLEXPORT Vec3f & getAtomXyz (int idx);
   DLLEXPORT void setAtomXyz (int idx, float x, float y, float z);

   MoleculeStereocenters stereocenters;
   MoleculeCisTrans cis_trans;

   bool have_xyz;

   Array<char> name;

   DLLEXPORT static bool hasZCoord (BaseMolecule &mol);

   DLLEXPORT void mergeWithSubmolecule (BaseMolecule &mol, const Array<int> &vertices, 
                              const Array<int> *edges, Array<int> *mapping_out,
                              int skip_flags = 0);

   DLLEXPORT int mergeAtoms (int atom1, int atom2);

   DLLEXPORT void flipBond (int atom_parent, int atom_from, int atom_to);

   DLLEXPORT void makeSubmolecule (BaseMolecule &mol, const Array<int> &vertices, 
                         Array<int> *mapping_out, int skip_flags = 0);
   DLLEXPORT void makeSubmolecule (BaseMolecule &other, const Filter &filter,
                         Array<int> *mapping_out, Array<int> *inv_mapping,
                         int skip_flags = 0);
   DLLEXPORT void makeEdgeSubmolecule (BaseMolecule &mol, const Array<int> &vertices, 
                             const Array<int> &edges, Array<int> *v_mapping,
                             int skip_flags = 0);
   
   DLLEXPORT void clone (BaseMolecule &other, Array<int> *mapping, Array<int> *inv_mapping, int skip_flags = 0);

   DLLEXPORT void mergeWithMolecule (BaseMolecule &other, Array<int> *mapping, int skip_flags = 0);

   DLLEXPORT void removeAtoms (const Array<int> &indices);
   DLLEXPORT void removeAtoms (const Filter &filter);
   DLLEXPORT void removeAtom  (int idx);
   DLLEXPORT void removeBonds (const Array<int> &indices);
   DLLEXPORT void removeBond  (int idx);

   DLLEXPORT static int getVacantPiOrbitals (int group, int charge, int radical, int conn, int *lonepairs_out);

   DEF_ERROR("molecule");
protected:

   virtual void _mergeWithSubmolecule (BaseMolecule &mol, const Array<int> &vertices,
           const Array<int> *edges, const Array<int> &mapping, int skip_flags);

   virtual void _postMergeWithSubmolecule (BaseMolecule &mol, const Array<int> &vertices,
           const Array<int> *edges, const Array<int> &mapping, int skip_flags);

   virtual void _flipBond (int atom_parent, int atom_from, int atom_to);

   virtual void _removeAtoms (const Array<int> &indices, const int *mapping);
   virtual void _removeBonds (const Array<int> &indices);

   int _addBaseAtom ();
   int _addBaseBond (int beg, int end);

   Array<Vec3f> _xyz;
   ObjArray< Array<int> > _rsite_attachment_points;
};

}

#endif
