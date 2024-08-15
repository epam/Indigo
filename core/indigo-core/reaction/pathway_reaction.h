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

#pragma once

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

#include "reaction/base_reaction.h"
#include <deque>

namespace indigo
{

    class Reaction;

    class DLLEXPORT PathwayReaction : public BaseReaction
    {
        static constexpr float MARGIN = 1.f;
        static constexpr float ARROW_HEAD_WIDTH = 2.5f;
        static constexpr float ARROW_TAIL_WIDTH = 0.5f;
        static constexpr float ARROW_WIDTH = ARROW_HEAD_WIDTH + ARROW_TAIL_WIDTH;

    public:
        PathwayReaction();
        PathwayReaction(std::deque<Reaction>&);
        ~PathwayReaction() override;

        int reactionId(int moleculeId) const;
        int reactionsCount() const;
        void clone(PathwayReaction&);
        std::pair<std::vector<std::pair<int, Vec2f>>, std::vector<std::vector<Vec2f>>> makeTreePoints();

        BaseReaction* neu() override;
        bool aromatize(const AromaticityOptions& options) override;

        DECL_ERROR;

    protected:
        int _addBaseMolecule(int side) override;

    private:
        Array<int> _reactions;
    };

} // namespace indigo

#ifdef _WIN32
#pragma warning(pop)
#endif
