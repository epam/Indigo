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

#ifndef __molecule_logp_h__
#define __molecule_logp_h__

#include "base_cpp/tlscont.h"
#include "base_cpp/obj_array.h"
#include "base_cpp/red_black.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4251)
#endif

namespace indigo {

class Molecule;

struct LogPOptions
{
   enum LogPModel { LOGP_MODEL_SIMPLE, LOGP_MODEL_ADVANCED };

   LogPModel model;

   LogPOptions (LogPModel model = LOGP_MODEL_SIMPLE) : model(model) {}
};

class MoleculeLogPModel
{
public:
   DECL_ERROR;
   static float estimateLogPValue (Molecule &mol, const LogPOptions &options);
   static float estimateLogDValue (Molecule &mol, float pH, const LogPOptions &options);

private:
   MoleculeLogPModel ();
   static MoleculeLogPModel _model;

   static void _loadSimpleLogPModel ();
   static void _loadAdvancedLogPModel ();
   static float _estimate_LogP_Simple (Molecule &mol, const LogPOptions &options);
   static float _estimate_LogP_Advanced (Molecule &mol, const LogPOptions &options);
};

}

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
