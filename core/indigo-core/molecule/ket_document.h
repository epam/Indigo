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

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

namespace indigo
{
    class BaseMolecule;

    class DLLEXPORT KetDocument : public MonomerTemplateLibrary
    {
    public:
        DECL_ERROR;
        KetDocument() : _molecules(), original_format(0){};
        KetDocument(const KetDocument& other) = delete;
        KetDocument& operator=(const KetDocument&) = delete;

        KetMolecule& addMolecule(const std::string& ref);

        KetMonomer& addMonomer(const std::string& alias, const std::string& template_id);
        KetMonomer& addMonomer(const std::string& ref, const std::string& id, const std::string& alias, const std::string& template_id);

        void addMonomerTemplate(const MonomerTemplate& monomer_template);

        KetVariantMonomerTemplate& addVariantMonomerTemplate(const std::string& subtype, const std::string& id, const std::string& name,
                                                             std::vector<KetVariantMonomerOption>& options);
        KetVariantMonomer& addVariantMonomer(const std::string& id, const std::string& template_id);

        using molecules_map = std::map<std::string, KetMolecule>;
        using templates_map = std::map<std::string, MonomerTemplate>;
        using monomers_map = std::map<std::string, KetMonomer>;
        using variant_templates_map = std::map<std::string, KetVariantMonomerTemplate>;
        using variant_monomers_map = std::map<std::string, KetVariantMonomer>;

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

        inline const std::vector<std::string>& monomersRefs() const
        {
            return _monomers_refs;
        };

        inline const templates_map& templates() const
        {
            return _templates;
        };

        inline const std::vector<std::string>& templatesRefs() const
        {
            return _templates_refs;
        };

        KetConnection& addConnection(KetConnectionEndPoint ep1, KetConnectionEndPoint ep2)
        {
            _connections.emplace_back(ep1, ep2);
            return *_connections.rbegin();
        };

        const std::vector<KetConnection> connections() const
        {
            return _connections;
        };

        int original_format;

        const variant_templates_map& variantTemplates() const
        {
            return _variant_templates;
        };

        const std::vector<std::string>& variantTemplatesRefs() const
        {
            return _variant_templates_refs;
        };

        const variant_monomers_map& variantMonomers() const
        {
            return _variant_monomers;
        };

        const std::vector<std::string>& variantMonomersRefs() const
        {
            return _variant_monomers_refs;
        };

        BaseMolecule& getBaseMolecule();

        bool hasVariantMonomerTemplate(const std::string& id) const
        {
            auto ref = KetVariantMonomerTemplate::ref_prefix + id;
            return _variant_templates.find(ref) != _variant_templates.end();
        };

    private:
        molecules_map _molecules;
        std::vector<std::string> _molecule_refs;
        monomers_map _monomers;
        std::vector<std::string> _monomers_refs;
        templates_map _templates;
        std::vector<std::string> _templates_refs;
        variant_templates_map _variant_templates;
        std::vector<std::string> _variant_templates_refs;
        variant_monomers_map _variant_monomers;
        std::vector<std::string> _variant_monomers_refs;
        std::vector<KetConnection> _connections;
    };
}

#endif