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

#ifndef __icr_loader__
#define __icr_loader__

#include "base_cpp/exception.h"

namespace indigo {

class Scanner;
class Reaction;

class IcrLoader
{
public:

   // external dictionary, internal decoder
   explicit IcrLoader (Scanner &scanner);

   void loadReaction (Reaction &reaction);

   DECL_ERROR;

protected:
   Scanner &_scanner;

private:
   IcrLoader (const IcrLoader &); // no implicit copy
};

}

#endif
