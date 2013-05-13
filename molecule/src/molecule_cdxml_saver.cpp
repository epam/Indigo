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

#include "base_cpp/output.h"
#include "molecule/molecule_cdxml_saver.h"
#include "molecule/molecule.h"
#include "molecule/elements.h"
#include "base_cpp/locale_guard.h"

using namespace indigo;

IMPL_ERROR(MoleculeCdxmlSaver, "molecule CMXML saver");

MoleculeCdxmlSaver::MoleculeCdxmlSaver (Output &output) : _output(output)
{
   skip_main_tag = false;
}

void MoleculeCdxmlSaver::saveMolecule (Molecule &mol)
{
   LocaleGuard locale_guard;

   _mol = &mol;

   float bondLength = 30;

   if (!skip_main_tag)
   {
      _output.printf("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n");
      _output.printf("<!DOCTYPE CDXML SYSTEM \"http://www.cambridgesoft.com/xml/cdxml.dtd\" >\n");
      
      _output.printf("<CDXML BondLength=\"%f\">\n", bondLength);
   }

   _output.printf("<page>\n");
   _output.printf("<fragment>\n");

   bool have_hyz = _mol->have_xyz;
   bool have_z = BaseMolecule::hasZCoord(*_mol);

   Vec3f min_coord, max_coord;
   if (have_hyz)
   {
      for (int i = _mol->vertexBegin(); i != _mol->vertexEnd(); i = _mol->vertexNext(i))
      {
         Vec3f &pos = _mol->getAtomXyz(i);
         if (i == _mol->vertexBegin())
            min_coord = max_coord = pos;
         else
         {
            min_coord.min(pos);
            max_coord.max(pos);
         }
      }

      // Add margins
      max_coord.add(Vec3f(1, 1, 1));
      min_coord.sub(Vec3f(1, 1, 1));
   }
   else
   {
      min_coord.set(0, 0, 0);
      max_coord.set(0, 0, 0);
   }

   if (_mol->vertexCount() > 0)
   {
      for (int i = _mol->vertexBegin(); i != _mol->vertexEnd(); i = _mol->vertexNext(i))
      {
         if (_mol->isRSite(i))
            throw Error("R-sites are not supported");

         int atom_number = _mol->getAtomNumber(i);

         _output.printf("    <n id=\"%d\" Element=\"%d\"", i + 1, atom_number);

         if (_mol->getAtomIsotope(i) != 0)
         {
            _output.printf(" Isotope=\"%d\"", _mol->getAtomIsotope(i));
         }

         if (_mol->getAtomCharge(i) != 0)
            _output.printf(" Charge=\"%d\"", _mol->getAtomCharge(i));

         if (_mol->getAtomRadical_NoThrow(i, 0) != 0)
         {
            int radical = _mol->getAtomRadical(i);
            const char *radical_str = NULL;
            if (radical == RADICAL_DOUBLET)
               radical_str = "Doublet";
            else if (radical == RADICAL_SINGLET)
               radical_str = "Singlet";
            else
               throw Error("Radical type %d is not supported", radical);

            _output.printf(" Radical=\"%s\"", radical_str);
         }

         if (Molecule::shouldWriteHCount(*_mol, i))
         {
            int hcount;

            try
            {
               hcount = _mol->getAtomTotalH(i);
            }
            catch (Exception &)
            {
               hcount = -1;
            }

            if (hcount >= 0)
               _output.printf(" NumHydrogens=\"%d\"", hcount);
         }

         if (have_hyz)
         {
            Vec3f pos = _mol->getAtomXyz(i);
            pos.x -= min_coord.x;
            pos.y -= max_coord.y;
            pos.scale(bondLength);

            if (have_z)
               _output.printf("\n         p=\"%f %f %f\"", pos.x, -pos.y, pos.z);
            else
               _output.printf("\n         p=\"%f %f\"", pos.x, -pos.y);
         }
         else
         {
            if (_mol->stereocenters.getType(i) > MoleculeStereocenters::ATOM_ANY)
            {
               _output.printf(" Geometry=\"Tetrahedral\"");

               const int *pyramid = _mol->stereocenters.getPyramid(i);
               // 0 means atom absence
               _output.printf(" BondOrdering=\"%d %d %d %d\"", 
                  pyramid[0] + 1, pyramid[1] + 1, pyramid[2] + 1, pyramid[3] + 1); 
            }
         }

         if (_mol->isPseudoAtom(i))
         {
            throw Error("Pseudoatoms are not supported yet");
         }
         _output.printf("/>\n");
      }
   }

   if (_mol->edgeCount() > 0)
   {
      for (int i = _mol->edgeBegin(); i != _mol->edgeEnd(); i = _mol->edgeNext(i))
      {
         const Edge &edge = _mol->getEdge(i);

         _output.printf("    <b B=\"%d\" E=\"%d\"", edge.beg + 1, edge.end + 1);

         int order = _mol->getBondOrder(i);

         if (order == BOND_DOUBLE || order == BOND_TRIPLE)
            _output.printf(" Order=\"%d\"", order);
         else if (order == BOND_AROMATIC)
            _output.printf(" Order=\"1.5\"");
         else
            ; // Do not write single bond order

         int dir = _mol->getBondDirection(i);
         int parity = _mol->cis_trans.getParity(i);

         if (_mol->have_xyz && (dir == BOND_UP || dir == BOND_DOWN))
         {
            _output.printf(" Display=\"%s\"",
                    (dir == BOND_UP) ? "WedgeBegin" : "WedgedHashBegin");
         }
         else if (!_mol->have_xyz && parity != 0)
         {
            const int *subst = _mol->cis_trans.getSubstituents(i);

            int s3 = subst[2] + 1, s4  = subst[3] + 1;
            if (parity == MoleculeCisTrans::TRANS)
            {
               int tmp;
               __swap(s3, s4, tmp);
            }
            _output.printf(" BondCircularOrdering=\"%d %d %d %d\"",
                    subst[0] + 1, subst[1] + 1, s3, s4);
         }

         _output.printf("/>\n");
      }
   }

   _output.printf("</fragment>\n");
   _output.printf("</page>\n");
   if (!skip_main_tag)
      _output.printf("</CDXML>\n");
}
