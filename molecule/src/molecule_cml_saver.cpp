/****************************************************************************
 * Copyright (C) 2009-2015 EPAM Systems
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
#include "tinyxml.h"

using namespace indigo;

IMPL_ERROR(MoleculeCmlSaver, "molecule CML saver");

MoleculeCmlSaver::MoleculeCmlSaver (Output &output) : _output(output)
{
   skip_cml_tag = false;
   _doc = new TiXmlDocument();
   _root = 0;
}

void MoleculeCmlSaver::saveMolecule (Molecule &mol)
{
   LocaleGuard locale_guard;
   int i;

   _mol = &mol;

   if (!skip_cml_tag)
   {
      TiXmlDeclaration * decl = new TiXmlDeclaration("1.0", "", "");
      _doc->LinkEndChild(decl);
      _root = new TiXmlElement("cml");
      _doc->LinkEndChild(_root);
   }

   TiXmlElement * molecule = new TiXmlElement("molecule");
   if (_root != 0)
      _root->LinkEndChild(molecule);
   else
      _doc->LinkEndChild(molecule);


   if (_mol->name.ptr() != 0)
   {
      if (strchr(_mol->name.ptr(), '\"') != NULL)
         throw Error("can not save molecule with '\"' in title");
      molecule->SetAttribute("title", _mol->name.ptr());
   }

   bool have_hyz = _mol->have_xyz;
   bool have_z = BaseMolecule::hasZCoord(*_mol);

   if (_mol->vertexCount() > 0)
   {
      TiXmlElement * atomarray = new TiXmlElement("atomArray");
      molecule->LinkEndChild(atomarray);

      for (i = _mol->vertexBegin(); i != _mol->vertexEnd(); i = _mol->vertexNext(i))
      {

         int atom_number = _mol->getAtomNumber(i);

         const char *atom_str;

         if (_mol->isRSite(i))
            atom_str = "A";
         else if (_mol->isPseudoAtom(i))
            atom_str = _mol->getPseudoAtom(i);
         else
            atom_str = Element::toString(atom_number);

         TiXmlElement * atom = new TiXmlElement("atom");
         atomarray->LinkEndChild(atom);

         QS_DEF(Array<char>, buf);
         ArrayOutput out(buf);
         out.printf("a%d", i);
         buf.push(0);
         atom->SetAttribute("id", buf.ptr());
         atom->SetAttribute("elementType", atom_str);

         if (_mol->getAtomIsotope(i) != 0)
         {
            atom->SetAttribute("isotope", _mol->getAtomIsotope(i));
            // for inchi-1 program which ignores "isotope" property (version 1.03)
            atom->SetAttribute("isotopeNumber", _mol->getAtomIsotope(i));
         }

         if (_mol->getAtomCharge(i) != 0)
            atom->SetAttribute("formalCharge", _mol->getAtomCharge(i));


         if (!_mol->isRSite(i) && !_mol->isPseudoAtom(i))
         {
            if (_mol->getAtomRadical_NoThrow(i, 0) != 0)
               atom->SetAttribute("spinMultiplicity", _mol->getAtomRadical(i));
   
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
                  atom->SetAttribute("hydrogenCount", hcount);
            }
         }

         if (have_hyz)
         {
            Vec3f &pos = _mol->getAtomXyz(i);

            if (have_z)
            {
               atom->SetDoubleAttribute("x3", pos.x);
               atom->SetDoubleAttribute("y3", pos.y);
               atom->SetDoubleAttribute("z3", pos.z);
            }
            else
            {
               atom->SetDoubleAttribute("x2", pos.x);
               atom->SetDoubleAttribute("y2", pos.y);
            }
         }
         else
         {
            if (_mol->stereocenters.getType(i) > MoleculeStereocenters::ATOM_ANY)
            {
               TiXmlElement * atomparity = new TiXmlElement("atomParity");
               atom->LinkEndChild(atomparity);

               QS_DEF(Array<char>, buf);
               ArrayOutput out(buf);
               const int *pyramid = _mol->stereocenters.getPyramid(i);
               if (pyramid[3] == -1)
                  out.printf("a%d a%d a%d a%d", pyramid[0], pyramid[1], pyramid[2], i);
                else
                  out.printf("a%d a%d a%d a%d", pyramid[0], pyramid[1], pyramid[2], pyramid[3]);
               buf.push(0);
               atomparity->SetAttribute("atomRefs4", buf.ptr());

               atomparity->LinkEndChild(new TiXmlText("1"));
            }
         }
      }
   }

   if (_mol->edgeCount() > 0)
   {
      TiXmlElement * bondarray = new TiXmlElement("bondArray");
      molecule->LinkEndChild(bondarray);

      for (i = _mol->edgeBegin(); i != _mol->edgeEnd(); i = _mol->edgeNext(i))
      {
         const Edge &edge = _mol->getEdge(i);

         TiXmlElement * bond = new TiXmlElement("bond");
         bondarray->LinkEndChild(bond);

         QS_DEF(Array<char>, buf);
         ArrayOutput out(buf);
         out.printf("a%d a%d", edge.beg, edge.end);
         buf.push(0);
         bond->SetAttribute("atomRefs2", buf.ptr());


         int order = _mol->getBondOrder(i);

         if (order == BOND_SINGLE || order == BOND_DOUBLE || order == BOND_TRIPLE)
            bond->SetAttribute("order", order);
         else
            bond->SetAttribute("order", "A");


         int dir = _mol->getBondDirection(i);
         int parity = _mol->cis_trans.getParity(i);

         if (_mol->have_xyz && (dir == BOND_UP || dir == BOND_DOWN))
         {
            TiXmlElement * bondstereo = new TiXmlElement("bondStereo");
            bond->LinkEndChild(bondstereo);
            bondstereo->LinkEndChild(new TiXmlText((dir == BOND_UP) ? "W" : "H"));
         }
         else if (!_mol->have_xyz && parity != 0)
         {
            TiXmlElement * bondstereo = new TiXmlElement("bondStereo");
            bond->LinkEndChild(bondstereo);

            QS_DEF(Array<char>, buf);
            ArrayOutput out(buf);

            const int *subst = _mol->cis_trans.getSubstituents(i);
            out.printf("a%d a%d a%d a%d", subst[0], edge.beg, edge.end, subst[2]);
            buf.push(0);
            bondstereo->SetAttribute("atomRefs4", buf.ptr());
            bondstereo->LinkEndChild(new TiXmlText((parity == MoleculeCisTrans::CIS) ? "C" : "T"));
         }
      }
   }

   TiXmlPrinter printer;
   _doc->Accept(&printer);
   _output.printf("%s", printer.CStr());
}
