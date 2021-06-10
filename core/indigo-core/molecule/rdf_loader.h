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

#ifndef _RDF_LOADER_H__
#define _RDF_LOADER_H__

#include "base_cpp/obj.h"
#include "base_cpp/properties_map.h"
#include "base_cpp/tlscont.h"

namespace indigo
{

    class Scanner;
    /*
     * RD files loader
     * An RDfile (reaction-data file) consists of a set of editable “records.” Each record defines a
     * molecule or reaction, and its associated data
     * Note: internal-regno and external-regno are placed into the properties with corresponding names
     */
    class RdfLoader
    {
        /*
         * Max data size is 100 Mb
         */
        enum
        {
            MAX_DATA_SIZE = 104857600
        };

    public:
        RdfLoader(Scanner& scanner);
        ~RdfLoader();

        bool isEOF();
        void readNext();
        void readAt(int index);
        long long tell();
        int currentNumber();
        int count();

        CP_DECL;
        /*
         * Data buffer with reaction or molecule for current record
         */
        TL_CP_DECL(Array<char>, data);
        /*
         * Properties map for current record
         */
        TL_CP_DECL(PropertiesMap, properties);

        /*
         * Defines is molecule or reaction there in the current record
         */
        bool isMolecule() const
        {
            return _isMolecule;
        }

        DECL_ERROR;

    protected:
        bool _readIdentifiers(bool);
        inline Scanner& _getScanner() const;
        static bool _readLine(Scanner&, Array<char>&);

        inline bool _startsWith(const char* str) const
        {
            return ((size_t)_innerBuffer.size() >= strlen(str) && strncmp(_innerBuffer.ptr(), str, strlen(str)) == 0);
        }

        TL_CP_DECL(Array<char>, _innerBuffer);
        bool _ownScanner;
        Scanner* _scanner;
        bool _isMolecule;

        TL_CP_DECL(Array<long long>, _offsets);
        int _current_number;
        long long _max_offset;
    };

} // namespace indigo

#endif /* _RDF_READER_H */
