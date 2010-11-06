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

#include "molecule/molecule_stereocenters.h"
#include "molecule/base_molecule.h"
#include "graph/filter.h"
#include "molecule/molecule.h"
#include "molecule/elements.h"

using namespace indigo;

MoleculeStereocenters::MoleculeStereocenters ()
{
}

BaseMolecule & MoleculeStereocenters::_getMolecule () const
{
   char dummy[sizeof(BaseMolecule)];

   int offset = (int)((char *)(&((BaseMolecule *)dummy)->stereocenters) - dummy);

   return *(BaseMolecule *)((char *)this - offset);
}

void MoleculeStereocenters::clear ()
{
   _bond_directions.clear();
   _stereocenters.clear();
}

void MoleculeStereocenters::buildFromBonds (const int *atom_types, const int *atom_groups, const int *bond_orientations, bool ignore_errors)
{
   const BaseMolecule &mol = _getMolecule();
   _bond_directions.copy(bond_orientations, mol.edgeEnd());

   for (int atom_idx = mol.vertexBegin(); atom_idx != mol.vertexEnd();
            atom_idx = mol.vertexNext(atom_idx))
   {
      if (atom_types[atom_idx] == 0)
         continue;

      if (ignore_errors)
      {
         try
         {
            _buildOneCenter(atom_idx, atom_groups[atom_idx], atom_types[atom_idx], bond_orientations);
         }
         catch (Error &)
         {
         }
      }
      else
         _buildOneCenter(atom_idx, atom_groups[atom_idx], atom_types[atom_idx], bond_orientations);
   }
}

