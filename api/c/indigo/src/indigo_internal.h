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

#ifndef __indigo_internal__
#define __indigo_internal__

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

#include <memory>
#include <utility>

#include "indigo.h"

#include "base_cpp/cancellation_handler.h"
#include "base_cpp/exception.h"
#include "base_cpp/io_base.h"

#include "molecule/molecule_fingerprint.h"
#include "molecule/molecule_gross_formula.h"
#include "molecule/molecule_ionize.h"
#include "molecule/molecule_mass_options.h"
#include "molecule/molecule_standardize_options.h"
#include "molecule/molecule_stereocenter_options.h"
#include "molecule/molecule_tautomer.h"
#include "option_manager.h"

/* When Indigo internal code is used dynamically the INDIGO_VERSION define
 * should be compared with indigoVersion() to ensure libraries binary
 * compatibility. */

using namespace indigo;

namespace indigo
{
    class BaseReaction;
    class QueryReaction;
    class Reaction;
    class Output;
    class Scanner;
    class SdfLoader;
    class RdfLoader;
    class MolfileSaver;
    class RxnfileSaver;
    class PropertiesMap;
    class MoleculeJsonSaver;
    class ReactionJsonSaver;

    typedef ObjArray<PropertiesMap> MonomersProperties;
} // namespace indigo

extern DLLEXPORT sf::safe_shared_hide_obj<IndigoOptionManager>& indigoGetOptionManager(const qword id = TL_GET_SESSION_ID());

class DLLEXPORT IndigoObject
{
public:
    explicit IndigoObject(int type_);
    virtual ~IndigoObject();

    enum : int
    {
        SCANNER = 1,
        MOLECULE,
        QUERY_MOLECULE,
        REACTION,
        QUERY_REACTION,
        OUTPUT,
        REACTION_ITER,
        REACTION_MOLECULE,
        GROSS_MOLECULE,
        SDF_LOADER,
        SDF_SAVER,
        RDF_MOLECULE,
        RDF_REACTION,
        RDF_LOADER,
        SMILES_MOLECULE,
        SMILES_REACTION,
        MULTILINE_SMILES_LOADER,
        ATOM,
        ATOMS_ITER,
        RGROUP,
        RGROUPS_ITER,
        RGROUP_FRAGMENT,
        RGROUP_FRAGMENTS_ITER,
        ARRAY,
        ARRAY_ITER,
        ARRAY_ELEMENT,
        MOLECULE_SUBSTRUCTURE_MATCH_ITER,
        MOLECULE_SUBSTRUCTURE_MATCHER,
        REACTION_SUBSTRUCTURE_MATCHER,
        SCAFFOLD,
        DECONVOLUTION,
        DECONVOLUTION_ELEM,
        DECONVOLUTION_ITER,
        COMPOSITION_ELEM,
        COMPOSITION_ITER,
        PROPERTIES_ITER,
        PROPERTY,
        FINGERPRINT,
        BOND,
        BONDS_ITER,
        ATOM_NEIGHBOR,
        ATOM_NEIGHBORS_ITER,
        SUPERATOM,
        SUPERATOMS_ITER,
        DATA_SGROUP,
        DATA_SGROUPS_ITER,
        REPEATING_UNIT,
        REPEATING_UNITS_ITER,
        MULTIPLE_GROUP,
        MULTIPLE_GROUPS_ITER,
        GENERIC_SGROUP,
        GENERIC_SGROUPS_ITER,
        SGROUP_ATOMS_ITER,
        SGROUP_BONDS_ITER,
        DECOMPOSITION,
        COMPONENT,
        COMPONENTS_ITER,
        COMPONENT_ATOMS_ITER,
        COMPONENT_BONDS_ITER,
        SUBMOLECULE,
        SUBMOLECULE_ATOMS_ITER,
        SUBMOLECULE_BONDS_ITER,
        MAPPING,
        REACTION_MAPPING,
        SSSR_ITER,
        SUBTREES_ITER,
        RINGS_ITER,
        EDGE_SUBMOLECULE_ITER,
        CML_MOLECULE,
        CML_REACTION,
        MULTIPLE_CML_LOADER,
        SAVER,
        ATTACHMENT_POINTS_ITER,
        DECOMPOSITION_MATCH,
        DECOMPOSITION_MATCH_ITER,
        CDX_MOLECULE,
        CDX_REACTION,
        MULTIPLE_CDX_LOADER,
        CDX_SAVER,
        SGROUP,
        SGROUPS_ITER,
        TAUTOMER_ITER,
        TAUTOMER_MOLECULE,
        TGROUP,
        TGROUPS_ITER,
        GROSS_REACTION,
        JSON_MOLECULE,
        JSON_REACTION,
        INDIGO_OBJECT_LAST_TYPE // must be the last element in the enum
    };

    int type;
    virtual const char* getTypeName() const;

    virtual const char* debugInfo() const;

    virtual void toString(Array<char>& str);
    virtual void toBuffer(Array<char>& buf);
    virtual BaseMolecule& getBaseMolecule();
    virtual QueryMolecule& getQueryMolecule();
    virtual Molecule& getMolecule();
    virtual const Molecule& getMolecule() const;

    virtual BaseReaction& getBaseReaction();
    virtual QueryReaction& getQueryReaction();
    virtual Reaction& getReaction();

    virtual IndigoObject* clone();

    virtual const char* getName();

    virtual int getIndex();

    virtual IndigoObject* next();

    virtual bool hasNext();

    virtual void remove();

    virtual PropertiesMap& getProperties();
    virtual MonomersProperties& getMonomersProperties();
    virtual void copyProperties(PropertiesMap&);

private:
    IndigoObject(const IndigoObject&);
};

class IndigoMoleculeGross : public IndigoObject
{
public:
    IndigoMoleculeGross();
    ~IndigoMoleculeGross() override;

    void toString(Array<char>& str) override;

    std::unique_ptr<GROSS_UNITS> gross;
};

class IndigoReactionGross : public IndigoObject
{
public:
    IndigoReactionGross();
    ~IndigoReactionGross() override;

    void toString(Array<char>& str) override;

    std::unique_ptr<std::pair<PtrArray<GROSS_UNITS>, PtrArray<GROSS_UNITS>>> gross;
};

struct DLLEXPORT ProductEnumeratorParams
{
    ProductEnumeratorParams()
    {
        clear();
    }

    void clear()
    {
        is_multistep_reactions = false;
        is_one_tube = false;
        is_self_react = false;
        is_layout = true;
        transform_is_layout = true;
        max_deep_level = 2;
        max_product_count = 1000;
    }

    bool is_multistep_reactions;
    bool is_one_tube;
    bool is_self_react;
    bool is_layout;
    bool transform_is_layout;
    int max_deep_level;
    int max_product_count;
};

class DLLEXPORT Indigo
{
public:
    Indigo();
    ~Indigo();

    IndigoObject& getObject(int handle);
    int countObjects() const;

    int addObject(IndigoObject* obj);
    int addObject(std::unique_ptr<IndigoObject>&& obj);

    void removeObject(int id);

    void removeAllObjects();

    void init();

    int getId() const;

    struct TmpData
    {
        Array<char> string;
        float xyz[3];

        void clear();
    };

    // Method that returns temporary buffer that can be returned from Indigo C API methods
    static TmpData& getThreadTmpData();

    ProductEnumeratorParams rpe_params;
    MoleculeFingerprintParameters fp_params;
    PtrArray<TautomerRule> tautomer_rules;

    StereocentersOptions stereochemistry_options;
    MassOptions mass_options;
    GrossFormulaOptions gross_formula_options;

    bool ignore_noncritical_query_features;
    bool treat_x_as_pseudoatom;
    bool aromatize_skip_superatoms;
    bool skip_3d_chirality;
    bool ignore_no_chiral_flag;
    int treat_stereo_as;

