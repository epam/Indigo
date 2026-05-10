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

#ifndef __EMSCRIPTEN__

#include "cairo_render_backend.h"
#include "base_cpp/output.h"

#include <cstring>

using namespace indigo;

std::mutex CairoRenderBackend::_mutex;

CairoRenderBackend::CairoRenderBackend() : _cr(nullptr), _surface(nullptr), _pngImage(nullptr), _pattern(nullptr), _fontOptions(nullptr)
{
}

CairoRenderBackend::~CairoRenderBackend()
{
    if (_pattern)
        cairo_pattern_destroy(_pattern);
    if (_fontOptions)
        cairo_font_options_destroy(_fontOptions);
    if (_cr)
        cairo_destroy(_cr);
    if (_surface)
        cairo_surface_destroy(_surface);
}

// ---- writer callback ----

cairo_status_t CairoRenderBackend::_writer(void* closure, const unsigned char* data, unsigned int length)
{
    try
    {
        ((Output*)closure)->write(data, length);
    }
    catch (Output::Error&)
    {
        return CAIRO_STATUS_WRITE_ERROR;
    }
    return CAIRO_STATUS_SUCCESS;
}

// ---- Surface lifecycle ----

void CairoRenderBackend::createSurface(int mode, int width, int height, void* output)
{
    std::lock_guard<std::mutex> lock(_mutex);
    switch (mode)
    {
    case RBMODE_PDF:
        _surface = cairo_pdf_surface_create_for_stream(_writer, output, width, height);
        break;
    case RBMODE_SVG:
        _surface = cairo_svg_surface_create_for_stream(_writer, output, width, height);
        break;
    case RBMODE_PNG:
        _surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
        break;
    default:
        break; // HDC/PRN/EMF handled separately in RenderContext
    }
}

void CairoRenderBackend::closeSurface(int mode, bool discard, void* output)
{
    if (mode == RBMODE_PNG && !discard && _surface)
    {
        cairo_surface_write_to_png_stream(_surface, _writer, output);
    }

    if (_surface)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        cairo_surface_destroy(_surface);
        _surface = nullptr;
    }
}

void CairoRenderBackend::createContext()
{
    _cr = cairo_create(_surface);
}

void CairoRenderBackend::destroyContext()
{
    if (_cr)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        cairo_destroy(_cr);
        _cr = nullptr;
    }
}

// ---- Path operations ----

void CairoRenderBackend::beginPath()
{
    cairo_new_path(_cr);
}

void CairoRenderBackend::closePath()
{
    cairo_close_path(_cr);
}

void CairoRenderBackend::moveTo(float x, float y)
{
    cairo_move_to(_cr, x, y);
}

void CairoRenderBackend::lineTo(float x, float y)
{
    cairo_line_to(_cr, x, y);
}

void CairoRenderBackend::relMoveTo(float dx, float dy)
{
    cairo_rel_move_to(_cr, dx, dy);
}

void CairoRenderBackend::relLineTo(float dx, float dy)
{
    cairo_rel_line_to(_cr, dx, dy);
}

void CairoRenderBackend::curveTo(float x1, float y1, float x2, float y2, float x3, float y3)
{
    cairo_curve_to(_cr, x1, y1, x2, y2, x3, y3);
}

void CairoRenderBackend::arc(float cx, float cy, float r, float a0, float a1)
{
    cairo_arc(_cr, cx, cy, r, a0, a1);
}

void CairoRenderBackend::arcNegative(float cx, float cy, float r, float a0, float a1)
{
    cairo_arc_negative(_cr, cx, cy, r, a0, a1);
}

void CairoRenderBackend::rect(float x, float y, float w, float h)
{
    cairo_rectangle(_cr, x, y, w, h);
}

// ---- Drawing operations ----

void CairoRenderBackend::fill()
{
    cairo_fill(_cr);
}

void CairoRenderBackend::stroke()
{
    cairo_stroke(_cr);
}

void CairoRenderBackend::paint()
{
    cairo_paint(_cr);
}

// ---- Style ----

void CairoRenderBackend::setSourceRGB(float r, float g, float b)
{
    cairo_set_source_rgb(_cr, r, g, b);
}

void CairoRenderBackend::setSourceRGBA(float r, float g, float b, float a)
{
    cairo_set_source_rgba(_cr, r, g, b, a);
}

void CairoRenderBackend::setLineWidth(float w)
{
    cairo_set_line_width(_cr, w);
}

void CairoRenderBackend::setLineJoin(int join)
{
    cairo_line_join_t j;
    switch (join)
    {
    case 0:
        j = CAIRO_LINE_JOIN_MITER;
        break;
    case 1:
        j = CAIRO_LINE_JOIN_ROUND;
        break;
    default:
        j = CAIRO_LINE_JOIN_BEVEL;
        break;
    }
    cairo_set_line_join(_cr, j);
}

void CairoRenderBackend::setDash(const double* pattern, int count, double offset)
{
    cairo_set_dash(_cr, pattern, count, offset);
}

void CairoRenderBackend::setOperator(int op)
{
    cairo_set_operator(_cr, (cairo_operator_t)op);
}

void CairoRenderBackend::setAntialias(int mode)
{
    cairo_set_antialias(_cr, (cairo_antialias_t)mode);
}

// ---- Transform ----

void CairoRenderBackend::save()
{
    cairo_save(_cr);
}

void CairoRenderBackend::restore()
{
    cairo_restore(_cr);
}

void CairoRenderBackend::translate(float dx, float dy)
{
    cairo_translate(_cr, dx, dy);
}

void CairoRenderBackend::scale(float sx, float sy)
{
    cairo_scale(_cr, sx, sy);
}

void CairoRenderBackend::rotate(float angle)
{
    cairo_rotate(_cr, angle);
}

void CairoRenderBackend::getMatrix(double m[6])
{
    cairo_matrix_t mt;
    cairo_get_matrix(_cr, &mt);
    m[0] = mt.xx;
    m[1] = mt.yx;
    m[2] = mt.xy;
    m[3] = mt.yy;
    m[4] = mt.x0;
    m[5] = mt.y0;
}

void CairoRenderBackend::setMatrix(const double m[6])
{
    cairo_matrix_t mt;
    mt.xx = m[0];
    mt.yx = m[1];
    mt.xy = m[2];
    mt.yy = m[3];
    mt.x0 = m[4];
    mt.y0 = m[5];
    cairo_set_matrix(_cr, &mt);
}

void CairoRenderBackend::initIdentityMatrix(double m[6])
{
    cairo_matrix_t mt;
    cairo_matrix_init_identity(&mt);
    m[0] = mt.xx;
    m[1] = mt.yx;
    m[2] = mt.xy;
    m[3] = mt.yy;
    m[4] = mt.x0;
    m[5] = mt.y0;
}

void CairoRenderBackend::userToDevice(double& x, double& y)
{
    cairo_user_to_device(_cr, &x, &y);
}

void CairoRenderBackend::deviceToUser(double& x, double& y)
{
    cairo_device_to_user(_cr, &x, &y);
}

// ---- Extents ----

void CairoRenderBackend::strokeExtents(double& x1, double& y1, double& x2, double& y2)
{
    cairo_stroke_extents(_cr, &x1, &y1, &x2, &y2);
}

void CairoRenderBackend::pathExtents(double& x1, double& y1, double& x2, double& y2)
{
    cairo_path_extents(_cr, &x1, &y1, &x2, &y2);
}

// ---- Text ----

void CairoRenderBackend::selectFontFace(const char* family, bool italic, bool bold)
{
    std::lock_guard<std::mutex> lock(_mutex);
    cairo_select_font_face(_cr, family, italic ? CAIRO_FONT_SLANT_ITALIC : CAIRO_FONT_SLANT_NORMAL, bold ? CAIRO_FONT_WEIGHT_BOLD : CAIRO_FONT_WEIGHT_NORMAL);
}

void CairoRenderBackend::setFontSize(float size)
{
    cairo_set_font_size(_cr, size);
}

void CairoRenderBackend::textExtents(const char* text, float& width, float& height, float& x_bearing, float& y_bearing)
{
    std::lock_guard<std::mutex> lock(_mutex);
    cairo_text_extents_t te;
    cairo_text_extents(_cr, text, &te);
    width = (float)te.width;
    height = (float)te.height;
    x_bearing = (float)te.x_bearing;
    y_bearing = (float)te.y_bearing;
}

