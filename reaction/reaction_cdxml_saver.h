/****************************************************************************
 * Copyright (C) 2016 EPAM Systems
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

#ifndef __reaction_cdxml_saver__
#define __reaction_cdxml_saver__

#include "base_cpp/exception.h"
#include "base_cpp/array.h"
#include "base_cpp/obj_array.h"

namespace indigo {

class Output;
class BaseReaction;
class MoleculeCdxmlSaver;

class DLLEXPORT ReactionCdxmlSaver
{
public:
   explicit ReactionCdxmlSaver (Output &output);
   ~ReactionCdxmlSaver ();

   void saveReaction (BaseReaction &rxn);

   DECL_ERROR;

protected:
   BaseReaction *_rxn;
   Output   &_output;

private:
   ReactionCdxmlSaver (const ReactionCdxmlSaver &); // no implicit copy

   void _addDefaultFontTable (MoleculeCdxmlSaver &molsaver);
   void _addDefaultColorTable (MoleculeCdxmlSaver &molsaver);
   void _addPlusses (BaseReaction &rxn, MoleculeCdxmlSaver &molsaver);
   void _addArrow (BaseReaction &rxn, MoleculeCdxmlSaver &molsaver, int arrow_id);
   void _addScheme (MoleculeCdxmlSaver &molsaver);
   void _closeScheme (MoleculeCdxmlSaver &molsaver);
   void _addStep (BaseReaction &rxn, MoleculeCdxmlSaver &molsaver, Array<int> &reactants_ids, Array<int> &products_ids,
           ObjArray< Array<int> > &nodes_ids, int arrow_id);
   void _generateCdxmlObjIds (BaseReaction &rxn, Array<int> &reactants_ids, Array<int> &products_ids,
           ObjArray< Array<int> > &nodes_ids, int &arrow_id);
   void _addTitle (BaseReaction &rxn, MoleculeCdxmlSaver &molsaver);

};

}

#endif