void MoleculeStereocenters::_buildOneCenter (int atom_idx, int group, int type, const int *bond_orientations)
{
   BaseMolecule &mol = _getMolecule();

   const Vertex &vertex = mol.getVertex(atom_idx);

   int degree = vertex.degree();

   _Atom stereocenter;

   stereocenter.group = group;
   stereocenter.type = type;

   int *pyramid = stereocenter.pyramid;
   int nei_idx = 0;
   _EdgeIndVec edge_ids[4];

   int last_atom_dir = 0;
   int sure_double_bonds = 0;
   int possible_double_bonds = 0;

   pyramid[0] = -1;
   pyramid[1] = -1;
   pyramid[2] = -1;
   pyramid[3] = -1;

   int n_pure_hydrogens = 0;

   if (degree > 4)
      throw Error("stereocenter with %d bonds are not supported", degree);

   for (int i = vertex.neiBegin(); i != vertex.neiEnd(); i = vertex.neiNext(i))
   {
      int e_idx = vertex.neiEdge(i);
      int v_idx = vertex.neiVertex(i);

      edge_ids[nei_idx].edge_idx = e_idx;
      edge_ids[nei_idx].nei_idx = v_idx;

      if (mol.possibleAtomNumberAndIsotope(v_idx, ELEM_H, 0))
      {
         if (mol.getAtomNumber(v_idx) == ELEM_H && mol.getAtomIsotope(v_idx) == 0)
            n_pure_hydrogens++;
         edge_ids[nei_idx].rank = 10000;
      }
      else
         edge_ids[nei_idx].rank = v_idx;

      edge_ids[nei_idx].vec.diff(mol.getAtomXyz(v_idx), mol.getAtomXyz(atom_idx));

      if (!edge_ids[nei_idx].vec.normalize())
         throw Error("zero bond length");

      if (mol.getBondOrder(e_idx) == BOND_TRIPLE)
         throw Error("non-single bonds not allowed near stereocenter");
      if (mol.getBondOrder(e_idx) == BOND_AROMATIC)
         throw Error("aromatic bonds not allowed near stereocenter");

      if (mol.getBondOrder(e_idx) == BOND_DOUBLE)
         sure_double_bonds++;
      else if (mol.possibleBondOrder(e_idx, BOND_DOUBLE))
         possible_double_bonds++;

      nei_idx++;
   }

   _EdgeIndVec tmp;

   static const _Configuration allowed_stereocenters [] =
   {
      // element, charge, degree, double bonds, implicit degree
      {ELEM_C,  0, 3, 0, 4},
      {ELEM_C,  0, 4, 0, 4},
      {ELEM_Si, 0, 3, 0, 4},
      {ELEM_Si, 0, 4, 0, 4},
      {ELEM_N,  1, 3, 0, 4},
      {ELEM_N,  1, 4, 0, 4},
      {ELEM_N,  0, 3, 0, 3},
      {ELEM_S,  0, 4, 2, 4},
      {ELEM_S,  1, 3, 0, 3},
      {ELEM_S,  1, 4, 1, 4},
      {ELEM_S,  0, 3, 1, 3},
      {ELEM_P,  0, 3, 0, 3},
      {ELEM_P,  1, 4, 0, 4},
      {ELEM_P,  0, 4, 1, 4},
   };

   bool possible = false;
   bool possible_implicit_h = false;
   bool possible_lone_pair = false;
   int i;

   for (i = 0; i < (int)NELEM(allowed_stereocenters); i++)
   {
      const _Configuration & as = allowed_stereocenters[i];

      if (as.degree != degree)
         continue;

      if (as.n_double_bonds < sure_double_bonds ||
          as.n_double_bonds > sure_double_bonds + possible_double_bonds)
         continue;

      if (!mol.possibleAtomNumberAndCharge(atom_idx, as.elem, as.charge))
         continue;

      possible = true;

      if (as.implicit_degree == 4 && degree == 3)
         possible_implicit_h = true;
      
      if (as.implicit_degree == 3)
         possible_lone_pair = true;
   }

   if (!possible)
   {
      QS_DEF(Array<char>, desc);
      mol.getAtomDescription(atom_idx, desc);

      if (possible_double_bonds == 0)
         throw Error("unknown stereocenter configuration: %s, %d bonds (%d double)",
            desc.ptr(), degree, sure_double_bonds);
      else
         throw Error("unknown stereocenter configuration: %s, %d bonds (%d-%d double)",
            desc.ptr(), degree, sure_double_bonds, sure_double_bonds + possible_double_bonds);
   }

   if (degree == 4 && n_pure_hydrogens > 1)
      throw Error("%d hydrogens near stereocenter", n_pure_hydrogens);

   if (degree == 3 && n_pure_hydrogens > 0 && !possible_lone_pair)
      throw Error("have hydrogen(s) besides implicit hydrogen near stereocenter");

   if (stereocenter.type == ATOM_ANY)
   {
      for (i = 0; i < nei_idx; i++)
         stereocenter.pyramid[i] = edge_ids[i].nei_idx;
      _stereocenters.insert(atom_idx, stereocenter);
      return;
   }

   if (degree == 4)
   {
      // sort by neighbor atom index (ascending)
      if (edge_ids[0].rank > edge_ids[1].rank)
         __swap(edge_ids[0], edge_ids[1], tmp);
      if (edge_ids[1].rank > edge_ids[2].rank)
         __swap(edge_ids[1], edge_ids[2], tmp);
      if (edge_ids[2].rank > edge_ids[3].rank)
         __swap(edge_ids[2], edge_ids[3], tmp);
      if (edge_ids[1].rank > edge_ids[2].rank)
         __swap(edge_ids[1], edge_ids[2], tmp);
      if (edge_ids[0].rank > edge_ids[1].rank)
         __swap(edge_ids[0], edge_ids[1], tmp);
      if (edge_ids[1].rank > edge_ids[2].rank)
         __swap(edge_ids[1], edge_ids[2], tmp);

      int main1 = -1, main2 = -1, side1 = -1, side2 = -1;
      int main_dir = 0;

      for (nei_idx = 0; nei_idx < 4; nei_idx++)
      {
         int stereo = _getBondStereo(atom_idx, edge_ids[nei_idx].nei_idx);

         if (stereo == BOND_UP || stereo == BOND_DOWN)
         {
            main1 = nei_idx;
            main_dir = stereo;
            break;
         }
      }

      if (main1 == -1)
         throw Error("none of 4 bonds going from stereocenter is stereobond");

      int xyz1, xyz2;

      // find main2 as opposite to main1
      if (main2 == -1)
      {
         xyz1 = _xyzzy(edge_ids[main1].vec, edge_ids[(main1 + 1) % 4].vec, edge_ids[(main1 + 2) % 4].vec);
         xyz2 = _xyzzy(edge_ids[main1].vec, edge_ids[(main1 + 1) % 4].vec, edge_ids[(main1 + 3) % 4].vec);

         if (xyz1 + xyz2 == 3 || xyz1 + xyz2 == 12)
         {
            main2 = (main1 + 1) % 4;
            side1 = (main1 + 2) % 4;
            side2 = (main1 + 3) % 4;
         }
      }
      if (main2 == -1)
      {
         xyz1 = _xyzzy(edge_ids[main1].vec, edge_ids[(main1 + 2) % 4].vec, edge_ids[(main1 + 1) % 4].vec);
         xyz2 = _xyzzy(edge_ids[main1].vec, edge_ids[(main1 + 2) % 4].vec, edge_ids[(main1 + 3) % 4].vec);

         if (xyz1 + xyz2 == 3 || xyz1 + xyz2 == 12)
         {
            main2 = (main1 + 2) % 4;
            side1 = (main1 + 1) % 4;
            side2 = (main1 + 3) % 4;
         }
      }
      if (main2 == -1)
      {
         xyz1 = _xyzzy(edge_ids[main1].vec, edge_ids[(main1 + 3) % 4].vec, edge_ids[(main1 + 1) % 4].vec);
         xyz2 = _xyzzy(edge_ids[main1].vec, edge_ids[(main1 + 3) % 4].vec, edge_ids[(main1 + 2) % 4].vec);

         if (xyz1 + xyz2 == 3 || xyz1 + xyz2 == 12)
         {
            main2 = (main1 + 3) % 4;
            side1 = (main1 + 2) % 4;
            side2 = (main1 + 1) % 4;
         }
      }

      if (main2 == -1)
         throw Error("internal error: can not find opposite bond");

      if (main_dir == BOND_UP && _getBondStereo(atom_idx, edge_ids[main2].nei_idx) == BOND_DOWN)
         throw Error("stereo types of the opposite bonds mismatch");
      if (main_dir == BOND_DOWN && _getBondStereo(atom_idx, edge_ids[main2].nei_idx) == BOND_UP)
         throw Error("stereo types of the opposite bonds mismatch");

      if (main_dir == _getBondStereo(atom_idx, edge_ids[side1].nei_idx))
         throw Error("stereo types of non-opposite bonds match");
      if (main_dir == _getBondStereo(atom_idx, edge_ids[side2].nei_idx))
         throw Error("stereo types of non-opposite bonds match");

      if (main1 == 3 || main2 == 3)
         last_atom_dir = main_dir;
      else
         last_atom_dir = (main_dir == BOND_UP ? BOND_DOWN : BOND_UP);

      int sign = _sign(edge_ids[0].vec, edge_ids[1].vec, edge_ids[2].vec);

      if ((last_atom_dir == BOND_UP && sign > 0) ||
          (last_atom_dir == BOND_DOWN && sign < 0))
      {
         pyramid[0] = edge_ids[0].nei_idx;
         pyramid[1] = edge_ids[1].nei_idx;
         pyramid[2] = edge_ids[2].nei_idx;
      }
      else
      {
         pyramid[0] = edge_ids[0].nei_idx;
         pyramid[1] = edge_ids[2].nei_idx;
         pyramid[2] = edge_ids[1].nei_idx;
      }

      pyramid[3] = edge_ids[3].nei_idx;
   }
   else if (degree == 3)
   {
      // sort by neighbor atom index (ascending)
      if (edge_ids[0].rank > edge_ids[1].rank)
         __swap(edge_ids[0], edge_ids[1], tmp);
      if (edge_ids[1].rank > edge_ids[2].rank)
         __swap(edge_ids[1], edge_ids[2], tmp);
      if (edge_ids[0].rank > edge_ids[1].rank)
         __swap(edge_ids[0], edge_ids[1], tmp);

      int n_up = 0, n_down = 0;
      bool invert = false;
      bool not_invert = false;

      for (nei_idx = 0; nei_idx < 3; nei_idx++)
      {
         int stereo = _getBondStereo(atom_idx, edge_ids[nei_idx].nei_idx);

         if (stereo == BOND_UP)
            n_up++;
         else if (stereo == BOND_DOWN)
            n_down++;

         int xyzzy = _xyzzy(edge_ids[nei_idx].vec, edge_ids[(nei_idx + 1) % 3].vec,
                            edge_ids[(nei_idx + 2) % 3].vec);

         if (xyzzy == 1)
            invert = true;
         if (xyzzy == 2)
            not_invert = true;
      }

      if (!invert && !not_invert)
         throw Error("degenerate case for 3 bonds near stereoatom");

      if (n_down > 0 && n_up > 0)
         throw Error("one bond up, one bond down -- indefinite case");
      else if (n_down == 0 && n_up == 0)
         throw Error("no up-down bonds attached to stereocenter");

      if (!possible_lone_pair)
      {
         if (n_up == 3)
            throw Error("all 3 bonds up near stereoatom");
         if (n_down == 3)
            throw Error("all 3 bonds down near stereoatom");
      }

      int dir = 1;

      if (n_down > 0)
         dir = -1;

      if (invert)
         dir = -dir;

      int sign = _sign(edge_ids[0].vec, edge_ids[1].vec, edge_ids[2].vec);

      if (sign == dir)
      {
         pyramid[0] = edge_ids[0].nei_idx;
         pyramid[1] = edge_ids[2].nei_idx;
         pyramid[2] = edge_ids[1].nei_idx;
      }
      else
      {
         pyramid[0] = edge_ids[0].nei_idx;
         pyramid[1] = edge_ids[1].nei_idx;
         pyramid[2] = edge_ids[2].nei_idx;
      }
      pyramid[3] = -1;
   }

   _stereocenters.insert(atom_idx, stereocenter);
}

