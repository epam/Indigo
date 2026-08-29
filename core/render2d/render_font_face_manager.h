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

#include "font_lang_detector.h"
#include "render_common.h"

#include <cairo-ft.h>
#include <freetype/freetype.h>

#include <memory>
#include <string>
namespace indigo
{
    class RenderFontFaceManager
    {
        struct Face
        {
            FT_Face ft_face = nullptr;
            cairo_font_face_t* cairo_face = nullptr;
            std::unique_ptr<cairo_user_data_key_t> key;

            Face()
            {
                key = std::make_unique<cairo_user_data_key_t>();
            }
        };

    private:
        FT_Library _library;

        Face _face_regular;
        Face _face_italic;
        Face _face_bold;
        Face _face_bold_italic;

#ifdef RENDER_ENABLE_CJK
        FontLangDetector _lang_detector;
        Face _face_cjk_regular;
        Face _face_cjk_bold;
#endif

        void _loadFontFaces();
        void _loadFontFace(FT_Library library, Face* face, const unsigned char font[], int font_size, const std::string& name);

    public:
        RenderFontFaceManager();
        ~RenderFontFaceManager();

        RenderFontFaceManager(const RenderFontFaceManager&) = delete;
        RenderFontFaceManager& operator=(const RenderFontFaceManager&) = delete;

        RenderFontFaceManager(RenderFontFaceManager&&) = delete;
        RenderFontFaceManager& operator=(RenderFontFaceManager&&) = delete;

        cairo_font_face_t* selectCairoFontFace(const TextItem& ti);
    };
} // namespace indigo