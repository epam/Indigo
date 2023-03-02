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

#include "base_cpp/output.h"
#include "math/algebra.h"
#include "render_context.h"

#ifdef _WIN32
#include <windows.h>
#endif

using namespace indigo;

void RenderContext::cairoCheckStatus() const
{
#ifdef DEBUG
    cairo_status_t s;
    if (_cr)
    {
        s = cairo_status(_cr);
        if (s != CAIRO_STATUS_SUCCESS /*&& s <= CAIRO_STATUS_INVALID_WEIGHT*/)
            throw Error("Cairo error: %i -- %s\n", s, cairo_status_to_string(s));
    }
#endif
}

void RenderContext::fontsClear()
{
    memset(_scaled_fonts, 0, FONT_SIZE_COUNT * 2 * sizeof(cairo_scaled_font_t*));

    cairoFontFaceRegular = NULL;
    cairoFontFaceBold = NULL;
    fontOptions = NULL;

    cairo_matrix_init_identity(&fontCtm);
    cairoCheckStatus();
    cairo_matrix_init_identity(&fontScale);
    cairoCheckStatus();
}

void RenderContext::fontsInit()
{
    fontsDispose();
    fontOptions = cairo_font_options_create();
    cairoCheckStatus();
    cairo_font_options_set_antialias(fontOptions, CAIRO_ANTIALIAS_GRAY);
    cairoCheckStatus();
    cairo_set_font_options(_cr, fontOptions);
    cairoCheckStatus();
}

void RenderContext::fontsDispose()
{
    for (int i = 0; i < FONT_SIZE_COUNT * 2; ++i)
    {
        if (_scaled_fonts[i] != NULL)
        {
            cairo_scaled_font_destroy(_scaled_fonts[i]);
            cairoCheckStatus();
        }
    }
    if (cairoFontFaceRegular != NULL)
    {
        cairo_font_face_destroy(cairoFontFaceRegular);
        cairoCheckStatus();
    }
    if (cairoFontFaceBold != NULL)
    {
        cairo_font_face_destroy(cairoFontFaceBold);
        cairoCheckStatus();
    }

    if (fontOptions != NULL)
    {
        cairo_font_options_destroy(fontOptions);
        cairoCheckStatus();
    }
    fontsClear();
}

double RenderContext::fontGetSize(FONT_SIZE size)
{
    if (size == FONT_SIZE_COMMENT)
        return opt.commentFontFactor;
    if (size == FONT_SIZE_TITLE)
        return opt.titleFontFactor;
    return _settings.fzz[size];
}

#ifndef RENDER_USE_FONT_MANAGER
void RenderContext::fontsSetFont(const TextItem& ti)
{
    std::lock_guard<std::mutex> _lock(_cairo_mutex);
    cairo_select_font_face(_cr, _fontfamily.ptr(), ti.italic ? CAIRO_FONT_SLANT_ITALIC : CAIRO_FONT_SLANT_NORMAL,
                           ti.bold ? CAIRO_FONT_WEIGHT_BOLD : CAIRO_FONT_WEIGHT_NORMAL);

    cairoCheckStatus();
    cairo_set_font_size(_cr, ti.size > 0 ? ti.size : fontGetSize(ti.fontsize));
    cairoCheckStatus();
}
#else
void RenderContext::fontsSetFont(const TextItem& ti)
{
    std::lock_guard<std::mutex> _lock(_cairo_mutex);

    cairo_font_face_t* _cairo_face = _font_face_manager.selectCairoFontFace(ti);
    cairoCheckStatus();

    cairo_set_font_face(_cr, _cairo_face);
    cairoCheckStatus();
    cairo_set_font_size(_cr, ti.size > 0 ? ti.size : fontGetSize(ti.fontsize));
    cairoCheckStatus();
}
#endif

void RenderContext::fontsGetTextExtents(cairo_t* cr, const char* text, int size, float& dx, float& dy, float& rx, float& ry)
{
    std::lock_guard<std::mutex> _lock(_cairo_mutex);
    cairo_text_extents_t te;
    cairo_text_extents(cr, text, &te);
    cairoCheckStatus();

    dx = (float)te.width;
    dy = (float)te.height;
    rx = (float)-te.x_bearing;
    ry = (float)-te.y_bearing;
}

void RenderContext::fontsDrawText(const TextItem& ti, const Vec3f& color, bool idle)
{
    /*
     * cairo treats all surfaces as bounded and drops glyphs from a rendering
     * path if they don't belong to the surface yet, making it difficult to
     * calculate a desired surface size, which would include all elements of
     * a chemical structure being rendered.
     * Due to this limitation, we cannot calculate a size of the surface for
     * "real" rendering.
     *
     * If idle, we don't render glyphs for the text item. Rather, we calculate
     * the bounding rectangle for the text item and add it to the cairo path.
     * Later, glyphs will be rendered in a usual way during a "real" stage.
     *
     * This also saves resources, as we don't make heavyweight glyph rendering
     * twice.
     */
    if (idle)
    {
        cairo_move_to(_cr, ti.bbp.x, ti.bbp.y);
        cairo_rectangle(_cr, ti.bbp.x, ti.bbp.y, ti.bbsz.x, ti.bbsz.y);
        bbIncludePath(false);
        return;
    }

    setSingleSource(color);
    moveTo(ti.bbp);
    cairo_matrix_t m;
    cairo_get_matrix(_cr, &m);
    float scale = (float)m.xx;
    double v = scale * (ti.size > 0 ? ti.size : fontGetSize(ti.fontsize));
    if (opt.mode != MODE_PDF && opt.mode != MODE_SVG && v < 1.5)
    {
        cairo_rectangle(_cr, ti.bbp.x + ti.bbsz.x / 4, ti.bbp.y + ti.bbsz.y / 4, ti.bbsz.x / 2, ti.bbsz.y / 2);
        bbIncludePath(false);
        cairo_set_line_width(_cr, _settings.unit / 2);
        cairo_stroke(_cr);
        return;
    }
    moveToRel(ti.relpos);

    {
        std::lock_guard<std::mutex> _lock(_cairo_mutex);
        cairo_text_path(_cr, ti.text.ptr());
    }

    bbIncludePath(false);
    cairo_new_path(_cr);
    moveTo(ti.bbp);
    moveToRel(ti.relpos);

    if (metafileFontsToCurves)
    { // TODO: remove
        {
            std::lock_guard<std::mutex> _lock(_cairo_mutex);
            cairo_text_path(_cr, ti.text.ptr());
        }

        cairoCheckStatus();
        cairo_fill(_cr);
        cairoCheckStatus();
    }
    else
    {
        std::lock_guard<std::mutex> _lock(_cairo_mutex);
        cairo_show_text(_cr, ti.text.ptr());
        cairoCheckStatus();
    }
}
