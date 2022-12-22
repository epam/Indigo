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
#include <cairo-ft.h>
#include <freetype/freetype.h>

#include <string>

namespace indigo
{
    class RenderFontFaceManager
    {
    private:
        FT_Library  _library;
        FT_Face     _face_regular;
        FT_Face     _face_italic;
        FT_Face     _face_bold;
        FT_Face     _face_bold_italic;

        cairo_font_face_t* _cairo_face_regular = nullptr;
        cairo_font_face_t* _cairo_face_bold = nullptr;
        cairo_font_face_t* _cairo_face_italic = nullptr;
        cairo_font_face_t* _cairo_face_bold_italic = nullptr;

        void _loadFontFaces();
        void _loadFontFace( FT_Library  library, 
                            FT_Face* face,
                            cairo_font_face_t** cairo_face,
                            const cairo_user_data_key_t *key,
                            const unsigned char font[],
                            int font_size,
                            const std::string &name);
    public:
        RenderFontFaceManager();
        ~RenderFontFaceManager();

        cairo_font_face_t* selectCairoFontFace(bool is_bold, bool is_italic);
    };
} // namespace indigo