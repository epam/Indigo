/****************************************************************************
 * Copyright (C) 2009-2010 GGA Software Services LLC
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

#include "base_cpp/output.h"
#include "reaction/icr_saver.h"
#include "reaction/crf_saver.h"

using namespace indigo;

IcrSaver::IcrSaver (Output &output) : _output(output)
{
}

void IcrSaver::saveReaction (Reaction &reaction)
{
   _output.writeString("ICR");

   if (save_xyz)
      _output.writeChar(1);
   else
      _output.writeChar(0);

   CrfSaver saver(_output);

   if (save_xyz)
      saver.xyz_output = &_output;

   saver.skip_implicit_h = true;
   saver.saveReaction(reaction);
}
