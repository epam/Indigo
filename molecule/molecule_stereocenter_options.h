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

#ifndef __molecule_stereocenter_options__
#define __molecule_stereocenter_options__

#include "base_c/defs.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4251)
#endif

namespace indigo {

class DLLEXPORT StereocentersOptions
{
public:
   StereocentersOptions ();

   void reset ();

   // Ignore all stereocenter errors. Default is false.
   bool ignore_errors;

   // Treat stereobond direction bond not only for a pointed stereocenter, but for the 
   // neighbour as well. Default is false.
   bool bidirectional_mode;

   // Detect Haworth projection. Default is false.
   bool detect_haworth_projection;
};

}

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif // __molecule_stereocenter_options__
