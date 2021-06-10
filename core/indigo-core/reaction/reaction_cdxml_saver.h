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

#ifndef __reaction_cdxml_saver__
#define __reaction_cdxml_saver__

#include "base_cpp/array.h"
#include "base_cpp/exception.h"
#include "base_cpp/obj_array.h"

namespace indigo
{

    class Output;
    class BaseReaction;
    class MoleculeCdxmlSaver;

    class DLLEXPORT ReactionCdxmlSaver
    {
    public:
        explicit ReactionCdxmlSaver(Output& output);
        ~ReactionCdxmlSaver();

        void saveReaction(BaseReaction& rxn);

        DECL_ERROR;

    protected:
        BaseReaction* _rxn;
        Output& _output;

    private:
        ReactionCdxmlSaver(const ReactionCdxmlSaver&); // no implicit copy

        void _addPlusses(BaseReaction& rxn, MoleculeCdxmlSaver& molsaver);
        void _addArrow(BaseReaction& rxn, MoleculeCdxmlSaver& molsaver, int arrow_id);
        void _addScheme(MoleculeCdxmlSaver& molsaver);
        void _closeScheme(MoleculeCdxmlSaver& molsaver);
        void _addStep(BaseReaction& rxn, MoleculeCdxmlSaver& molsaver, ArrayInt& reactants_ids, ArrayInt& products_ids, ObjArray<ArrayInt>& nodes_ids,
                      int arrow_id);
        void _generateCdxmlObjIds(BaseReaction& rxn, ArrayInt& reactants_ids, ArrayInt& products_ids, ObjArray<ArrayInt>& nodes_ids, int& arrow_id);
        void _addTitle(BaseReaction& rxn, MoleculeCdxmlSaver& molsaver);
    };

} // namespace indigo

#endif
