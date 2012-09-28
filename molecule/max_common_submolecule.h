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

#ifndef _max_common_submolecule
#define _max_common_submolecule
#include "base_cpp/scanner.h"
#include "molecule/molfile_loader.h"
#include "molecule/molfile_saver.h"
#include "graph/max_common_subgraph.h"
#include "time.h"
#include "molecule/molecule.h"

namespace indigo {

class DLLEXPORT MaxCommonSubmolecule: public MaxCommonSubgraph{
public:
   MaxCommonSubmolecule(BaseMolecule& submol, BaseMolecule& supermol);

   static bool matchBonds (Graph &g1, Graph &g2, int i, int j, void* userdata);
   static bool matchAtoms (Graph &g1, Graph &g2, const int *core_sub, int i, int j, void* userdata);
   
};

}

#endif
