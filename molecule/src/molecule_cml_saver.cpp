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
#include "molecule/molecule_cml_saver.h"
#include "molecule/molecule.h"
#include "molecule/elements.h"
#include "base_cpp/locale_guard.h"

using namespace indigo;

IMPL_ERROR(MoleculeCmlSaver, "molecule CML saver");

MoleculeCmlSaver::MoleculeCmlSaver (Output &output) : _output(output)
{
   skip_cml_tag = false;
}

void MoleculeCmlSaver::saveMolecule (Molecule &mol)
{
   LocaleGuard locale_guard;
   int i;

   _mol = &mol;

   if (!skip_cml_tag)
   {
      _output.printf("<?xml version=\"1.0\" ?>\n");
      _output.printf("<cml>\n");
   }

   if (_mol->name.ptr() != 0)
   {
      if (strchr(_mol->name.ptr(), '\"') != NULL)
         throw Error("can not save molecule with '\"' in title");
      _output.printf("<molecule title=\"%s\">\n", _mol->name.ptr());
   }
   else
      _output.printf("<molecule>\n");

   bool have_hyz = _mol->have_xyz;
   bool have_z = BaseMolecule::hasZCoord(*_mol);

   if (_mol->vertexCount() > 0)
   {
      _output.printf("  <atomArray>\n");
      for (i = _mol->vertexBegin(); i != _mol->vertexEnd(); i = _mol->vertexNext(i))
      {
         if (_mol->isRSite(i))
            throw Error("R-sites are not supported");

         int atom_number = _mol->getAtomNumber(i);

         const char *atom_str;

         if (_mol->isPseudoAtom(i))
            atom_str = _mol->getPseudoAtom(i);
         else
            atom_str = Element::toString(atom_number);

         _output.printf("    <atom id=\"a%d\" elementType=\"%s\"", i, atom_str);

         if (_mol->getAtomIsotope(i) != 0)
         {
            _output.printf(" isotope=\"%d\"", _mol->getAtomIsotope(i));
            // for inchi-1 program which ignores "isotope" property (version 1.03)
            _output.printf(" isotopeNumber=\"%d\"", _mol->getAtomIsotope(i));
         }

         if (_mol->getAtomCharge(i) != 0)
            _output.printf(" formalCharge=\"%d\"", _mol->getAtomCharge(i));

         if (_mol->getAtomRadical_NoThrow(i, 0) != 0)
            _output.printf(" spinMultiplicity=\"%d\"", _mol->getAtomRadical(i));

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
               _output.printf(" hydrogenCount=\"%d\"", hcount);
         }

         if (have_hyz)
         {
            Vec3f &pos = _mol->getAtomXyz(i);

            if (have_z)
               _output.printf("\n         x3=\"%f\" y3=\"%f\" z3=\"%f\"", pos.x, pos.y, pos.z);
            else
               _output.printf("\n         x2=\"%f\" y2=\"%f\"", pos.x, pos.y);
            _output.printf("/>\n");
         }
         else
         {
            if (_mol->stereocenters.getType(i) > MoleculeStereocenters::ATOM_ANY)
            {
               _output.printf(">\n      <atomParity atomRefs4=\"");
               const int *pyramid = _mol->stereocenters.getPyramid(i);
               if (pyramid[3] == -1)
                  _output.printf("a%d a%d a%d a%d", pyramid[0], pyramid[1], pyramid[2], i);
               else
                  _output.printf("a%d a%d a%d a%d", pyramid[0], pyramid[1], pyramid[2], pyramid[3]);
               _output.printf("\">1</atomParity>\n    </atom>\n");
            }
            else
               _output.printf("/>\n");
         }


      }
      _output.printf("  </atomArray>\n");
   }

   if (_mol->edgeCount() > 0)
   {
      _output.printf("  <bondArray>\n");
      for (i = _mol->edgeBegin(); i != _mol->edgeEnd(); i = _mol->edgeNext(i))
      {
         const Edge &edge = _mol->getEdge(i);

         _output.printf("    <bond atomRefs2=\"a%d a%d\"", edge.beg, edge.end);

         int order = _mol->getBondOrder(i);

         if (order == BOND_SINGLE || order == BOND_DOUBLE || order == BOND_TRIPLE)
            _output.printf(" order=\"%d\"", order);
         else
            _output.printf(" order=\"A\"");

         int dir = _mol->getBondDirection(i);
         int parity = _mol->cis_trans.getParity(i);

         if (_mol->have_xyz && (dir == BOND_UP || dir == BOND_DOWN))
         {
            _output.printf(">\n      <bondStereo>%s</bondStereo>\n    </bond>\n",
                    (dir == BOND_UP) ? "W" : "H");
         }
         else if (!_mol->have_xyz && parity != 0)
         {
            const int *subst = _mol->cis_trans.getSubstituents(i);
            _output.printf(">\n      <bondStereo atomRefs4=\"a%d a%d a%d a%d\">%s</bondStereo>\n    </bond>\n",
                    subst[0], edge.beg, edge.end, subst[2],
                    (parity == MoleculeCisTrans::CIS) ? "C" : "T");
         }
         else
            _output.printf("/>\n");
      }
      _output.printf("  </bondArray>\n");
   }
   _output.printf("</molecule>\n");
   if (!skip_cml_tag)
      _output.printf("</cml>\n");
}
