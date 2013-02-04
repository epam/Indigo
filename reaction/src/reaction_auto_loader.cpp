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

#include "reaction/reaction_auto_loader.h"
#include "reaction/rsmiles_loader.h"
#include "reaction/rxnfile_loader.h"
#include "reaction/icr_loader.h"
#include "reaction/icr_saver.h"
#include "gzip/gzip_scanner.h"
#include "reaction/reaction.h"
#include "reaction/query_reaction.h"
#include "molecule/molecule_auto_loader.h"
#include "reaction/reaction_cml_loader.h"

using namespace indigo;

void ReactionAutoLoader::_init ()
{
   treat_x_as_pseudoatom = false;
   ignore_closing_bond_direction_mismatch = false;
   ignore_stereocenter_errors = false;
   ignore_noncritical_query_features = false;
}

IMPL_ERROR(ReactionAutoLoader, "reaction auto loader");

ReactionAutoLoader::ReactionAutoLoader (Scanner &scanner)
{
   _scanner = &scanner;
   _own_scanner = false;
   _init();
}

ReactionAutoLoader::ReactionAutoLoader (const Array<char> &arr)
{
   _scanner = new BufferScanner(arr);
   _own_scanner = true;
   _init();
}

ReactionAutoLoader::ReactionAutoLoader (const char *str)
{
   _scanner = new BufferScanner(str);
   _own_scanner = true;
   _init();
}

ReactionAutoLoader::~ReactionAutoLoader ()
{
   if (_own_scanner)
      delete _scanner;
}

void ReactionAutoLoader::loadReaction (Reaction &reaction)
{
   _loadReaction(reaction, false);
}

void ReactionAutoLoader::loadQueryReaction (QueryReaction &reaction)
{
   _loadReaction(reaction, true);
}

void ReactionAutoLoader::_loadReaction (BaseReaction &reaction, bool query)
{
   // check fir GZip format
   if (_scanner->length() >= 2)
   {
      byte id[2];
      int pos = _scanner->tell();

      _scanner->readCharsFix(2, (char *)id);
      _scanner->seek(pos, SEEK_SET);
      
      if (id[0] == 0x1f && id[1] == 0x8b)
      {
         GZipScanner gzscanner(*_scanner);
         QS_DEF(Array<char>, buf);

         gzscanner.readAll(buf);
         ReactionAutoLoader loader2(buf);

         loader2.ignore_stereocenter_errors = ignore_stereocenter_errors;
         loader2.ignore_noncritical_query_features = ignore_noncritical_query_features;
         loader2.treat_x_as_pseudoatom = treat_x_as_pseudoatom;
         if (query)
            loader2.loadQueryReaction((QueryReaction &)reaction);
         else
            loader2.loadReaction((Reaction &)reaction);
         return;
      }
   }

   // check for MDLCT format
   {
      QS_DEF(Array<char>, buf);
      if (MoleculeAutoLoader::tryMDLCT(*_scanner, buf))
      {
         BufferScanner scanner2(buf);
         RxnfileLoader loader(scanner2);
         loader.treat_x_as_pseudoatom = treat_x_as_pseudoatom;
         loader.ignore_stereocenter_errors = ignore_stereocenter_errors;
         loader.ignore_noncritical_query_features = ignore_noncritical_query_features;
         if (query)
            loader.loadQueryReaction((QueryReaction &)reaction);
         else
            loader.loadReaction((Reaction &)reaction);
         return;
      }
   }

   // check for ICM format
   if (_scanner->length() >= 4)
   {
      char id[3];
      int pos = _scanner->tell();

      _scanner->readCharsFix(3, id);
      _scanner->seek(pos, SEEK_SET);
      if (IcrSaver::checkVersion(id))
      {
         if (query)
            throw Error("cannot load query reaction from ICR format");

         IcrLoader loader(*_scanner);
         loader.loadReaction((Reaction &)reaction);
         return;
      }
   }

   // check for CML format
   {
      int pos = _scanner->tell();
      _scanner->skipSpace();

      if (_scanner->lookNext() == '<')
      {
         if (_scanner->findWord("<reaction"))
         {
            ReactionCmlLoader loader(*_scanner);
            loader.ignore_stereochemistry_errors = ignore_stereocenter_errors;
            if (query)
               throw Error("CML queries not supported");
            loader.loadReaction((Reaction &)reaction);
            return;
         }
      }

      _scanner->seek(pos, SEEK_SET);
   }

   // check for SMILES format
   if (Scanner::isSingleLine(*_scanner))
   {
      RSmilesLoader loader(*_scanner);

      loader.ignore_closing_bond_direction_mismatch =
             ignore_closing_bond_direction_mismatch;
      if (query)
         loader.loadQueryReaction((QueryReaction &)reaction);
      else
         loader.loadReaction((Reaction &)reaction);
   }

   // default is Rxnfile format
   else
   {
      RxnfileLoader loader(*_scanner);
      loader.treat_x_as_pseudoatom = treat_x_as_pseudoatom;
      loader.ignore_stereocenter_errors = ignore_stereocenter_errors;
      loader.ignore_noncritical_query_features = ignore_noncritical_query_features;
      if (query)
         loader.loadQueryReaction((QueryReaction &)reaction);
      else
         loader.loadReaction((Reaction &)reaction);
   }
}