int MoleculeStereocenters::_getBondStereo (int center_idx, int nei_idx) const
{
   const BaseMolecule &mol = _getMolecule();
   int idx = mol.findEdgeIndex(center_idx, nei_idx);

   if (idx == -1)
      throw Error("_getBondStereo(): can not find bond");

   if (center_idx != mol.getEdge(idx).beg)
      return 0;

   return _bond_directions[idx];
}

// 1 -- in the smaller angle, 2 -- in the bigger angle,
// 4 -- in the 'positive' straight angle, 8 -- in the 'negative' straight angle
int MoleculeStereocenters::_xyzzy (const Vec3f &v1, const Vec3f &v2, const Vec3f &u)
{
   float eps = 1e-3f;

   Vec3f cross;

   cross.cross(v1, v2);

   float sine1 = cross.z;
   float cosine1 = Vec3f::dot(v1, v2);

   cross.cross(v1, u);

   float sine2 = cross.z;
   float cosine2 = Vec3f::dot(v1 ,u);

   if (fabsf(sine1) < eps)
   {
      if (fabsf(sine2) < eps)
         throw Error("degenerate case -- bonds overlap");

      return (sine2 > 0) ? 4 : 8;
   }

   if (sine1 * sine2 < -eps * eps)
      return 2;

   if (cosine2 < cosine1)
      return 2;

   return 1;
}

int MoleculeStereocenters::_sign (const Vec3f &v1, const Vec3f &v2, const Vec3f &v3)
{
   float res = (v1.x - v3.x) * (v2.y - v3.y) - (v1.y - v3.y) * (v2.x - v3.x);
   float eps = 1e-3f;

   if (res > eps)
      return 1;
   if (res < -eps)
      return -1;

   throw Error("degenerate triangle");
}

int MoleculeStereocenters::getType (int idx) const
{
   _Atom *atom = _stereocenters.at2(idx);

   if (atom == 0)
      return 0;

   return atom->type;
}

int MoleculeStereocenters::getGroup (int idx) const
{
   return _stereocenters.at(idx).group;
}

void MoleculeStereocenters::setType (int idx, int type, int group)
{
   _stereocenters.at(idx).type = type;
   _stereocenters.at(idx).group = group;
}

const int * MoleculeStereocenters::getPyramid (int idx) const
{
   return _stereocenters.at(idx).pyramid;
}

int * MoleculeStereocenters::getPyramid (int idx)
{
   _Atom *stereo = _stereocenters.at2(idx);

   if (stereo != 0)
      return stereo->pyramid;
   else
      return 0;
}

void MoleculeStereocenters::inversePyramid (int idx)
{
   int tmp;

   __swap(_stereocenters.at(idx).pyramid[0], _stereocenters.at(idx).pyramid[1], tmp);
}

void MoleculeStereocenters::getAbsAtoms (Array<int> &indices)
{
   indices.clear();

   for (int i = _stereocenters.begin(); i != _stereocenters.end();
        i = _stereocenters.next(i))
   {
      if (_stereocenters.value(i).type == ATOM_ABS)
         indices.push(_stereocenters.key(i));
   }
}

