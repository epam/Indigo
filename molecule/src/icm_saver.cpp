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
#include "molecule/icm_saver.h"
#include "molecule/cmf_saver.h"
#include "molecule/icm_common.h"

using namespace indigo;

const char* IcmSaver::VERSION2 = "IM2";
const char* IcmSaver::VERSION1 = "ICM";

IMPL_ERROR(IcmSaver, "ICM saver");

bool IcmSaver::checkVersion (const char *prefix)
{
   return strncmp(prefix, VERSION1, 3) == 0 || strncmp(prefix, VERSION2, 3) == 0;
}

IcmSaver::IcmSaver (Output &output) : _output(output)
{
   save_xyz = false;
   save_bond_dirs = false;
   save_highlighting = false;
   save_ordering = false;
}

void IcmSaver::saveMolecule (Molecule &mol)
{
   _output.writeString(VERSION2);

   int features = 0;

   if (save_xyz)
      features |= ICM_XYZ;

   if (save_bond_dirs)
      features |= ICM_BOND_DIRS;

   _output.writeChar(features);
   
   CmfSaver saver(_output);

   saver.save_bond_dirs = save_bond_dirs;
   saver.save_highlighting = save_highlighting;
   saver.save_mapping = save_ordering;

   saver.saveMolecule(mol);
   if (save_xyz)
      saver.saveXyz(_output);
}
