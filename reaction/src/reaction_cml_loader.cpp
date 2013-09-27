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

#include "reaction/reaction_cml_loader.h"
#include "reaction/reaction.h"
#include "base_cpp/tlscont.h"
#include "base_cpp/scanner.h"
#include "tinyxml.h"
#include "molecule/molecule_cml_loader.h"

using namespace indigo;

IMPL_ERROR(ReactionCmlLoader, "reaction CML loader");

ReactionCmlLoader::ReactionCmlLoader (Scanner &scanner) : _scanner(scanner)
{
   ignore_stereochemistry_errors = false;
}

ReactionCmlLoader::~ReactionCmlLoader ()
{
}

void ReactionCmlLoader::loadReaction (Reaction &rxn)
{
   rxn.clear();

   QS_DEF(Array<char>, buf);
   _scanner.readAll(buf);
   buf.push(0);

   TiXmlDocument xml;

   xml.Parse(buf.ptr());

   if (xml.Error())
      throw Error("XML parsing error: %s", xml.ErrorDesc());

   TiXmlHandle hxml(&xml);
   TiXmlElement *elem;
   TiXmlHandle hroot(0);

   elem = hxml.FirstChild("reaction").Element();
   if (elem == 0)
      elem = hxml.FirstChild("cml").FirstChild("reaction").Element();
   if (elem == 0)
      throw Error("no <reaction>?");
   hroot = TiXmlHandle(elem);

   const char *title = elem->Attribute("title");

   if (title != 0)
      rxn.name.readString(title, true);

   QS_DEF(Molecule, mol);

   elem = hroot.FirstChild("reactantList").FirstChild().Element();
   for (; elem; elem = elem->NextSiblingElement())
   {
      if (strcasecmp(elem->Value(), "molecule") != 0)
         continue;
      TiXmlHandle handle(elem);
      MoleculeCmlLoader loader(handle);
      loader.ignore_stereochemistry_errors = ignore_stereochemistry_errors;
      loader.loadMolecule(mol);
      rxn.addReactantCopy(mol, 0, 0);
   }

   elem = hroot.FirstChild("productList").FirstChild().Element();
   for (; elem; elem = elem->NextSiblingElement())
   {
      if (strcasecmp(elem->Value(), "molecule") != 0)
         continue;
      TiXmlHandle handle(elem);
      MoleculeCmlLoader loader(handle);
      loader.ignore_stereochemistry_errors = ignore_stereochemistry_errors;
      loader.loadMolecule(mol);
      rxn.addProductCopy(mol, 0, 0);
   }

   elem = hroot.FirstChild("spectatorList").FirstChild().Element();
   for (; elem; elem = elem->NextSiblingElement())
   {
      if (strcasecmp(elem->Value(), "molecule") != 0)
         continue;
      TiXmlHandle handle(elem);
      MoleculeCmlLoader loader(handle);
      loader.ignore_stereochemistry_errors = ignore_stereochemistry_errors;
      loader.loadMolecule(mol);
      rxn.addCatalystCopy(mol, 0, 0);
   }
}