void MoleculeStereocenters::_getGroups (int type, Array<int> &numbers)
{
   numbers.clear();

   for (int i = _stereocenters.begin(); i != _stereocenters.end();
        i = _stereocenters.next(i))
   {
      if (_stereocenters.value(i).type == type)
      {
         int group = _stereocenters.value(i).group;

         if (numbers.find(group) == -1)
            numbers.push(group);
      }
   }
}

void MoleculeStereocenters::_getGroup (int type, int number, Array<int> &indices)
{
   indices.clear();

   for (int i = _stereocenters.begin(); i != _stereocenters.end();
            i = _stereocenters.next(i))
   {
      const _Atom &atom = _stereocenters.value(i);

      if (atom.type == type && atom.group == number)
         indices.push(_stereocenters.key(i));
   }
}

void MoleculeStereocenters::getOrGroups (Array<int> &numbers)
{
   _getGroups(ATOM_OR, numbers);
}

void MoleculeStereocenters::getAndGroups (Array<int> &numbers)
{
   _getGroups(ATOM_AND, numbers);
}

void MoleculeStereocenters::getOrGroup (int number, Array<int> &indices)
{
   _getGroup(ATOM_OR, number, indices);
}

void MoleculeStereocenters::getAndGroup (int number, Array<int> &indices)
{
   _getGroup(ATOM_AND, number, indices);
}

bool MoleculeStereocenters::sameGroup (int idx1, int idx2)
{
   _Atom *center1 = _stereocenters.at2(idx1);
   _Atom *center2 = _stereocenters.at2(idx2);

   if (center1 == 0 && center2 == 0)
      return true;

   if (center1 == 0 || center2 == 0)
      return false;

   if (center1->type == ATOM_ABS)
      return center2->type == ATOM_ABS;

   if (center1->type == ATOM_OR)
      return center2->type == ATOM_OR && center1->group == center2->group;

   if (center1->type == ATOM_AND)
      return center2->type == ATOM_AND && center1->group == center2->group;

   return false;
}

int MoleculeStereocenters::getBondDirection (int idx) const
{
   if (idx > _bond_directions.size() - 1)
      return 0;

   return _bond_directions[idx];
}

bool MoleculeStereocenters::haveAllAbs ()
{
   int i;

   for (i = _stereocenters.begin(); i != _stereocenters.end(); i = _stereocenters.next(i))
      if (_stereocenters.value(i).type != ATOM_ABS)
         return false;

   return true;
}

bool MoleculeStereocenters::haveAllAbsAny ()
{
   int i;

   for (i = _stereocenters.begin(); i != _stereocenters.end(); i = _stereocenters.next(i))
      if (_stereocenters.value(i).type != ATOM_ABS && _stereocenters.value(i).type != ATOM_ANY)
         return false;

   return true;
}

bool MoleculeStereocenters::haveAllAndAny ()
{
   int i;
   int groupno = -1;

   for (i = _stereocenters.begin(); i != _stereocenters.end(); i = _stereocenters.next(i))
   {
      if (_stereocenters.value(i).type == ATOM_ANY)
         continue;
      if (_stereocenters.value(i).type != ATOM_AND)
         return false;
      if (groupno == -1)
         groupno = _stereocenters.value(i).group;
      else if (groupno != _stereocenters.value(i).group)
         return false;
   }

   return true;
}

bool MoleculeStereocenters::checkSub (const MoleculeStereocenters &query,
                                      const MoleculeStereocenters &target, 
                                      const int *mapping, bool reset_h_isotopes,
                                      Filter *stereocenters_vertex_filter)
{
   QS_DEF(Array<int>, flags);

   flags.clear_resize(query._stereocenters.end());
   flags.zerofill();

   int i, j;

   for (i = query._stereocenters.begin(); i != query._stereocenters.end();
        i = query._stereocenters.next(i))
   {
      if (flags[i])
         continue;

      flags[i] = 1;

      const _Atom &cq = query._stereocenters.value(i);
      int iq = query._stereocenters.key(i);

      if (mapping[iq] < 0)
         continue; // happens only on Exact match (when some query fragments are disabled)

      if (stereocenters_vertex_filter != 0 && !stereocenters_vertex_filter->valid(iq))
         continue;

      if (cq.type < ATOM_AND)
         continue;

      int stereo_group_and = -1;
      int stereo_group_or = -1;
      int revert = -1; // 0 -- not revert, 1 -- revert
      bool have_abs = false;

      int pyramid_mapping[4];
      int type = cq.type;

      if (type == ATOM_ABS)
      {
         getPyramidMapping(query, target, iq, mapping, pyramid_mapping, reset_h_isotopes);

         if (!isPyramidMappingRigid(pyramid_mapping))
            return false;
      }
      else if (type == ATOM_OR || type == ATOM_AND)
      {
         for (j = i; j != query._stereocenters.end(); j = query._stereocenters.next(j))
         {
            int iq2 = query._stereocenters.key(j);
            const _Atom &cq2 = query._stereocenters.value(j);

            if (cq2.type != type)
               continue;

            if (cq2.group != cq.group)
               continue;
            
            const _Atom *ct2 = target._stereocenters.at2(mapping[iq2]);

            if (ct2 == 0)
               return false;

            if (ct2->type < type)
               return false;

            flags[j] = 1;

            if (ct2->type == ATOM_AND)
            {
               if (stereo_group_or != -1)
                  return false;
               if (have_abs)
                  return false;

               if (stereo_group_and == -1)
                  stereo_group_and = ct2->group;
               else if (stereo_group_and != ct2->group)
                  return false;
            }
            else if (ct2->type == ATOM_OR)
            {
               if (stereo_group_and != -1)
                  return false;
               if (have_abs)
                  return false;

               if (stereo_group_or == -1)
                  stereo_group_or = ct2->group;
               else if (stereo_group_or != ct2->group)
                  return false;
            }
            else if (ct2->type == ATOM_ABS)
            {
               if (stereo_group_and != -1)
                  return false;
               if (stereo_group_or != -1)
                  return false;

               have_abs = true;
            }

            getPyramidMapping(query, target, iq2, mapping, pyramid_mapping, reset_h_isotopes);
            int not_equal = isPyramidMappingRigid(pyramid_mapping) ? 0 : 1;

            if (revert == -1)
               revert = not_equal;
            else if (revert != not_equal)
               return false;
         }
      }
   }

   return true;
}

