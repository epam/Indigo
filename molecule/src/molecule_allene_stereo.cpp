/****************************************************************************
 * Copyright (C) 2011 GGA Software Services LLC
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

#include "molecule/base_molecule.h"
#include "molecule/molecule_allene_stereo.h"
#include "molecule/elements.h"
#include "molecule/cmf_loader.h"

using namespace indigo;

MoleculeAlleneStereo::MoleculeAlleneStereo ()
{
}

BaseMolecule & MoleculeAlleneStereo::_getMolecule ()
{
   char dummy[sizeof(BaseMolecule)];

   int offset = (int)((char *)(&((BaseMolecule *)dummy)->allene_stereo) - dummy);

   return *(BaseMolecule *)((char *)this - offset);
}

int MoleculeAlleneStereo::sameside (const Vec3f &dir1, const Vec3f &dir2, const Vec3f &sep)
{
   Vec3f norm, norm_cross;

   // Use double cross product for getting vector lying the same plane with dir1 and sep
   norm_cross.cross(sep, dir1);
   norm.cross(norm_cross, sep);

   if (!norm.normalize())
      throw Error("internal: zero vector length");

   float prod1 = Vec3f::dot(dir1, norm);
   float prod2 = Vec3f::dot(dir2, norm);

   if ((float)(fabs(prod1)) < 1e-3 || (float)(fabs(prod2)) < 1e-3)
      return 0;

   return (prod1 * prod2 > 0) ? 1 : -1;
}


bool MoleculeAlleneStereo::_isAlleneCenter (BaseMolecule &mol, int idx, _Atom &atom, int *sensible_bonds_out)
{
   const Vertex &vertex = mol.getVertex(idx);

   // Check that we have [C,Si]=[C,Si]=[C,Si] fragment with the middle "C" being the i-th atom.
   if (vertex.degree() != 2)
      return false;

   if (mol.getAtomNumber(idx) != ELEM_C && mol.getAtomNumber(idx) != ELEM_Si)
      return false;

   int j = vertex.neiBegin();
   int left_edge = vertex.neiEdge(j);
   int right_edge = vertex.neiEdge(vertex.neiNext(j));

   atom.left = vertex.neiVertex(j);
   atom.right  = vertex.neiVertex(vertex.neiNext(j));
   
   if (mol.getBondOrder(left_edge) != BOND_DOUBLE || mol.getBondOrder(right_edge) != BOND_DOUBLE)
      return false;

   if (mol.getAtomNumber(atom.left) != ELEM_C && mol.getAtomNumber(atom.left) != ELEM_Si)
      return false;

   if (mol.getAtomNumber(atom.right) != ELEM_C && mol.getAtomNumber(atom.right) != ELEM_Si)
      return false;

   // Also check that left and right "C" atoms have one or two single bonds
   const Vertex &v_left = mol.getVertex(atom.left);
   const Vertex &v_right = mol.getVertex(atom.right);

   if (v_left.degree() < 2 || v_left.degree() > 3)
      return false;

   if (v_right.degree() < 2 || v_right.degree() > 3)
      return false;

   int k;

   atom.subst[0] = -1;
   atom.subst[1] = -1;
   atom.subst[2] = -1;
   atom.subst[3] = -1;

   int dirs[4] = {0, 0, 0, 0};
   Vec3f subst_vecs[4];

   for (k = 0, j = v_left.neiBegin(); j != v_left.neiEnd(); j = v_left.neiNext(j))
   {
      if (v_left.neiVertex(j) == idx)
         continue;
      if (mol.getBondOrder(v_left.neiEdge(j)) != BOND_SINGLE)
         return false;
      atom.subst[k] = v_left.neiVertex(j);
      dirs[k] = mol.getBondDirection2(atom.left, v_left.neiVertex(j));
      subst_vecs[k].diff(mol.getAtomXyz(v_left.neiVertex(j)), mol.getAtomXyz(atom.left));
      k++;
   }

   for (k = 2, j = v_right.neiBegin(); j != v_right.neiEnd(); j = v_right.neiNext(j))
   {
      if (v_right.neiVertex(j) == idx)
         continue;
      if (mol.getBondOrder(v_right.neiEdge(j)) != BOND_SINGLE)
         return false;
      atom.subst[k] = v_right.neiVertex(j);
      dirs[k] = mol.getBondDirection2(atom.right, v_right.neiVertex(j));
      subst_vecs[k].diff(mol.getAtomXyz(v_right.neiVertex(j)), mol.getAtomXyz(atom.right));
      k++;
   }

   if (dirs[0] == 0 && dirs[1] == 0 && dirs[2] == 0 && dirs[3] == 0)
      return false; // no oriented bonds => no stereochemistry

   // check that they do not have the same orientation
   if (dirs[0] != 0 && dirs[0] == dirs[1] && dirs[0] != BOND_EITHER)
      return false;
   if (dirs[2] != 0 && dirs[2] == dirs[3] && dirs[2] != BOND_EITHER)
      return false;

   Vec3f pos_center = mol.getAtomXyz(idx);
   Vec3f vec_left = mol.getAtomXyz(atom.left);
   Vec3f vec_right = mol.getAtomXyz(atom.right);

   vec_left.sub(pos_center);
   vec_right.sub(pos_center);

   if (!vec_left.normalize() || !vec_right.normalize())
      throw Error("zero bond length");

   // they should go in one line
   if (fabs(Vec3f::dot(vec_left, vec_right) + 1) > 0.001)
      return false;

   // check that if there are two left substituents, they do not lie on the same side
   if (atom.subst[1] != -1 && sameside(subst_vecs[0], subst_vecs[1], vec_left) != -1)
      return false;
   // the same check for the two right substituents
   if (atom.subst[3] != -1 && sameside(subst_vecs[2], subst_vecs[3], vec_right) != -1)
      return false;

   if (dirs[0] == BOND_EITHER || dirs[1] == BOND_EITHER || dirs[2] == BOND_EITHER || dirs[3] == BOND_EITHER)
      atom.parity = 3;
   else
   {
      if (dirs[0] == 0 && dirs[1] != 0)
         dirs[0] = 3 - dirs[1];
      if (dirs[2] == 0 && dirs[3] != 0)
         dirs[2] = 3 - dirs[3];

      int ss = sameside(subst_vecs[0], subst_vecs[2], vec_right);

      if (ss == 0)
         return false;

      if (dirs[0] == 0)
         dirs[0] = (ss == 1) ? 3 - dirs[2] : dirs[2];
      else if (dirs[2] == 0)
         dirs[2] = (ss == 1) ? 3 - dirs[0] : dirs[0];

      if ((ss == 1 && dirs[0] == dirs[2]) ||
          (ss == -1 && dirs[0] != dirs[2]))
         return false; // square-planar configuration?

      if ((ss == 1 && dirs[0] == BOND_UP) ||
          (ss == -1 && dirs[0] == BOND_DOWN))
         atom.parity = 1;
      else
         atom.parity = 2;
   }

   for (k = 0, j = v_left.neiBegin(); j != v_left.neiEnd(); j = v_left.neiNext(j))
   {
      int dir = mol.getBondDirection2(atom.left, v_left.neiVertex(j));
      if (dir != 0)
         sensible_bonds_out[v_left.neiEdge(j)] = 1;
   }
   for (k = 0, j = v_right.neiBegin(); j != v_right.neiEnd(); j = v_right.neiNext(j))
   {
      int dir = mol.getBondDirection2(atom.right, v_right.neiVertex(j));
      if (dir != 0)
         sensible_bonds_out[v_right.neiEdge(j)] = 1;
   }

   return atom.parity != 3;
}

void MoleculeAlleneStereo::buildFromBonds (bool ignore_errors, int *sensible_bonds_out)
{
   BaseMolecule &mol = _getMolecule();
   int i;

   for (i = mol.vertexBegin(); i != mol.vertexEnd(); i = mol.vertexNext(i))
   {
      _Atom atom;

      if (!_isAlleneCenter(mol, i, atom, sensible_bonds_out))
         continue;
      
      _centers.insert(i, atom);
   }
}

void MoleculeAlleneStereo::clear ()
{
   _centers.clear();
}

bool MoleculeAlleneStereo::isCenter (int atom_idx)
{
   return _centers.at2(atom_idx) != 0;
}

int MoleculeAlleneStereo::size ()
{
   return _centers.size();
}

bool MoleculeAlleneStereo::checkSub (BaseMolecule &query, BaseMolecule &target, const int *mapping)
{
   int i;

   for (i = query.vertexBegin(); i != query.vertexEnd(); i = query.vertexNext(i))
   {
      const _Atom *qa = query.allene_stereo._centers.at2(i);

      if (qa == 0)
         continue;

      const _Atom *ta = target.allene_stereo._centers.at2(mapping[i]);

      if (ta == 0)
         return false;

      int parity = qa->parity;
      int qs[4], ts[4];
      int tmp;

      memcpy(qs, qa->subst, 4 * sizeof(int));
      memcpy(ts, ta->subst, 4 * sizeof(int));

      if (mapping[qs[0]] == ts[2] || mapping[qs[0]] == ts[3])
      {
         __swap(qs[0], qs[2], tmp);
         __swap(qs[1], qs[3], tmp);
      }

      if (mapping[qs[0]] == ts[0])
         ;
      else if (mapping[qs[0]] == ts[1])
         parity = 3 - parity;
      else
         throw Error("checkSub() subst[0] not mapped");

      if (mapping[qs[2]] == ts[2])
         ;
      else if (mapping[qs[2]] == ts[3])
         parity = 3 - parity;
      else
         throw Error("checkSub() subst[2] not mapped");

      if (parity != ta->parity)
         return false;
   }
   return true;
}
