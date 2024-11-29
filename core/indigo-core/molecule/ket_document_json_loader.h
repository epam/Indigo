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

#ifndef __ket_document_json_loader__
#define __ket_document_json_loader__

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

#include "common/base_c/defs.h"
#include "common/base_cpp/exception.h"

#include <functional>
#include <optional>

#include "rapidjson/document.h"

namespace indigo
{

    class IdtAlias;
    class KetDocument;
    class KetMolecule;
    class KetMonomer;
    class MonomerTemplate;
    class MonomerTemplateLibrary;

    class DLLEXPORT KetDocumentJsonLoader
    {
    public:
        DECL_ERROR;
        using lib_ref = std::optional<std::reference_wrapper<MonomerTemplateLibrary>>;
        void parseJson(const std::string& json_str, KetDocument& document, lib_ref library = std::nullopt);

        static void parseMonomerTemplate(const rapidjson::Value& mt_json, KetDocument& document);
        static void parseMonomerTemplate(const rapidjson::Value& mt_json, MonomerTemplateLibrary& library);

    protected:
        void parseKetMolecule(std::string& ref, rapidjson::Value& json, KetDocument& document);
        void parseKetRgroup(std::string& ref, rapidjson::Value& json, KetDocument& document);
        void parseKetMonomer(std::string& ref, rapidjson::Value& json, KetDocument& document);
        void parseKetVariantMonomer(std::string& ref, rapidjson::Value& json, KetDocument& document);
        void parseKetMonomerShape(std::string& ref, rapidjson::Value& json, KetDocument& document);

        using template_add_func = std::function<MonomerTemplate&(const std::string& id, const std::string& monomer_class, IdtAlias idt_alias, bool unresolved)>;
        static void parseMonomerTemplate(const rapidjson::Value& mt_json, template_add_func addMonomerTemplate);
        static void parseVariantMonomerTemplate(const rapidjson::Value& mt_json, KetDocument& document);

    private:
    };

} // namespace indigo

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif
