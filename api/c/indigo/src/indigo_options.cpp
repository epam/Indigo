/****************************************************************************
 * Copyright (C) from 2009 to Present EPAM Systems.
 *
 * This file is part of Indigo toolkit.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ***************************************************************************/

#include "indigo_internal.h"
#include "molecule/molfile_saver.h"
#include "molecule/smiles_saver.h"

static void setStrValue(const char* source, char* dest, int len)
{
    if (strlen(source) > len)
        throw IndigoError("invalid string value len: expected len: %d, actual len: %d", len, strlen(source));
    strcpy(dest, source);
}

static void indigoSetMolfileSavingMode(const char* mode)
{
    Indigo& self = indigoGetInstance();
    self.molfile_saving_mode = MolfileSaver::parseFormatMode(mode);
}

static void indigoGetMolfileSavingMode(Array<char>& value)
{
    Indigo& self = indigoGetInstance();
    MolfileSaver::saveFormatMode(self.molfile_saving_mode, value);
}

static void indigoSetSmilesSavingFormat(const char* mode)
{
    Indigo& self = indigoGetInstance();
    self.smiles_saving_format = SmilesSaver::parseFormatMode(mode);
}

static void indigoGetSmilesSavingFormat(Array<char>& value)
{
    Indigo& self = indigoGetInstance();
    std::string str_val;
    SmilesSaver::saveFormatMode(self.smiles_saving_format, str_val);
    value.readString(str_val.c_str(), true);
}

static void indigoSetFilenameEncoding(const char* encoding)
{
    Indigo& self = indigoGetInstance();
    if (strcasecmp(encoding, "ASCII") == 0)
        self.filename_encoding = ENCODING_ASCII;
    else if (strcasecmp(encoding, "UTF-8") == 0)
        self.filename_encoding = ENCODING_UTF8;
    else
        throw IndigoError("unknown value: %s", encoding);
}

static void indigoGetFilenameEncoding(Array<char>& value)
{
    Indigo& self = indigoGetInstance();
    if (self.filename_encoding == ENCODING_ASCII)
        value.readString("ASCII", true);
    else
        value.readString("UTF-8", true);
}

static void indigoSetLayoutOrientation(const char* orientation)
{
    Indigo& self = indigoGetInstance();
    if (strcasecmp(orientation, "unspecified") == 0)
        self.layout_orientation = 0;
    else if (strcasecmp(orientation, "horizontal") == 0)
        self.layout_orientation = 1;
    else if (strcasecmp(orientation, "vertical") == 0)
        self.layout_orientation = 2;
    else
        throw IndigoError("unknown value: %s", orientation);
}

static void indigoGetLayoutOrientation(Array<char>& value)
{
    Indigo& self = indigoGetInstance();
    switch (self.layout_orientation)
    {
    case 0:
        value.readString("unspecified", true);
        break;
    case 1:
        value.readString("horizontal", true);
        break;
    case 2:
        value.readString("vertical", true);
        break;
    }
}

