/****************************************************************************
 * Copyright (C) 2009-2010 GGA Software Services LLC
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

#include <time.h>

#include "reaction/reaction.h"
#include "reaction/query_reaction.h"
#include "reaction/rxnfile_saver.h"
#include "base_cpp/output.h"
#include "reaction/reaction_highlighting.h"
#include "molecule/molfile_saver.h"

RxnfileSaver::RxnfileSaver(Output &output) : 
   highlighting(0),
   _output(output)
{
   molfile_saving_mode = MolfileSaver::MODE_AUTO;
}

RxnfileSaver::~RxnfileSaver(){
}

void RxnfileSaver::saveBaseReaction (BaseReaction& reaction)
{
   if (reaction.isQueryReaction())
      saveQueryReaction(reaction.asQueryReaction());
   else
      saveReaction(reaction.asReaction());
}

void RxnfileSaver::saveReaction (Reaction &reaction)
{
   _rxn = &reaction;
   _brxn = &reaction;
   _qrxn = 0;
   _saveReaction();
}

void RxnfileSaver::saveQueryReaction (QueryReaction &reaction)
{
   _qrxn = &reaction;
   _brxn = &reaction;
   _rxn = 0;
   _saveReaction();
}

void RxnfileSaver::_saveReaction(){
   int i;

   if (molfile_saving_mode == MolfileSaver::MODE_3000)
      _v2000 = false;
   else if (molfile_saving_mode == MolfileSaver::MODE_2000)
      _v2000 = true;
   else
   {
      _v2000 = true;

      for (i = _brxn->begin(); i != _brxn->end(); i = _brxn->next(i))
      {
         if (highlighting != 0 && highlighting->getGraphHighlighting(i).numEdges() + 
             highlighting->getGraphHighlighting(i).numVertices() > 0)
         {
            _v2000 = false;
            break;
         }
         if (!_brxn->getBaseMolecule(i).stereocenters.haveAllAbsAny() &&
             !_brxn->getBaseMolecule(i).stereocenters.haveAllAndAny())
         {
            _v2000 = false;
            break;
         }
      }
   }

   MolfileSaver molfileSaver(_output);
   molfileSaver.mode = _v2000 ? MolfileSaver::MODE_2000 : MolfileSaver::MODE_3000;
   
   _writeRxnHeader(*_brxn, _brxn->reactantsCount(), _brxn->productsCount());

   _writeReactantsHeader();

   for (i = _brxn->reactantBegin(); i < _brxn->reactantEnd(); i = _brxn->reactantNext(i) )
   {
      _writeMolHeader();
      if(highlighting != 0)
         molfileSaver.highlighting = &highlighting->getGraphHighlighting(i);
      _writeMol(molfileSaver, i);
   }

   _writeReactantsFooter();
   _writeProductsHeader();

   for (int i = _brxn->productBegin(); i < _brxn->productEnd(); i = _brxn->productNext(i))
   {
      _writeMolHeader();
      if(highlighting != 0)
         molfileSaver.highlighting = &highlighting->getGraphHighlighting(i);
      _writeMol(molfileSaver, i);
   }

   _writeProductsFooter();
   _writeRxnFooter();
}

void RxnfileSaver::_writeRxnHeader(BaseReaction &reaction, int reactantSize, int productSize)
{
   if (_v2000)
      _output.writeStringCR("$RXN");
   else
      _output.writeStringCR("$RXN V3000");

   time_t tm = time(NULL);
   const struct tm *lt = localtime(&tm);

   if (reaction.name.ptr() != 0)
      _output.printfCR("%s", reaction.name.ptr());
   else
      _output.writeCR();
   _output.printfCR("  -BINGO- %02d%02d%02d%02d%02d", lt->tm_mon + 1, lt->tm_mday,
      lt->tm_year % 100, lt->tm_hour, lt->tm_min);
   _output.writeCR();

   if (_v2000)
      _output.printf("%3d%3d\n", reactantSize, productSize);
   else
      _output.printf("M  V30 COUNTS %d %d\n", reactantSize, productSize);
}

void RxnfileSaver::_writeMolHeader ()
{
   if (_v2000)
      _output.writeStringCR("$MOL");
}

void RxnfileSaver::_writeReactantsHeader ()
{
   if (!_v2000)
      _output.writeStringCR("M  V30 BEGIN REACTANT");
}

void RxnfileSaver::_writeProductsHeader ()
{
   if (!_v2000)
      _output.writeStringCR("M  V30 BEGIN PRODUCT");
}

void RxnfileSaver::_writeMol (MolfileSaver &saver, int index) {
   
   saver.reactionAtomMapping = &_brxn->getAAMArray(index);
   saver.reactionAtomInversion = &_brxn->getInversionArray(index);
   saver.reactionBondReactingCenter = &_brxn->getReactingCenterArray(index);

   if (_qrxn != 0)
      saver.reactionAtomExactChange = &_qrxn->getExactChangeArray(index);

   if (_qrxn != 0)
   {
      if (_v2000)
         saver.saveQueryMolecule(_qrxn->getQueryMolecule(index));
      else
         saver.saveQueryCtab3000(_qrxn->getQueryMolecule(index));
   }
   else
   {
      if (_v2000)
         saver.saveMolecule(_rxn->getMolecule(index));
      else
         saver.saveCtab3000(_rxn->getMolecule(index));
   }
}

void RxnfileSaver::_writeReactantsFooter ()
{
   if (!_v2000)
      _output.writeStringCR("M  V30 END REACTANT");
}

void RxnfileSaver::_writeProductsFooter ()
{
   if (!_v2000)
      _output.writeStringCR("M  V30 END PRODUCT");
}

void RxnfileSaver::_writeRxnFooter ()
{
   if (!_v2000)
      _output.writeStringCR("M  END");
}
