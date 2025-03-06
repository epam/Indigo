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
#include "molecule/elements.h"
#include <cstdint>

#ifdef _WIN32
#pragma warning(push, 4)
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
            enum class Type
            {
                EMolecule = 0,
                ESpace = 1
            };

            LayoutItem()
            {
                clear();
            }

            void clear()
            {
                verticalAlign = ItemVerticalAlign::ECenter;
                type = Type::EMolecule;
                id = 0;
                isMoleculeFragment = false;
                min.zero();
                max.zero();
                scaledSize.zero();
                scaledOffset.zero();
                scaleFactor.zero();
                minScaledSize.zero();
            }
            Type type;
            int id;
            bool isMoleculeFragment;
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
            float offset;

        private:
            LayoutLine(const LayoutLine&);
        };

        Metalayout();
        void clear();
        bool isEmpty() const;
        void prepare(); // calculates averageBondLength and scaleFactor
        float getAverageBondLength() const;
        float getScaleFactor() const;
        const Vec2f& getContentSize() const;
        void process();
        LayoutLine& newLine();
        static void getBoundRect(Vec2f& min, Vec2f& max, BaseMolecule& mol);
        void calcContentSize();
        void scaleMoleculesSize();

        void* context;
        void (*cb_process)(LayoutItem& item, const Vec2f& pos, void* context);
        BaseMolecule& (*cb_getMol)(int id, void* context);

        static float getTotalMoleculeBondLength(BaseMolecule& mol);
        static float getTotalMoleculeClosestDist(BaseMolecule& mol);

        // utility function to use in MoleculeLayout & ReactionLayout
        void adjustMol(BaseMolecule& mol, const Vec2f& min, const Vec2f& pos) const;

        float reactionComponentMarginSize; // in angstrom
        float verticalIntervalFactor;
        float bondLength; // in angstrom
        DECL_ERROR;

    private:
        Vec2f _contentSize;
        float _avel, _scaleFactor;

        float _getAverageBondLength();

        ReusableObjArray<LayoutLine> _layout;
    };

    struct UnitsOfMeasure
    {
        enum TYPE
        {
            PT,
            PX,
            INCH,
            CM
        };

        static constexpr float INCH_TO_CM = 2.54f;
        static constexpr float INCH_TO_PT = 72.0f;

        static float convertInchesToPx(const float inches, const int32_t ppi)
        {
            return inches * ppi;
        }
        static float convertPxToInches(const float pixels, const int32_t ppi)
        {
            return pixels / ppi;
        }
        static float convertToPx(const float input, const TYPE units, const int32_t ppi)
        {
            switch (units)
            {
            case (PT):
                return convertInchesToPx(input / INCH_TO_PT, ppi);
                break;
            case (INCH):
                return convertInchesToPx(input, ppi);
                break;
            case (CM):
                return convertInchesToPx(input / INCH_TO_CM, ppi);
                break;
            default:
                return input;
            }
        }

        static float convertToPt(const float input, const TYPE units, const int32_t ppi)
        {
            switch (units)
            {
            case (PX):
                return convertPxToInches(input, ppi) * INCH_TO_PT;
                break;
            case (INCH):
                return input * INCH_TO_PT;
                break;
            case (CM):
                return (input * INCH_TO_PT) / INCH_TO_CM;
                break;
            default:
                return input;
            }
        }

        static float convertToInches(const float input, const TYPE units, const int32_t ppi)
        {
            switch (units)
            {
            case (PT):
                return input / INCH_TO_PT;
                break;
            case (PX):
                return convertPxToInches(input, ppi);
                break;
            case (CM):
                return input / INCH_TO_CM;
                break;
            default:
                return input;
            }
        }

        static float convertToCm(const float input, const TYPE units, const int32_t ppi)
        {
            switch (units)
            {
            case (PT):
                return (input * INCH_TO_CM) / INCH_TO_PT;
                break;
            case (INCH):
                return input * INCH_TO_CM;
                break;
            case (PX):
                return convertPxToInches(input, ppi) * INCH_TO_CM;
                break;
            default:
                return input;
            }
        }
        static float convertPtTo(float pt, UnitsOfMeasure::TYPE unit, int32_t ppi)
        {
            switch (unit)
            {
            case UnitsOfMeasure::CM:
                return UnitsOfMeasure::convertToCm(pt, UnitsOfMeasure::PT, ppi);
                break;
            case UnitsOfMeasure::PT:
                return pt;
                break;
            case UnitsOfMeasure::INCH:
                return UnitsOfMeasure::convertToInches(pt, UnitsOfMeasure::PT, ppi);
                break;
            case UnitsOfMeasure::PX:
                return UnitsOfMeasure::convertToPx(pt, UnitsOfMeasure::PT, ppi);
                break;
            }
            throw Exception("Unknown unit of measure: %d", unit);
        };

        static float convertToAngstrom(float input, TYPE units, int32_t ppi, float bond_length_px)
        {
            return convertToPx(input, units, ppi) / bond_length_px;
        };
    };

    struct LayoutOptions
    {
        // FIXME: The value is 1.6 instead of 1.0 due to backward compatibility, needs to be refactored
        static constexpr float DEFAULT_BOND_LENGTH = 1.0f;         // default length of inter-chemical bonds
        static constexpr float DEFAULT_MONOMER_BOND_LENGTH = 1.5f; // default length of inter-chemical bonds
        static constexpr float DEFAULT_PLUS_SIZE = DEFAULT_BOND_LENGTH / 2;
        static constexpr float DEFAULT_BOND_LENGTH_PX = 100.0f; // 100 pixel
        static constexpr float DEFAULT_FONT_SIZE_PX = DEFAULT_BOND_LENGTH_PX * 0.4f;
        static constexpr int32_t DEFAULT_PPI = 72;

        float bondLength{DEFAULT_BOND_LENGTH_PX};
        UnitsOfMeasure::TYPE bondLengthUnit{UnitsOfMeasure::TYPE::PX};
        float reactionComponentMarginSize{DEFAULT_BOND_LENGTH_PX / 2};
        UnitsOfMeasure::TYPE reactionComponentMarginSizeUnit{UnitsOfMeasure::TYPE::PX};
        int32_t ppi{72};
        float fontSize{-1};
        UnitsOfMeasure::TYPE fontSizeUnit{UnitsOfMeasure::PT};
        float fontSizeSub{-1};
        UnitsOfMeasure::TYPE fontSizeSubUnit{UnitsOfMeasure::PT};
        LABEL_MODE labelMode{LABEL_MODE_TERMINAL_HETERO};

        void reset()
        {
            bondLength = DEFAULT_BOND_LENGTH_PX;
            bondLengthUnit = UnitsOfMeasure::TYPE::PX;
            reactionComponentMarginSize = DEFAULT_BOND_LENGTH_PX / 2;
            reactionComponentMarginSizeUnit = UnitsOfMeasure::TYPE::PX;
            ppi = DEFAULT_PPI;
            fontSize = -1;
            fontSizeUnit = UnitsOfMeasure::PT;
            fontSizeSub = -1;
            fontSizeSubUnit = UnitsOfMeasure::PT;
            labelMode = LABEL_MODE_TERMINAL_HETERO;
        };

        float getBondLengthPx()
        {
            return UnitsOfMeasure::convertToPx(bondLength, bondLengthUnit, ppi);
        };

        void setBondLengthPx(float value)
        {
            bondLength = UnitsOfMeasure::convertPtTo(UnitsOfMeasure::INCH_TO_PT * value / ppi, bondLengthUnit, ppi);
        };

        float getMarginSizeInAngstroms() const
        {
            auto marginSizePt = UnitsOfMeasure::convertToPt(reactionComponentMarginSize, reactionComponentMarginSizeUnit, ppi);
            auto bondLengthPt = UnitsOfMeasure::convertToPt(bondLength, bondLengthUnit, ppi);

            return marginSizePt / bondLengthPt;
        };

        void setMarginSizeInAngstroms(float value)
        {
            float angs_to_pt = UnitsOfMeasure::convertToPt(bondLength, bondLengthUnit, ppi);
            reactionComponentMarginSize = UnitsOfMeasure::convertPtTo(value * angs_to_pt, reactionComponentMarginSizeUnit, ppi);
        };

        float getFontSizeInAngstroms() const
        {
            auto fontSizePt = UnitsOfMeasure::convertToPt(fontSize, fontSizeUnit, ppi);
            auto bondLengthPt = UnitsOfMeasure::convertToPt(bondLength, bondLengthUnit, ppi);

            return fontSizePt / bondLengthPt;
        };
    };

} // namespace indigo

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif //__metalayout_h__
