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

#ifndef __ket_document__
#define __ket_document__

#include "molecule/ket_objects.h"
#include "molecule/monomers_template_library.h"

#include <rapidjson/document.h> // Temporary until direct conversion to molecule supported

#include <deque>
#include <set>

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

namespace indigo
{
    class BaseMolecule;
    class Output;

    class DLLEXPORT KetDocument : public MonomerTemplateLibrary
    {
    public:
        DECL_ERROR;
        KetDocument()
            : _molecules(), original_format(0), _meta_objects(rapidjson::kArrayType), _r_groups(rapidjson::kArrayType), _json_molecules(rapidjson::kArrayType),
              _json_document(){};
        KetDocument(const KetDocument& other) = delete;
        KetDocument& operator=(const KetDocument&) = delete;

        KetMolecule& addMolecule(const std::string& ref);

        std::unique_ptr<KetBaseMonomer>& addMonomer(const std::string& alias, const std::string& template_id);
        std::unique_ptr<KetBaseMonomer>& addMonomer(const std::string& id, const std::string& alias, const std::string& template_id);
        std::unique_ptr<KetBaseMonomer>& addMonomer(const std::string& id, const std::string& alias, const std::string& template_id, const std::string& ref);
        const std::unique_ptr<KetBaseMonomer>& getMonomerById(const std::string& ref) const;

        MonomerTemplate& addMonomerTemplate(const std::string& id, const std::string& monomer_class, IdtAlias idt_alias, bool unresolved = false);
        void addMonomerTemplate(const MonomerTemplate& monomer_template);

        bool hasAmbiguousMonomerTemplateWithId(const std::string& id) const;
        KetAmbiguousMonomerTemplate& addAmbiguousMonomerTemplate(const std::string& subtype, const std::string& id, const std::string& name, IdtAlias idt_alias,
                                                                 std::vector<KetAmbiguousMonomerOption>& options);
        std::unique_ptr<KetBaseMonomer>& addAmbiguousMonomer(const std::string& alias, const std::string& template_id);
        std::unique_ptr<KetBaseMonomer>& addAmbiguousMonomer(const std::string& id, const std::string& alias, const std::string& template_id);
        std::unique_ptr<KetBaseMonomer>& addAmbiguousMonomer(const std::string& id, const std::string& alias, const std::string& template_id,
                                                             const std::string& ref);

        using molecules_map = std::map<std::string, KetMolecule>;
        using templates_map = std::map<std::string, MonomerTemplate>;
        using monomers_map = std::map<std::string, std::unique_ptr<KetBaseMonomer>>;
        using ambiguous_templates_map = std::map<std::string, KetAmbiguousMonomerTemplate>;

        inline const molecules_map& molecules() const
        {
            return _molecules;
        };

        inline const std::vector<std::string>& moleculesRefs() const
        {
            return _molecule_refs;
        };

        inline const monomers_map& monomers() const
        {
            return _monomers;
        };

        inline const std::vector<std::string>& monomersIds() const
        {
            return _monomers_ids;
        };

        inline const templates_map& templates() const
        {
            return _templates;
        };

        inline const std::vector<std::string>& templatesIds() const
        {
            return _templates_ids;
        };

        KetConnection& addConnection(const std::string& conn_type, KetConnectionEndPoint ep1, KetConnectionEndPoint ep2);

        KetConnection& addConnection(KetConnectionEndPoint ep1, KetConnectionEndPoint ep2);

        KetConnection& addConnection(const std::string& mon1, const std::string& ap1, const std::string& mon2, const std::string& ap2);

        void connectMonomerTo(const std::string& mon1, const std::string& ap1, const std::string& mon2, const std::string& ap2);

        const std::vector<KetConnection> connections() const
        {
            return _connections;
        };