bool MoleculeStereocenters::isPyramidMappingRigid (const int mapping[4])
{
   int arr[4], tmp;
   bool rigid = true;

   memcpy(arr, mapping, 4 * sizeof(int));

   if (arr[0] > arr[1])
      __swap(arr[0], arr[1], tmp), rigid = !rigid;
   if (arr[1] > arr[2])
      __swap(arr[1], arr[2], tmp), rigid = !rigid;
   if (arr[2] > arr[3])
      __swap(arr[2], arr[3], tmp), rigid = !rigid;
   if (arr[1] > arr[2])
      __swap(arr[1], arr[2], tmp), rigid = !rigid;
   if (arr[0] > arr[1])
      __swap(arr[0], arr[1], tmp), rigid = !rigid;
   if (arr[1] > arr[2])
      __swap(arr[1], arr[2], tmp), rigid = !rigid;

   return rigid;
}

bool MoleculeStereocenters::isPyramidMappingRigid_Sort (int *pyramid, const int *mapping)
{
   bool rigid = true;
   int i, tmp;

   for (i = 0; i < 4; i++)
      if (pyramid[i] != -1 && mapping[pyramid[i]] < 0)
         pyramid[i] = -1;

   if (pyramid[0] == -1 || (pyramid[1] >= 0 && mapping[pyramid[0]] > mapping[pyramid[1]]))
      __swap(pyramid[0], pyramid[1], tmp), rigid = !rigid;
   if (pyramid[1] == -1 || (pyramid[2] >= 0 && mapping[pyramid[1]] > mapping[pyramid[2]]))
      __swap(pyramid[1], pyramid[2], tmp), rigid = !rigid;
   if (pyramid[2] == -1 || (pyramid[3] >= 0 && mapping[pyramid[2]] > mapping[pyramid[3]]))
      __swap(pyramid[2], pyramid[3], tmp), rigid = !rigid;
   if (pyramid[1] == -1 || (pyramid[2] >= 0 && mapping[pyramid[1]] > mapping[pyramid[2]]))
      __swap(pyramid[1], pyramid[2], tmp), rigid = !rigid;
   if (pyramid[0] == -1 || (pyramid[1] >= 0 && mapping[pyramid[0]] > mapping[pyramid[1]]))
      __swap(pyramid[0], pyramid[1], tmp), rigid = !rigid;
   if (pyramid[1] == -1 || (pyramid[2] >= 0 && mapping[pyramid[1]] > mapping[pyramid[2]]))
      __swap(pyramid[1], pyramid[2], tmp), rigid = !rigid;

   return rigid;
}

bool MoleculeStereocenters::isPyramidMappingRigid (const int *pyramid, int size, const int *mapping)
{
   if (size == 3)
   {
      int order[3] = {mapping[pyramid[0]], mapping[pyramid[1]], mapping[pyramid[2]]};
      int min = __min3(order[0], order[1], order[2]);

      while (order[0] != min) 
      {
         int t = order[2];
         order[2] = order[1];
         order[1] = order[0];
         order[0] = t;
      }

      return order[1] < order[2];
   }

   if (size == 4)
   {
      int arr[4];

      arr[0] = mapping[pyramid[0]];
      arr[1] = mapping[pyramid[1]];
      arr[2] = mapping[pyramid[2]];
      arr[3] = mapping[pyramid[3]];

      return isPyramidMappingRigid(arr);
   }

   throw Error("IsPyramidMappingRigid: size = %d", size);
}

void MoleculeStereocenters::getPyramidMapping (const MoleculeStereocenters &query,
         const MoleculeStereocenters &target, int query_atom, const int *mapping, int *mapping_out, bool reset_h_isotopes)
{
   int i, j;

   BaseMolecule &tmol = target._getMolecule();
   BaseMolecule &qmol = query._getMolecule();

   const int *seq1 = query.getPyramid(query_atom);
   const int *seq2 = target.getPyramid(mapping[query_atom]);

   int seq2_matched[] = {0, 0, 0, 0};

   for (i = 0; i < 4; i++)
      mapping_out[i] = -1;

   for (i = 0; i < 4; i++)
   {
      // skip implicit hydrogen for the first pass
      if (seq1[i] == -1)
         continue;

      // unmapped atom?
      if (mapping[seq1[i]] < 0)
      {
         // only hydrogens are allowed to be unmapped
         if (qmol.getAtomNumber(seq1[i]) != ELEM_H)
            throw Error("unmapped non-hydrogen atom (atom number %d)",
                    qmol.getAtomNumber(seq1[i]));
         continue;
      }

      for (j = 0; j < 4; j++)
         if (seq2[j] == mapping[seq1[i]]) 
            break;

      if (j == 4)
         throw Error("cannot map pyramid");

      mapping_out[i] = j;
      seq2_matched[j] = 1;
   }

   // take implicit hydrogen to the second pass
   for (i = 0; i < 4; i++)
   {
      if (mapping_out[i] != -1)
         continue;
      
      for (j = 0; j < 4; j++)
      {
         if (seq2[j] == -1)
            break; // match to implicit hydrogen
         
         if (!seq2_matched[j])
         {
            if (tmol.getAtomNumber(seq2[j]) == ELEM_H)
               break; // match to explicit hydrogen

            // rare cases like '[S@](F)(Cl)=O' on '[S@](F)(Cl)(=O)=N'
            if (seq1[i] == -1 && tmol.getAtomNumber(mapping[query_atom]) == ELEM_S)
               break; // match free electron pair to an atom
         }
      }

      if (j == 4)
         throw Error("cannot map pyramid");

      mapping_out[i] = j;
      seq2_matched[j] = 1;
   }

}

void MoleculeStereocenters::remove (int idx)
{
   _stereocenters.remove(idx);
}

void MoleculeStereocenters::removeAtoms (const Array<int> &indices)
{
   const BaseMolecule &mol = _getMolecule();

   for (int i = 0; i < indices.size(); i++)
   {
      int idx = indices[i];
      if (_stereocenters.find(idx))
         _stereocenters.remove(idx);
      else
      {
         const Vertex &vertex = mol.getVertex(idx);

         for (int k = vertex.neiBegin(); k != vertex.neiEnd(); k = vertex.neiNext(k))
         {
            int nei_vertex = vertex.neiVertex(k);
            _removeBondDir(idx, nei_vertex);
         }
      }
   }
}

void MoleculeStereocenters::removeBonds (const Array<int> &indices)
{
   const BaseMolecule &mol = _getMolecule();

   for (int i = 0; i < indices.size(); i++)
   {
      const Edge &edge = mol.getEdge(indices[i]);

      _removeBondDir(edge.beg, edge.end);
      _removeBondDir(edge.end, edge.beg);
   }
}

void MoleculeStereocenters::_removeBondDir (int atom_from, int atom_to)
{
   _Atom *stereo_atom = _stereocenters.at2(atom_to);
   if (stereo_atom != 0)
   {
      if (stereo_atom->pyramid[3] == -1)
         _stereocenters.remove(atom_to);
      else
         _convertAtomToImplicitHydrogen(stereo_atom->pyramid, atom_from);
   }
}

void MoleculeStereocenters::buildOnSubmolecule (const MoleculeStereocenters &super, int *mapping)
{
   int i, j;
   const BaseMolecule &mol = _getMolecule();

   _bond_directions.clear_resize(mol.edgeEnd());
   _bond_directions.zerofill();

   const BaseMolecule &supermol = super._getMolecule();

   // trick for molecules with incorrect stereochemistry, of which we do permutations
   if (supermol.vertexCount() == mol.vertexCount() &&
       supermol.edgeCount() == mol.edgeCount())
   {
      for (j = supermol.edgeBegin(); j != supermol.edgeEnd(); j = supermol.edgeNext(j))
      {
         const Edge &edge = supermol.getEdge(j);
         
         if (super.getBondDirection(j) != 0)
            _bond_directions[mol.findEdgeIndex(mapping[edge.beg], mapping[edge.end])] =
                    super.getBondDirection(j);
      }
   }
        
   for (i = super._stereocenters.begin(); i != super._stereocenters.end();
        i = super._stereocenters.next(i))
   {
      int super_idx = super._stereocenters.key(i);
      const _Atom &super_stereocenter = super._stereocenters.value(i);
      int sub_idx = mapping[super_idx];

      if (sub_idx < 0)
         continue;

      _Atom new_stereocenter;

      new_stereocenter.group = super_stereocenter.group;
      new_stereocenter.type = super_stereocenter.type;

      for (j = 0; j < 4; j++)
      {
         int idx = super_stereocenter.pyramid[j];
      
         if (idx == -1)
            new_stereocenter.pyramid[j] = -1;
         else
         {
            int val = mapping[idx];
            if (val != -1 && mol.findEdgeIndex(sub_idx, val) == -1)
               val = -1;
            new_stereocenter.pyramid[j] = val;
         }     
      }

      moveMinimalToEnd(new_stereocenter.pyramid);
      if (new_stereocenter.pyramid[0] == -1 || new_stereocenter.pyramid[1] == -1 || 
            new_stereocenter.pyramid[2] == -1)
         // pyramid is not mapped completely
         continue;

      _stereocenters.insert(sub_idx, new_stereocenter);

      const Vertex &super_vertex = super._getMolecule().getVertex(super_idx);


      for (j = super_vertex.neiBegin(); j != super_vertex.neiEnd();
           j = super_vertex.neiNext(j))
      {
         int super_edge = super_vertex.neiEdge(j);
         if (mapping[super_vertex.neiVertex(j)] == -1)
            continue;

         if (super.getBondDirection(super_edge) != 0)
            _bond_directions[_getMolecule().findEdgeIndex(sub_idx, mapping[super_vertex.neiVertex(j)])] =
                    super.getBondDirection(super_edge);
      }
   }
}

int MoleculeStereocenters::size () const
{
   return _stereocenters.size();
}

void MoleculeStereocenters::add (int atom_idx, int type, int group, bool inverse_pyramid)
{
   int pyramid[4];
   _restorePyramid(atom_idx, pyramid, inverse_pyramid);
   add(atom_idx, type, group, pyramid);
}

void MoleculeStereocenters::add (int atom_idx, int type, int group, const int pyramid[4])
{
   if (atom_idx == -1)
      throw Error("stereocenter index is invalid");
   if (pyramid[0] == -1 || pyramid[1] == -1 || pyramid[2] == -1)
      throw Error("stereocenter pyramid must have at least 3 atoms");

   _Atom center;

   center.type = type;
   center.group = group;
   memcpy(center.pyramid, pyramid, 4 * sizeof(int));
   
   _stereocenters.insert(atom_idx, center);
}

int MoleculeStereocenters::begin () const
{
   return _stereocenters.begin();
}

