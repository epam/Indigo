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

#include "molecule/molecule_stereocenter_options.h"

using namespace indigo;

//
// StereocentersOptions
//
StereocentersOptions::StereocentersOptions ()
{
   reset();
}

void StereocentersOptions::reset ()
{
   ignore_errors = false;
   bidirectional_mode = false;
   detect_haworth_projection = false; 
}

