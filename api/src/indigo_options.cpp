/****************************************************************************
 * Copyright (C) 2010-2011 GGA Software Services LLC
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

#include "indigo_internal.h"
#include "molecule/molfile_saver.h"

static void indigoIgnoreStereochemistryErrors (int enabled)
{
   Indigo &self = indigoGetInstance();
   self.ignore_stereochemistry_errors = (enabled != 0);
}

static void indigoIgnoreNoncricicalQueryFeatures (int enabled)
{
   Indigo &self = indigoGetInstance();
   self.ignore_noncritical_query_features = (enabled != 0);
}


static void indigoTreatXAsPseudoatom (int enabled)
{
   Indigo &self = indigoGetInstance();
   self.treat_x_as_pseudoatom = (enabled != 0);
}

static void indigoSkip3dChirality (int enabled)
{
   Indigo &self = indigoGetInstance();
   self.skip_3d_chirality = (enabled != 0);
}

static void indigoDeconvolutionAromatization (int enabled)
{
   Indigo &self = indigoGetInstance();
   self.deconvolution_aromatization = (enabled != 0);
}
static void indigoDecoSaveAPBondOrders (int enabled)
{
   Indigo &self = indigoGetInstance();
   self.deco_save_ap_bond_orders = (enabled != 0);
}
static void indigoDecoIgnoreErrors (int enabled)
{
   Indigo &self = indigoGetInstance();
   self.deco_ignore_errors = (enabled != 0);
}

static void indigoSetMolfileSavingMode (const char *mode)
{
   Indigo &self = indigoGetInstance();
   if (strcasecmp(mode, "2000") == 0)
      self.molfile_saving_mode = MolfileSaver::MODE_2000;
   else if (strcasecmp(mode, "3000") == 0)
      self.molfile_saving_mode = MolfileSaver::MODE_3000;
   else if (strcasecmp(mode, "auto") == 0)
      self.molfile_saving_mode = MolfileSaver::MODE_AUTO;
   else
      throw IndigoError("unknown value: %s", mode);
}

static void indigoSetMolfileSavingNoChiral (int enabled)
{
   Indigo &self = indigoGetInstance();
   self.molfile_saving_no_chiral = (enabled != 0);
}

static void indigoSetMolfileSavingSkipDate (int enabled)
{
   Indigo &self = indigoGetInstance();
   self.molfile_saving_skip_date = (enabled != 0);
}

static void indigoSetSmilesSavingWriteName (int enabled)
{
   Indigo &self = indigoGetInstance();
   self.smiles_saving_write_name = (enabled != 0);
}

static void indigoSetFilenameEncoding (const char *encoding)
{
   Indigo &self = indigoGetInstance();
   if (strcasecmp(encoding, "ASCII") == 0)
      self.filename_encoding = ENCODING_ASCII;
   else if (strcasecmp(encoding, "UTF-8") == 0)
      self.filename_encoding = ENCODING_UTF8;
   else
      throw IndigoError("unknown value: %s", encoding);
}

static void indigoSetFPOrdQwords (int qwords)
{
   Indigo &self = indigoGetInstance();
   self.fp_params.ord_qwords = qwords;
}

static void indigoSetFPSimQwords (int qwords)
{
   Indigo &self = indigoGetInstance();
   self.fp_params.sim_qwords = qwords;
}

static void indigoSetFPTauQwords (int qwords)
{
   Indigo &self = indigoGetInstance();
   self.fp_params.tau_qwords = qwords;
}

static void indigoSetFPAnyQwords (int qwords)
{
   Indigo &self = indigoGetInstance();
   self.fp_params.any_qwords = qwords;
}

static void indigoSetFPExt (int enabled)
{
   Indigo &self = indigoGetInstance();
   self.fp_params.ext = (enabled != 0);
}

static void indigoSetEmbeddingUniqueness (const char *mode)
{
   Indigo &self = indigoGetInstance();
   if (strcasecmp(mode, "atoms") == 0)
   {
      self.embedding_edges_uniqueness = false;
      self.find_unique_embeddings = true;
   }
   else if (strcasecmp(mode, "bonds") == 0)
   {
      self.embedding_edges_uniqueness = true;
      self.find_unique_embeddings = true;
   }
   else if (strcasecmp(mode, "none") == 0)
   {
      self.find_unique_embeddings = false;
   }
   else
      throw IndigoError("unknown value: %s", mode);
}

static void indigoSetMaxEmbeddings (int value)
{
   Indigo &self = indigoGetInstance();
   if (value <= 0)
      throw IndigoError("Maximum allowed embeddings limit must be positive.");
   self.max_embeddings = value;
}

static void indigoSetLayoutMaxIterations (int value)
{
   Indigo &self = indigoGetInstance();
   self.layout_max_iterations = value;
}

static void indigoAAMSetCancellationTimeout (int value)
{
   Indigo &self = indigoGetInstance();
   self.aam_cancellation_timeout = value;
}

static void indigoSetCancellationTimeout (int value)
{
   Indigo &self = indigoGetInstance();
   self.cancellation_timeout = value;
}

static void indigoSetPreserveOrderingInSerialize (int enabled)
{
   Indigo &self = indigoGetInstance();
   self.preserve_ordering_in_serialize = (enabled != 0);
}

static void indigoSetAromaticityModel (const char *model)
{
   Indigo &self = indigoGetInstance();
   if (strcasecmp(model, "basic") == 0)
      self.arom_options.method = AromaticityOptions::BASIC;
   else if (strcasecmp(model, "generic") == 0)
      self.arom_options.method = AromaticityOptions::GENERIC;
   else
      throw IndigoError("unknown value: %s. Allowed values are \"basic\", \"generic\"", model);
}

static void indigoSetDearomatizeVerification (int enabled)
{
   Indigo &self = indigoGetInstance();
   self.arom_options.dearomatize_check = (enabled != 0);
}

static void indigoSetDearomatizeUnique (int enabled)
{
   Indigo &self = indigoGetInstance();
   self.unique_dearomatization = (enabled != 0);
}

_IndigoBasicOptionsHandlersSetter::_IndigoBasicOptionsHandlersSetter ()
{
   OptionManager &mgr = indigoGetOptionManager();
   OsLocker locker(mgr.lock);

   mgr.setOptionHandlerBool("ignore-stereochemistry-errors", indigoIgnoreStereochemistryErrors);
   mgr.setOptionHandlerBool("ignore-noncritical-query-features", indigoIgnoreNoncricicalQueryFeatures);
   mgr.setOptionHandlerBool("treat-x-as-pseudoatom", indigoTreatXAsPseudoatom);
   mgr.setOptionHandlerBool("skip-3d-chirality", indigoSkip3dChirality);
   mgr.setOptionHandlerBool("deconvolution-aromatization", indigoDeconvolutionAromatization);
   mgr.setOptionHandlerBool("deco-save-ap-bond-orders", indigoDecoSaveAPBondOrders);
   mgr.setOptionHandlerBool("deco-ignore-errors", indigoDecoIgnoreErrors);
   mgr.setOptionHandlerString("molfile-saving-mode", indigoSetMolfileSavingMode);
   mgr.setOptionHandlerBool("molfile-saving-no-chiral", indigoSetMolfileSavingNoChiral);
   mgr.setOptionHandlerBool("molfile-saving-skip-date", indigoSetMolfileSavingSkipDate);
   mgr.setOptionHandlerBool("smiles-saving-write-name", indigoSetSmilesSavingWriteName);
   mgr.setOptionHandlerString("filename-encoding", indigoSetFilenameEncoding);
   mgr.setOptionHandlerInt("fp-ord-qwords", indigoSetFPOrdQwords);
   mgr.setOptionHandlerInt("fp-sim-qwords", indigoSetFPSimQwords);
   mgr.setOptionHandlerInt("fp-any-qwords", indigoSetFPAnyQwords);
   mgr.setOptionHandlerInt("fp-tau-qwords", indigoSetFPTauQwords);
   mgr.setOptionHandlerBool("fp-ext-enabled", indigoSetFPExt);

   mgr.setOptionHandlerString("embedding-uniqueness", indigoSetEmbeddingUniqueness);
   mgr.setOptionHandlerInt("max-embeddings", indigoSetMaxEmbeddings);

   mgr.setOptionHandlerInt("layout-max-iterations", indigoSetLayoutMaxIterations);

   mgr.setOptionHandlerInt("aam-timeout", indigoAAMSetCancellationTimeout);
   mgr.setOptionHandlerInt("timeout", indigoSetCancellationTimeout);

   mgr.setOptionHandlerBool("serialize-preserve-ordering", indigoSetPreserveOrderingInSerialize);

   mgr.setOptionHandlerString("aromaticity-model", indigoSetAromaticityModel);
   mgr.setOptionHandlerBool("dearomatize-verification", indigoSetDearomatizeVerification);
   mgr.setOptionHandlerBool("unique-dearomatization", indigoSetDearomatizeUnique);
}

_IndigoBasicOptionsHandlersSetter::~_IndigoBasicOptionsHandlersSetter ()
{
}
