/****************************************************************************
 * Copyright (C) 2010 GGA Software Services LLC
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

static void indigoIgnoreStereochemistryErrors (int enabled)
{
   Indigo &self = indigoGetInstance();
   self.ignore_stereochemistry_errors = (enabled != 0);
}

static void indigoTreatXAsPseudoatom (int enabled)
{
   Indigo &self = indigoGetInstance();
   self.treat_x_as_pseudoatom = (enabled != 0);
}

static void indigoDeconvolutionAromatization (int enabled)
{
   Indigo &self = indigoGetInstance();
   self.deconvolution_aromatization = (enabled != 0);
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

static void indigoSetEmbeddingUniqueness (const char *mode)
{
   Indigo &self = indigoGetInstance();
   if (strcasecmp(mode, "atoms") == 0)
      self.embedding_edges_uniqueness = false;
   else if (strcasecmp(mode, "bonds") == 0)
      self.embedding_edges_uniqueness = true;
   else
      throw IndigoError("unknown value: %s", mode);
}

static void indigoSetMaxEmbeddings (int value)
{
   Indigo &self = indigoGetInstance();
   self.max_embeddings = value;
}

_IndigoBasicOptionsHandlersSetter::_IndigoBasicOptionsHandlersSetter ()
{
   OptionManager &mgr = indigoGetOptionManager();
   OsLocker locker(mgr.lock);

   mgr.setOptionHandlerBool("ignore-stereochemistry-errors", indigoIgnoreStereochemistryErrors);
   mgr.setOptionHandlerBool("treat-x-as-pseudoatom", indigoTreatXAsPseudoatom);
   mgr.setOptionHandlerBool("deconvolution-aromatization", indigoDeconvolutionAromatization);
   mgr.setOptionHandlerString("molfile-saving-mode", indigoSetMolfileSavingMode);
   mgr.setOptionHandlerString("filename-encoding", indigoSetFilenameEncoding);
   mgr.setOptionHandlerInt("fp-ord-qwords", indigoSetFPOrdQwords);
   mgr.setOptionHandlerInt("fp-sim-qwords", indigoSetFPSimQwords);
   mgr.setOptionHandlerInt("fp-any-qwords", indigoSetFPAnyQwords);
   mgr.setOptionHandlerInt("fp-tau-qwords", indigoSetFPTauQwords);

   mgr.setOptionHandlerString("embedding-uniqueness", indigoSetEmbeddingUniqueness);
   mgr.setOptionHandlerInt("max-embeddings", indigoSetMaxEmbeddings);
}