    bool ignore_closing_bond_direction_mismatch;
    bool ignore_bad_valence;

    bool deconvolution_aromatization;
    bool deco_save_ap_bond_orders;
    bool deco_ignore_errors;

    int molfile_saving_mode; // MolfileSaver::MODE_***, default is zero
    int smiles_saving_format;
    bool molfile_saving_no_chiral;
    int molfile_saving_chiral_flag;
    bool molfile_saving_skip_date;
    bool molfile_saving_add_stereo_desc;
    bool molfile_saving_add_implicit_h;
    bool json_saving_add_stereo_desc;
    bool smiles_saving_write_name;
    bool smiles_saving_smarts_mode;

    Encoding filename_encoding;

    bool embedding_edges_uniqueness, find_unique_embeddings;
    int max_embeddings;

    int layout_max_iterations; // default is zero -- no limit
    bool smart_layout = false;
    float layout_horintervalfactor = 1.4f;

    int layout_orientation = 0;

    int aam_cancellation_timeout; // default is zero - no timeout

    int cancellation_timeout; // default is 0 seconds - no timeout

    void updateCancellationHandler();

    void initMolfileSaver(MolfileSaver& saver);
    void initRxnfileSaver(RxnfileSaver& saver);
    void initMoleculeJsonSaver(MoleculeJsonSaver& saver);
    void initReactionJsonSaver(ReactionJsonSaver& saver);

    bool preserve_ordering_in_serialize;

    AromaticityOptions arom_options;
    // This option is moved out of arom_options because it should be used only in indigoDearomatize method
    bool unique_dearomatization;

    StandardizeOptions standardize_options;

    IonizeOptions ionize_options;

    bool scsr_ignore_chem_templates;

    static const Array<char>& getErrorMessage();
    static void clearErrorMessage();
    static void setErrorMessage(const char* message);
    static void handleError(const char* message);
    static void setErrorHandler(INDIGO_ERROR_HANDLER handler, void* context);

private:
    static Array<char>& error_message();
    static INDIGO_ERROR_HANDLER& error_handler();
    static void*& error_handler_context();

    struct ObjectsHolder
    {
        std::unordered_map<int, std::unique_ptr<IndigoObject>> objects;
        int next_id = 1;
    };
    sf::safe_shared_hide_obj<ObjectsHolder> _objects_holder;

    int _indigo_id;
};

class DLLEXPORT IndigoPluginContext
{
public:
    IndigoPluginContext();

    void validate();

protected:
    virtual void init() = 0;
    int indigo_id;
};

// Used when we don't need Indigo session, just handle errors
#define INDIGO_BEGIN_STATIC                                                                                                                                    \
    {                                                                                                                                                          \
        try                                                                                                                                                    \
        {                                                                                                                                                      \
            Indigo::clearErrorMessage();

#define INDIGO_BEGIN                                                                                                                                           \
    INDIGO_BEGIN_STATIC                                                                                                                                        \
    Indigo& self = indigoGetInstance();                                                                                                                        \
    self.updateCancellationHandler();

#define INDIGO_END(fail)                                                                                                                                       \
    }                                                                                                                                                          \
    catch (Exception & ex)                                                                                                                                     \
    {                                                                                                                                                          \
        Indigo::handleError(ex.message());                                                                                                                     \
        return fail;                                                                                                                                           \
    }                                                                                                                                                          \
    catch (const std::exception& ex)                                                                                                                           \
    {                                                                                                                                                          \
        Indigo::handleError(ex.what());                                                                                                                        \
        return fail;                                                                                                                                           \
    }                                                                                                                                                          \
    return fail;                                                                                                                                               \
    }

DLLEXPORT Indigo& indigoGetInstance();

class DLLEXPORT IndigoError : public Exception
{
public:
    explicit IndigoError(const char* format, ...);
};

class IndigoOptionHandlerSetter
{
public:
    static void setBasicOptionHandlers(const qword id);
};

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
