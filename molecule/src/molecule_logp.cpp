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

#include "molecule/molecule_logp.h"
#include "molecule/molecule_ionize.h"
#include "molecule/base_molecule.h"
#include "molecule/elements.h"
#include "molecule/molecule.h"
#include "molecule/smiles_loader.h"
#include "molecule/molfile_loader.h"
#include "molecule/molfile_saver.h"
#include "base_cpp/scanner.h"
#include "molecule/query_molecule.h"
#include "molecule/molecule_substructure_matcher.h"
#include "base_cpp/output.h"
#include "molecule/sdf_loader.h"
#include "base_cpp/queue.h"
#include "molecule/molecule_automorphism_search.h"

using namespace indigo;

MoleculeLogPModel MoleculeLogPModel::_model;

IMPL_ERROR(MoleculeLogPModel, "Molecule LogP Model");

MoleculeLogPModel::MoleculeLogPModel ()
{
   _loadSimpleLogPModel();
   _loadAdvancedLogPModel();
}

float MoleculeLogPModel::estimateLogPValue (Molecule &mol, const LogPOptions &options)
{
   if (options.model == LogPOptions::LOGP_MODEL_SIMPLE)
   {
      return _estimate_LogP_Simple(mol, options);
   }
   else if (options.model == LogPOptions::LOGP_MODEL_ADVANCED)
   {
      return _estimate_LogP_Advanced(mol, options);
   }
   else
      throw Error("Unsupported LogP model: %d", options.model);

   return 0.f;
}

float MoleculeLogPModel::estimateLogDValue (Molecule &mol, float pH, const LogPOptions &options)
{
   float logp;
   if (options.model == LogPOptions::LOGP_MODEL_SIMPLE)
   {
      logp = _estimate_LogP_Simple(mol, options);
   }
   else if (options.model == LogPOptions::LOGP_MODEL_ADVANCED)
   {
      logp = _estimate_LogP_Advanced(mol, options);
   }
   else
      throw Error("Unsupported LogP model: %d", options.model);

   
   return logp;
}


void MoleculeLogPModel::_loadSimpleLogPModel()
{
}

void MoleculeLogPModel::_loadAdvancedLogPModel()
{
}

float MoleculeLogPModel::_estimate_LogP_Simple (Molecule &mol, const LogPOptions &options)
{
   return 0.f;
}

float MoleculeLogPModel::_estimate_LogP_Advanced (Molecule &mol, const LogPOptions &options)
{
   return 0.f;
}

