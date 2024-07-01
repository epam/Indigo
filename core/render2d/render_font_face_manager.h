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

#include "render_common.h"

#include <cairo-ft.h>
#include <freetype/freetype.h>

#include <memory>
#include <string>
namespace indigo
{

#ifdef RENDER_ENABLE_CJK
    class CharacterRange
    {
    private:
        int _first;
        int _last;

    public:
        bool isInRange(int id) const
        {
            return id >= _first && id <= _last;
        }
        CharacterRange(int first, int last) : _first(first), _last(last)
        {
        }
        ~CharacterRange()
        {
        }
    };

    class FontRanges
    {
    private:
        std::vector<CharacterRange> _ranges;

    public:
        void addRange(const CharacterRange& range)
        {
            _ranges.push_back(range);
        }

        bool isInRanges(int idx)
        {
            for (const auto& r : _ranges)
            {
                if (r.isInRange(idx))
                {
                    return true;
                }
            }
            return false;
        }

        FontRanges()
        {
        }
        ~FontRanges()
        {
        }
    };

    enum FONT_LANG
    {
        NO_CJK,
        CJK,
        KOREAN,
        JAPANESE
    };

    class FontLangDetector
    {
    private:
        FontRanges _j_ranges;          // japan
        FontRanges _k_ranges;          // korean
        FontRanges _cjk_common_ranges; // common_cjk_ranges

        void prepareRanges()
        {
            CharacterRange CJK_Unified_Ideographs(0x4E00, 0x9FFF);             // 一丁
            CharacterRange CJK_Compatibility_Ideographs(0xF900, 0xFAFF);       // 豈類
            CharacterRange CJK_Unified_Ideographs_Extension_A(0x3400, 0x4DBF); // 㐀㐁

            _cjk_common_ranges.addRange(CJK_Unified_Ideographs);
            _cjk_common_ranges.addRange(CJK_Compatibility_Ideographs);
            _cjk_common_ranges.addRange(CJK_Unified_Ideographs_Extension_A);

            // Korean
            // https://en.wikipedia.org/wiki/Hangul
            // U+AC00-U+D7AF Hangul Syllables
            // U+1100–U+11FF Hangul Jamo
            // U+3130–U+318F Hangul Compatibility Jamo
            // U+A960–U+A97F Hangul Jamo Extended-A
            // U+D7B0–U+D7FF Hangul Jamo Extended-B
            // TODO: Hangul subset of Enclosed CJK Letters and Months
            // TODO: Hangul subset of Halfwidth and Fullwidth Forms
            CharacterRange Hangul_Syllables(0xAC00, 0xD7AF);          // 가각
            CharacterRange Hangul_Jamo(0x1100, 0x11FF);               // ᄀᄁ
            CharacterRange Hangul_Compatibility_Jamo(0x3130, 0x318F); // ㄱㄲ
            CharacterRange Hangul_Jamo_Extended_A(0xA960, 0xA97F);    // ꥠꥡ
            CharacterRange Hangul_Jamo_Extended_B(0xD7B0, 0xD7FF);    // ힰힱ

            _k_ranges.addRange(Hangul_Syllables);
            _k_ranges.addRange(Hangul_Jamo);
            _k_ranges.addRange(Hangul_Compatibility_Jamo);

            // Japan
            // https://en.wikipedia.org/wiki/Japanese_writing_system
            // U+4E00–U+9FBF Kanji - same range as CJK_Unified_Ideographs
            // U+3040–U+309F Hiragana
            // U+30A0–U+30FF Katakana
            CharacterRange Hiragana(0x3040, 0x309F); // ぁあ
            CharacterRange Katakana(0x30A0, 0x30FF); // ゠ァジ

            _j_ranges.addRange(Hiragana);
            _j_ranges.addRange(Katakana);
        }

        // TODO: use wstring_convert instead of custom converter
        std::vector<unsigned int> utf8_indexes(const TextItem& ti)
        {
            std::vector<unsigned int> indexes;
            for (unsigned int i = 0; i < ti.text.size();)
            {
                unsigned int utf8_char = static_cast<unsigned char>(ti.text[i]);
                ++i;
                if (utf8_char >= 0x80)
                {
                    int extra_bytes = 0;
                    if ((utf8_char & 0xE0) == 0xC0)
                    {
                        extra_bytes = 1;
                        utf8_char &= 0x1F;
                    }
                    else if ((utf8_char & 0xF0) == 0xE0)
                    {
                        extra_bytes = 2;
                        utf8_char &= 0x0F;
                    }
                    else if ((utf8_char & 0xF8) == 0xF0)
                    {
                        extra_bytes = 3;
                        utf8_char &= 0x07;
                    }
                    while (extra_bytes && i < ti.text.size())
                    {
                        unsigned char cont_byte = static_cast<unsigned char>(ti.text[i]);
                        if ((cont_byte & 0xC0) != 0x80)
                        {
                            break;
                        }
                        utf8_char = (utf8_char << 6) | (cont_byte & 0x3F);
                        ++i;
                        --extra_bytes;
                    }
                }
                indexes.push_back(utf8_char);
            }
            return indexes;
        }

    public:
        FontLangDetector()
        {
            prepareRanges();
        };
        ~FontLangDetector(){};

        FONT_LANG detectLang(const TextItem& ti)
        {
            if (ti.text.size() == 0)
            {
                return FONT_LANG::NO_CJK;
            }

            std::vector<unsigned int> indexes = utf8_indexes(ti);

            for (auto idx : indexes)
            {
                if (_k_ranges.isInRanges(idx))
                {
                    return FONT_LANG::KOREAN;
                }
            }

            for (auto idx : indexes)
            {
                if (_j_ranges.isInRanges(idx))
                {
                    return FONT_LANG::JAPANESE;
                }
            }

            for (auto idx : indexes)
            {
                if (_cjk_common_ranges.isInRanges(idx))
                {
                    return FONT_LANG::CJK;
                }
            }

            return FONT_LANG::NO_CJK;
        }
    };
#endif

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