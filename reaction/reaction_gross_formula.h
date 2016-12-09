/****************************************************************************
 * Copyright (C) 2009-2015 EPAM Systems
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

#ifndef __reaction_gross_formula__
#define __reaction_gross_formula__

#include <utility>

#include "base_cpp/array.h"
#include "base_cpp/obj_array.h"

#include "reaction/base_reaction.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4251)
#endif

namespace indigo {


class BaseReaction;

class DLLEXPORT ReactionGrossFormula
{
public:
   static void collect (BaseReaction &rxn, std::pair<ObjArray<std::pair<ObjArray<Array<char> >, ObjArray<Array<int> > > > , ObjArray<std::pair<ObjArray<Array<char> >, ObjArray<Array<int> > > > > &gross);
   static void toString_Hill (const std::pair<ObjArray<std::pair<ObjArray<Array<char> >, ObjArray<Array<int> > > > , ObjArray<std::pair<ObjArray<Array<char> >, ObjArray<Array<int> > > > > &gross, Array<char> &str, bool add_rsites);
};
    
}

#endif