int MoleculeStereocenters::end () const
{
   return _stereocenters.end();
}

int MoleculeStereocenters::next (int i) const
{
   return _stereocenters.next(i);
}

bool MoleculeStereocenters::exists (int atom_idx) const
{
   return _stereocenters.at2(atom_idx) != 0;
}

void MoleculeStereocenters::get (int i, int &atom_idx, int &type, int &group, int *pyramid) const
{
   const _Atom &center = _stereocenters.value(i);

   atom_idx = _stereocenters.key(i);
   type = center.type;
   group = center.group;
   if (pyramid != 0)
      memcpy(pyramid, center.pyramid, 4 * sizeof(int));
}

int MoleculeStereocenters::getAtomIndex (int i) const
{
   return _stereocenters.key(i);
}

void MoleculeStereocenters::get (int atom_idx, int &type, int &group, int *pyramid) const
{
   const _Atom &center = _stereocenters.at(atom_idx);

   type = center.type;
   group = center.group;
   if (pyramid != 0)
      memcpy(pyramid, center.pyramid, 4 * sizeof(int));
}

void MoleculeStereocenters::registerUnfoldedHydrogen (int atom_idx, int added_hydrogen)
{
   _Atom *center = _stereocenters.at2(atom_idx);
   if (center == 0)
      return;

   if (center->pyramid[3] != -1)
      throw Error("cannot unfold hydrogens for stereocenter without implicit hydrogens");
   center->pyramid[3] = added_hydrogen;
}

void MoleculeStereocenters::flipBond (int atom_parent, int atom_from, int atom_to)
{
   if (exists(atom_from))
   {  
       _Atom *from_center = _stereocenters.at2(atom_from);
      
      if (from_center->pyramid[3] == -1)
         remove(atom_from);
      else
      {
         for (int i = 0; i < 4; i++)
            if (from_center->pyramid[i] == atom_parent)
               from_center->pyramid[i] = -1;
         moveMinimalToEnd(from_center->pyramid);
      }
   }

   if (exists(atom_to))
   {
      _Atom *to_center = _stereocenters.at2(atom_to);
      
      if (to_center->pyramid[3] != -1)
         throw Error("Bad bond flipping. Stereocenter pyramid is already full");

      to_center->pyramid[3] = atom_parent;
   }

   if (!exists(atom_parent))
      return;

   _Atom *center = _stereocenters.at2(atom_parent);
   for (int i = 0; i < 4; i++)
      if (center->pyramid[i] == atom_from)
      {
         center->pyramid[i] = atom_to;
         break;
      }

}

void MoleculeStereocenters::_restorePyramid (int idx, int pyramid[4], int invert_pyramid)
{
   BaseMolecule &mol = _getMolecule();
   const Vertex &vertex = mol.getVertex(idx);
   int j, count = 0;

   pyramid[0] = -1;
   pyramid[1] = -1;
   pyramid[2] = -1;
   pyramid[3] = -1;
   
   for (j = vertex.neiBegin(); j != vertex.neiEnd(); j = vertex.neiNext(j))
   {
      int nei = vertex.neiVertex(j);

      if (vertex.degree() == 3 ||
          mol.getAtomNumber(nei) != ELEM_H || mol.getAtomIsotope(nei) != 0)
      {
         if (count == 4)
            throw Error("restorePyramid(): stereocenter has more than 4 neighbors");

         pyramid[count++] = nei;
      }
      else if (pyramid[3] == -1)
         pyramid[3] = nei;
      else
         throw Error("restorePyramid(): extra hydrogen");
   }

   int tmp;

   // sort pyramid indices
   if (pyramid[3] == -1)
   {
      if (pyramid[0] > pyramid[1])
         __swap(pyramid[0], pyramid[1], tmp);
      if (pyramid[1] > pyramid[2])
         __swap(pyramid[1], pyramid[2], tmp);
      if (pyramid[0] > pyramid[1])
         __swap(pyramid[0], pyramid[1], tmp);
   }
   else
   {
      if (pyramid[0] > pyramid[1])
         __swap(pyramid[0], pyramid[1], tmp);
      if (pyramid[1] > pyramid[2])
         __swap(pyramid[1], pyramid[2], tmp);
      if (pyramid[2] > pyramid[3])
         __swap(pyramid[2], pyramid[3], tmp);
      if (pyramid[1] > pyramid[2])
         __swap(pyramid[1], pyramid[2], tmp);
      if (pyramid[0] > pyramid[1])
         __swap(pyramid[0], pyramid[1], tmp);
      if (pyramid[1] > pyramid[2])
         __swap(pyramid[1], pyramid[2], tmp);
   }
   if (invert_pyramid)
      __swap(pyramid[1], pyramid[2], j);
}

void MoleculeStereocenters::_rotatePyramid (int *pyramid)
{
   int tmp;

   tmp = pyramid[0];
   pyramid[0] = pyramid[1];
   pyramid[1] = pyramid[2];

   if (pyramid[3] == -1)
   {
      pyramid[2] = tmp;
   }
   else
   {
      pyramid[2] = pyramid[3];
      pyramid[3] = tmp;
   }
}

void MoleculeStereocenters::moveImplicitHydrogenToEnd (int pyramid[4])
{
   moveMinimalToEnd(pyramid);
   if (pyramid[3] != -1)
      throw Error("moveImplicitHydrogenToEnd(): no implicit hydrogen");
}

void MoleculeStereocenters::moveMinimalToEnd (int pyramid[4])
{
   int cnt = 0;

   int min_element = __min(__min(pyramid[0], pyramid[1]), __min(pyramid[2], pyramid[3]));

   while (pyramid[3] != min_element)
   {
      if (cnt == 4)
         throw Error("moveMinimalToEnd(): internal error");

      _rotatePyramid(pyramid);
      cnt++;
   }

   if (cnt & 1)
      __swap(pyramid[0], pyramid[1], cnt);
}