        // Return list of connections between simple polimers
        // i.e. non-backbone and not sugar-base connections
        // connections that create a cycle place here too
        // parseSimplePolymers should be called to fill this list
        const std::vector<KetConnection> nonSequenceConnections() const
        {
            return _non_sequence_connections;
        };

        int original_format;

        const ambiguous_templates_map& ambiguousTemplates() const
        {
            return _ambiguous_templates;
        };

        const std::vector<std::string>& ambiguousTemplatesIds() const
        {
            return _ambiguous_templates_ids;
        };

        BaseMolecule& getBaseMolecule();

        bool hasAmbiguousMonomerTemplate(const std::string& id) const
        {
            return _ambiguous_templates.find(id) != _ambiguous_templates.end();
        };

        void processAmbiguousMonomerTemplates();

        // Parse list of monomer and ist of connections stored in document and return list of simple polimers in term of HELM
        // Each simple polymer represented as list of monomer IDs.
        // Monomers connected from m[i] R2 to m[i+1] R1 for peptides
        // For RNA/DNA monomer placed in order Sugar-Base-Phosphate-Sugar... with standard connections
        // Each CHEM returned as separate simple polymer
        // Also store non-standard or creating cycle connections in nonSequenceConnections list
        void parseSimplePolymers(std::vector<std::deque<std::string>>& sequences, bool for_idt = false);

        MonomerClass getMonomerClass(const KetBaseMonomer& monomer) const;

        MonomerClass getMonomerClass(const std::string& monomer_id) const;

        const KetBaseMonomerTemplate& getMonomerTemplate(const std::string& template_id) const;

        void addMetaObject(const rapidjson::Value& node);

        void addRGroup(const rapidjson::Value& node);

        void addMolecule(const rapidjson::Value& node, std::string& ref);

        const rapidjson::Value& metaObjects() const
        {
            return _meta_objects;
        };

        const rapidjson::Value& rgroups() const
        {
            return _r_groups;
        };

        const rapidjson::Value& jsonMolecules() const
        {
            return _json_molecules;
        };

        void setFastaProps(std::vector<std::string> fasta_properties)
        {
            _fasta_properties = fasta_properties;
        };

        const std::vector<std::string>& fastaProps()
        {
            return _fasta_properties;
        };

        const std::string& monomerIdByRef(const std::string& ref);

        void addMonomerShape(const std::string& id, bool collapsed, const std::string& shape, Vec2f position, const std::vector<std::string>& monomers)
        {
            _monomer_shapes.emplace_back(id, collapsed, shape, position, monomers);
        }

        const std::vector<KetMonomerShape>& monomerShapes() const
        {
            return _monomer_shapes;
        }

        void CalculateMacroProps(Output& output, bool pretty_json = false);

    protected:
        void collect_sequence_side(const std::string& monomer_id, bool left_side, std::set<std::string>& monomers, std::set<std::string>& used_monomers,
                                   std::deque<std::string>& sequence, std::map<std::pair<std::string, std::string>, const KetConnection&>& ap_to_connection);

    private:
        molecules_map _molecules;
        std::vector<std::string> _molecule_refs;
        monomers_map _monomers;
        std::vector<std::string> _monomers_ids;
        std::map<std::string, std::string> _monomer_ref_to_id;
        templates_map _templates;
        std::vector<std::string> _templates_ids;
        ambiguous_templates_map _ambiguous_templates;
        std::vector<std::string> _ambiguous_templates_ids;
        std::vector<KetConnection> _connections;
        std::vector<KetConnection> _non_sequence_connections;
        std::map<std::string, KetBaseMonomerTemplate::TemplateType> _template_id_to_type;
        rapidjson::Value _meta_objects;
        rapidjson::Value _r_groups;
        rapidjson::Value _json_molecules;
        std::map<std::string, int> _mol_ref_to_idx;
        rapidjson::Document _json_document;
        std::vector<std::string> _fasta_properties;
        std::vector<KetMonomerShape> _monomer_shapes;
    };
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif