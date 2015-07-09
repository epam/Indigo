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

#include "indigo_internal.h"
#include "molecule/molfile_saver.h"

static void indigoIgnoreStereochemistryErrors (int enabled)
{
   Indigo &self = indigoGetInstance();
   self.stereochemistry_options.ignore_errors = (enabled != 0);
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

static void indigoSetFPExt(int enabled)
{
   Indigo &self = indigoGetInstance();
   self.fp_params.ext = (enabled != 0);
}

static void indigoSetSmartLayout(int enabled)
{
   Indigo &self = indigoGetInstance();
   self.smart_layout = (enabled != 0);
}

static void indigoSetEmbeddingUniqueness(const char *mode)
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

static void indigoSetBidirectionalMode (int enabled)
{
   Indigo &self = indigoGetInstance();
   self.stereochemistry_options.bidirectional_mode = (enabled != 0);
}

static void indigoSetDetectHaworthProjection (int enabled)
{
   Indigo &self = indigoGetInstance();
   self.stereochemistry_options.detect_haworth_projection = (enabled != 0);
}

static void indigoSetStandardizeStereo (int enabled)
{
   Indigo &self = indigoGetInstance();
   self.standardize_options.standardize_stereo = (enabled != 0);
}

static void indigoSetStandardizeCharges (int enabled)
{
   Indigo &self = indigoGetInstance();
   self.standardize_options.standardize_charges = (enabled != 0);
}

static void indigoSetStandardizeCenterMolecule (int enabled)
{
   Indigo &self = indigoGetInstance();
   self.standardize_options.center_molecule = (enabled != 0);
}

static void indigoSetStandardizeRemoveSingleAtoms (int enabled)
{
   Indigo &self = indigoGetInstance();
   self.standardize_options.remove_single_atom_fragments = (enabled != 0);
}

static void indigoSetStandardizeKeepSmallestFragment (int enabled)
{
   Indigo &self = indigoGetInstance();
   self.standardize_options.keep_smallest_fragment = (enabled != 0);
}

static void indigoSetStandardizeKeepLargestFragment (int enabled)
{
   Indigo &self = indigoGetInstance();
   self.standardize_options.keep_largest_fragment = (enabled != 0);
}

static void indigoSetStandardizeRemoveLargestFragment (int enabled)
{
   Indigo &self = indigoGetInstance();
   self.standardize_options.remove_largest_fragment = (enabled != 0);
}

static void indigoSetStandardizeMakeNonHtoCAtoms (int enabled)
{
   Indigo &self = indigoGetInstance();
   self.standardize_options.make_non_h_atoms_c_atoms = (enabled != 0);
}

static void indigoSetStandardizeMakeNonHtoAAtoms (int enabled)
{
   Indigo &self = indigoGetInstance();
   self.standardize_options.make_non_h_atoms_a_atoms = (enabled != 0);
}

static void indigoSetStandardizeMakeNonHCtoQAtoms (int enabled)
{
   Indigo &self = indigoGetInstance();
   self.standardize_options.make_non_c_h_atoms_q_atoms = (enabled != 0);
}

static void indigoSetStandardizeMakeAllBondsSingle (int enabled)
{
   Indigo &self = indigoGetInstance();
   self.standardize_options.make_all_bonds_single = (enabled != 0);
}

static void indigoSetStandardizeClearCoordinates (int enabled)
{
   Indigo &self = indigoGetInstance();
   self.standardize_options.clear_coordinates = (enabled != 0);
}

static void indigoSetStandardizeStraightenTripleBonds (int enabled)
{
   Indigo &self = indigoGetInstance();
   self.standardize_options.straighten_triple_bonds = (enabled != 0);
}

static void indigoSetStandardizeStraightenAllens (int enabled)
{
   Indigo &self = indigoGetInstance();
   self.standardize_options.straighten_allenes = (enabled != 0);
}

static void indigoSetStandardizeClearMolecule (int enabled)
{
   Indigo &self = indigoGetInstance();
   self.standardize_options.clear_molecule = (enabled != 0);
}

static void indigoSetStandardizeClearStereo (int enabled)
{
   Indigo &self = indigoGetInstance();
   self.standardize_options.clear_stereo = (enabled != 0);
}

static void indigoSetStandardizeClearEnhancedStereo (int enabled)
{
   Indigo &self = indigoGetInstance();
   self.standardize_options.clear_enhanced_stereo = (enabled != 0);
}

static void indigoSetStandardizeClearUnknownStereo (int enabled)
{
   Indigo &self = indigoGetInstance();
   self.standardize_options.clear_unknown_stereo = (enabled != 0);
}

static void indigoSetStandardizeClearUnknownAtomStereo (int enabled)
{
   Indigo &self = indigoGetInstance();
   self.standardize_options.clear_unknown_atom_stereo = (enabled != 0);
}

static void indigoSetStandardizeClearUnknownBondStereo (int enabled)
{
   Indigo &self = indigoGetInstance();
   self.standardize_options.clear_unknown_cis_trans_bond_stereo = (enabled != 0);
}

static void indigoSetStandardizeClearCisTransStereo (int enabled)
{
   Indigo &self = indigoGetInstance();
   self.standardize_options.clear_cis_trans_bond_stereo = (enabled != 0);
}

static void indigoSetStandardizeStereoFromCoordinates (int enabled)
{
   Indigo &self = indigoGetInstance();
   self.standardize_options.set_stereo_from_coordinates = (enabled != 0);
}

static void indigoSetStandardizeRepositonStereoBonds (int enabled)
{
   Indigo &self = indigoGetInstance();
   self.standardize_options.reposition_stereo_bonds = (enabled != 0);
}

static void indigoSetStandardizeRepositonAxialStereoBonds (int enabled)
{
   Indigo &self = indigoGetInstance();
   self.standardize_options.reposition_axial_stereo_bonds = (enabled != 0);
}

static void indigoSetStandardizeFixDirectionWedgeBonds (int enabled)
{
   Indigo &self = indigoGetInstance();
   self.standardize_options.fix_direction_of_wedge_bonds = (enabled != 0);
}

static void indigoSetStandardizeClearCharges (int enabled)
{
   Indigo &self = indigoGetInstance();
   self.standardize_options.clear_charges = (enabled != 0);
}

static void indigoSetStandardizeClearHighlightColors (int enabled)
{
   Indigo &self = indigoGetInstance();
   self.standardize_options.clear_highlight_colors = (enabled != 0);
}

static void indigoSetStandardizeNeutralizeZwitterions (int enabled)
{
   Indigo &self = indigoGetInstance();
   self.standardize_options.neutralize_bonded_zwitterions = (enabled != 0);
}

static void indigoSetStandardizeClearUnususalValences (int enabled)
{
   Indigo &self = indigoGetInstance();
   self.standardize_options.clear_unusual_valence = (enabled != 0);
}

static void indigoSetStandardizeClearIsotopes (int enabled)
{
   Indigo &self = indigoGetInstance();
   self.standardize_options.clear_isotopes = (enabled != 0);
}

static void indigoSetStandardizeClearDativeBonds (int enabled)
{
   Indigo &self = indigoGetInstance();
   self.standardize_options.clear_dative_bonds = (enabled != 0);
}

static void indigoSetStandardizeClearHydrogenBonds (int enabled)
{
   Indigo &self = indigoGetInstance();
   self.standardize_options.clear_hydrogen_bonds = (enabled != 0);
}

static void indigoSetStandardizeLocalizeMarkushRAtomsOnRings (int enabled)
{
   Indigo &self = indigoGetInstance();
   self.standardize_options.localize_markush_r_atoms_on_rings = (enabled != 0);
}

static void indigoSetStandardizeCreateDativeBonds (int enabled)
{
   Indigo &self = indigoGetInstance();
   self.standardize_options.create_coordination_bonds = (enabled != 0);
}

static void indigoSetStandardizeCreateHydrogenBonds (int enabled)
{
   Indigo &self = indigoGetInstance();
   self.standardize_options.create_hydrogen_bonds = (enabled != 0);
}

static void indigoSetPkaModel (const char *model)
{
   Indigo &self = indigoGetInstance();
   if (strcasecmp(model, "simple") == 0)
      self.ionize_options.model = IonizeOptions::PKA_MODEL_SIMPLE;
   else if (strcasecmp(model, "advanced") == 0)
      self.ionize_options.model = IonizeOptions::PKA_MODEL_ADVANCED;
   else
      throw IndigoError("unknown value: %s. Allowed values are \"simple\", \"advanced\"", model);
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
   mgr.setOptionHandlerBool("smart-layout", indigoSetSmartLayout);

   mgr.setOptionHandlerString("embedding-uniqueness", indigoSetEmbeddingUniqueness);
   mgr.setOptionHandlerInt("max-embeddings", indigoSetMaxEmbeddings);

   mgr.setOptionHandlerInt("layout-max-iterations", indigoSetLayoutMaxIterations);

   mgr.setOptionHandlerInt("aam-timeout", indigoAAMSetCancellationTimeout);
   mgr.setOptionHandlerInt("timeout", indigoSetCancellationTimeout);

   mgr.setOptionHandlerBool("serialize-preserve-ordering", indigoSetPreserveOrderingInSerialize);

   mgr.setOptionHandlerString("aromaticity-model", indigoSetAromaticityModel);
   mgr.setOptionHandlerBool("dearomatize-verification", indigoSetDearomatizeVerification);
   mgr.setOptionHandlerBool("unique-dearomatization", indigoSetDearomatizeUnique);
   mgr.setOptionHandlerBool("stereochemistry-bidirectional-mode", indigoSetBidirectionalMode);
   mgr.setOptionHandlerBool("stereochemistry-detect-haworth-projection", indigoSetDetectHaworthProjection);

   mgr.setOptionHandlerBool("standardize-stereo", indigoSetStandardizeStereo);
   mgr.setOptionHandlerBool("standardize-charges", indigoSetStandardizeCharges);
   mgr.setOptionHandlerBool("standardize-center-molecule", indigoSetStandardizeCenterMolecule);
   mgr.setOptionHandlerBool("standardize-remove-single-atoms", indigoSetStandardizeRemoveSingleAtoms);
   mgr.setOptionHandlerBool("standardize-keep-smallest", indigoSetStandardizeKeepSmallestFragment);
   mgr.setOptionHandlerBool("standardize-keep-largest", indigoSetStandardizeKeepLargestFragment);
   mgr.setOptionHandlerBool("standardize-remove-largest", indigoSetStandardizeRemoveLargestFragment);
   mgr.setOptionHandlerBool("standardize-make-non-h-to-c-atoms", indigoSetStandardizeMakeNonHtoCAtoms);
   mgr.setOptionHandlerBool("standardize-make-non-h-to-a-atoms", indigoSetStandardizeMakeNonHtoAAtoms);
   mgr.setOptionHandlerBool("standardize-make-non-h-c-to-q-atoms", indigoSetStandardizeMakeNonHCtoQAtoms);
   mgr.setOptionHandlerBool("standardize-make-all-bonds-single", indigoSetStandardizeMakeAllBondsSingle);
   mgr.setOptionHandlerBool("standardize-clear-coordinates", indigoSetStandardizeClearCoordinates);
   mgr.setOptionHandlerBool("standardize-straighten-triple-bonds", indigoSetStandardizeStraightenTripleBonds);
   mgr.setOptionHandlerBool("standardize-straighten-allens", indigoSetStandardizeStraightenAllens);
   mgr.setOptionHandlerBool("standardize-clear-molecule", indigoSetStandardizeClearMolecule);
   mgr.setOptionHandlerBool("standardize-clear-stereo", indigoSetStandardizeClearStereo);
   mgr.setOptionHandlerBool("standardize-clear-enhanced-stereo", indigoSetStandardizeClearEnhancedStereo);
   mgr.setOptionHandlerBool("standardize-clear-unknown-stereo", indigoSetStandardizeClearUnknownStereo);
   mgr.setOptionHandlerBool("standardize-clear-unknown-atom-stereo", indigoSetStandardizeClearUnknownAtomStereo);
   mgr.setOptionHandlerBool("standardize-clear-unknown-bond-stereo", indigoSetStandardizeClearUnknownBondStereo);
   mgr.setOptionHandlerBool("standardize-clear-cis-trans", indigoSetStandardizeClearCisTransStereo);
   mgr.setOptionHandlerBool("standardize-stereo-from-coordinates", indigoSetStandardizeStereoFromCoordinates);
   mgr.setOptionHandlerBool("standardize-reposition-stereo-bonds", indigoSetStandardizeRepositonStereoBonds);
   mgr.setOptionHandlerBool("standardize-reposition-axial-stereo-bonds", indigoSetStandardizeRepositonAxialStereoBonds);
   mgr.setOptionHandlerBool("standardize-fix-direction-wedge-bonds", indigoSetStandardizeFixDirectionWedgeBonds);
   mgr.setOptionHandlerBool("standardize-clear-charges", indigoSetStandardizeClearCharges);
   mgr.setOptionHandlerBool("standardize-highlight-colors", indigoSetStandardizeClearHighlightColors);
   mgr.setOptionHandlerBool("standardize-neutralize-zwitterions", indigoSetStandardizeNeutralizeZwitterions);
   mgr.setOptionHandlerBool("standardize-clear-unusual-valences", indigoSetStandardizeClearUnususalValences);
   mgr.setOptionHandlerBool("standardize-clear-isotopes", indigoSetStandardizeClearIsotopes);
   mgr.setOptionHandlerBool("standardize-clear-dative-bonds", indigoSetStandardizeClearDativeBonds);
   mgr.setOptionHandlerBool("standardize-clear-hydrogen-bonds", indigoSetStandardizeClearHydrogenBonds);
   mgr.setOptionHandlerBool("standardize-localize-markush-r-atoms-on-rings", indigoSetStandardizeLocalizeMarkushRAtomsOnRings);
   mgr.setOptionHandlerBool("standardize-create-dative-bonds", indigoSetStandardizeCreateDativeBonds);
   mgr.setOptionHandlerBool("standardize-create-hydrogen-bonds", indigoSetStandardizeCreateHydrogenBonds);

   mgr.setOptionHandlerString("pKa-model", indigoSetPkaModel);
}

_IndigoBasicOptionsHandlersSetter::~_IndigoBasicOptionsHandlersSetter ()
{
}
