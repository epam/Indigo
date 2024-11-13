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

#include "indigo_ket_document.h"
#include "molecule/ket_document_json_loader.h"

//
// IndigoKetDocument
//

IndigoKetDocument::IndigoKetDocument() : IndigoObject(KET_DOCUMENT), _document()
{
}

IndigoKetDocument::~IndigoKetDocument()
{
}

bool IndigoKetDocument::is(const IndigoObject& object)
{
    return object.type == KET_DOCUMENT;
}

KetDocument& IndigoKetDocument::get(IndigoObject& obj)
{
    if (obj.type != KET_DOCUMENT)
        throw IndigoError("%s is not a ket document", obj.debugInfo());
    return reinterpret_cast<IndigoKetDocument&>(obj)._document;
}

const char* IndigoKetDocument::debugInfo() const
{
    return "<KetDocument>";
}

KetDocument& IndigoKetDocument::getKetDocument()
{
    return _document;
}

BaseMolecule& IndigoKetDocument::getBaseMolecule()
{
    return _document.getBaseMolecule();
}
