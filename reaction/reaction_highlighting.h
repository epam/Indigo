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

#ifndef __reaction_highlighting_h__
#define __reaction_highlighting_h__

#include "graph/graph_highlighting.h"
#include "base_cpp/obj_array.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4251)
#endif

namespace indigo {

class BaseReaction;

class DLLEXPORT ReactionHighlighting
{
public:
   ReactionHighlighting ();
   ~ReactionHighlighting ();

   void clear ();
   void init (BaseReaction &reaction);
   void nondestructiveInit (BaseReaction &reaction);

   void copy (ReactionHighlighting &reaction, const ObjArray< Array<int> >& mapping);

   GraphHighlighting & getGraphHighlighting (int index);
   int getCount() const;
protected:
   ObjArray<GraphHighlighting> _graphHighlightings;
};

}

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
