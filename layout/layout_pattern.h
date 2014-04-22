/****************************************************************************
 * Copyright (C) 2009-2013 GGA Software Services LLC
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

#ifndef __layout_pattern_h__
#define __layout_pattern_h__

#include "base_c/defs.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4251)
#endif

namespace indigo
{

class MoleculeLayoutGraph;
class Graph;

class DLLEXPORT PatternLayoutFinder
{      
public:
   static bool tryToFindPattern (MoleculeLayoutGraph &layout_graph);

private:

   static void _initPatterns ();
   static bool _matchPatternBond (Graph &subgraph, Graph &supergraph, int self_idx, int other_idx, void *userdata);
};

}

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