void MoleculeStereocenters::_convertAtomToImplicitHydrogen (int pyramid[4], int atom_to_remove)
{
   if (pyramid[3] == -1)
      throw Error("Cannot remove atoms form sterecenter with implicit hydrogen. "
         "Stereocenter should be removed");

   bool removed = false;

   for (int i = 0; i < 4; i++)
      if (pyramid[i] == atom_to_remove)
      {
         pyramid[i] = -1;
         removed = true;
         break;
      }

   if (!removed)
      throw Error("Specified atom %d wasn't found in the stereopyramid", atom_to_remove);

   moveImplicitHydrogenToEnd(pyramid);
}

void MoleculeStereocenters::markBonds ()
{
   BaseMolecule &mol = _getMolecule();
   int i, j;

   _bond_directions.clear_resize(mol.edgeEnd());
   _bond_directions.zerofill();

   for (i = _stereocenters.begin(); i != _stereocenters.end(); i = _stereocenters.next(i))
   {
      int atom_idx = _stereocenters.key(i);
      const _Atom &atom = _stereocenters.value(i);
      int pyramid[4];
      int mult = 1;
      int size = 0;
      
      memcpy(pyramid, atom.pyramid, 4 * sizeof(int));

      if (atom.type <= ATOM_ANY)
      {
         const Vertex &vertex = mol.getVertex(atom_idx);

         // fill the pyramid
         for (j = vertex.neiBegin(); j != vertex.neiEnd() && size < 4; j = vertex.neiNext(j))
            pyramid[size++] = vertex.neiVertex(j);
      }
      else
         size = (pyramid[3] == -1 ? 3 : 4);

      int edge_idx = -1;

      for (j = 0; j < size; j++)
      {
         edge_idx = mol.findEdgeIndex(atom_idx, pyramid[size - 1]);
         if (_bond_directions[edge_idx] == 0 && mol.getVertex(pyramid[size - 1]).degree() == 1)
            break;
         _rotatePyramid(pyramid);
         if (size == 4)
            mult = -mult;
      }

      if (j == size)
      {
         for (j = 0; j < size; j++)
         {
            edge_idx = mol.findEdgeIndex(atom_idx, pyramid[size - 1]);
            if (_bond_directions[edge_idx] == 0 &&
                mol.getBondTopology(edge_idx) == TOPOLOGY_CHAIN &&
                getType(pyramid[size - 1]) == 0)
               break;
            _rotatePyramid(pyramid);
            if (size == 4)
               mult = -mult;
         }
      }

      if (j == size)
      {
         for (j = 0; j < size; j++)
         {
            edge_idx = mol.findEdgeIndex(atom_idx, pyramid[size - 1]);
            if (_bond_directions[edge_idx] == 0 && getType(pyramid[size - 1]) == 0)
               break;
            _rotatePyramid(pyramid);
            if (size == 4)
               mult = -mult;
         }
      }

      if (j == size)
      {
         for (j = 0; j < size; j++)
         {
            edge_idx = mol.findEdgeIndex(atom_idx, pyramid[size - 1]);
            if (_bond_directions[edge_idx] == 0 && mol.getBondTopology(edge_idx) == TOPOLOGY_CHAIN)
               break;
            _rotatePyramid(pyramid);
            if (size == 4)
               mult = -mult;
         }
      }

      if (j == size)
      {
         for (j = 0; j < size; j++)
         {
            edge_idx = mol.findEdgeIndex(atom_idx, pyramid[size - 1]);
            if (_bond_directions[edge_idx] == 0)
               break;
            _rotatePyramid(pyramid);
            if (size == 4)
               mult = -mult;
         }
      }

      if (j == size)
         throw Error("no bond can be marked");

      if (mol.getEdge(edge_idx).beg != atom_idx)
         mol.swapEdgeEnds(edge_idx);

      if (atom.type > ATOM_ANY)
      {
         Vec3f dirs[4];

         for (j = 0; j < size; j++)
            dirs[j] = mol.getAtomXyz(pyramid[j]);

         int sign = _sign(dirs[0], dirs[1], dirs[2]);

         if (size == 3)
            _bond_directions[edge_idx] = (sign == 1) ? BOND_DOWN : BOND_UP;
         else
            _bond_directions[edge_idx] = (sign * mult == 1) ? BOND_UP : BOND_DOWN;
      }
      else
         _bond_directions[edge_idx] = BOND_EITHER;
   }
}

bool MoleculeStereocenters::isAutomorphism (BaseMolecule &mol, const Array<int> &mapping, const Filter *filter)
{
   MoleculeStereocenters &stereocenters = mol.stereocenters;

   for (int i = stereocenters.begin(); i != stereocenters.end(); i = stereocenters.next(i))
   {
      if (filter && !filter->valid(i))
         continue;

      int idx, type, group;
      int pyramid[4];
      int j;

      stereocenters.get(i, idx, type, group, pyramid);

      if (mapping[idx] == -1)
         continue;

      int size = 0;

      for (j = 0; j < 4; j++)
         if (pyramid[j] >= 0)
         {
            if (mapping[pyramid[j]] >= 0)
               size++;
            else
               pyramid[j] = -1;
         }

      if (size < 3)
         continue;

      if (type < MoleculeStereocenters::ATOM_AND)
         continue;

      if (stereocenters.getType(mapping[idx]) != type)
         return false;

      int pyra_map[4];

      MoleculeStereocenters::getPyramidMapping(stereocenters, stereocenters, idx, mapping.ptr(), pyra_map, false);

      if (!MoleculeStereocenters::isPyramidMappingRigid(pyra_map))
         return false;
   }
   return true;
}
