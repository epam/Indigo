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

#ifndef __ket_document_json_saver__
#define __ket_document_json_saver__

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

#include <sstream>

#include "base_cpp/locale_guard.h"
#include "base_cpp/output.h"
#include "molecule/elements.h"
#include "molecule/json_writer.h"
#include "molecule/meta_commons.h"

namespace indigo
{

    class Output;
    class MonomerTemplate;
    class KetMolecule;
    class KetMonomer;
    class KetAmbiguousMonomer;
    class KetAmbiguousMonomerTemplate;
    class KetDocument;

    class DLLEXPORT KetDocumentJsonSaver
    {
    public:
        explicit KetDocumentJsonSaver(Output& output) : _output(output), pretty_json(false){};

        KetDocumentJsonSaver(const KetDocumentJsonSaver&) = delete; // no implicit copy

        void saveKetDocument(const KetDocument& document);

        static void saveKetDocument(JsonWriter& writer, const KetDocument& document);

        static void saveMonomerTemplate(JsonWriter& writer, const MonomerTemplate& monomer_template);

        bool pretty_json;

    protected:
        static void saveMolecule(JsonWriter& writer, const std::string& ref, const KetMolecule& molecule);
        static void saveMonomer(JsonWriter& writer, const KetMonomer& monomer);
        static void saveVariantMonomer(JsonWriter& writer, const KetAmbiguousMonomer& monomer);
        static void saveVariantMonomerTemplate(JsonWriter& writer, const KetAmbiguousMonomerTemplate& monomer_template);
        static void saveMonomerShape(JsonWriter& writer, const KetMonomerShape& monomer_shape);

        DECL_ERROR;

        Output& _output;

    private:
    };

} // namespace indigo

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif
