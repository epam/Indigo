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

#include "molecule/molfile_saver.h"

#include <time.h>

#include "base_cpp/output.h"
#include "base_cpp/locale_guard.h"
#include "molecule/molecule.h"
#include "molecule/molecule_stereocenters.h"
#include "molecule/query_molecule.h"
#include "molecule/elements.h"
#include "molecule/molecule_savers.h"

using namespace indigo;

IMPL_ERROR(MolfileSaver, "molfile saver");

MolfileSaver::MolfileSaver (Output &output) :
reactionAtomMapping(0),
reactionAtomInversion(0),
reactionAtomExactChange(0),
reactionBondReactingCenter(0),
 _output(output),
TL_CP_GET(_atom_mapping),
TL_CP_GET(_bond_mapping)
{
   mode = MODE_AUTO;
   no_chiral = false;
   skip_date = false;
}

void MolfileSaver::saveBaseMolecule (BaseMolecule &mol)
{
   _saveMolecule(mol, mol.isQueryMolecule());
}

void MolfileSaver::saveMolecule (Molecule &mol)
{
   _saveMolecule(mol, false);
}

void MolfileSaver::saveQueryMolecule (QueryMolecule &mol)
{
   _saveMolecule(mol, true);
}

void MolfileSaver::_saveMolecule (BaseMolecule &mol, bool query)
{
   LocaleGuard locale_guard;
   
   QueryMolecule *qmol = 0;

   if (query)
      qmol = (QueryMolecule *)(&mol);

   if (mode == MODE_2000)
      _v2000 = true;
   else if (mode == MODE_3000)
      _v2000 = false;
   else
   {
      // auto-detect the format: save to v3000 molfile only
      // if v2000 is not enough
      _v2000 = true;

      if (mol.hasHighlighting())
         _v2000 = false;
      else if (!mol.stereocenters.haveAllAbsAny() && !mol.stereocenters.haveAllAndAny())
         _v2000 = false;
      else if (mol.vertexCount() > 999 || mol.edgeCount() > 999)
         _v2000 = false;
   }

   bool rg2000 = (_v2000 && mol.rgroups.getRGroupCount() > 0);

   if (rg2000)
   {
      struct tm lt;
      if (skip_date)
         memset(&lt, 0, sizeof(lt));
      else
      {
         time_t tm = time(NULL);
         lt = *localtime(&tm);
      }
      _output.printfCR("$MDL  REV  1 %02d%02d%02d%02d%02d",
         lt.tm_mon + 1, lt.tm_mday, lt.tm_year % 100, lt.tm_hour, lt.tm_min);
      _output.writeStringCR("$MOL");
      _output.writeStringCR("$HDR");
   }
      
   _writeHeader(mol, _output, BaseMolecule::hasZCoord(mol));

   if (rg2000)
   {
      _output.writeStringCR("$END HDR");
      _output.writeStringCR("$CTAB");
   }

   if (_v2000)
   {
      _writeCtabHeader2000(_output, mol);
      _writeCtab2000(_output, mol, query);
   }
   else
   {
      _writeCtabHeader(_output);
      _writeCtab(_output, mol, query);
   }

   if (_v2000)
   {
      _writeRGroupIndices2000(_output, mol);
      _writeAttachmentValues2000(_output, mol);
   }
   
   if (rg2000)
   {
      int i, j;

      MoleculeRGroups &rgroups = mol.rgroups;
      int n_rgroups = rgroups.getRGroupCount();

      for (i = 1; i <= n_rgroups; i++)
      {
         RGroup &rgroup = rgroups.getRGroup(i);

         if (rgroup.fragments.size() == 0)
            continue;

         _output.printf("M  LOG  1 %3d %3d %3d  ", i, rgroup.if_then, rgroup.rest_h);

         QS_DEF(Array<char>, occ);
         ArrayOutput occ_out(occ);

         _writeOccurrenceRanges(occ_out, rgroup.occurrence);

         for (j = 0; j < 3 - occ.size(); j++)
            _output.writeChar(' ');

         _output.write(occ.ptr(), occ.size());
         _output.writeCR();
      }

      _output.writeStringCR("M  END");
      _output.writeStringCR("$END CTAB");

      for (i = 1; i <= n_rgroups; i++)
      {
         PtrPool<BaseMolecule> &frags = rgroups.getRGroup(i).fragments;

         if (frags.size() == 0)
            continue;

         _output.writeStringCR("$RGP");
         _output.printfCR("%4d", i);

         for (j = frags.begin(); j != frags.end(); j = frags.next(j))
         {
            BaseMolecule *fragment = frags[j];

            _output.writeStringCR("$CTAB");
            _writeCtabHeader2000(_output, *fragment);
            _writeCtab2000(_output, *fragment, query);
            _writeRGroupIndices2000(_output, *fragment);
            _writeAttachmentValues2000(_output, *fragment);

            _output.writeStringCR("M  END");
            _output.writeStringCR("$END CTAB");
         }
         _output.writeStringCR("$END RGP");
      }
      _output.writeStringCR("$END MOL");
   }
   else
      _output.writeStringCR("M  END");
}

void MolfileSaver::saveCtab3000 (Molecule &mol)
{
   _writeCtab(_output, mol, false);
}

void MolfileSaver::saveQueryCtab3000 (QueryMolecule &mol)
{
   _writeCtab(_output, mol, true);
}

void MolfileSaver::_writeHeader (BaseMolecule &mol, Output &output, bool zcoord)
{
   struct tm lt;
   if (skip_date)
      memset(&lt, 0, sizeof(lt));
   else
   {
      time_t tm = time(NULL);
      lt = *localtime(&tm);
   }

   const char *dim;

   if (zcoord)
      dim = "3D";
   else
      dim = "2D";

   if (mol.name.ptr() != 0)
      output.printfCR("%s", mol.name.ptr());
   else
      output.writeCR();
   output.printfCR("  -INDIGO-%02d%02d%02d%02d%02d%s", lt.tm_mon + 1, lt.tm_mday,
      lt.tm_year % 100, lt.tm_hour, lt.tm_min, dim);
   output.writeCR();
}

