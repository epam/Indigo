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

#ifndef __rxnfile_saver__
#define __rxnfile_saver__

#include "base_cpp/exception.h"

namespace indigo {

class Output;
class Reaction;
class BaseReaction;
class QueryReaction;
class MolfileSaver;

class RxnfileSaver
{
public:

   RxnfileSaver(Output& output);
   ~RxnfileSaver();

   void saveBaseReaction(BaseReaction& reaction);
   void saveReaction(Reaction& reaction);
   void saveQueryReaction(QueryReaction& reaction);

   int molfile_saving_mode; // MolfileSaver::MODE_***, default zero
   bool skip_date;

   DECL_ERROR;

protected:

   void _saveReaction();
   bool _v2000;

   BaseReaction  *_brxn;
   QueryReaction *_qrxn;
   Reaction      *_rxn;

   Output &_output;
   void _writeRxnHeader (BaseReaction &reaction);
   void _writeReactantsHeader ();
   void _writeProductsHeader ();
   void _writeCatalystsHeader ();
   void _writeReactantsFooter ();
   void _writeProductsFooter ();
   void _writeCatalystsFooter ();
   void _writeMolHeader ();
   void _writeMol (MolfileSaver &saver, int index);
   void _writeRxnFooter ();
};

}

#endif
