/****************************************************************************
 * Copyright (C) 2009-2018 EPAM Systems
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

#include "base_cpp/output.h"
#include "base_cpp/scanner.h"
#include "molecule/structure_checker.h"
#include "molecule/molecule.h"
#include "molecule/query_molecule.h"
#include "molecule/elements.h"
#include "molecule/molecule_automorphism_search.h"

#include "third_party/rapidjson/writer.h"
#include "third_party/rapidjson/stringbuffer.h"
#include <sstream>


using namespace indigo;
using namespace rapidjson;
//#include "third_party/json/json.hpp"

IMPL_ERROR(StructureChecker, "Structure checker");

static StructureChecker::CheckType check_list[] = {
   {"NONE", StructureChecker::CHECK_NONE}, 
   {"LOAD", StructureChecker::CHECK_LOAD}, 
   {"VALENCE", StructureChecker::CHECK_VALENCE},
   {"RADICAL", StructureChecker::CHECK_RADICAL},
   {"PSEUDOATOM", StructureChecker::CHECK_PSEUDOATOM},
   {"STEREO", StructureChecker::CHECK_STEREO},
   {"QUERY", StructureChecker::CHECK_QUERY},
   {"OVERLAP_ATOM", StructureChecker::CHECK_OVERLAP_ATOM},
   {"OVERLAP_BOND", StructureChecker::CHECK_OVERLAP_BOND},
   {"RGROUP", StructureChecker::CHECK_RGROUP},
   {"SGROUP", StructureChecker::CHECK_SGROUP},
   {"TGROUP", StructureChecker::CHECK_TGROUP},
   {"CHIRALITY", StructureChecker::CHECK_CHIRALITY},
   {"CHIRAL_FLAG", StructureChecker::CHECK_CHIRAL_FLAG},
   {"3D_COORD", StructureChecker::CHECK_3D_COORD},
   {"CHARGE", StructureChecker::CHECK_CHARGE},
   {"SALT", StructureChecker::CHECK_SALT},
   {"AMBIGUOUS_H", StructureChecker::CHECK_AMBIGUOUS_H},
   {"COORD", StructureChecker::CHECK_COORD},
   {"ALL", StructureChecker::CHECK_ALL}
};

static StructureChecker::CheckMessage message_list[] = {
   {StructureChecker::CHECK_MSG_LOAD, StructureChecker::CHECK_LOAD, "Error at loading structure, wrong format found"},
   {StructureChecker::CHECK_MSG_EMPTY, StructureChecker::CHECK_LOAD, "Input structure is empty"},
   {StructureChecker::CHECK_MSG_VALENCE, StructureChecker::CHECK_VALENCE, "Structure contains atoms with unusuall valence"},
   {StructureChecker::CHECK_MSG_IGNORE_VALENCE_ERROR, StructureChecker::CHECK_VALENCE, "IGNORE_BAD_VALENCE flag is active"},
   {StructureChecker::CHECK_MSG_RADICAL, StructureChecker::CHECK_RADICAL, "Structure contains radicals"},
   {StructureChecker::CHECK_MSG_PSEUDOATOM, StructureChecker::CHECK_PSEUDOATOM, "Structure contains pseudoatoms"},
   {StructureChecker::CHECK_MSG_WRONG_STEREO, StructureChecker::CHECK_STEREO, "Structure contains incorrect stereochemistry"},
   {StructureChecker::CHECK_MSG_QUERY, StructureChecker::CHECK_QUERY, "Structure contains query features"},
   {StructureChecker::CHECK_MSG_OVERLAP_ATOM, StructureChecker::CHECK_OVERLAP_ATOM, "Structure contains overlapping atoms"},
   {StructureChecker::CHECK_MSG_OVERLAP_BOND, StructureChecker::CHECK_OVERLAP_BOND, "Structure contains overlapping bonds."},
   {StructureChecker::CHECK_MSG_RGROUP, StructureChecker::CHECK_RGROUP, "Structure contains R-groups"},
   {StructureChecker::CHECK_MSG_SGROUP, StructureChecker::CHECK_SGROUP, "Structure contains S-groups"},
   {StructureChecker::CHECK_MSG_TGROUP, StructureChecker::CHECK_TGROUP, "Structure contains SCSR templates"},
   {StructureChecker::CHECK_MSG_3D_STEREO, StructureChecker::CHECK_STEREO, "Structure contains stereocenters defined by 3D coordinates"},
   {StructureChecker::CHECK_MSG_UNDEFINED_STEREO, StructureChecker::CHECK_STEREO, "Structure contains stereocenters with undefined stereo configuration"},
   {StructureChecker::CHECK_MSG_CHIRAL_FLAG, StructureChecker::CHECK_CHIRAL_FLAG, "Structure contains wrong chiral flag"},
   {StructureChecker::CHECK_MSG_3D_COORD, StructureChecker::CHECK_3D_COORD, "Structure contains 3D coordinates"},
   {StructureChecker::CHECK_MSG_CHARGE, StructureChecker::CHECK_CHARGE, "Structure has non-zero charge"},
   {StructureChecker::CHECK_MSG_SALT, StructureChecker::CHECK_SALT, "Structure contains charged fragments (possible salt)"},
   {StructureChecker::CHECK_MSG_AMBIGUOUS_H, StructureChecker::CHECK_AMBIGUOUS_H, "Structure contains ambiguous hydrogens"},
   {StructureChecker::CHECK_MSG_ZERO_COORD, StructureChecker::CHECK_COORD, "Structure has no atoms coordinates"}
};

const char * StructureChecker::typeToString(dword check_type)
{
   for (int i = 0; i < NELEM(check_list); i++)
   {
      if (check_type == check_list[i].t_flag)
         return check_list[i].t_text;
   }
   return NULL;
}

dword StructureChecker::getType(const char * check_type)
{
   for (int i = 0; i < NELEM(check_list); i++)
   {
      if (strcasecmp(check_type, check_list[i].t_text) == 0)
      {
         return check_list[i].t_flag;
      }
   }
   return -1;
}


StructureChecker::StructureChecker (Output &output) : _output(output)
{
   check_flags = CHECK_ALL;
   check_result = 0;
   _results.clear();
}

void StructureChecker::checkMolecule (Scanner &scanner, const char *params)
{

}

void StructureChecker::checkMolecule (Molecule &mol)
{
   _results.clear();
   _checkMolecule(mol, false);
   buildCheckResult();
}

void StructureChecker::checkQueryMolecule (QueryMolecule &mol)
{
   _results.clear();
   _checkMolecule(mol, true);
   buildCheckResult();
}

bool StructureChecker::CheckType::compare (const char *text) const
{
   return strcasecmp(t_text, text) == 0 ? true : false;
}

void StructureChecker::parseCheckTypes (const char *params)
{
   int i;

   if (params == 0)
   {
      check_flags = CHECK_ALL;
      return;
   }

   dword inp_flags = CHECK_NONE;

   BufferScanner scanner(params);
   QS_DEF(Array<char>, word);
   QS_DEF(Array<int>, atoms);
   QS_DEF(Array<int>, bonds);

   scanner.skipSpace();
   if (scanner.isEOF())
      return;

   while (!scanner.isEOF())
   {
      scanner.readWord(word, " ,:");
      if (!scanner.isEOF())
         scanner.skip(1);

      for (i = 0; i < NELEM(check_list); i++)
      {
         if (check_list[i].compare(word.ptr()))
         {
            inp_flags |= check_list[i].t_flag;
            break;
         }
         else if (word[0] == '-' && check_list[i].compare(word.ptr() + 1))
         {
            inp_flags &= ~check_list[i].t_flag;
            break;
         }
         else if (strcasecmp(word.ptr(), "ATOMS") == 0)
         {
            scanner.skip(1);
            _parseSelection(scanner, atoms);
            break;
         }
         else if (strcasecmp(word.ptr(), "BONDS") == 0)
         {
            scanner.skip(1);
            _parseSelection(scanner, bonds);
            break;
         }
      }
   }
   if (inp_flags == CHECK_NONE)
      check_flags = CHECK_ALL;

   addAtomSelection (atoms);
   addBondSelection (bonds);
}

void StructureChecker::addAtomSelection (Array<int> &atoms)
{
   if (atoms.size() > 0)
   {
      _selected_atoms.clear();
      for (int i = 0; i < atoms.size(); i++)
      {
         _selected_atoms.push(atoms[i] - 1);
      }
   }
}

void StructureChecker::addBondSelection (Array<int> &bonds)
{
   if (bonds.size() > 0)
   {
      _selected_bonds.clear();
      for (int i = 0; i< bonds.size(); i++)
      {
         _selected_bonds.push(bonds[i] - 1);
      }
   }
}

void StructureChecker::buildCheckResult ()
{
   std::stringstream result;
//   json j;
   StringBuffer s;
   Writer<StringBuffer> writer(s);

   if (check_result == 0)
   {
      _output.writeString("{}");
      _output.writeChar(0);
      return;
   }

   writer.StartObject(); 


   for (int i = 0; i < _results.size(); i++)
   {
      CheckResult &res = _results[i];

      for (int m = 0; m < NELEM(message_list); m++)
      {
         if (res.m_id == message_list[m].m_id)
         {
            writer.Key(StructureChecker::typeToString(message_list[m].m_flag)); 
            writer.StartObject(); 
            writer.Key("message"); 
            writer.String(message_list[m].m_text); 
            if (res.atom_ids.size() > 0)
            {
               writer.Key("atoms"); 
               writer.StartArray();

               for (int k = 0; k < res.atom_ids.size(); k++)
                 writer.Uint(res.atom_ids[k] + 1);
               writer.EndArray();
            }

            if (res.bond_ids.size() > 0)
            {
               writer.Key("bonds"); 
               writer.StartArray();

               for (int k = 0; k < res.bond_ids.size(); k++)
                 writer.Uint(res.bond_ids[k] + 1);
               writer.EndArray();
            }
            writer.EndObject();
         }
      }
   }

   writer.EndObject();

   if (s.GetSize() > 0)
   {
      result << s.GetString();
      _output.printf("%s", result.str().c_str());
      _output.writeChar(0);
   }
   else
   {
      _output.writeString("{}");
      _output.writeChar(0);
   }
}

void StructureChecker::_parseSelection (Scanner &sc, Array<int> &ids)
{
   int id;
   ids.clear();

   while (!sc.isEOF())
   {
      if ( (sc.lookNext() == -1) || (sc.lookNext() == ',') )
         return;

      id = sc.readInt1();
      if (id > 0)
         ids.push(id);
   }
}

void StructureChecker::_checkMolecule (BaseMolecule &mol, bool query)
{
   QS_DEF(Molecule, target);
   bool _saved_valence_flag;
   check_result = 0;

   if ( (check_flags & CHECK_LOAD) && (mol.vertexCount() == 0) )
   {
      check_result |= CHECK_LOAD;
      CheckResult *res = &_results.push();
      res->m_id = CHECK_MSG_EMPTY;
      return;
   }

   if ( (check_flags & CHECK_CHIRAL_FLAG) && (mol.getChiralFlag() > 0) && (mol.stereocenters.size() == 0) )
   {
      check_result |= CHECK_CHIRAL_FLAG;
      CheckResult *res = &_results.push();
      res->m_id = CHECK_MSG_CHIRAL_FLAG;
   }

   if ( (check_flags & CHECK_RGROUP) && (mol.rgroups.getRGroupCount() > 0) )
   {
      check_result |= CHECK_RGROUP;
      CheckResult *res = &_results.push();
      res->m_id = CHECK_MSG_RGROUP;
   }

   if ( (check_flags & CHECK_TGROUP) && (mol.tgroups.getTGroupCount() > 0) )
   {
      check_result |= CHECK_TGROUP;
      CheckResult *res = &_results.push();
      res->m_id = CHECK_MSG_TGROUP;
   }

   if (BaseMolecule::hasCoord(mol) && ( (check_flags & CHECK_OVERLAP_ATOM) || (check_flags & CHECK_OVERLAP_BOND) ) )
   {
      mean_dist = 0.0;
      for (auto i : mol.edges())
      {
         const Edge &edge = mol.getEdge(i);
         Vec3f &a = mol.getAtomXyz(edge.beg);
         Vec3f &b = mol.getAtomXyz(edge.end);
         mean_dist += Vec3f::dist(a, b);
      }
      if (mol.edgeCount() > 0)
         mean_dist = mean_dist/mol.edgeCount();
   }

   if (!query)
   {
      _saved_valence_flag = mol.asMolecule().getIgnoreBadValenceFlag();
   }

   if ( (check_flags & CHECK_STEREO) && !query)
   {
      mol.asMolecule().setIgnoreBadValenceFlag(true);
      target.clone_KeepIndices(mol);

      for (auto i : target.vertices())
      {
         if (!target.stereocenters.exists(i) && target.stereocenters.isPossibleStereocenter(i))
         {
            target.stereocenters.add(i, MoleculeStereocenters::ATOM_ABS, 0, false);
         }
      }                          

      MoleculeAutomorphismSearch as;
      
      as.detect_invalid_cistrans_bonds = true;
      as.detect_invalid_stereocenters = true;
      as.find_canonical_ordering = false;
      as.process(target);

      for (auto i : target.vertices())
      {
         if (target.stereocenters.exists(i) && as.invalidStereocenter(i))
         {
            target.stereocenters.remove(i);
         }
      }
      mol.asMolecule().setIgnoreBadValenceFlag(false);
   }


   if ( (_selected_atoms.size() == 0) && (_selected_bonds.size() == 0) )
   {
      for (auto i : mol.vertices())
      {
         _checkAtom (mol, target, i, query);
      }

      for (auto i : mol.edges())
      {
         _checkBond (mol, target, i, query);
      }
   }
   else if ( (_selected_atoms.size() > 0) && (_selected_bonds.size() == 0) )
   {
      for (auto i = 0; i < _selected_atoms.size(); i++)
      {
         int idx = _selected_atoms[i];
         if ( (idx < 0) || (idx >= mol.vertexEnd()) )
            continue;

         _checkAtom (mol, target, idx, query);
      }
   }
   else if ( (_selected_atoms.size() == 0) && (_selected_bonds.size() > 0) )
   {
      for (auto i = 0; i < _selected_bonds.size(); i++)
      {
         int idx = _selected_bonds[i];
         if ( (idx < 0) || (idx >= mol.edgeEnd()) )
            continue;

         _checkBond (mol, target, idx, query);

         const Edge &edge = mol.getEdge(idx);
         _checkAtom (mol, target, edge.beg, query);
         _checkAtom (mol, target, edge.end, query);
      }
   }
   else
   {
      for (auto i = 0; i < _selected_atoms.size(); i++)
      {
         int idx = _selected_atoms[i];
         if ( (idx < 0) || (idx >= mol.vertexEnd()) )
            continue;

         _checkAtom (mol, target, idx, query);
      }

      for (auto i = 0; i < _selected_bonds.size(); i++)
      {
         int idx = _selected_bonds[i];
         if ( (idx < 0) || (idx >= mol.edgeEnd()) )
            continue;

         _checkBond (mol, target, idx, query);

         const Edge &edge = mol.getEdge(idx);
         if (_selected_atoms.find(edge.beg) == -1)
         {
            _checkAtom (mol, target, edge.beg, query);
            _selected_atoms.push(edge.beg);
         }
         if (_selected_atoms.find(edge.end) == -1)
         {
            _checkAtom (mol, target, edge.end, query);
            _selected_atoms.push(edge.end);
         }
      }
   }

   if (_saved_valence_flag)
      mol.asMolecule().setIgnoreBadValenceFlag(true);

   if (_bad_val_ids.size() > 0)
   {
      check_result |= CHECK_VALENCE;
      CheckResult *res = &_results.push();
      res->m_id = CHECK_MSG_VALENCE;
      res->atom_ids.copy(_bad_val_ids);
   }

   if (_rad_ids.size() > 0)
   {
      check_result |= CHECK_RADICAL;
      CheckResult *res = &_results.push();
      res->m_id = CHECK_MSG_RADICAL;
      res->atom_ids.copy(_rad_ids);
   }

   if ( (_atom_qf_ids.size() > 0) || (_bond_qf_ids.size() > 0) )
   {
      check_result |= CHECK_QUERY;
      CheckResult *res = &_results.push();
      res->m_id = CHECK_MSG_QUERY;
      res->atom_ids.copy(_atom_qf_ids);
      res->bond_ids.copy(_bond_qf_ids);
   }

   if ( (_sg_atom_ids.size() > 0) || (_sg_bond_ids.size() > 0) )
   {
      check_result |= CHECK_SGROUP;
      CheckResult *res = &_results.push();
      res->m_id = CHECK_MSG_SGROUP;
      res->atom_ids.copy(_sg_atom_ids);
      res->bond_ids.copy(_sg_bond_ids);
   }

   if (_atom_3d_ids.size() > 0) 
   {
      check_result |= CHECK_3D_COORD;
      CheckResult *res = &_results.push();
      res->m_id = CHECK_MSG_3D_COORD;
      res->atom_ids.copy(_atom_3d_ids);
   }

   if (_atom_amb_h_ids.size() > 0) 
   {
      check_result |= CHECK_AMBIGUOUS_H;
      CheckResult *res = &_results.push();
      res->m_id = CHECK_MSG_AMBIGUOUS_H;
      res->atom_ids.copy(_atom_amb_h_ids);
   }

   if (_atom_3d_stereo_ids.size() > 0) 
   {
      check_result |= CHECK_STEREO;
      CheckResult *res = &_results.push();
      res->m_id = CHECK_MSG_3D_STEREO;
      res->atom_ids.copy(_atom_3d_stereo_ids);
   }

   if (_atom_wrong_stereo_ids.size() > 0) 
   {
      check_result |= CHECK_STEREO;
      CheckResult *res = &_results.push();
      res->m_id = CHECK_MSG_WRONG_STEREO;
      res->atom_ids.copy(_atom_wrong_stereo_ids);
   }

   if (_atom_undefined_stereo_ids.size() > 0) 
   {
      check_result |= CHECK_STEREO;
      CheckResult *res = &_results.push();
      res->m_id = CHECK_MSG_UNDEFINED_STEREO;
      res->atom_ids.copy(_atom_undefined_stereo_ids);
   }

   if (_overlapped_atom_ids.size() > 0) 
   {
      check_result |= CHECK_OVERLAP_ATOM;
      CheckResult *res = &_results.push();
      res->m_id = CHECK_MSG_OVERLAP_ATOM;
      res->atom_ids.copy(_overlapped_atom_ids);
   }

   if (_overlapped_bond_ids.size() > 0) 
   {
      check_result |= CHECK_OVERLAP_BOND;
      CheckResult *res = &_results.push();
      res->m_id = CHECK_MSG_OVERLAP_BOND;
      res->bond_ids.copy(_overlapped_bond_ids);
   }

}

void StructureChecker::_checkAtom (BaseMolecule &mol, Molecule &target, int idx, bool query)
{
   if (!mol.isPseudoAtom(idx) && !mol.isRSite(idx) && !mol.isTemplateAtom(idx))
   {
      if ((check_flags & CHECK_VALENCE) && !query)
      {
         if (mol.getAtomValence_NoThrow(idx, BAD_VALENCE) == BAD_VALENCE)
            _bad_val_ids.push(idx);
      }

      if ((check_flags & CHECK_RADICAL))
      {
         int rad = mol.getAtomRadical_NoThrow(idx, BAD_RADICAL);
         if ( (rad > 0) && (rad != BAD_RADICAL) )
            _rad_ids.push(idx);
      }

      if ((check_flags & CHECK_QUERY) && !query)
      {
         if ( (mol.reaction_atom_inversion[idx] > 0) || (mol.reaction_atom_exact_change[idx] > 0) )
            _atom_qf_ids.push(idx);
      }

      if ((check_flags & CHECK_AMBIGUOUS_H) && !query)
      {
        if (mol.asMolecule().getImplicitH_NoThrow(idx, -1) == -1 && mol.getAtomAromaticity(idx) == ATOM_AROMATIC)
           _atom_amb_h_ids.push(idx);
      }

      if ((check_flags & CHECK_STEREO) && BaseMolecule::hasZCoord(mol))
      {
         if (mol.stereocenters.exists(idx))
         {
            bool stereo_3d = true;
            const Vertex &vertex = mol.getVertex(idx);
            for (auto j = vertex.neiBegin(); j != vertex.neiEnd(); j = vertex.neiNext(j))
               if (mol.getBondDirection2(idx, vertex.neiVertex(j)) > 0)
                  stereo_3d = false;
            if (stereo_3d)
               _atom_3d_stereo_ids.push(idx);
         }
      }

      if ((check_flags & CHECK_STEREO) && !query)
      {
         if ( (mol.stereocenters.exists(idx) && target.stereocenters.exists(idx)) &&
              (mol.stereocenters.getType(idx) != target.stereocenters.getType(idx)) )
         {
            _atom_wrong_stereo_ids.push(idx);
         }
         else if (mol.stereocenters.exists(idx) && !target.stereocenters.exists(idx))
         {
            _atom_wrong_stereo_ids.push(idx);
         }
         else if (!mol.stereocenters.exists(idx) && target.stereocenters.exists(idx))
         {
            _atom_undefined_stereo_ids.push(idx);
         }
      }
   }
   else if (mol.isPseudoAtom(idx) && (check_flags & CHECK_PSEUDOATOM))
   {
      _pseudo_ids.push(idx);
   }

   if ( (check_flags & CHECK_SGROUP) && (mol.sgroups.getSGroupCount() > 0) )
   {
      for (int i = mol.sgroups.begin(); i != mol.sgroups.end(); i = mol.sgroups.next(i))
      {
         SGroup &sg = mol.sgroups.getSGroup(i);
         if ( (sg.atoms.find(idx) != -1) && (_sg_atom_ids.find(idx) == -1) )
            _sg_atom_ids.push(idx);
      }
   }

   if (check_flags & CHECK_3D_COORD) 
   {
      if (fabs(mol.getAtomXyz(idx).z) > 0.001) 
         _atom_3d_ids.push(idx);
   }

   if (BaseMolecule::hasCoord(mol) && (check_flags & CHECK_OVERLAP_ATOM))
   {
      if (_overlapped_atom_ids.find(idx) != -1)
         return;

      Vec3f &a = mol.getAtomXyz(idx);
      bool added = false;
      for (auto i : mol.vertices())
      {
         if ( (i != idx) && (_overlapped_atom_ids.find(i) == -1) )
         {
            Vec3f &b = mol.getAtomXyz(i);
            if ( (mean_dist > 0.0) && (Vec3f::dist(a,b) < 0.25*mean_dist) )
            {
               if (!added)
               {
                  _overlapped_atom_ids.push(idx);
                  added = true;
               }
               _overlapped_atom_ids.push(i);
            }
         }
      }
   }
   else if (!BaseMolecule::hasCoord(mol) && mol.vertexCount() > 1)
   {
      check_result |= CHECK_COORD;
      CheckResult *res = &_results.push();
      res->m_id = CHECK_MSG_ZERO_COORD;
   }
}

void StructureChecker::_checkBond (BaseMolecule &mol, Molecule &target, int idx, bool query)
{
   if ((check_flags & CHECK_QUERY) && !query)
   {
      if (mol.reaction_bond_reacting_center[idx] != 0) 
      {
         _bond_qf_ids.push(idx);
      }
   }

   if (BaseMolecule::hasCoord(mol) && (check_flags & CHECK_OVERLAP_BOND))
   {
      if (_overlapped_bond_ids.find(idx) != -1)
         return;

      bool added = false;
      const Edge &e1 = mol.getEdge(idx);
      Vec2f a1, b1, a2, b2;
      Vec2f::projectZ(a1, mol.getAtomXyz(e1.beg));
      Vec2f::projectZ(b1, mol.getAtomXyz(e1.end));

      for (auto i : mol.edges())
      {
         if ( (i != idx) && (_overlapped_bond_ids.find(i) == -1) )
         {
            const Edge &e2 = mol.getEdge(i);
            Vec2f::projectZ(a2, mol.getAtomXyz(e2.beg));
            Vec2f::projectZ(b2, mol.getAtomXyz(e2.end));
            if ( (Vec2f::dist(a1, a2) < 0.01*mean_dist) || (Vec2f::dist(b1, b2) < 0.01*mean_dist) ||
                 (Vec2f::dist(a1, b2) < 0.01*mean_dist) || (Vec2f::dist(b1, a2) < 0.01*mean_dist) )
               continue;

            if (Vec2f::segmentsIntersect(a1, b1, a2, b2))
            {
               if (!added)
               {
                  _overlapped_bond_ids.push(idx);
                  added = true;
               }
               _overlapped_bond_ids.push(i);
            }
         }
      }
   }
}