static void indigoSetEmbeddingUniqueness(const char* mode)
{
    Indigo& self = indigoGetInstance();
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

static void indigoGetEmbeddingUniqueness(Array<char>& value)
{
    Indigo& self = indigoGetInstance();
    if (self.find_unique_embeddings == false)
        value.readString("none", true);
    else if (self.embedding_edges_uniqueness == false)
        value.readString("atoms", true);
    else
        value.readString("bonds", true);
}

static void indigoSetLayoutHorIntervalFactor(float value)
{
    Indigo& self = indigoGetInstance();
    self.layout_horintervalfactor = value;
}

static void indigoGetLayoutHorIntervalFactor(float& value)
{
    Indigo& self = indigoGetInstance();
    value = self.layout_horintervalfactor;
}

static void indigoSetAromaticityModel(const char* model)
{
    Indigo& self = indigoGetInstance();
    if (strcasecmp(model, "basic") == 0)
        self.arom_options.method = AromaticityOptions::BASIC;
    else if (strcasecmp(model, "generic") == 0)
        self.arom_options.method = AromaticityOptions::GENERIC;
    else
        throw IndigoError("unknown value: %s. Allowed values are \"basic\", \"generic\"", model);
}

static void indigoGetAromaticityModel(Array<char>& value)
{
    Indigo& self = indigoGetInstance();
    if (self.arom_options.method == AromaticityOptions::BASIC)
        value.readString("basic", true);
    else
        value.readString("generic", true);
}

static void indigoSetPkaModel(const char* model)
{
    Indigo& self = indigoGetInstance();
    if (strcasecmp(model, "simple") == 0)
        self.ionize_options.model = IonizeOptions::PKA_MODEL_SIMPLE;
    else if (strcasecmp(model, "advanced") == 0)
        self.ionize_options.model = IonizeOptions::PKA_MODEL_ADVANCED;
    else
        throw IndigoError("unknown value: %s. Allowed values are \"simple\", \"advanced\"", model);
}

static void indigoGetPkaModel(Array<char>& value)
{
    Indigo& self = indigoGetInstance();
    if (self.ionize_options.model == IonizeOptions::PKA_MODEL_SIMPLE)
        value.readString("simple", true);
    else
        value.readString("advanced", true);
}

static void indigoSetMaxEmbeddings(int value)
{
    Indigo& self = indigoGetInstance();
    if (value <= 0)
        throw IndigoError("Maximum allowed embeddings limit must be positive.");
    self.max_embeddings = value;
}

static void indigoGetMaxEmbeddings(int& value)
{
    Indigo& self = indigoGetInstance();
    value = self.max_embeddings;
}

static void indigoSetStereoOption(const char* option)
{
    Indigo& self = indigoGetInstance();
    if (strcasecmp(option, "abs") == 0)
        self.treat_stereo_as = MoleculeStereocenters::ATOM_ABS;
    else if (strcasecmp(option, "rel") == 0)
        self.treat_stereo_as = MoleculeStereocenters::ATOM_OR;
    else if (strcasecmp(option, "rac") == 0)
        self.treat_stereo_as = MoleculeStereocenters::ATOM_AND;
    else if (strcasecmp(option, "any") == 0)
        self.treat_stereo_as = MoleculeStereocenters::ATOM_ANY;
    else if (strcasecmp(option, "ucf") == 0)
        self.treat_stereo_as = 0;
    else
        throw IndigoError("unknown value: %s. Allowed values are \"abs\", \"rel\", \"rac\", \"any\", \"ucf\"", option);
}

static void indigoGetStereoOption(Array<char>& option)
{
    Indigo& self = indigoGetInstance();
    if (self.treat_stereo_as == 0)
        option.readString("ucf", true);
    else if (self.treat_stereo_as == MoleculeStereocenters::ATOM_ABS)
        option.readString("abs", true);
    else if (self.treat_stereo_as == MoleculeStereocenters::ATOM_OR)
        option.readString("rel", true);
    else if (self.treat_stereo_as == MoleculeStereocenters::ATOM_AND)
        option.readString("rac", true);
    else if (self.treat_stereo_as == MoleculeStereocenters::ATOM_ANY)
        option.readString("any", true);
}

static void indigoResetBasicOptions()
{
    Indigo& self = indigoGetInstance();
    self.standardize_options.reset();
    self.ionize_options = IonizeOptions();
    self.init();
}

void indigoProductEnumeratorSetOneTubeMode(const char* mode_string)
{
    Indigo& self = indigoGetInstance();
    if (strcmp(mode_string, "one-tube") == 0)
        self.rpe_params.is_one_tube = true;
    else if (strcmp(mode_string, "grid") == 0)
        self.rpe_params.is_one_tube = false;
    else
        throw IndigoError("%s is bad reaction product enumerator mode string", mode_string);
}

void indigoProductEnumeratorGetOneTubeMode(Array<char>& value)
{
    Indigo& self = indigoGetInstance();
    if (self.rpe_params.is_one_tube)
        value.readString("one-tube", true);
    else
        value.readString("grid", true);
}

void IndigoOptionHandlerSetter::setBasicOptionHandlers(const qword id)
{
    auto mgr = sf::xlock_safe_ptr(indigoGetOptionManager(id));

#define indigo indigoGetInstance()

    mgr->setOptionHandlerBool("ignore-stereochemistry-errors", SETTER_GETTER_BOOL_OPTION(indigo.stereochemistry_options.ignore_errors));
    mgr->setOptionHandlerBool("ignore-noncritical-query-features", SETTER_GETTER_BOOL_OPTION(indigo.ignore_noncritical_query_features));
    mgr->setOptionHandlerBool("ignore-no-chiral-flag", SETTER_GETTER_BOOL_OPTION(indigo.ignore_no_chiral_flag));
    mgr->setOptionHandlerString("treat-stereo-as", indigoSetStereoOption, indigoGetStereoOption);
    mgr->setOptionHandlerBool("ignore-closing-bond-direction-mismatch", SETTER_GETTER_BOOL_OPTION(indigo.ignore_closing_bond_direction_mismatch));
    mgr->setOptionHandlerBool("ignore-bad-valence", SETTER_GETTER_BOOL_OPTION(indigo.ignore_bad_valence));
    mgr->setOptionHandlerBool("treat-x-as-pseudoatom", SETTER_GETTER_BOOL_OPTION(indigo.treat_x_as_pseudoatom));
    mgr->setOptionHandlerBool("dearomatize-on-load", SETTER_GETTER_BOOL_OPTION(indigo.dearomatize_on_load));
    mgr->setOptionHandlerBool("aromatize-skip-superatoms", SETTER_GETTER_BOOL_OPTION(indigo.aromatize_skip_superatoms));
    mgr->setOptionHandlerBool("skip-3d-chirality", SETTER_GETTER_BOOL_OPTION(indigo.skip_3d_chirality));
    mgr->setOptionHandlerBool("deconvolution-aromatization", SETTER_GETTER_BOOL_OPTION(indigo.deconvolution_aromatization));
    mgr->setOptionHandlerBool("deco-save-ap-bond-orders", SETTER_GETTER_BOOL_OPTION(indigo.deco_save_ap_bond_orders));
    mgr->setOptionHandlerBool("deco-ignore-errors", SETTER_GETTER_BOOL_OPTION(indigo.deco_ignore_errors));
    mgr->setOptionHandlerString("molfile-saving-mode", indigoSetMolfileSavingMode, indigoGetMolfileSavingMode);
    mgr->setOptionHandlerString("smiles-saving-format", indigoSetSmilesSavingFormat, indigoGetSmilesSavingFormat);

    mgr->setOptionHandlerInt("molfile-saving-no-chiral", SETTER_GETTER_INT_OPTION(indigo.molfile_saving_no_chiral));
    mgr->setOptionHandlerInt("molfile-saving-chiral-flag", SETTER_GETTER_INT_OPTION(indigo.molfile_saving_chiral_flag));
    mgr->setOptionHandlerBool("molfile-saving-skip-date", SETTER_GETTER_BOOL_OPTION(indigo.molfile_saving_skip_date));
    mgr->setOptionHandlerBool("molfile-saving-add-stereo-desc", SETTER_GETTER_BOOL_OPTION(indigo.molfile_saving_add_stereo_desc));
    mgr->setOptionHandlerBool("json-saving-add-stereo-desc", SETTER_GETTER_BOOL_OPTION(indigo.json_saving_add_stereo_desc));
    mgr->setOptionHandlerBool("json-saving-pretty", SETTER_GETTER_BOOL_OPTION(indigo.json_saving_pretty));
    mgr->setOptionHandlerBool("json-use-native-precision", SETTER_GETTER_BOOL_OPTION(indigo.json_use_native_precision));
    mgr->setOptionHandlerBool("molfile-saving-add-implicit-h", SETTER_GETTER_BOOL_OPTION(indigo.molfile_saving_add_implicit_h));
    mgr->setOptionHandlerBool("molfile-saving-add-mrv-sma", SETTER_GETTER_BOOL_OPTION(indigo.molfile_saving_add_mrv_sma));
    mgr->setOptionHandlerBool("smiles-saving-write-name", SETTER_GETTER_BOOL_OPTION(indigo.smiles_saving_write_name));
    mgr->setOptionHandlerString("filename-encoding", indigoSetFilenameEncoding, indigoGetFilenameEncoding);
    mgr->setOptionHandlerInt("fp-ord-qwords", SETTER_GETTER_INT_OPTION(indigo.fp_params.ord_qwords));
    mgr->setOptionHandlerInt("fp-sim-qwords", SETTER_GETTER_INT_OPTION(indigo.fp_params.sim_qwords));
    mgr->setOptionHandlerInt("fp-any-qwords", SETTER_GETTER_INT_OPTION(indigo.fp_params.any_qwords));
    mgr->setOptionHandlerInt("fp-tau-qwords", SETTER_GETTER_INT_OPTION(indigo.fp_params.tau_qwords));
    mgr->setOptionHandlerBool("fp-ext-enabled", SETTER_GETTER_BOOL_OPTION(indigo.fp_params.ext));
    mgr->setOptionHandlerBool("smart-layout", SETTER_GETTER_BOOL_OPTION(indigo.smart_layout));
    mgr->setOptionHandlerString("layout-orientation", indigoSetLayoutOrientation, indigoGetLayoutOrientation);
    mgr->setOptionHandlerString(
        "similarity-type", [](const char* value) { indigo.fp_params.similarity_type = MoleculeFingerprintBuilder::parseSimilarityType(value); },
        [](Array<char>& value) {
            const char* str = MoleculeFingerprintBuilder::printSimilarityType(indigo.fp_params.similarity_type);
            value.copy(str, strlen(str));
        });

    mgr->setOptionHandlerString("embedding-uniqueness", indigoSetEmbeddingUniqueness, indigoGetEmbeddingUniqueness);
    mgr->setOptionHandlerInt("max-embeddings", indigoSetMaxEmbeddings, indigoGetMaxEmbeddings);

    mgr->setOptionHandlerInt("layout-max-iterations", SETTER_GETTER_INT_OPTION(indigo.layout_max_iterations));
    mgr->setOptionHandlerInt("layout-preserve-existing", SETTER_GETTER_BOOL_OPTION(indigo.layout_preserve_existing));
    mgr->setOptionHandlerFloat("layout-horintervalfactor", indigoSetLayoutHorIntervalFactor, indigoGetLayoutHorIntervalFactor);

    mgr->setOptionHandlerInt("aam-timeout", SETTER_GETTER_INT_OPTION(indigo.aam_cancellation_timeout));
    mgr->setOptionHandlerInt("timeout", SETTER_GETTER_INT_OPTION(indigo.cancellation_timeout));

    mgr->setOptionHandlerBool("serialize-preserve-ordering", SETTER_GETTER_BOOL_OPTION(indigo.preserve_ordering_in_serialize));

    mgr->setOptionHandlerString("aromaticity-model", indigoSetAromaticityModel, indigoGetAromaticityModel);
    mgr->setOptionHandlerBool("dearomatize-verification", SETTER_GETTER_BOOL_OPTION(indigo.arom_options.dearomatize_check));
    mgr->setOptionHandlerBool("unique-dearomatization", SETTER_GETTER_BOOL_OPTION(indigo.unique_dearomatization));
    mgr->setOptionHandlerBool("stereochemistry-bidirectional-mode", SETTER_GETTER_BOOL_OPTION(indigo.stereochemistry_options.bidirectional_mode));
    mgr->setOptionHandlerBool("stereochemistry-detect-haworth-projection", SETTER_GETTER_BOOL_OPTION(indigo.stereochemistry_options.detect_haworth_projection));

    mgr->setOptionHandlerBool("standardize-stereo", SETTER_GETTER_BOOL_OPTION(indigo.standardize_options.standardize_stereo));
    mgr->setOptionHandlerBool("standardize-charges", SETTER_GETTER_BOOL_OPTION(indigo.standardize_options.standardize_charges));
    mgr->setOptionHandlerBool("standardize-center-molecule", SETTER_GETTER_BOOL_OPTION(indigo.standardize_options.center_molecule));
    mgr->setOptionHandlerBool("standardize-remove-single-atoms", SETTER_GETTER_BOOL_OPTION(indigo.standardize_options.remove_single_atom_fragments));
    mgr->setOptionHandlerBool("standardize-keep-smallest", SETTER_GETTER_BOOL_OPTION(indigo.standardize_options.keep_smallest_fragment));
    mgr->setOptionHandlerBool("standardize-keep-largest", SETTER_GETTER_BOOL_OPTION(indigo.standardize_options.keep_largest_fragment));
    mgr->setOptionHandlerBool("standardize-remove-largest", SETTER_GETTER_BOOL_OPTION(indigo.standardize_options.remove_largest_fragment));
    mgr->setOptionHandlerBool("standardize-make-non-h-to-c-atoms", SETTER_GETTER_BOOL_OPTION(indigo.standardize_options.make_non_h_atoms_c_atoms));
    mgr->setOptionHandlerBool("standardize-make-non-h-to-a-atoms", SETTER_GETTER_BOOL_OPTION(indigo.standardize_options.make_non_h_atoms_a_atoms));
    mgr->setOptionHandlerBool("standardize-make-non-h-c-to-q-atoms", SETTER_GETTER_BOOL_OPTION(indigo.standardize_options.make_non_c_h_atoms_q_atoms));
    mgr->setOptionHandlerBool("standardize-make-all-bonds-single", SETTER_GETTER_BOOL_OPTION(indigo.standardize_options.make_all_bonds_single));
    mgr->setOptionHandlerBool("standardize-clear-coordinates", SETTER_GETTER_BOOL_OPTION(indigo.standardize_options.clear_coordinates));
    mgr->setOptionHandlerBool("standardize-straighten-triple-bonds", SETTER_GETTER_BOOL_OPTION(indigo.standardize_options.straighten_triple_bonds));
    mgr->setOptionHandlerBool("standardize-straighten-allens", SETTER_GETTER_BOOL_OPTION(indigo.standardize_options.straighten_allenes));
    mgr->setOptionHandlerBool("standardize-clear-molecule", SETTER_GETTER_BOOL_OPTION(indigo.standardize_options.clear_molecule));
    mgr->setOptionHandlerBool("standardize-clear-stereo", SETTER_GETTER_BOOL_OPTION(indigo.standardize_options.clear_stereo));
    mgr->setOptionHandlerBool("standardize-clear-enhanced-stereo", SETTER_GETTER_BOOL_OPTION(indigo.standardize_options.clear_enhanced_stereo));
    mgr->setOptionHandlerBool("standardize-clear-unknown-stereo", SETTER_GETTER_BOOL_OPTION(indigo.standardize_options.clear_unknown_stereo));
    mgr->setOptionHandlerBool("standardize-clear-unknown-atom-stereo", SETTER_GETTER_BOOL_OPTION(indigo.standardize_options.clear_unknown_atom_stereo));
    mgr->setOptionHandlerBool("standardize-clear-unknown-bond-stereo",
                              SETTER_GETTER_BOOL_OPTION(indigo.standardize_options.clear_unknown_cis_trans_bond_stereo));
    mgr->setOptionHandlerBool("standardize-clear-cis-trans", SETTER_GETTER_BOOL_OPTION(indigo.standardize_options.clear_cis_trans_bond_stereo));
    mgr->setOptionHandlerBool("standardize-stereo-from-coordinates", SETTER_GETTER_BOOL_OPTION(indigo.standardize_options.set_stereo_from_coordinates));
    mgr->setOptionHandlerBool("standardize-reposition-stereo-bonds", SETTER_GETTER_BOOL_OPTION(indigo.standardize_options.reposition_stereo_bonds));
    mgr->setOptionHandlerBool("standardize-reposition-axial-stereo-bonds", SETTER_GETTER_BOOL_OPTION(indigo.standardize_options.reposition_axial_stereo_bonds));
    mgr->setOptionHandlerBool("standardize-fix-direction-wedge-bonds", SETTER_GETTER_BOOL_OPTION(indigo.standardize_options.fix_direction_of_wedge_bonds));
    mgr->setOptionHandlerBool("standardize-clear-charges", SETTER_GETTER_BOOL_OPTION(indigo.standardize_options.clear_charges));
    mgr->setOptionHandlerBool("standardize-highlight-colors", SETTER_GETTER_BOOL_OPTION(indigo.standardize_options.clear_highlight_colors));
    mgr->setOptionHandlerBool("standardize-neutralize-zwitterions", SETTER_GETTER_BOOL_OPTION(indigo.standardize_options.neutralize_bonded_zwitterions));
    mgr->setOptionHandlerBool("standardize-clear-unusual-valences", SETTER_GETTER_BOOL_OPTION(indigo.standardize_options.clear_unusual_valence));
    mgr->setOptionHandlerBool("standardize-clear-isotopes", SETTER_GETTER_BOOL_OPTION(indigo.standardize_options.clear_isotopes));
    mgr->setOptionHandlerBool("standardize-clear-dative-bonds", SETTER_GETTER_BOOL_OPTION(indigo.standardize_options.clear_dative_bonds));
    mgr->setOptionHandlerBool("standardize-clear-hydrogen-bonds", SETTER_GETTER_BOOL_OPTION(indigo.standardize_options.clear_hydrogen_bonds));
    mgr->setOptionHandlerBool("standardize-localize-markush-r-atoms-on-rings",
                              SETTER_GETTER_BOOL_OPTION(indigo.standardize_options.localize_markush_r_atoms_on_rings));
    mgr->setOptionHandlerBool("standardize-create-dative-bonds", SETTER_GETTER_BOOL_OPTION(indigo.standardize_options.create_coordination_bonds));
    mgr->setOptionHandlerBool("standardize-create-hydrogen-bonds", SETTER_GETTER_BOOL_OPTION(indigo.standardize_options.create_hydrogen_bonds));
    mgr->setOptionHandlerBool("standardize-remove-extra-stereo-bonds", SETTER_GETTER_BOOL_OPTION(indigo.standardize_options.remove_extra_stereo_bonds));

    mgr->setOptionHandlerString("pKa-model", indigoSetPkaModel, indigoGetPkaModel);
    mgr->setOptionHandlerInt("pKa-model-level", SETTER_GETTER_INT_OPTION(indigo.ionize_options.level));
    mgr->setOptionHandlerInt("pKa-model-min-level", SETTER_GETTER_INT_OPTION(indigo.ionize_options.min_level));

    mgr->setOptionHandlerVoid("reset-basic-options", indigoResetBasicOptions);

    mgr->setOptionHandlerBool("mass-skip-error-on-pseudoatoms", SETTER_GETTER_BOOL_OPTION(indigo.mass_options.skip_error_on_pseudoatoms));
    mgr->setOptionHandlerBool("gross-formula-add-rsites", SETTER_GETTER_BOOL_OPTION(indigo.gross_formula_options.add_rsites));
    mgr->setOptionHandlerBool("gross-formula-add-isotopes", SETTER_GETTER_BOOL_OPTION(indigo.gross_formula_options.add_isotopes));

    mgr->setOptionHandlerBool("scsr-ignore-chem-templates", SETTER_GETTER_BOOL_OPTION(indigo.scsr_ignore_chem_templates));

    mgr->setOptionHandlerBool("rpe-multistep-reactions", SETTER_GETTER_BOOL_OPTION(indigo.rpe_params.is_multistep_reactions));
    mgr->setOptionHandlerString("rpe-mode", indigoProductEnumeratorSetOneTubeMode, indigoProductEnumeratorGetOneTubeMode);
    mgr->setOptionHandlerBool("rpe-self-reaction", SETTER_GETTER_BOOL_OPTION(indigo.rpe_params.is_self_react));
    mgr->setOptionHandlerInt("rpe-max-depth", SETTER_GETTER_INT_OPTION(indigo.rpe_params.max_deep_level));
    mgr->setOptionHandlerInt("rpe-max-products-count", SETTER_GETTER_INT_OPTION(indigo.rpe_params.max_product_count));
    mgr->setOptionHandlerBool("rpe-layout", SETTER_GETTER_BOOL_OPTION(indigo.rpe_params.is_layout));
    mgr->setOptionHandlerBool("transform-layout", SETTER_GETTER_BOOL_OPTION(indigo.rpe_params.transform_is_layout));
}