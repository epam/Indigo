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

#include "sans_bold.h"
#include "sans_bold_italic.h"
#include "sans_italic.h"
#include "sans_regular.h"

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
        cairo_font_face_destroy(_cairo_face_regular);
        cairo_font_face_destroy(_cairo_face_bold);
        cairo_font_face_destroy(_cairo_face_italic);
        cairo_font_face_destroy(_cairo_face_bold_italic);
#endif
    }

    cairo_font_face_t* RenderFontFaceManager::selectCairoFontFace(bool is_bold, bool is_italic)
    {
        if (is_bold && is_italic)
        {
            return _cairo_face_bold_italic;
        }
        else if (is_bold)
        {
            return _cairo_face_bold;
        }
        else if (is_italic)
        {
            return _cairo_face_italic;
        }
        else
        {
            return _cairo_face_regular;
        }
    }

    void RenderFontFaceManager::_loadFontFace(FT_Library library, FT_Face* face, cairo_font_face_t** cairo_face, const cairo_user_data_key_t* key,
                                              const unsigned char font[], int font_size, const std::string& name)
    {
        int error = FT_New_Memory_Face(library, font, font_size, 0, face);
        if (error)
        {
            throw std::runtime_error("error loading font regular");
        }

        *cairo_face = cairo_ft_font_face_create_for_ft_face(*face, 0);
        auto status = cairo_font_face_set_user_data(*cairo_face, key, *face, (cairo_destroy_func_t)FT_Done_Face);
        if (status)
        {
            cairo_font_face_destroy(*cairo_face);
            FT_Done_Face(*face);
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

        static const cairo_user_data_key_t key = {0};
        _loadFontFace(_library, &_face_regular, &_cairo_face_regular, &key, sans_regular, sans_regular_size, "regular");

        static const cairo_user_data_key_t key1 = {0};
        _loadFontFace(_library, &_face_bold, &_cairo_face_bold, &key1, sans_bold, sans_bold_size, "bold");

        static const cairo_user_data_key_t key2 = {0};
        _loadFontFace(_library, &_face_italic, &_cairo_face_italic, &key2, sans_italic, sans_italic_size, "italic");

        static const cairo_user_data_key_t key3 = {0};
        _loadFontFace(_library, &_face_bold_italic, &_cairo_face_bold_italic, &key3, sans_bold_italic, sans_bold_italic_size, "bold italic");
    }
} // namespace indigo