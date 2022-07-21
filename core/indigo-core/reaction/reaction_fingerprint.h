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

#ifndef __reaction_fingerprint__
#define __reaction_fingerprint__

#include "base_cpp/cancellation_handler.h"
#include "base_cpp/tlscont.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

namespace indigo
{

    class BaseReaction;
    struct MoleculeFingerprintParameters;

    class DLLEXPORT ReactionFingerprintBuilder
    {
    public:
        ReactionFingerprintBuilder(BaseReaction& reaction, const MoleculeFingerprintParameters& parameters);

        bool query;
        bool skip_ord;
        bool skip_sim;
        bool skip_ext;

        void process();

        byte* get();
        byte* getSim();

        void parseFingerprintType(const char* type, bool query);

        DECL_ERROR;

    protected:
        BaseReaction& _reaction;
        const MoleculeFingerprintParameters& _parameters;

        CP_DECL;
        TL_CP_DECL(Array<byte>, _fingerprint);

    private:
        ReactionFingerprintBuilder(const ReactionFingerprintBuilder&); // no implicit copy
    };

} // namespace indigo

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
