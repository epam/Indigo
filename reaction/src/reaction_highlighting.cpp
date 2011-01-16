/****************************************************************************
 * Copyright (C) 2009-2011 GGA Software Services LLC
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

#include "reaction/reaction_highlighting.h"
#include "reaction/base_reaction.h"

using namespace indigo;

ReactionHighlighting::ReactionHighlighting ()
{
}

ReactionHighlighting::~ReactionHighlighting ()
{
}

int ReactionHighlighting::getCount() const
{
   return _graphHighlightings.size();
}
 
GraphHighlighting & ReactionHighlighting::getGraphHighlighting (int index)
{
   return _graphHighlightings[index];
}

void ReactionHighlighting::clear ()
{
   _graphHighlightings.clear();
}

void ReactionHighlighting::init(BaseReaction &reaction) {
   _graphHighlightings.clear();
   for(int i = 0; i < reaction.end(); i++) {
      _graphHighlightings.push();
   }
   for(int i = reaction.begin();i < reaction.end(); i = reaction.next(i))
      _graphHighlightings[i].init(reaction.getBaseMolecule(i));
}

void ReactionHighlighting::copy (ReactionHighlighting &rhl, const ObjArray< Array<int> >& mapping)
{
   for(int i = 0; i < rhl.getCount(); ++i)
      if (rhl._graphHighlightings[i].numVertices() > 0)
         _graphHighlightings[i].copy(rhl.getGraphHighlighting(i), &mapping[i]);
}


void ReactionHighlighting::nondestructiveInit(BaseReaction &reaction) {
   while (_graphHighlightings.size() < reaction.end())
      _graphHighlightings.push();
}
