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

#ifndef __indigo_ket_documnet__
#define __indigo_ket_documnet__

#include "indigo_internal.h"
#include "molecule/base_molecule.h"
#include "molecule/ket_document.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

class DLLEXPORT IndigoKetDocument : public IndigoObject
{
public:
    explicit IndigoKetDocument();

    ~IndigoKetDocument() override;

    static bool is(const IndigoObject& obj);

    static KetDocument& get(IndigoObject& obj);

    inline KetDocument& get()
    {
        return _document;
    }

    BaseMolecule& getBaseMolecule() override;

    const char* debugInfo() const override;

    KetDocument& getKetDocument() override;

private:
    KetDocument _document;
};

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
