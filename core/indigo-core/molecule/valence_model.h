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

#ifndef __valence_model_h__
#define __valence_model_h__

#include "base_c/defs.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

namespace indigo
{

    // Valence specification mode: pre-2014 (legacy) vs BIOVIA post-2014
    enum class ValenceMode
    {
        BIOVIA_2009, // BIOVIA 2009: all main-group elements get full valence calculation
        BIOVIA_2017  // BIOVIA 2017: only 22 listed non-metals + Al⁻ exception
    };

    // Abstract base for valence calculation strategies.
    // Template Method pattern: calcValence() is non-virtual and calls
    // the ONE virtual hook interceptMainGroupMetal().
    class DLLEXPORT ValenceModel
    {
    public:
        virtual ~ValenceModel() = default;

        // Template Method: shared valence logic with one virtual hook.
        // Permissive model: always returns true when to_throw=false.
        // When the connectivity exceeds the model prediction, sets
        // valence=conn, hyd=0 and *nonStandard=true (if provided).
        bool calcValence(int elem, int charge, int radical, int conn, int& valence, int& hyd, bool to_throw, bool* nonStandard = nullptr) const;

        // Factory: returns singleton for the given mode
        static const ValenceModel& instance(ValenceMode mode = ValenceMode::BIOVIA_2009);

        ValenceModel(const ValenceModel&) = delete;
        ValenceModel& operator=(const ValenceModel&) = delete;

    protected:
        ValenceModel() = default;

        // Virtual hook: should a main-group element (not a transition metal)
        // be intercepted and assigned valence=conn, hyd=0?
        virtual bool interceptMainGroupMetal(int elem, int charge) const = 0;
    };

    // DEFAULT mode: all main-group elements go through normal valence calculation
    class DLLEXPORT DefaultValenceModel final : public ValenceModel
    {
    protected:
        bool interceptMainGroupMetal(int elem, int charge) const override;
    };

    // BIOVIA_2017 mode: only 22 non-metals have valence entries; metals get hyd=0
    class DLLEXPORT Biovia2017ValenceModel final : public ValenceModel
    {
    protected:
        bool interceptMainGroupMetal(int elem, int charge) const override;
    };

} // namespace indigo

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif
