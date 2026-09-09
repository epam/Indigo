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

#include "render_font_face_manager.h"

#include "NotoSansCJK_Bold.h"
#include "NotoSans_Bold.h"

#include "NotoSans_BoldItalic.h"

#include "NotoSans_Italic.h"

#include "NotoSansCJK_Regular.h"
#include "NotoSans_Regular.h"

#include <freetype/ftmm.h>

#include <stdexcept>
#include <utility>

namespace indigo
{
    namespace
    {
        // Stores an opened font together with the library and byte array it depends on
        struct FreeTypeFace
        {
            FreeTypeFace(const std::shared_ptr<FT_LibraryRec_>& library, const unsigned char font[], size_t font_size,
                         std::shared_ptr<std::vector<byte>> data, const std::string& name)
                : library(library), data(std::move(data))
            {
                if (FT_New_Memory_Face(library.get(), font, static_cast<FT_Long>(font_size), 0, &face))
                    throw std::runtime_error("error loading font " + name);
            }

            ~FreeTypeFace()
            {
                if (face)
                    FT_Done_Face(face);
            }

            FT_Face face = nullptr;
            std::shared_ptr<FT_LibraryRec_> library;
            std::shared_ptr<std::vector<byte>> data;
        };

        // Stores the variable-font axes returned by FreeType and releases them later
        struct FreeTypeMMVar
        {
            FreeTypeMMVar(FT_Library library, FT_MM_Var* var) : library(library), var(var)
            {
            }

            ~FreeTypeMMVar()
            {
                FT_Done_MM_Var(library, var);
            }

            FT_Library library;
            FT_MM_Var* var;
        };

        // Names of variable-font axes and values used to create text styles
        constexpr FT_ULong WEIGHT_AXIS_TAG = FT_MAKE_TAG('w', 'g', 'h', 't');
        constexpr FT_ULong ITALIC_AXIS_TAG = FT_MAKE_TAG('i', 't', 'a', 'l');
        constexpr FT_ULong SLANT_AXIS_TAG = FT_MAKE_TAG('s', 'l', 'n', 't');
        constexpr FT_Fixed REGULAR_WEIGHT = static_cast<FT_Fixed>(400) << 16;
        constexpr FT_Fixed BOLD_WEIGHT = static_cast<FT_Fixed>(700) << 16;

        // Label Cairo uses to attach FreeTypeFace to each Cairo font face
        cairo_user_data_key_t free_type_face_key = {};

        // Uses the nearest supported value when the requested value is outside the axis range
        FT_Fixed clampAxisValue(FT_Fixed value, const FT_Var_Axis& axis)
        {
            if (value < axis.minimum)
                return axis.minimum;
            if (value > axis.maximum)
                return axis.maximum;
            return value;
        }

        // Returns an axis value that renders text without italic slant
        FT_Fixed getUprightAxisValue(const FT_Var_Axis& axis)
        {
            return clampAxisValue(0, axis);
        }

        // Returns an axis value that renders text in italic
        FT_Fixed getItalicAxisValue(const FT_Var_Axis& axis)
        {
            if (axis.tag == ITALIC_AXIS_TAG)
                return axis.maximum;

            // For slnt, choose the endpoint furthest from zero
            if (axis.minimum >= 0)
                return axis.maximum;
            if (axis.maximum <= 0)
                return axis.minimum;
            return axis.minimum <= -axis.maximum ? axis.minimum : axis.maximum;
        }

        void destroyFreeTypeFace(void* face)
        {
            delete static_cast<FreeTypeFace*>(face);
        }

    } // namespace

    RenderFontFaceManager::RenderFontFaceManager(const PtrArray<RenderFont>& fonts)
    {
        _initFreeType();
        _loadCustomFontFaces(fonts);
    }

    RenderFontFaceManager::~RenderFontFaceManager() = default;

    RenderFontFaceManager::Face::~Face()
    {
        if (cairo_face)
            cairo_font_face_destroy(cairo_face);
    }

    cairo_font_face_t* RenderFontFaceManager::selectCairoFontFace(const TextItem& ti)
    {
        if (auto custom_face = _findFontFace(_custom_faces, ti))
            return custom_face;

        bool is_bold = ti.bold;
        bool is_italic = ti.italic;

#ifdef RENDER_ENABLE_CJK
        auto lang = _lang_detector.detectLang(ti);

        if (lang != FONT_LANG::NO_CJK)
        {
            if (is_bold)
            {
                if (!_face_cjk_bold.cairo_face)
                    _loadFontFace(_face_cjk_bold, sans_cjk_bold, sans_cjk_bold_size, "CJK bold");
                return _face_cjk_bold.cairo_face;
            }
            else
            {
                if (!_face_cjk_regular.cairo_face)
                    _loadFontFace(_face_cjk_regular, sans_cjk_regular, sans_cjk_regular_size, "CJK regular");
                return _face_cjk_regular.cairo_face;
            }
        }
#endif

        if (is_bold && is_italic)
        {
            if (!_face_bold_italic.cairo_face)
                _loadFontFace(_face_bold_italic, sans_bold_italic, sans_bold_italic_size, "bold italic");
            return _face_bold_italic.cairo_face;
        }
        else if (is_bold)
        {
            if (!_face_bold.cairo_face)
                _loadFontFace(_face_bold, sans_bold, sans_bold_size, "bold");
            return _face_bold.cairo_face;
        }
        else if (is_italic)
        {
            if (!_face_italic.cairo_face)
                _loadFontFace(_face_italic, sans_italic, sans_italic_size, "italic");
            return _face_italic.cairo_face;
        }
        else
        {
            if (!_face_regular.cairo_face)
                _loadFontFace(_face_regular, sans_regular, sans_regular_size, "regular");
            return _face_regular.cairo_face;
        }
    }

