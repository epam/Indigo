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

#ifndef __render_svg_ids_h__
#define __render_svg_ids_h__

#include <string>

namespace indigo
{
    // Cairo's SVG backend numbers glyphs, gradients, masks, filters and patterns from zero in every
    // document, so two SVGs inlined into one page collide. These helpers make the ids unique.

    // "<hex>-" where <hex> = crc32(svg) + crc32(current time and a process-wide counter).
    // The same drawing rendered twice still gets different prefixes.
    std::string makeSvgIdPrefix(const std::string& svg);

    // Parses cairo's SVG output and rewrites every id="x" to id="<prefix>x", together with every
    // href="#x" / xlink:href="#x" and url(#x) that refers to it. Throws on malformed XML.
    std::string prefixSvgIds(const std::string& svg, const std::string& prefix);
}

#endif
