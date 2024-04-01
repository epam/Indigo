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

#include <cstdlib>

#include <stdexcept>

namespace indigo
{
    RenderFontFaceManager::RenderFontFaceManager()
    {
        _loadFontFaces();
    }

    RenderFontFaceManager::~RenderFontFaceManager()
    {
        // TODO: fix faces realease
#ifndef RENDER_EMSCRIPTEN
        // there is a function signature mismatch in _cairo_user_data_array_fini when using EMSCRIPTEN
        if (_face_regular.cairo_face)
            cairo_font_face_destroy(_face_regular.cairo_face);
        if (_face_bold.cairo_face)
            cairo_font_face_destroy(_face_bold.cairo_face);
        if (_face_italic.cairo_face)
            cairo_font_face_destroy(_face_italic.cairo_face);
        if (_face_bold_italic.cairo_face)
            cairo_font_face_destroy(_face_bold_italic.cairo_face);
#ifdef RENDER_ENABLE_CJK
        if (_face_cjk_regular.cairo_face)
            cairo_font_face_destroy(_face_cjk_regular.cairo_face);
        if (_face_cjk_bold.cairo_face)
            cairo_font_face_destroy(_face_cjk_bold.cairo_face);
#endif
#endif
    }

    cairo_font_face_t* RenderFontFaceManager::selectCairoFontFace(const TextItem& ti)
    {
        bool is_bold = ti.bold;
        bool is_italic = ti.italic;

#ifdef RENDER_ENABLE_CJK
        auto lang = _lang_detector.detectLang(ti);

        if (lang != FONT_LANG::NO_CJK)
        {
            if (is_bold)
            {
                if (!_face_cjk_bold.cairo_face)
                    _loadFontFace(_library, &_face_cjk_bold, sans_cjk_bold, sans_cjk_bold_size, "CJK bold");
                return _face_cjk_bold.cairo_face;
            }
            else
            {
                if (!_face_cjk_regular.cairo_face)
                    _loadFontFace(_library, &_face_cjk_regular, sans_cjk_regular, sans_cjk_regular_size, "CJK regular");
                return _face_cjk_regular.cairo_face;
            }
        }
#endif

        if (is_bold && is_italic)
        {
            if (!_face_bold_italic.cairo_face)
                _loadFontFace(_library, &_face_bold_italic, sans_bold_italic, sans_bold_italic_size, "bold italic");
            return _face_bold_italic.cairo_face;
        }
        else if (is_bold)
        {
            if (!_face_bold.cairo_face)
                _loadFontFace(_library, &_face_bold, sans_bold, sans_bold_size, "bold");
            return _face_bold.cairo_face;
        }
        else if (is_italic)
        {
            if (!_face_italic.cairo_face)
                _loadFontFace(_library, &_face_italic, sans_italic, sans_italic_size, "italic");
            return _face_italic.cairo_face;
        }
        else
        {
            if (!_face_regular.cairo_face)
                _loadFontFace(_library, &_face_regular, sans_regular, sans_regular_size, "regular");
            return _face_regular.cairo_face;
        }
    }

    void RenderFontFaceManager::_loadFontFace(FT_Library library, Face* face, const unsigned char font[], int font_size, const std::string& name)
    {
        int error = FT_New_Memory_Face(library, font, font_size, 0, &(face->ft_face));
        if (error)
        {
            throw std::runtime_error("error loading font regular");
        }

        face->cairo_face = cairo_ft_font_face_create_for_ft_face(face->ft_face, 0);
        auto status = cairo_font_face_set_user_data(face->cairo_face, face->key.get(), face->ft_face, (cairo_destroy_func_t)FT_Done_Face);
        if (status)
        {
            cairo_font_face_destroy(face->cairo_face);
            FT_Done_Face(face->ft_face);
            throw std::runtime_error("error creating cairo font face " + name);
        }
    }

    void RenderFontFaceManager::_loadFontFaces()
    {
        int error = FT_Init_FreeType(&_library);
        if (error)
        {
            throw std::runtime_error("error loading freetype");
        }
    }
} // namespace indigo