    cairo_font_face_t* RenderFontFaceManager::_findFontFace(const PtrArray<Face>& faces, const TextItem& ti) const
    {
        for (int i = 0; i < faces.size(); ++i)
        {
            const Face& face = faces[i];
            if (face.bold == ti.bold && face.italic == ti.italic)
                return face.cairo_face;
        }

        return nullptr;
    }

    void RenderFontFaceManager::_loadFontFace(Face& face, const unsigned char font[], size_t font_size, const std::string& name,
                                              std::vector<FT_Fixed>* variation_coordinates)
    {
        auto free_type_face = std::make_unique<FreeTypeFace>(_library, font, font_size, face.data, name);

        if (variation_coordinates && FT_Set_Var_Design_Coordinates(free_type_face->face, variation_coordinates->size(), variation_coordinates->data()))
            throw std::runtime_error("error setting font variation " + name);

        cairo_font_face_t* cairo_face = cairo_ft_font_face_create_for_ft_face(free_type_face->face, 0);
        auto status = cairo_font_face_status(cairo_face);
        if (status == CAIRO_STATUS_SUCCESS)
            status = cairo_font_face_set_user_data(cairo_face, &free_type_face_key, free_type_face.get(), destroyFreeTypeFace);
        if (status)
        {
            cairo_font_face_destroy(cairo_face);
            throw std::runtime_error("error creating cairo font face " + name);
        }

        face.ft_face = free_type_face->face;
        face.cairo_face = cairo_face;
        free_type_face.release();
    }

    void RenderFontFaceManager::_loadCustomFontFaces(const PtrArray<RenderFont>& fonts)
    {
        for (int i = 0; i < fonts.size(); ++i)
        {
            const RenderFont& font = fonts[i];
            FreeTypeFace free_type_face(_library, font.data->data(), font.data->size(), font.data, font.name);

            if (FT_HAS_MULTIPLE_MASTERS(free_type_face.face))
            {
                _loadVariableFontFaceVariants(font, free_type_face.face);
                continue;
            }

            Face& face = _custom_faces.emplace(font);
            _loadFontFace(face, face.data->data(), face.data->size(), face.name);
            face.bold = (face.ft_face->style_flags & FT_STYLE_FLAG_BOLD) != 0;
            face.italic = (face.ft_face->style_flags & FT_STYLE_FLAG_ITALIC) != 0;
        }
    }

    void RenderFontFaceManager::_loadVariableFontFaceVariants(const RenderFont& font, FT_Face ft_face)
    {
        FT_MM_Var* mm_var = nullptr;
        if (FT_Get_MM_Var(ft_face, &mm_var))
            throw std::runtime_error("error reading variable font " + font.name);
        FreeTypeMMVar mm_var_holder(_library.get(), mm_var);

        int weight_axis = -1;
        int italic_axis = -1;
        std::vector<FT_Fixed> default_coordinates(mm_var->num_axis);
        for (FT_UInt i = 0; i < mm_var->num_axis; ++i)
        {
            const FT_Var_Axis& axis = mm_var->axis[i];
            default_coordinates[i] = axis.def;
            if (axis.tag == WEIGHT_AXIS_TAG)
                weight_axis = i;
            else if (axis.tag == ITALIC_AXIS_TAG)
                italic_axis = i;
            else if (axis.tag == SLANT_AXIS_TAG && italic_axis == -1)
                italic_axis = i;
        }

        const bool source_bold = (ft_face->style_flags & FT_STYLE_FLAG_BOLD) != 0;
        const bool source_italic = (ft_face->style_flags & FT_STYLE_FLAG_ITALIC) != 0;
        const int bold_style_count = weight_axis == -1 ? 1 : 2;
        const int italic_style_count = italic_axis == -1 ? 1 : 2;
        for (int bold_index = 0; bold_index < bold_style_count; ++bold_index)
        {
            for (int italic_index = 0; italic_index < italic_style_count; ++italic_index)
            {
                std::vector<FT_Fixed> coordinates(default_coordinates);
                if (weight_axis != -1)
                    coordinates[weight_axis] = clampAxisValue(bold_index ? BOLD_WEIGHT : REGULAR_WEIGHT, mm_var->axis[weight_axis]);
                if (italic_axis != -1)
                    coordinates[italic_axis] = italic_index ? getItalicAxisValue(mm_var->axis[italic_axis]) : getUprightAxisValue(mm_var->axis[italic_axis]);

                Face& variable_face = _custom_faces.emplace(font);
                _loadFontFace(variable_face, variable_face.data->data(), variable_face.data->size(), variable_face.name, &coordinates);
                variable_face.bold = weight_axis == -1 ? source_bold : bold_index != 0;
                variable_face.italic = italic_axis == -1 ? source_italic : italic_index != 0;
            }
        }
    }

    void RenderFontFaceManager::_initFreeType()
    {
        FT_Library library;
        if (FT_Init_FreeType(&library))
            throw std::runtime_error("error loading freetype");

        _library = std::shared_ptr<FT_LibraryRec_>(library, [](FT_Library library) { FT_Done_FreeType(library); });
    }
} // namespace indigo
