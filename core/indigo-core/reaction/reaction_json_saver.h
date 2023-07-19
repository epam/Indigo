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

#ifndef __reaction_json_saver__
#define __reaction_json_saver__

#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include "base_cpp/exception.h"
#include "molecule/ket_commons.h"

namespace indigo
{

    class Output;
    class BaseReaction;
    class BaseMolecule;
    class Vec2f;
    class MoleculeJsonSaver;

    class ReactionJsonSaver
    {
    public:
        explicit ReactionJsonSaver(Output& output);
        ~ReactionJsonSaver();

        void saveReaction(BaseReaction& rxn);
        void saveReaction(BaseReaction& rxn, BaseMolecule& merged, MoleculeJsonSaver& json_saver);
        void saveReactionWithMetaData(BaseReaction& rxn, BaseMolecule& merged, MoleculeJsonSaver& json_saver);
        bool add_stereo_desc;
        bool pretty_json;
        DECL_ERROR;

    protected:
        Output& _output;

    private:
        ReactionJsonSaver(const ReactionJsonSaver&); // no implicit copy
        static void _getBounds(BaseMolecule& mol, Vec2f& min_vec, Vec2f& max_vec, float scale);
        void _fixLayout(BaseReaction& rxn);

        std::unordered_map<int, std::string> _arrow_type2string = {
            {ReactionComponent::ARROW_BASIC, "open-angle"},
            {ReactionComponent::ARROW_FILLED_TRIANGLE, "filled-triangle"},
            {ReactionComponent::ARROW_FILLED_BOW, "filled-bow"},
            {ReactionComponent::ARROW_DASHED, "dashed-open-angle"},
            {ReactionComponent::ARROW_FAILED, "failed"},
            {ReactionComponent::ARROW_BOTH_ENDS_FILLED_TRIANGLE, "both-ends-filled-triangle"},
            {ReactionComponent::ARROW_EQUILIBRIUM_FILLED_HALF_BOW, "equilibrium-filled-half-bow"},
            {ReactionComponent::ARROW_EQUILIBRIUM_FILLED_TRIANGLE, "equilibrium-filled-triangle"},
            {ReactionComponent::ARROW_EQUILIBRIUM_OPEN_ANGLE, "equilibrium-open-angle"},
            {ReactionComponent::ARROW_UNBALANCED_EQUILIBRIUM_FILLED_HALF_BOW, "unbalanced-equilibrium-filled-half-bow"},
            {ReactionComponent::ARROW_UNBALANCED_EQUILIBRIUM_LARGE_FILLED_HALF_BOW, "unbalanced-equilibrium-large-filled-half-bow"},
            {ReactionComponent::ARROW_BOTH_ENDS_FILLED_TRIANGLE, "unbalanced-equilibrium-filled-half-triangle"}};
    };

} // namespace indigo

#endif
