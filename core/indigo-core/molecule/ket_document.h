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

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

namespace indigo
{
    class DLLEXPORT KetDocument : public MonomerTemplateLibrary
    {
    public:
        DECL_ERROR;
        KetDocument() : _molecules(){};
        KetDocument(const KetDocument& other) = delete;
        KetDocument& operator=(const KetDocument&) = delete;

        KetMolecule& addMolecule(const std::string& ref);

        KetMonomer& addMonomer(const std::string& ref, const std::string& id, const std::string& template_id);

        using molecules_map = std::map<std::string, KetMolecule>;
        using templates_maps = std::map<std::string, MonomerTemplate>;
        using monomers_maps = std::map<std::string, KetMonomer>;

        inline const molecules_map& molecules()
        {
            return _molecules;
        };

        inline const monomers_maps& monomers()
        {
            return _monomers;
        };

    private:
        molecules_map _molecules;
        monomers_maps _monomers;
    };
}

#endif