void CairoRenderBackend::fontExtents(double& height)
{
    cairo_font_extents_t fe;
    cairo_font_extents(_cr, &fe);
    height = fe.height;
}

void CairoRenderBackend::showText(const char* text)
{
    std::lock_guard<std::mutex> lock(_mutex);
    cairo_show_text(_cr, text);
}

void CairoRenderBackend::textPath(const char* text)
{
    std::lock_guard<std::mutex> lock(_mutex);
    cairo_text_path(_cr, text);
}

// ---- Font options ----

void CairoRenderBackend::createFontOptions()
{
    _fontOptions = cairo_font_options_create();
}

void CairoRenderBackend::destroyFontOptions()
{
    if (_fontOptions)
    {
        cairo_font_options_destroy(_fontOptions);
        _fontOptions = nullptr;
    }
}

void CairoRenderBackend::setFontOptionsAntialias(int mode)
{
    if (_fontOptions)
        cairo_font_options_set_antialias(_fontOptions, (cairo_antialias_t)mode);
}

void CairoRenderBackend::applyFontOptions()
{
    if (_fontOptions && _cr)
        cairo_set_font_options(_cr, _fontOptions);
}

// ---- Gradient ----

void CairoRenderBackend::setLinearGradient(float x0, float y0, float x1, float y1, float r1, float g1, float b1, float r2, float g2, float b2)
{
    if (_pattern)
    {
        cairo_pattern_destroy(_pattern);
        _pattern = nullptr;
    }
    _pattern = cairo_pattern_create_linear(x0, y0, x1, y1);
    cairo_pattern_add_color_stop_rgb(_pattern, 0, r1, g1, b1);
    cairo_pattern_add_color_stop_rgb(_pattern, 1, r2, g2, b2);
    cairo_set_source(_cr, _pattern);
}

void CairoRenderBackend::clearPattern()
{
    if (_pattern)
    {
        cairo_pattern_destroy(_pattern);
        _pattern = nullptr;
    }
}

// ---- Image ----

struct PngReadCtx
{
    const unsigned char* data;
    size_t size;
    size_t offset;
};

static cairo_status_t _pngReadFunc(void* closure, unsigned char* data, unsigned int length)
{
    PngReadCtx* ctx = static_cast<PngReadCtx*>(closure);
    if (ctx->offset + length > ctx->size)
        return CAIRO_STATUS_READ_ERROR;
    memcpy(data, ctx->data + ctx->offset, length);
    ctx->offset += length;
    return CAIRO_STATUS_SUCCESS;
}

void CairoRenderBackend::drawPngImage(const void* data, int dataLen, float x, float y, float w, float h)
{
    PngReadCtx ctx = {(const unsigned char*)data, (size_t)dataLen, 0};
    cairo_surface_t* image = cairo_image_surface_create_from_png_stream(_pngReadFunc, &ctx);
    if (cairo_surface_status(image) != CAIRO_STATUS_SUCCESS)
    {
        cairo_surface_destroy(image);
        return;
    }
    double imgW = cairo_image_surface_get_width(image);
    double imgH = cairo_image_surface_get_height(image);

    cairo_save(_cr);
    cairo_translate(_cr, x, y);
    cairo_scale(_cr, w / imgW, h / imgH);
    cairo_set_source_surface(_cr, image, 0, 0);
    cairo_paint(_cr);
    cairo_restore(_cr);
    cairo_surface_destroy(image);
}

void CairoRenderBackend::writeSurfaceToPng(void* output)
{
    if (_surface)
        cairo_surface_write_to_png_stream(_surface, _writer, output);
}

// ---- Path debugging ----

bool CairoRenderBackend::isPathEmpty()
{
    cairo_path_t* p = cairo_copy_path(_cr);
    bool empty = (p->num_data == 0);
    cairo_path_destroy(p);
    return empty;
}

// ---- Surface source ----

void CairoRenderBackend::setSourceSurface(float x, float y)
{
    if (_pngImage)
        cairo_set_source_surface(_cr, _pngImage, x, y);
}

#endif // !__EMSCRIPTEN__
