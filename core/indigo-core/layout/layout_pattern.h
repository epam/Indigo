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

#ifndef __layout_pattern_h__
#define __layout_pattern_h__

#include "base_c/defs.h"
#include "base_cpp/array.h"
#include "graph/graph.h"
#include "math/algebra.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

namespace indigo
{

    struct PatternAtom
    {
        explicit PatternAtom(Vec2f pos_) : pos(pos_)
        {
        }
        Vec2f pos;
    };

    struct PatternBond
    {
        explicit PatternBond(int type_) : type(type_), parity(0)
        {
        }

        int type; // see BOND_***
        int parity;
    };

    class DLLEXPORT PatternLayout : public Graph
    {
    public:
        explicit PatternLayout();
        ~PatternLayout() override;

        int addBond(int atom_beg, int atom_end, int type);
        int addAtom(float x, float y);
        int addOutlinePoint(float x, float y);
        bool isFixed() const
        {
            return _fixed;
        }
        void fix()
        {
            _fixed = true;
        }
        void setName(const char* name)
        {
            _name.readString(name, true);
        }
        const char* getName() const
        {
            return _name.ptr();
        }

        void calcMorganCode();
        long morganCode() const
        {
            return _morgan_code;
        }

        const PatternAtom& getAtom(int idx) const;
        const PatternBond& getBond(int idx) const;
        const Array<Vec2f>& getOutline() const
        {
            return _outline;
        }

        DECL_ERROR;

    protected:
        Array<PatternAtom> _atoms;
        Array<PatternBond> _bonds;

        Array<Vec2f> _outline;

        Array<char> _name;

        long _morgan_code;
        bool _fixed;

        // no implicit copy
        PatternLayout(const PatternLayout&);
    };

} // namespace indigo

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
