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

#ifndef __rxnfile_saver__
#define __rxnfile_saver__

#include "base_cpp/exception.h"

namespace indigo
{

    class Output;
    class Reaction;
    class BaseReaction;
    class QueryReaction;
    class MolfileSaver;

    class RxnfileSaver
    {
    public:
        RxnfileSaver(Output& output);
        ~RxnfileSaver();

        void saveBaseReaction(BaseReaction& reaction);
        void saveReaction(Reaction& reaction);
        void saveQueryReaction(QueryReaction& reaction);

        int molfile_saving_mode; // MolfileSaver::MODE_***, default zero
        bool skip_date;
        bool add_stereo_desc;
        bool add_implicit_h;

        DECL_ERROR;

    protected:
        void _saveReaction();
        bool _v2000;

        BaseReaction* _brxn;
        QueryReaction* _qrxn;
        Reaction* _rxn;

        Output& _output;
        void _writeRxnHeader(BaseReaction& reaction);
        void _writeReactantsHeader();
        void _writeProductsHeader();
        void _writeCatalystsHeader();
        void _writeReactantsFooter();
        void _writeProductsFooter();
        void _writeCatalystsFooter();
        void _writeMolHeader();
        void _writeMol(MolfileSaver& saver, int index);
        void _writeRxnFooter();
    };

} // namespace indigo

#endif
