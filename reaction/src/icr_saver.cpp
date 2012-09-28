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

#include "base_cpp/output.h"
#include "reaction/icr_saver.h"
#include "reaction/crf_saver.h"
#include "molecule/icm_common.h"

using namespace indigo;

const char* IcrSaver::VERSION = "IR2";

IMPL_ERROR(IcrSaver, "ICR saver");

IcrSaver::IcrSaver (Output &output) : _output(output)
{
   save_xyz = false;
   save_bond_dirs = false;
   save_highlighting = false;
}

void IcrSaver::saveReaction (Reaction &reaction)
{
   _output.writeString(VERSION);

   int features = 0;

   if (save_xyz)
      features |= ICM_XYZ;

   if (save_bond_dirs)
      features |= ICM_BOND_DIRS;

   _output.writeChar(features);

   CrfSaver saver(_output);

   if (save_xyz)
      saver.xyz_output = &_output;

   saver.save_bond_dirs = save_bond_dirs;
   saver.save_highlighting = save_highlighting;
   saver.saveReaction(reaction);
}
