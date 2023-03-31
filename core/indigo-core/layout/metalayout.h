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

#ifndef __metalayout_h__
#define __metalayout_h__

#include "base_cpp/obj_array.h"
#include "base_cpp/reusable_obj_array.h"
#include "math/algebra.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

namespace indigo
{

    class BaseMolecule;

    class DLLEXPORT Metalayout
    {
    public:
        struct DLLEXPORT LayoutItem
        {
            enum class ItemVerticalAlign
            {
                ECenter,
                ETop,
                EBottom
            };

            LayoutItem()
            {
                clear();
            }

            void clear()
            {
                verticalAlign = ItemVerticalAlign::ECenter;
                type = 0;
                id = 0;
                fragment = false;
                min.zero();
                max.zero();
                scaledSize.zero();
                scaledOffset.zero();
                scaleFactor.zero();
                minScaledSize.zero();
            }
            int type;
            int id;
            bool fragment;
            ItemVerticalAlign verticalAlign;

            Vec2f min, max;
            Vec2f scaledSize, scaledOffset, minScaledSize;
            Vec2f scaleFactor;
        };

        class DLLEXPORT LayoutLine
        {
        public:
            LayoutLine();
            ~LayoutLine();
            void clear();

            ObjArray<LayoutItem> items;
            float height;
            float top_height;
            float bottom_height;
            float width;

        private:
            LayoutLine(const LayoutLine&);
        };

        Metalayout();
        void clear();
        bool isEmpty() const;
        void prepare();
        float getAverageBondLength() const;
        float getScaleFactor() const;
        const Vec2f& getContentSize() const;
        void setScaleFactor();
        void process();
        LayoutLine& newLine();
        static void getBoundRect(Vec2f& min, Vec2f& max, BaseMolecule& mol);
        void calcContentSize();
        void scaleSz();

        void* context;
        void (*cb_process)(LayoutItem& item, const Vec2f& pos, void* context);
        BaseMolecule& (*cb_getMol)(int id, void* context);

        static float getTotalMoleculeBondLength(BaseMolecule& mol);
        static float getTotalMoleculeClosestDist(BaseMolecule& mol);

        // utility function to use in MoleculeLayout & ReactionLayout
        void adjustMol(BaseMolecule& mol, const Vec2f& min, const Vec2f& pos);

        float horizontalIntervalFactor;
        float verticalIntervalFactor;
        float bondLength;

        DECL_ERROR;

    private:
        Vec2f _contentSize;
        float _avel, _scaleFactor, _offset;

        float _getAverageBondLength();

        ReusableObjArray<LayoutLine> _layout;
    };

} // namespace indigo

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif //__metalayout_h__
