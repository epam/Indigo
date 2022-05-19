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

#ifndef __rowid_saver_h__
#define __rowid_saver_h__

#include "base_cpp/bitoutworker.h"
#include "lzw/lzw_dictionary.h"
#include "lzw/lzw_encoder.h"

#include "oracle/rowid_symbol_codes.h"

namespace indigo
{
    class Output;

    class RowIDSaver
    {

    public:
        DECL_ERROR;

        RowIDSaver(LzwDict& NewDict, Output& NewOut);

        void saveRowID(const char* RowID);

    private:
        void _encodeSymbol(char Symbol);

        void _encode(int NextSymbol);

        std::unique_ptr<LzwEncoder> _encoder_obj;

        LzwEncoder* _encoder;

        // no implicit copy
        RowIDSaver(const RowIDSaver&);
    };

} // namespace indigo

#endif /* __rowid_saver_h__ */

/* END OF 'ROWID_SAVER.H' FILE */
