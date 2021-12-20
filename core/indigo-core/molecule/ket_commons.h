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

#ifndef __ket_commons_h__
#define __ket_commons_h__

#include "common/math/algebra.h"
#include "graph/graph.h"

namespace indigo
{
    constexpr std::uint32_t string_hash(char const* s, std::size_t count)
    {
        return ((count ? string_hash(s, count - 1) : 2166136261u) ^ s[count]) * 16777619u;
    }

    constexpr std::uint32_t operator"" _hash(char const* s, std::size_t count)
    {
        return string_hash(s, count);
    }

    class KETSimpleObject : public GraphMetaObject
    {
    public:
        static const std::uint32_t cid = "KET simple object"_hash;
        KETSimpleObject(int mode, const Rect2f& rect) : GraphMetaObject(cid)
        {
            _mode = mode;
            _rect = rect;
        };

        GraphMetaObject* clone() const override
        {
            return new KETSimpleObject(_mode, _rect);
        }

        enum
        {
            EKETEllipse,
            EKETRectangle,
            EKETLine
        };
        int _mode;
        Rect2f _rect;
    };

    class KETTextObject : public GraphMetaObject
    {
    public:
        static const std::uint32_t cid = "KET text object"_hash;

        KETTextObject(const Vec3f& pos, const std::string& content) : GraphMetaObject(cid)
        {
            _pos = pos;
            _content = content;
        }

        GraphMetaObject* clone() const override
        {
            return new KETTextObject(_pos, _content);
        }

        struct KETTextStyle
        {
            int _offset;
            int _size;
            bool _italic;
            bool _bold;
            bool _subscript;
            bool _superscript;
            int _font_size;
        };

        struct KETTextLine
        {
            std::string _text;
            std::list<KETTextStyle> _inline_styles;
        };

        std::string _content;
        std::list<KETTextLine> _block;
        Vec3f _pos;
    };
}
#endif
