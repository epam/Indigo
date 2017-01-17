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
#include "base_cpp/ptr_array.h"
#include "molecule/molecule_gross_formula.h"


#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4251)
#endif

namespace indigo {


class BaseReaction;

class DLLEXPORT ReactionGrossFormula
{
public:
   static std::unique_ptr<std::pair<PtrArray<GROSS_UNITS> , PtrArray<GROSS_UNITS> > > collect (BaseReaction &rxn);
   static void toString_Hill (std::pair<PtrArray<GROSS_UNITS> , PtrArray<GROSS_UNITS> > &gross, Array<char> &str, bool add_rsites);
};
    
}

#endif

