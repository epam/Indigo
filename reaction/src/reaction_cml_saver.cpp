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

#include "reaction/reaction_cml_saver.h"
#include "base_cpp/output.h"
#include "reaction/reaction.h"
#include "molecule/molecule_cml_saver.h"

using namespace indigo;

IMPL_ERROR(ReactionCmlSaver, "reaction CML saver");

ReactionCmlSaver::ReactionCmlSaver (Output &output) : _output(output)
{
   skip_cml_tag = false;
}

ReactionCmlSaver::~ReactionCmlSaver ()
{
}

void ReactionCmlSaver::saveReaction (Reaction &rxn)
{
   if (!skip_cml_tag)
   {
      _output.printf("<?xml version=\"1.0\" ?>\n");
      _output.printf("<cml>\n");
   }

   if (rxn.name.ptr() != 0)
   {
      if (strchr(rxn.name.ptr(), '\"') != NULL)
         throw Error("can not save reaction with '\"' in title");
      _output.printf("<reaction title=\"%s\">\n", rxn.name.ptr());
   }
   else
      _output.printf("<reaction>\n");

   int i;

   MoleculeCmlSaver molsaver(_output);

   molsaver.skip_cml_tag = true;

   if (rxn.reactantsCount() > 0)
   {
      _output.printf("<reactantList>\n");
      for (i = rxn.reactantBegin(); i != rxn.reactantEnd(); i = rxn.reactantNext(i))
         molsaver.saveMolecule(rxn.getMolecule(i));
      _output.printf("</reactantList>\n");
   }
   if (rxn.productsCount() > 0)
   {
      _output.printf("<productList>\n");
      for (i = rxn.productBegin(); i != rxn.productEnd(); i = rxn.productNext(i))
         molsaver.saveMolecule(rxn.getMolecule(i));
      _output.printf("</productList>\n");
   }
   if (rxn.catalystCount() > 0)
   {
      _output.printf("<spectatorList>\n");
      for (i = rxn.catalystBegin(); i != rxn.catalystEnd(); i = rxn.catalystNext(i))
         molsaver.saveMolecule(rxn.getMolecule(i));
      _output.printf("</spectatorList>\n");
   }
   _output.printf("</reaction>\n");
   if (!skip_cml_tag)
      _output.printf("</cml>\n");
}
