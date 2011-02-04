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

#include "molecule/molecule_cml_loader.h"
#include "base_cpp/scanner.h"
#include "molecule/molecule.h"
#include "tinyxml.h"
#include "molecule/elements.h"
#include "molecule/molecule_scaffold_detection.h"

using namespace indigo;

MoleculeCmlLoader::MoleculeCmlLoader (Scanner &scanner) : _scanner(scanner)
{
   ignore_stereochemistry_errors = false;
}

void MoleculeCmlLoader::loadMolecule (Molecule &mol)
{
   mol.clear();

   QS_DEF(Array<char>, buf);
   _scanner.readAll(buf);
   buf.push(0);

   QS_DEF(RedBlackStringMap<int>, atoms);
   QS_DEF(Array<int>, bond_orientations);
   QS_DEF(Array<int>, atom_stereo_types);
   QS_DEF(Array<int>, total_h_count);
   atoms.clear();
   bond_orientations.clear();
   atom_stereo_types.clear();
   total_h_count.clear();

   TiXmlDocument xml;

   xml.Parse(buf.ptr());

   if (xml.Error())
      throw Error("XML parsing error: %s", xml.ErrorDesc());

   TiXmlHandle hxml(&xml);
   TiXmlElement *elem;
   TiXmlHandle hroot(0);

   elem = hxml.FirstChild("molecule").Element();
   if (elem == 0)
      elem = hxml.FirstChild("cml").FirstChild("molecule").Element();
   if (elem == 0)
      throw Error("no <molecule>?");
   hroot = TiXmlHandle(elem);

   const char *title = elem->Attribute("title");

   if (title != 0)
      mol.name.readString(title, true);

   // Atoms

   elem = hroot.FirstChild("atomArray").FirstChild().Element();
   for (; elem; elem = elem->NextSiblingElement())
   {
      if (strncmp(elem->Value(), "atom", 4) != 0)
         continue;
      const char *id = elem->Attribute("id");
      if (id == 0)
         throw Error("atom without an id");

      const char *element_type = elem->Attribute("elementType");

      if (element_type == 0)
         throw Error("atom without an elementType");

      int label = Element::fromString2(element_type);

      if (label == -1)
         label = ELEM_PSEUDO;

      int idx = mol.addAtom(label);

      total_h_count.expandFill(idx + 1, -1);

      atoms.insert(id, idx);

      const char *isotope = elem->Attribute("isotope");

      if (isotope == 0)
         isotope = elem->Attribute("isotopeNumber");

      if (isotope != 0)
      {
         int val;
         if (sscanf(isotope, "%d", &val) != 1)
            throw Error("error parsing isotope");
         mol.setAtomIsotope(idx, val);
      }

      const char *charge = elem->Attribute("formalCharge");

      if (charge != 0)
      {
         int val;
         if (sscanf(charge, "%d", &val) != 1)
            throw Error("error parsing charge");
         mol.setAtomCharge(idx, val);
      }

      const char *radical = elem->Attribute("spinMultiplicity");
      
      if (radical != 0)
      {
         int val;
         if (sscanf(radical, "%d", &val) != 1)
            throw Error("error parsing spin multiplicity");
         mol.setAtomRadical(idx, val);
      }

      const char *hcount = elem->Attribute("hydrogenCount");

      if (hcount != 0)
      {
         int val;
         if (sscanf(hcount, "%d", &val) != 1)
            throw Error("error parsing hydrogen count");
         if (val < 0)
            throw Error("negative hydrogen count");
         total_h_count[idx] = val;
      }

      const char *x2 = elem->Attribute("x2");
      const char *y2 = elem->Attribute("y2");

      if (x2 != 0 && y2 != 0)
      {
         if (sscanf(x2, "%f", &mol.getAtomXyz(idx).x) != 1)
            throw Error("error parsing x2");
         if (sscanf(y2, "%f", &mol.getAtomXyz(idx).y) != 1)
            throw Error("error parsing y2");
      }

      const char *x3 = elem->Attribute("x3");
      const char *y3 = elem->Attribute("y3");
      const char *z3 = elem->Attribute("z3");

      if (x3 != 0 && y3 != 0 && z3 != 0)
      {
         if (sscanf(x3, "%f", &mol.getAtomXyz(idx).x) != 1)
            throw Error("error parsing x3");
         if (sscanf(y3, "%f", &mol.getAtomXyz(idx).y) != 1)
            throw Error("error parsing y3");
         if (sscanf(z3, "%f", &mol.getAtomXyz(idx).z) != 1)
            throw Error("error parsing z3");
      }
      atom_stereo_types.push(0);
   }

   // Bonds
   bool have_cistrans_notation = false;
   elem = hroot.FirstChild("bondArray").FirstChild().Element();

   for (; elem; elem = elem->NextSiblingElement())
   {
      if (strncmp(elem->Value(), "bond", 4) != 0)
         continue;
      const char *atom_refs = elem->Attribute("atomRefs2");
      if (atom_refs == 0)
         throw Error("bond without atomRefs2");

      BufferScanner strscan(atom_refs);
      QS_DEF(Array<char>, id);
      int beg;
      int end;

      strscan.readWord(id, 0);
      beg = atoms.at(id.ptr());
      strscan.skipSpace();
      strscan.readWord(id, 0);
      end = atoms.at(id.ptr());

      const char *order = elem->Attribute("order");
      if (order == 0)
         throw Error("bond without an order");

      int order_val;
      {
         if (order[0] == 'A' && order[1] == 0)
            order_val = BOND_AROMATIC;
         else if (order[0] == 'S' && order[1] == 0)
            order_val = BOND_SINGLE;
         else if (order[0] == 'D' && order[1] == 0)
            order_val = BOND_DOUBLE;
         else if (order[0] == 'T' && order[1] == 0)
            order_val = BOND_TRIPLE;
         else if (sscanf(order, "%d", &order_val) != 1)
            throw Error("error parsing order");
      }

      mol.addBond(beg, end, order_val);

      int dir = 0;

      TiXmlElement *bs_elem = elem->FirstChildElement("bondStereo");

      if (bs_elem != 0)
      {
         const char *text = bs_elem->GetText();
         if (text != 0)
         {
            if (text[0] == 'W' && text[1] == 0)
               dir = MoleculeStereocenters::BOND_UP;
            if (text[0] == 'H' && text[1] == 0)
               dir = MoleculeStereocenters::BOND_DOWN;
            if ((text[0] == 'C' || text[0] == 'T') && text[1] == 0)
               have_cistrans_notation = true;
         }
      }

      bond_orientations.push(dir);
      if (dir != 0)
         atom_stereo_types[beg] = MoleculeStereocenters::ATOM_ABS;
   }

   // Implicit H counts
   int i, j;

   for (i = mol.vertexBegin(); i != mol.vertexEnd(); i = mol.vertexNext(i))
   {
      int h = total_h_count[i];

      if (h < 0)
         continue;

      const Vertex &vertex = mol.getVertex(i);

      for (j = vertex.neiBegin(); j != vertex.neiEnd(); j = vertex.neiNext(j))
         if (mol.getAtomNumber(vertex.neiVertex(j)) == ELEM_H)
            h--;

      if (h < 0)
         throw Error("hydrogenCount on atom %d is less than the number of explicit hydrogens");

      mol.setImplicitH(i, h);
   }

   // Tetrahedral stereocenters
   elem = hroot.FirstChild("atomArray").FirstChild().Element();
   for (; elem; elem = elem->NextSiblingElement())
   {
      const char *id = elem->Attribute("id");
      
      if (id == 0)
         throw Error("atom without an id");

      int idx = atoms.at(id);

      TiXmlElement *ap_elem = elem->FirstChildElement("atomParity");

      if (ap_elem == 0)
         continue;

      const char *ap_text = ap_elem->GetText();

      if (ap_text == 0)
         continue;

      int val;
      if (sscanf(ap_text, "%d", &val) != 1)
         throw Error("error parsing atomParity");

      const char *refs4 = ap_elem->Attribute("atomRefs4");

      if (refs4 != 0)
      {
         BufferScanner strscan(refs4);
         QS_DEF(Array<char>, id);
         int k, pyramid[4];

         for (k = 0; k < 4; k++)
         {
            strscan.skipSpace();
            strscan.readWord(id, 0);
            pyramid[k] = atoms.at(id.ptr());
            if (pyramid[k] == idx)
               pyramid[k] = -1;
         }

         if (val < 0)
            __swap(pyramid[0], pyramid[1], k);

         MoleculeStereocenters::moveMinimalToEnd(pyramid);

         mol.stereocenters.add(idx, MoleculeStereocenters::ATOM_ABS, 0, pyramid);
      }
   }

   if (mol.stereocenters.size() == 0 && BaseMolecule::hasCoord(mol))
      mol.stereocenters.buildFromBonds(atom_stereo_types.ptr(), 0, bond_orientations.ptr(), ignore_stereochemistry_errors);

   // Cis-trans stuff
   if (have_cistrans_notation)
   {
      elem = hroot.FirstChild("bondArray").FirstChild().Element();

      int bond_idx = -1;

      for (; elem; elem = elem->NextSiblingElement())
      {
         if (strncmp(elem->Value(), "bond", 4) != 0)
            continue;
         bond_idx++;
         TiXmlElement *bs_elem = elem->FirstChildElement("bondStereo");

         if (bs_elem == 0)
            continue;

         const char *text = bs_elem->GetText();

         if (text == 0)
            continue;

         int parity;

         if (text[0] == 'C' && text[1] == 0)
            parity = MoleculeCisTrans::CIS;
         else if (text[0] == 'T' && text[1] == 0)
            parity = MoleculeCisTrans::TRANS;
         else
            continue;

         const char *atom_refs = bs_elem->Attribute("atomRefs4");

         if (atom_refs == 0)
            continue;

         int substituents[4];

         if (!MoleculeCisTrans::isGeomStereoBond(mol, bond_idx, substituents, false))
            throw Error("cis-trans notation on a non cis-trans bond #%d", bond_idx);

         if (!MoleculeCisTrans::sortSubstituents(mol, substituents))
            throw Error("cis-trans notation on a non cis-trans bond #%d", bond_idx);

         BufferScanner strscan(atom_refs);
         QS_DEF(Array<char>, id);
         int refs[4];
         int k;

         for (k = 0; k < 4; k++)
         {
            strscan.skipSpace();
            strscan.readWord(id, 0);
            refs[k] = atoms.at(id.ptr());
         }

         const Edge &edge = mol.getEdge(bond_idx);

         if (refs[1] == edge.beg && refs[2] == edge.end)
            ;
         else if (refs[1] == edge.end && refs[2] == edge.beg)
         {
            __swap(refs[0], refs[3], k);
            __swap(refs[1], refs[2], k);
         }
         else
            throw Error("atomRefs4 in bondStereo do not match the bond ends");

         bool invert = false;

         if (refs[0] == substituents[0])
            ;
         else if (refs[0] == substituents[1])
            invert = !invert;
         else
            throw Error("atomRefs4 in bondStereo do not match the substituents");

         if (refs[3] == substituents[2])
            ;
         else if (refs[3] == substituents[3])
            invert = !invert;
         else
            throw Error("atomRefs4 in bondStereo do not match the substituents");

         if (invert)
            parity = 3 - parity;

         mol.cis_trans.add(bond_idx, substituents, parity);
      }
   }
   else if (BaseMolecule::hasCoord(mol))
      mol.cis_trans.build(0);
}
