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

void RenderContext::backendCheckStatus() const
{
#ifdef DEBUG
    // Status checking is handled internally by the backend
#endif
}

void RenderContext::fontsClear()
{
    // scaled fonts managed by backend

    // font faces managed by backend

    // matrix init managed by backend
    backendCheckStatus();
    
    backendCheckStatus();
}

void RenderContext::fontsInit()
{
    fontsDispose();
    _backend->createFontOptions();
    _backend->setFontOptionsAntialias(0);
    _backend->applyFontOptions();
}

void RenderContext::fontsDispose()
{
    _backend->destroyFontOptions();
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
    _backend->selectFontFace(_fontfamily.ptr(), ti.italic, ti.bold);
    _backend->setFontSize(ti.size > 0 ? ti.size : fontGetSize(ti.fontsize));
}
#else
void RenderContext::fontsSetFont(const TextItem& ti)
{
    // Font manager uses cairo directly - only available on desktop
#ifndef __EMSCRIPTEN__
    std::lock_guard<std::mutex> _lock(_mutex);
    cairo_font_face_t* _cairo_face = _font_face_manager.selectCairoFontFace(ti);
    auto* cairoBackend = dynamic_cast<CairoRenderBackend*>(_backend.get());
    if (cairoBackend) {
        cairo_set_font_face(cairoBackend->getCr(), _cairo_face);
        cairo_set_font_size(cairoBackend->getCr(), ti.size > 0 ? ti.size : fontGetSize(ti.fontsize));
    }
#else
    _backend->selectFontFace(_fontfamily.ptr(), ti.italic, ti.bold);
    _backend->setFontSize(ti.size > 0 ? ti.size : fontGetSize(ti.fontsize));
#endif
}
#endif

void RenderContext::fontsGetTextExtents(const char* text, int /*size*/, float& dx, float& dy, float& rx, float& ry)
{
    float x_bearing, y_bearing;
    _backend->textExtents(text, dx, dy, x_bearing, y_bearing);
    rx = -x_bearing;
    ry = -y_bearing;
}

float RenderContext::getSpaceWidth()
{
    float w1, h1, xb1, yb1, w2, h2, xb2, yb2;
    _backend->textExtents(". .", w1, h1, xb1, yb1);
    _backend->textExtents("..", w2, h2, xb2, yb2);
    return w1 - w2;
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
        _backend->moveTo(ti.bbp.x, ti.bbp.y);
        _backend->rect(ti.bbp.x, ti.bbp.y, ti.bbsz.x, ti.bbsz.y);
        bbIncludePath(false);
        return;
    }

    setSingleSource(color);
    moveTo(ti.bbp);
    RenderMatrix m;
    _backend->getMatrix(m.m);
    float scale = (float)m.m[0];
    double v = scale * (ti.size > 0 ? ti.size : fontGetSize(ti.fontsize));
    if (opt.mode != MODE_PDF && opt.mode != MODE_SVG && v < 1.5)
    {
        _backend->rect(ti.bbp.x + ti.bbsz.x / 4, ti.bbp.y + ti.bbsz.y / 4, ti.bbsz.x / 2, ti.bbsz.y / 2);
        bbIncludePath(false);
        _backend->setLineWidth(_settings.unit / 2);
        _backend->stroke();
        return;
    }
    moveToRel(ti.relpos);

    {
        std::lock_guard<std::mutex> _lock(_mutex);
        _backend->textPath(ti.text.ptr());
    }

    bbIncludePath(false);
    _backend->beginPath();
    moveTo(ti.bbp);
    moveToRel(ti.relpos);
    if (metafileFontsToCurves)
    { // TODO: remove
        {
            std::lock_guard<std::mutex> _lock(_mutex);
            _backend->textPath(ti.text.ptr());
        }

        backendCheckStatus();
        _backend->fill();
        backendCheckStatus();
    }
    else
    {
        std::lock_guard<std::mutex> _lock(_mutex);
        _backend->showText(ti.text.ptr());
        backendCheckStatus();
    }
}
