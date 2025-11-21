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

#include "molecule/json_writer.h"

namespace indigo
{
    std::unique_ptr<JsonWriter> JsonWriter::createJsonWriter(bool pretty)
    {
        if (pretty)
            return std::unique_ptr<JsonWriter>(new PrettyJsonWriter());
        else
            return std::unique_ptr<JsonWriter>(new CompactJsonWriter());
    }

    std::unique_ptr<JsonWriter> JsonWriter::createJsonDocumentWriter()
    {
        return std::unique_ptr<JsonWriter>(new DocumentJsonWriter());
    }
} // namespace indigo