void MolfileSaver::_writeCtabHeader (Output &output)
{
   output.printfCR("%3d%3d%3d%3d%3d%3d%3d%3d%3d%3d%3d V3000",
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

void MolfileSaver::_writeCtabHeader2000 (Output &output, BaseMolecule &mol)
{
   int chiral = 0;

   if (!no_chiral && mol.isChrial())
      chiral = 1;

   output.printfCR("%3d%3d%3d%3d%3d%3d%3d%3d%3d%3d%3d V2000",
      mol.vertexCount(), mol.edgeCount(), 0, 0, chiral, 0, 0, 0, 0, 0, 999);
}

void MolfileSaver::_writeAtomLabel (Output &output, int label)
{
   output.writeString(Element::toString(label));
}

void MolfileSaver::_writeMultiString (Output &output, const char *string, int len)
{
   int limit = 70;
   while (len > 0)
   {
      output.writeString("M  V30 ");

      if (len <= limit)
         limit = len;

      output.write(string, limit);
      if (len != limit)
         output.writeString("-");
      output.writeCR();
      len -= limit;
      string += limit;
   }
}

bool MolfileSaver::_getRingBondCountFlagValue (QueryMolecule &qmol, int idx, int &value)
{
   QueryMolecule::Atom &atom = qmol.getAtom(idx);
   int rbc;
   if (atom.hasConstraint(QueryMolecule::ATOM_RING_BONDS))
   {
      if (atom.sureValue(QueryMolecule::ATOM_RING_BONDS, rbc))
      {
         value = rbc;
         if (value == 0)
            value = -1;
         return true;
      }
      int rbc_values[1] = { 4 };
      if (atom.sureValueBelongs(QueryMolecule::ATOM_RING_BONDS, rbc_values, 1))
      {
         value = 4;
         return true;
      }
   }
   else if (atom.sureValue(QueryMolecule::ATOM_RING_BONDS_AS_DRAWN, rbc))
   {
      value = -2;
      return true;
   }
   return false;
}

bool MolfileSaver::_getSubstitutionCountFlagValue (QueryMolecule &qmol, int idx, int &value)
{
   QueryMolecule::Atom &atom = qmol.getAtom(idx);
   int v;
   if (atom.hasConstraint(QueryMolecule::ATOM_SUBSTITUENTS))
   {
      if (atom.sureValue(QueryMolecule::ATOM_SUBSTITUENTS, v))
      {
         value = v;
         if (value == 0)
            value = -1;
         return true;
      }
      int values[1] = { 4 };
      if (atom.sureValueBelongs(QueryMolecule::ATOM_SUBSTITUENTS, values, 1))
      {
         value = 4;
         return true;
      }
   }
   else if (atom.sureValue(QueryMolecule::ATOM_SUBSTITUENTS_AS_DRAWN, v))
   {
      value = -2;
      return true;
   }
   return false;
}

void MolfileSaver::_writeCtab (Output &output, BaseMolecule &mol, bool query)
{
   QueryMolecule *qmol = 0;

   if (query)
      qmol = (QueryMolecule *)(&mol);

   output.writeStringCR("M  V30 BEGIN CTAB");
   output.printfCR("M  V30 COUNTS %d %d %d 0 0", mol.vertexCount(), mol.edgeCount(), mol.countSGroups());
   output.writeStringCR("M  V30 BEGIN ATOM");

   int i;
   int iw = 1;
   QS_DEF(Array<char>, buf);

   _atom_mapping.clear_resize(mol.vertexEnd());
   _bond_mapping.clear_resize(mol.edgeEnd());

   for (i = mol.vertexBegin(); i < mol.vertexEnd(); i = mol.vertexNext(i), iw++)
   {
      int atom_number = mol.getAtomNumber(i);
      int isotope = mol.getAtomIsotope(i);
      ArrayOutput out(buf);

      _atom_mapping[i] = iw;
      out.printf("%d ", iw);
      QS_DEF(Array<int>, list);
      int query_atom_type;

      if (atom_number == ELEM_H && isotope == 2)
      {
         out.writeChar('D');
         isotope = 0;
      }
      else if (atom_number == ELEM_H && isotope == 3)
      {
         out.writeChar('T');
         isotope = 0;
      }
      else if (mol.isPseudoAtom(i))
         out.writeString(mol.getPseudoAtom(i));
      else if (mol.isRSite(i))
         out.writeString("R#");
      else if (atom_number > 0)
      {
         _writeAtomLabel(out, atom_number);
      }
      else if (qmol != 0 &&
              (query_atom_type = QueryMolecule::parseQueryAtom(*qmol, i, list)) != -1)
      {
         if (query_atom_type == QueryMolecule::QUERY_ATOM_A)
            out.writeChar('A');
         else if (query_atom_type == QueryMolecule::QUERY_ATOM_Q)
            out.writeChar('Q');
         else if (query_atom_type == QueryMolecule::QUERY_ATOM_X)
            out.writeChar('X');
         else if (query_atom_type == QueryMolecule::QUERY_ATOM_LIST ||
                  query_atom_type == QueryMolecule::QUERY_ATOM_NOTLIST)
         {
            int k;

            if (query_atom_type == QueryMolecule::QUERY_ATOM_NOTLIST)
               out.writeString("NOT");
            
            out.writeChar('[');
            for (k = 0; k < list.size(); k++)
            {
               if (k > 0)
                  out.writeChar(',');
               _writeAtomLabel(out, list[k]);
            }
            out.writeChar(']');
         }
      }
      else if (atom_number == -1)
         out.writeChar('A');
      else
         throw Error("molfile 3000: can not save atom %d because of unsupported "
                     "query feature", i); 

      int aam = 0, ecflag = 0, irflag = 0;

      if (reactionAtomMapping != 0)
         aam = reactionAtomMapping->at(i);
      if (reactionAtomInversion != 0)
         irflag = reactionAtomInversion->at(i);
      if (reactionAtomExactChange != 0)
         ecflag = reactionAtomExactChange->at(i);

      Vec3f &xyz = mol.getAtomXyz(i);
      int charge = mol.getAtomCharge(i);
      int radical = 0;
      int valence = mol.getExplicitValence(i);
      int stereo_parity = _getStereocenterParity(mol, i);

      if (!mol.isRSite(i) && !mol.isPseudoAtom(i))
         radical = mol.getAtomRadical_NoThrow(i, 0);

      out.printf(" %f %f %f %d", xyz.x, xyz.y, xyz.z, aam);

      if ((mol.isQueryMolecule() && charge != CHARGE_UNKNOWN) || (!mol.isQueryMolecule() && charge != 0))
         out.printf(" CHG=%d", charge);

      int hcount = MoleculeSavers::getHCount(mol, i, atom_number, charge);
      if (hcount > 0)
         out.printf(" HCOUNT=%d", hcount);
      else if (hcount == 0)
         out.printf(" HCOUNT=-1");

      if (radical > 0)
         out.printf(" RAD=%d", radical);
      if (stereo_parity > 0)
         out.printf(" CFG=%d", stereo_parity);
      if (isotope > 0)
         out.printf(" MASS=%d", isotope);
      if (valence > 0)
         out.printf(" VAL=%d", valence);
      if (valence == 0)
         out.printf(" VAL=-1");
      if (irflag > 0)
         out.printf(" INVRET=%d", irflag);
      if (ecflag > 0)
         out.printf(" EXACHG=%d", ecflag);

      if (mol.isRSite(i))
      {
         int k;

         QS_DEF(Array<int>, rg_list);
         mol.getAllowedRGroups(i, rg_list);

         if (rg_list.size() > 0)
         {
            out.printf(" RGROUPS=(%d", rg_list.size());
            for (k = 0; k < rg_list.size(); k++)
               out.printf(" %d", rg_list[k]);
            out.writeChar(')');

            if (!_checkAttPointOrder(mol, i))
            {
               const Vertex &vertex = mol.getVertex(i);
               
               out.printf(" ATTCHORD=(%d", vertex.degree() * 2);
               for (k = 0; k < vertex.degree(); k++)
                  out.printf(" %d %d", _atom_mapping[mol.getRSiteAttachmentPointByOrder(i, k)], k + 1);

               out.writeChar(')');
            }
         }
      }

      if (mol.attachmentPointCount() > 0)
      {
         int val = 0;

         for (int idx = 1; idx <= mol.attachmentPointCount(); idx++)
         {
            for (int j = 0; mol.getAttachmentPoint(idx, j) != -1; j++)
               if (mol.getAttachmentPoint(idx, j) == i)
               {
                  val |= 1 << (idx - 1);
                  break;
               }
         }

         if (val > 0)
            out.printf(" ATTCHPT=%d", val == 3 ? -1 : val);
      }

      if (qmol != 0)
      {
         int unsat;
         if (qmol->getAtom(i).sureValue(QueryMolecule::ATOM_UNSATURATION, unsat))
            out.printf(" UNSAT=1");
         int subst;
         if (_getSubstitutionCountFlagValue(*qmol, i, subst))
            out.printf(" SUBST=%d", subst);
         int rbc;
         if (_getRingBondCountFlagValue(*qmol, i, rbc))
            out.printf(" RBCNT=%d", rbc);
      }

      _writeMultiString(output, buf.ptr(), buf.size());
   }

   output.writeStringCR("M  V30 END ATOM");
   output.writeStringCR("M  V30 BEGIN BOND");

   iw = 1;

   for (i = mol.edgeBegin(); i < mol.edgeEnd(); i = mol.edgeNext(i), iw++)
   {
      const Edge &edge = mol.getEdge(i);
      int bond_order = mol.getBondOrder(i);
      ArrayOutput out(buf);

      _bond_mapping[i] = iw;

      if (bond_order < 0 && qmol != 0)
      {
         int qb = QueryMolecule::getQueryBondType(qmol->getBond(i));

         if (qb == QueryMolecule::QUERY_BOND_SINGLE_OR_DOUBLE)
            bond_order = 5;
         else if (qb == QueryMolecule::QUERY_BOND_SINGLE_OR_AROMATIC)
            bond_order = 6;
         else if (qb == QueryMolecule::QUERY_BOND_DOUBLE_OR_AROMATIC)
            bond_order = 7;
         else if (qb == QueryMolecule::QUERY_BOND_ANY)
            bond_order = 8;
      }

      if (bond_order < 0)
         throw Error("unrepresentable query bond");

      out.printf("%d %d %d %d", iw, bond_order, _atom_mapping[edge.beg], _atom_mapping[edge.end]);

      int direction = mol.getBondDirection(i);

      switch (direction)
      {
         case BOND_UP:     out.printf(" CFG=1"); break;
         case BOND_EITHER: out.printf(" CFG=2"); break;
         case BOND_DOWN:   out.printf(" CFG=3"); break;
         case 0:
            if (mol.cis_trans.isIgnored(i))
               if (!_hasNeighborEitherBond(mol, i))
                  out.printf(" CFG=2");
            break;
      }

      int reacting_center = 0;

      if(reactionBondReactingCenter != 0 && reactionBondReactingCenter->at(i) != 0)
         reacting_center = reactionBondReactingCenter->at(i);

      if (reacting_center != 0)
         out.printf(" RXCTR=%d", reacting_center);

      int indigo_topology = -1;
      if (qmol != 0)
         qmol->getBond(i).sureValue(QueryMolecule::BOND_TOPOLOGY, indigo_topology);

      int topology = 0;
      if (indigo_topology == TOPOLOGY_RING)
         topology = 1;
      else if (indigo_topology == TOPOLOGY_CHAIN)
         topology = 2;

      if (topology != 0)
         out.printf(" TOPO=%d", topology);

      _writeMultiString(output, buf.ptr(), buf.size());
   }

   output.writeStringCR("M  V30 END BOND");

   MoleculeStereocenters &stereocenters = mol.stereocenters;

   if (stereocenters.begin() != stereocenters.end() || mol.hasHighlighting())
   {
      output.writeStringCR("M  V30 BEGIN COLLECTION");

      QS_DEF(Array<int>, processed);

      processed.clear_resize(mol.vertexEnd());
      processed.zerofill();

      for (i = mol.vertexBegin(); i != mol.vertexEnd(); i = mol.vertexNext(i))
      {
         if (processed[i])
            continue;

         ArrayOutput out(buf);
         int j, type = stereocenters.getType(i);

         if (type == MoleculeStereocenters::ATOM_ABS)
            out.writeString("MDLV30/STEABS ATOMS=(");
         else if (type == MoleculeStereocenters::ATOM_OR)
            out.printf("MDLV30/STEREL%d ATOMS=(", stereocenters.getGroup(i));
         else if (type == MoleculeStereocenters::ATOM_AND)
            out.printf("MDLV30/STERAC%d ATOMS=(", stereocenters.getGroup(i));
         else
            continue;
            
         QS_DEF(Array<int>, list);

         list.clear();
         list.push(i);

         for (j = mol.vertexNext(i); j < mol.vertexEnd(); j = mol.vertexNext(j))
            if (stereocenters.sameGroup(i, j))
            {
               list.push(j);
               processed[j] = 1;
            }

         out.printf("%d", list.size());
         for (j = 0; j < list.size(); j++)
            out.printf(" %d", _atom_mapping[list[j]]);
         out.writeChar(')');

         _writeMultiString(output, buf.ptr(), buf.size());
      }

      if (mol.hasHighlighting())
      {
         if (mol.countHighlightedBonds() > 0)
         {
            ArrayOutput out(buf);
            
            out.printf("MDLV30/HILITE BONDS=(%d", mol.countHighlightedBonds());
            
            for (i = mol.edgeBegin(); i != mol.edgeEnd(); i = mol.edgeNext(i))
               if (mol.isBondHighlighted(i))
                  out.printf(" %d", _bond_mapping[i]);
            out.writeChar(')');

            _writeMultiString(output, buf.ptr(), buf.size());
         }
         if (mol.countHighlightedAtoms() > 0)
         {
            ArrayOutput out(buf);
            out.printf("MDLV30/HILITE ATOMS=(%d", mol.countHighlightedAtoms());
            for (i = mol.vertexBegin(); i != mol.vertexEnd(); i = mol.vertexNext(i))
               if (mol.isAtomHighlighted(i))
                  out.printf(" %d", _atom_mapping[i]);
            out.writeChar(')');

            _writeMultiString(output, buf.ptr(), buf.size());
         }
      }
      output.writeStringCR("M  V30 END COLLECTION");
   }

   if (mol.countSGroups() > 0)
   {
      int idx = 1;

      output.writeStringCR("M  V30 BEGIN SGROUP");
      for (i = mol.generic_sgroups.begin(); i != mol.generic_sgroups.end(); i = mol.generic_sgroups.next(i))
      {
         ArrayOutput out(buf);
         _writeGenericSGroup3000(mol.generic_sgroups[i], idx++, "GEN", out);
         _writeMultiString(output, buf.ptr(), buf.size());
      }
      for (i = mol.superatoms.begin(); i != mol.superatoms.end(); i = mol.superatoms.next(i))
      {
         ArrayOutput out(buf);
         _writeGenericSGroup3000(mol.superatoms[i], idx++, "SUP", out);
         if (mol.superatoms[i].bond_idx >= 0)
            out.printf(" XBONDS=(1 %d)", _bond_mapping[mol.superatoms[i].bond_idx]);
         if (mol.superatoms[i].subscript.size() > 1)
            out.printf(" LABEL=%s", mol.superatoms[i].subscript.ptr());
         out.printf(" ESTATE=E");
         _writeMultiString(output, buf.ptr(), buf.size());
      }
      for (i = mol.data_sgroups.begin(); i != mol.data_sgroups.end(); i = mol.data_sgroups.next(i))
      {
         ArrayOutput out(buf);
         _writeGenericSGroup3000(mol.data_sgroups[i], idx++, "DAT", out);
         const char *desc = mol.data_sgroups[i].description.ptr();
         if (desc != 0 && strlen(desc) > 0)
         {
            out.writeString(" FIELDNAME=");
            bool space_found = (strchr(desc, ' ') != NULL);
            if (space_found)
               out.writeString("\"");
            out.writeString(desc);
            if (space_found)
               out.writeString("\"");
         }
         out.printf(" FIELDDISP=\"");
         _writeDataSGroupDisplay(mol.data_sgroups[i], out);
         out.printf("\"");
         if (mol.data_sgroups[i].data.size() > 0 && mol.data_sgroups[i].data[0] != 0)
         {
            // Split field data by new lines
            int len = mol.data_sgroups[i].data.size();
            char *data = mol.data_sgroups[i].data.ptr();
            while (len > 0)
            {
               int j;
               for (j = 0; j < len; j++)
                  if (data[j] == '\n')
                     break;

               out.printf(" FIELDDATA=\"%.*s\"", j, data);
               if (data[j] == '\n')
                  j++;

               data += j;
               len -= j;

               if (*data == 0)
                  break;
            }
         }
         _writeMultiString(output, buf.ptr(), buf.size());
      }
      for (i = mol.repeating_units.begin(); i != mol.repeating_units.end(); i = mol.repeating_units.next(i))
      {
         ArrayOutput out(buf);
         _writeGenericSGroup3000(mol.repeating_units[i], idx++, "SRU", out);
         if (mol.repeating_units[i].connectivity == BaseMolecule::RepeatingUnit::HEAD_TO_HEAD)
            out.printf(" CONNECT=HH");
         else if (mol.repeating_units[i].connectivity == BaseMolecule::RepeatingUnit::HEAD_TO_TAIL)
            out.printf(" CONNECT=HT");
         else
            out.printf(" CONNECT=EU");
         if (mol.repeating_units[i].subscript.size() > 1)
            out.printf(" LABEL=%s", mol.repeating_units[i].subscript.ptr());
         _writeMultiString(output, buf.ptr(), buf.size());
      }
      for (i = mol.multiple_groups.begin(); i != mol.multiple_groups.end(); i = mol.multiple_groups.next(i))
      {
         ArrayOutput out(buf);
         _writeGenericSGroup3000(mol.multiple_groups[i], idx++, "MUL", out);
         if (mol.multiple_groups[i].parent_atoms.size() > 0)
         {
            out.printf(" PATOMS=(%d", mol.multiple_groups[i].parent_atoms.size());
            int j;
            for (j = 0; j < mol.multiple_groups[i].parent_atoms.size(); j++)
               out.printf(" %d", _atom_mapping[mol.multiple_groups[i].parent_atoms[j]]);
            out.printf(")");
         }
         out.printf(" MULT=%d", mol.multiple_groups[i].multiplier);
         _writeMultiString(output, buf.ptr(), buf.size());
      }
      output.writeStringCR("M  V30 END SGROUP");
   }

   output.writeStringCR("M  V30 END CTAB");

   int n_rgroups = mol.rgroups.getRGroupCount();
   for (i = 1; i <= n_rgroups; i++)
      if (mol.rgroups.getRGroup(i).fragments.size() > 0)
         _writeRGroup(output, mol, i);
}

void MolfileSaver::_writeGenericSGroup3000 (BaseMolecule::SGroup &sgroup, int idx, const char *type, Output &output)
{
   int i;

   output.printf("%d %s %d", idx, type, idx);

   if (sgroup.atoms.size() > 0)
   {
      output.printf(" ATOMS=(%d", sgroup.atoms.size());
      for (i = 0; i < sgroup.atoms.size(); i++)
         output.printf(" %d", _atom_mapping[sgroup.atoms[i]]);
      output.printf(")");
   }
   if (sgroup.bonds.size() > 0)
   {
      output.printf(" BONDS=(%d", sgroup.bonds.size());
      for (i = 0; i < sgroup.bonds.size(); i++)
         output.printf(" %d", _bond_mapping[sgroup.bonds[i]]);
      output.printf(")");
   }
   for (i = 0; i < sgroup.brackets.size(); i++)
   {
      Vec2f *brackets = sgroup.brackets[i];
      output.printf(" BRKXYZ=(9 %f %f %f %f %f %f %f %f %f)",
         brackets[0].x, brackets[0].y, 0.f, brackets[1].x, brackets[1].y, 0.f, 0.f, 0.f, 0.f);
   }
}

void MolfileSaver::_writeOccurrenceRanges (Output &out, const Array<int> &occurrences)
{
   for (int i = 0; i < occurrences.size(); i++)
   {
      int occurrence = occurrences[i];

      if ((occurrence & 0xFFFF) == 0xFFFF)
         out.printf(">%d", (occurrence >> 16) - 1);
      else if ((occurrence >> 16) == (occurrence & 0xFFFF))
         out.printf("%d", occurrence >> 16);
      else if ((occurrence >> 16) == 0)
         out.printf("<%d", (occurrence & 0xFFFF) + 1);
      else
         out.printf("%d-%d", occurrence >> 16, occurrence & 0xFFFF);

      if (i != occurrences.size() - 1)
         out.printf(",");
   }
}

void MolfileSaver::_writeRGroup (Output &output, BaseMolecule &mol, int rg_idx)
{
   QS_DEF(Array<char>, buf);
   ArrayOutput out(buf);
   RGroup &rgroup = mol.rgroups.getRGroup(rg_idx);

   output.printfCR("M  V30 BEGIN RGROUP %d", rg_idx);

   out.printf("RLOGIC %d %d ", rgroup.if_then, rgroup.rest_h);

   _writeOccurrenceRanges(out, rgroup.occurrence);

   _writeMultiString(output, buf.ptr(), buf.size());

   PtrPool<BaseMolecule> &frags = rgroup.fragments;
   for (int j = frags.begin(); j != frags.end(); j = frags.next(j))
      _writeCtab(output, *rgroup.fragments[j], mol.isQueryMolecule());

   output.writeStringCR("M  V30 END RGROUP");
}

void MolfileSaver::_writeCtab2000 (Output &output, BaseMolecule &mol, bool query)
{
   QueryMolecule *qmol = 0;

   if (query)
      qmol = (QueryMolecule *)(&mol);

   int i;
   QS_DEF(Array<int[2]>, radicals);
   QS_DEF(Array<int>, charges);
   QS_DEF(Array<int>, isotopes);
   QS_DEF(Array<int>, pseudoatoms);
   QS_DEF(Array<int>, atom_lists);
   QS_DEF(Array<int>, unsaturated);
   QS_DEF(Array<int[2]>, substitution_count);
   QS_DEF(Array<int[2]>, ring_bonds);

   _atom_mapping.clear_resize(mol.vertexEnd());
   _bond_mapping.clear_resize(mol.edgeEnd());

   radicals.clear();
   charges.clear();
   isotopes.clear();
   pseudoatoms.clear();
   atom_lists.clear();
   unsaturated.clear();
   substitution_count.clear();
   ring_bonds.clear();

   int iw = 1;

   for (i = mol.vertexBegin(); i < mol.vertexEnd(); i = mol.vertexNext(i), iw++)
   {
      char label[3] = {' ', ' ', ' '};
      int valence = 0, radical = 0, charge = 0, stereo_parity = 0,
         hydrogens_count = 0, stereo_care = 0,
         aam = 0, irflag = 0, ecflag = 0;

      int atom_number = mol.getAtomNumber(i);
      int atom_isotope = mol.getAtomIsotope(i);
      int atom_charge = mol.getAtomCharge(i);
      int atom_radical = 0;

      _atom_mapping[i] = iw;

      if (!mol.isRSite(i) && !mol.isPseudoAtom(i))
         atom_radical = mol.getAtomRadical_NoThrow(i, 0);

      if (mol.isRSite(i))
      {
         label[0] = 'R';
         label[1] = '#';
      }
      else if (mol.isPseudoAtom(i))
      {
         const char *pseudo = mol.getPseudoAtom(i);

         if (strlen(pseudo) <= 3)
            memcpy(label, pseudo, __min(strlen(pseudo), 3));
         else
         {
            label[0] = 'A';
            pseudoatoms.push(i);
         }
      }
      else if (atom_number == -1)
      {
         if (qmol == 0)
            throw Error("internal: atom number = -1, but qmol == 0");

         QS_DEF(Array<int>, list);

         int query_atom_type = QueryMolecule::parseQueryAtom(*qmol, i, list);

         if (query_atom_type == QueryMolecule::QUERY_ATOM_A)
            label[0] = 'A';
         else if (query_atom_type == QueryMolecule::QUERY_ATOM_Q)
            label[0] = 'Q';
         else if (query_atom_type == QueryMolecule::QUERY_ATOM_X)
            label[0] = 'X';
         else if (query_atom_type == QueryMolecule::QUERY_ATOM_LIST ||
                  query_atom_type == QueryMolecule::QUERY_ATOM_NOTLIST)
         {
            label[0] = 'L';
            atom_lists.push(i);
         }
         else
            label[0] = 'A';
            //throw Error("error saving atom #%d: unsupported query atom", i);
      }
      else if (atom_number == ELEM_H && atom_isotope == 2)
         label[0] = 'D';   
      else if (atom_number == ELEM_H && atom_isotope == 3)
         label[0] = 'T';
      else
      {
         const char *str = Element::toString(atom_number);

         label[0] = str[0];
         if (str[1] != 0)
            label[1] = str[1];

         if (atom_isotope > 0)
            isotopes.push(i);
      }

      if (reactionAtomMapping != 0)
         aam = reactionAtomMapping->at(i);
      if (reactionAtomInversion != 0)
         irflag = reactionAtomInversion->at(i);
      if (reactionAtomExactChange != 0)
         ecflag = reactionAtomExactChange->at(i);

      int explicit_valence = -1;

      if (!mol.isRSite(i) && !mol.isPseudoAtom(i))
         explicit_valence = mol.getExplicitValence(i);

      if (explicit_valence > 0 && explicit_valence <= 14)
         valence = explicit_valence;
      if (explicit_valence == 0)
         valence = 15;

      if (atom_charge != CHARGE_UNKNOWN &&
          atom_charge >= -15 && atom_charge <= 15)
          charge = atom_charge;

      if (charge != 0)
         charges.push(i);

      if (atom_radical >= 0 && atom_radical <= 3)
         radical = atom_radical;

      if (radical != 0)
      {
         int *r = radicals.push();
         r[0] = i;
         r[1] = radical;
      }

      if (qmol != 0)
      {
         int unsat;
         if (qmol->getAtom(i).sureValue(QueryMolecule::ATOM_UNSATURATION, unsat))
            unsaturated.push(i);
         int rbc;
         if (_getRingBondCountFlagValue(*qmol, i, rbc))
         {
            int *r = ring_bonds.push();
            r[0] = i;
            r[1] = rbc;
         }
         int subst;
         if (_getSubstitutionCountFlagValue(*qmol, i, subst))
         {
            int *s = substitution_count.push();
            s[0] = i;
            s[1] = subst;
         }
      }

      stereo_parity = _getStereocenterParity(mol, i);

      hydrogens_count = MoleculeSavers::getHCount(mol, i, atom_number, atom_charge);
      if (hydrogens_count == -1)
         hydrogens_count = 0;
      else 
         // molfile stores h+1
         hydrogens_count++;

      Vec3f pos = mol.getAtomXyz(i);
      if (fabs(pos.x) < 1e-5f)
         pos.x = 0;
      if (fabs(pos.y) < 1e-5f)
         pos.y = 0;
      if (fabs(pos.z) < 1e-5f)
         pos.z = 0;

      output.printfCR("%10.4f%10.4f%10.4f %c%c%c%2d"
                    "%3d%3d%3d%3d%3d"
                    "%3d%3d%3d%3d%3d%3d",
         pos.x, pos.y, pos.z, label[0], label[1], label[2], 0,
         0, stereo_parity, hydrogens_count, stereo_care, valence,
         0, 0, 0,
         aam, irflag, ecflag);
   }

   iw = 1;

   for (i = mol.edgeBegin(); i < mol.edgeEnd(); i = mol.edgeNext(i))
   {
      const Edge &edge = mol.getEdge(i);
      int bond_order = mol.getBondOrder(i);

      int indigo_topology = -1;

      if (bond_order < 0 && qmol != 0)
      {
         int qb = QueryMolecule::getQueryBondType(qmol->getBond(i));

         if (qb == QueryMolecule::QUERY_BOND_SINGLE_OR_DOUBLE)
            bond_order = 5;
         else if (qb == QueryMolecule::QUERY_BOND_SINGLE_OR_AROMATIC)
            bond_order = 6;
         else if (qb == QueryMolecule::QUERY_BOND_DOUBLE_OR_AROMATIC)
            bond_order = 7;
         else if (qb == QueryMolecule::QUERY_BOND_ANY)
            bond_order = 8;
      }

      if (bond_order < 0)
      {
         Array<char> buf;
         qmol->getBondDescription(i, buf);
         throw Error("unrepresentable query bond: %s", buf.ptr());
      }

      int stereo = 0;
      int reacting_center = 0;

      int direction = mol.getBondDirection(i);

      switch (direction)
      {
         case BOND_UP: stereo = 1; break;
         case BOND_DOWN: stereo = 6; break;
         case BOND_EITHER: stereo = 4; break;
         case 0:
            if (mol.cis_trans.isIgnored(i))
               stereo = 3;
            break;
      }

      if (stereo == 3)
      {
         // discard it if we have a neighbor "either" bond
         if (_hasNeighborEitherBond(mol, i))
            stereo = 0;
      }

      if (qmol != 0 && indigo_topology == -1)
         qmol->getBond(i).sureValue(QueryMolecule::BOND_TOPOLOGY, indigo_topology);

      int topology = 0;
      if (indigo_topology == TOPOLOGY_RING)
         topology = 1;
      else if (indigo_topology == TOPOLOGY_CHAIN)
         topology = 2;

      if(reactionBondReactingCenter != 0 && reactionBondReactingCenter->at(i) != 0)
         reacting_center = reactionBondReactingCenter->at(i);

      output.printfCR("%3d%3d%3d%3d%3d%3d%3d",
                _atom_mapping[edge.beg], _atom_mapping[edge.end],
                bond_order, stereo, 0, topology, reacting_center);
      _bond_mapping[i] = iw++;
   }

   if (charges.size() > 0)
   {
      int j = 0;
      while (j < charges.size())
      {
         output.printf("M  CHG%3d", __min(charges.size(), j + 8) - j);
         for (i = j; i < __min(charges.size(), j + 8); i++)
            output.printf(" %3d %3d", _atom_mapping[charges[i]], mol.getAtomCharge(charges[i]));
         output.writeCR();
         j += 8;
      }
   }

   if (radicals.size() > 0)
   {
      int j = 0;
      while (j < radicals.size())
      {
         output.printf("M  RAD%3d", __min(radicals.size(), j + 8) - j);
         for (i = j; i < __min(radicals.size(), j + 8); i++)
            output.printf(" %3d %3d", _atom_mapping[radicals[i][0]], radicals[i][1]);
         output.writeCR();
         j += 8;
      }
   }

   if (isotopes.size() > 0)
   {
      int j = 0;
      while (j < isotopes.size())
      {
         output.printf("M  ISO%3d", __min(isotopes.size(), j + 8) - j);
         for (i = j; i < __min(isotopes.size(), j + 8); i++)
            output.printf(" %3d %3d", _atom_mapping[isotopes[i]], mol.getAtomIsotope(isotopes[i]));
         output.writeCR();
         j += 8;
      }
   }

   if (unsaturated.size() > 0)
   {
      int j = 0;
      while (j < unsaturated.size())
      {
         output.printf("M  UNS%3d", __min(unsaturated.size(), j + 8) - j);
         for (i = j; i < __min(unsaturated.size(), j + 8); i++)
            output.printf(" %3d %3d", _atom_mapping[unsaturated[i]], 1);
         output.writeCR();
         j += 8;
      }
   }

   if (substitution_count.size() > 0)
   {
      int j = 0;
      while (j < substitution_count.size())
      {
         output.printf("M  SUB%3d", __min(substitution_count.size(), j + 8) - j);
         for (i = j; i < __min(substitution_count.size(), j + 8); i++)
            output.printf(" %3d %3d", _atom_mapping[substitution_count[i][0]], substitution_count[i][1]);
         output.writeCR();
         j += 8;
      }
   }

   if (ring_bonds.size() > 0)
   {
      int j = 0;
      while (j < ring_bonds.size())
      {
         output.printf("M  RBC%3d", __min(ring_bonds.size(), j + 8) - j);
         for (i = j; i < __min(ring_bonds.size(), j + 8); i++)
            output.printf(" %3d %3d", _atom_mapping[ring_bonds[i][0]], ring_bonds[i][1]);
         output.writeCR();
         j += 8;
      }
   }

   for (i = 0; i < atom_lists.size(); i++)
   {
      int atom_idx = atom_lists[i];
      QS_DEF(Array<int>, list);

      int query_atom_type = QueryMolecule::parseQueryAtom(*qmol, atom_idx, list);

      if (query_atom_type != QueryMolecule::QUERY_ATOM_LIST &&
          query_atom_type != QueryMolecule::QUERY_ATOM_NOTLIST)
         throw Error("internal: atom list not recognized");

      if (list.size() < 1)
         throw Error("internal: atom list size is zero");

      output.printf("M  ALS %3d%3d %c ", _atom_mapping[atom_idx], list.size(),
         query_atom_type == QueryMolecule::QUERY_ATOM_NOTLIST ? 'T' : 'F');

      int j;

      for (j = 0; j < list.size(); j++)
      {
         char c1 = ' ', c2 = ' ';
         const char *str = Element::toString(list[j]);

         c1 = str[0];
         if (str[1] != 0)
            c2 = str[1];
         
         output.printf("%c%c  ", c1, c2);
      }
      output.writeCR();
   }
   
   for (i = 0; i < pseudoatoms.size(); i++)
   {
      output.printfCR("A  %3d", _atom_mapping[pseudoatoms[i]]);
      output.writeString(mol.getPseudoAtom(pseudoatoms[i]));
      output.writeCR();
   }

   QS_DEF(Array<int>, sgroup_ids);
   QS_DEF(Array<int>, sgroup_types);
   QS_DEF(Array<int>, inv_mapping_ru);

   sgroup_ids.clear();
   sgroup_types.clear();
   inv_mapping_ru.clear_resize(mol.repeating_units.end());

   for (i = mol.superatoms.begin(); i != mol.superatoms.end(); i = mol.superatoms.next(i))
   {
      sgroup_ids.push(i);
      sgroup_types.push(_SGROUP_TYPE_SUP);
   }
   for (i = mol.data_sgroups.begin(); i != mol.data_sgroups.end(); i = mol.data_sgroups.next(i))
   {
      sgroup_ids.push(i);
      sgroup_types.push(_SGROUP_TYPE_DAT);
   }
   for (i = mol.repeating_units.begin(); i != mol.repeating_units.end(); i = mol.repeating_units.next(i))
   {
      inv_mapping_ru[i] = sgroup_ids.size();
      sgroup_ids.push(i);
      sgroup_types.push(_SGROUP_TYPE_SRU);
   }
   for (i = mol.multiple_groups.begin(); i != mol.multiple_groups.end(); i = mol.multiple_groups.next(i))
   {
      sgroup_ids.push(i);
      sgroup_types.push(_SGROUP_TYPE_MUL);
   }
   for (i = mol.generic_sgroups.begin(); i != mol.generic_sgroups.end(); i = mol.generic_sgroups.next(i))
   {
      sgroup_ids.push(i);
      sgroup_types.push(_SGROUP_TYPE_GEN);
   }

   if (sgroup_ids.size() > 0)
   {
      int j;
      for (j = 0; j < sgroup_ids.size(); j += 8)
      {
         output.printf("M  STY%3d", __min(sgroup_ids.size(), j + 8) - j);
         for (i = j; i < __min(sgroup_ids.size(), j + 8); i++)
         {
            const char *type;

            switch (sgroup_types[i])
            {
               case _SGROUP_TYPE_DAT: type = "DAT"; break;
               case _SGROUP_TYPE_MUL: type = "MUL"; break;
               case _SGROUP_TYPE_SRU: type = "SRU"; break;
               case _SGROUP_TYPE_SUP: type = "SUP"; break;
               case _SGROUP_TYPE_GEN: type = "GEN"; break;
               default: throw Error("internal: bad sgroup type");
            }

            output.printf(" %3d %s", i + 1, type);
         }
         output.writeCR();
      }
      for (j = 0; j < sgroup_ids.size(); j += 8)
      {
         output.printf("M  SLB%3d", __min(sgroup_ids.size(), j + 8) - j);
         for (i = j; i < __min(sgroup_ids.size(), j + 8); i++)
            output.printf(" %3d %3d", i + 1, i + 1);
         output.writeCR();
      }

      for (j = 0; j < mol.repeating_units.size(); j += 8)
      {
         output.printf("M  SCN%3d", __min(mol.repeating_units.size(), j + 8) - j);
         for (i = j; i < __min(mol.repeating_units.size(), j + 8); i++)
         {
            BaseMolecule::RepeatingUnit &ru = mol.repeating_units[i];

            output.printf(" %3d ", inv_mapping_ru[i] + 1);

            if (ru.connectivity == BaseMolecule::RepeatingUnit::HEAD_TO_HEAD)
               output.printf("HH ");
            else if (ru.connectivity == BaseMolecule::RepeatingUnit::HEAD_TO_TAIL)
               output.printf("HT ");
            else
               output.printf("EU ");
         }
         output.writeCR();
      }

      for (i = 0; i < sgroup_ids.size(); i++)
      {
         BaseMolecule::SGroup *sgroup;

         switch (sgroup_types[i])
         {
            case _SGROUP_TYPE_DAT: sgroup = &mol.data_sgroups[sgroup_ids[i]]; break;
            case _SGROUP_TYPE_MUL: sgroup = &mol.multiple_groups[sgroup_ids[i]]; break;
            case _SGROUP_TYPE_SRU: sgroup = &mol.repeating_units[sgroup_ids[i]]; break;
            case _SGROUP_TYPE_SUP: sgroup = &mol.superatoms[sgroup_ids[i]]; break;
            case _SGROUP_TYPE_GEN: sgroup = &mol.generic_sgroups[sgroup_ids[i]]; break;
            default: throw Error("internal: bad sgroup type");
         }

         for (j = 0; j < sgroup->atoms.size(); j += 8)
         {
            int k;
            output.printf("M  SAL %3d%3d", i + 1, __min(sgroup->atoms.size(), j + 8) - j);
            for (k = j; k < __min(sgroup->atoms.size(), j + 8); k++)
               output.printf(" %3d", _atom_mapping[sgroup->atoms[k]]);
            output.writeCR();
         }
         for (j = 0; j < sgroup->bonds.size(); j += 8)
         {
            int k;
            output.printf("M  SBL %3d%3d", i + 1, __min(sgroup->bonds.size(), j + 8) - j);
            for (k = j; k < __min(sgroup->bonds.size(), j + 8); k++)
               output.printf(" %3d", _bond_mapping[sgroup->bonds[k]]);
            output.writeCR();
         }

         if (sgroup_types[i] == _SGROUP_TYPE_SUP)
         {
            BaseMolecule::Superatom &superatom = mol.superatoms[sgroup_ids[i]];

            if (superatom.subscript.size() > 1)
               output.printfCR("M  SMT %3d %s", i + 1, superatom.subscript.ptr());
            if (superatom.bond_idx >= 0)
            {
               output.printfCR("M  SBV %3d %3d %9.4f %9.4f", i + 1,
                       _bond_mapping[superatom.bond_idx], superatom.bond_dir.x, superatom.bond_dir.y);
            }
         }
         else if (sgroup_types[i] == _SGROUP_TYPE_SRU)
         {
            BaseMolecule::RepeatingUnit &sru = mol.repeating_units[sgroup_ids[i]];

            if (sru.subscript.size() > 1)
               output.printfCR("M  SMT %3d %s", i + 1, sru.subscript.ptr());
         }
         else if (sgroup_types[i] == _SGROUP_TYPE_DAT)
         {
            BaseMolecule::DataSGroup &datasgroup = mol.data_sgroups[sgroup_ids[i]];
            int k = 30;

            output.printf("M  SDT %3d ", i + 1);
            if (datasgroup.description.size() > 1)
            {
               output.printf("%s", datasgroup.description.ptr());
               k -= datasgroup.description.size() - 1;
            }
            while (k-- > 0)
               output.writeChar(' ');
            output.writeChar('F');
            output.writeCR();

            output.printf("M  SDD %3d ", i + 1);
            _writeDataSGroupDisplay(datasgroup, output);
            output.writeCR();

            k = datasgroup.data.size();
            if (k > 0 && datasgroup.data.top() == 0)
               k--; // Exclude terminating zero

            char *ptr = datasgroup.data.ptr();
            while (k > 0)
            {
               int j;
               for (j = 0; j < 69 && j < k; j++)
                  if (ptr[j] == '\n')
                     break;

               // Print ptr[0..i]
               output.writeString("M  ");
               if (j != 69 || j == k)
                  output.writeString("SED ");
               else
                  output.writeString("SCD ");
               output.printf("%3d ", i + 1);

               output.write(ptr, j);
               if (ptr[j] == '\n')
                  j++;

               ptr += j;
               k -= j;
               output.writeCR();
            }
         }
         else if (sgroup_types[i] == _SGROUP_TYPE_MUL)
         {
            BaseMolecule::MultipleGroup &mg = mol.multiple_groups[sgroup_ids[i]];

            for (j = 0; j < mg.parent_atoms.size(); j += 8)
            {
               int k;
               output.printf("M  SPA %3d%3d", i + 1, __min(mg.parent_atoms.size(), j + 8) - j);
               for (k = j; k < __min(mg.parent_atoms.size(), j + 8); k++)
                  output.printf(" %3d", _atom_mapping[mg.parent_atoms[k]]);
               output.writeCR();
            }

            output.printf("M  SMT %3d %d\n", i + 1, mg.multiplier);
         }
         for (j = 0; j < sgroup->brackets.size(); j++)
         {
            output.printf("M  SDI %3d  4 %9.4f %9.4f %9.4f %9.4f\n", i + 1,
                    sgroup->brackets[j][0].x, sgroup->brackets[j][0].y,
                    sgroup->brackets[j][1].x, sgroup->brackets[j][1].y);
         }

      }
   }

}

int MolfileSaver::_getStereocenterParity (BaseMolecule &mol, int idx)
{
   int type = mol.stereocenters.getType(idx);
   if (type == 0)
      return 0;
   if (type == MoleculeStereocenters::ATOM_ANY)
      return 3;

   // Reference from "CTfile Formats. Appendix A: Stereo Notes":
   // Number the atoms surrounding the stereo center with 1, 2, 3, and 4 in 
   // order of increasing atom number (position in the atom block) (a hydrogen 
   // atom should be considered the highest numbered atom, in this case atom 4).
   // View the center from a position such that the bond connecting the
   // highest-numbered atom (4) projects behind the plane formed by atoms 1, 2, and 3.

   int pyramid[4];
   memcpy(pyramid, mol.stereocenters.getPyramid(idx), sizeof(pyramid));
   if (pyramid[3] == -1)
   {
      if (mol.isQueryMolecule())
      {
         if (mol.getAtomNumber(idx) == -1)
            // This atom is not a pure atom
            // There are no implicit hydrogens for query molecules
            return 0;
      }

      // Assign implicit hydrogen the highest index
      pyramid[3] = mol.vertexEnd();
   }
   else
   {
      // Replace pure hydrogen atom with the highest value
      for (int i = 0; i < 4; i++)
      {
         int p = pyramid[i];
         if (mol.getAtomNumber(p) == ELEM_H)
         {
            bool pure_hydrogen = (mol.getAtomIsotope(p) == 0);
            if (!pure_hydrogen && mol.isQueryMolecule())
               pure_hydrogen = !mol.asQueryMolecule().getAtom(p).hasConstraint(QueryMolecule::ATOM_ISOTOPE);
            if (pure_hydrogen)
            {
               pyramid[i] = mol.vertexEnd();
               break;
            }
         }
      }
   }

   if (MoleculeStereocenters::isPyramidMappingRigid(pyramid))
      return 1; // odd parity
   return 2; // even parity
}

void MolfileSaver::_writeRGroupIndices2000 (Output &output, BaseMolecule &mol)
{
   int i, j;

   QS_DEF(Array<int>, atom_ids);
   QS_DEF(Array<int>, rg_ids);

   atom_ids.clear();
   rg_ids.clear();

   for (i = mol.vertexBegin(); i < mol.vertexEnd(); i = mol.vertexNext(i))
   {
      if (!mol.isRSite(i))
         continue;

      QS_DEF(Array<int>, rg_list);

      mol.getAllowedRGroups(i, rg_list);

      for (j = 0; j < rg_list.size(); j++)
      {
         atom_ids.push(_atom_mapping[i]);
         rg_ids.push(rg_list[j]);
      }
   }

   if (atom_ids.size() > 0)
   {
      output.printf("M  RGP%3d", atom_ids.size());
      for (i = 0; i < atom_ids.size(); i++)
         output.printf(" %3d %3d", atom_ids[i], rg_ids[i]);
      output.writeCR();
   }

   for (i = mol.vertexBegin(); i < mol.vertexEnd(); i = mol.vertexNext(i))
   {
      if (!mol.isRSite(i))
         continue;
      
      if (!_checkAttPointOrder(mol, i))
      {
         const Vertex &vertex = mol.getVertex(i);
         int k;

         output.printf("M  AAL %3d%3d", _atom_mapping[i], vertex.degree());
         for (k = 0; k < vertex.degree(); k++)
            output.printf(" %3d %3d", _atom_mapping[mol.getRSiteAttachmentPointByOrder(i, k)], k + 1);

         output.writeCR();
      }
   }
}

void MolfileSaver::_writeAttachmentValues2000 (Output &output, BaseMolecule &fragment)
{
   if (fragment.attachmentPointCount() == 0)
      return;

   RedBlackMap<int, int> orders;
   int i;

   for (i = 1; i <= fragment.attachmentPointCount(); i++)
   {
      int j = 0;
      int idx;

      while ((idx = fragment.getAttachmentPoint(i, j++)) != -1)
      {
         int *val;

         if ((val = orders.at2(_atom_mapping[idx])) == 0)
            orders.insert(_atom_mapping[idx], 1 << (i - 1));
         else
            *val |= 1 << (i - 1);
      }

   }

   output.printf("M  APO%3d", orders.size());

   for (i = orders.begin(); i < orders.end(); i = orders.next(i))
      output.printf(" %3d %3d", orders.key(i), orders.value(i));

   output.writeCR();
}

bool MolfileSaver::_checkAttPointOrder (BaseMolecule &mol, int rsite)
{
   const Vertex &vertex = mol.getVertex(rsite);
   int i;

   for (i = 0; i < vertex.degree() - 1; i++)
   {
      int cur = mol.getRSiteAttachmentPointByOrder(rsite, i);
      int next = mol.getRSiteAttachmentPointByOrder(rsite, i + 1);

      if (cur == -1 || next == -1)
         return true; // here we treat "undefined" as "ok"

      if (cur > next)
         return false;
   }

   return true;
}

void MolfileSaver::_writeDataSGroupDisplay (BaseMolecule::DataSGroup &datasgroup, Output &out)
{
   out.printf("%10.4f%10.4f    %c%c%c   ALL  1       %1d  ",
                datasgroup.display_pos.x, datasgroup.display_pos.y,
                datasgroup.detached ? 'D' : 'A',
                datasgroup.relative ? 'R' : 'A',
                datasgroup.display_units ? 'U' : ' ',
                datasgroup.dasp_pos);
}

bool MolfileSaver::_hasNeighborEitherBond (BaseMolecule &mol, int edge_idx)
{
   const Edge &edge = mol.getEdge(edge_idx);
   const Vertex &beg = mol.getVertex(edge.beg);
   const Vertex &end = mol.getVertex(edge.end);
   int k;

   for (k = beg.neiBegin(); k != beg.neiEnd(); k = beg.neiNext(k))
      if (mol.getBondDirection2(edge.beg, beg.neiVertex(k)) == BOND_EITHER)
         return true;

   for (k = end.neiBegin(); k != end.neiEnd(); k = end.neiNext(k))
      if (mol.getBondDirection2(edge.end, end.neiVertex(k)) == BOND_EITHER)
         return true;
   return false;
}
