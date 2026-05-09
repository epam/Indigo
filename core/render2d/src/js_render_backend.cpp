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

#ifdef __EMSCRIPTEN__

#include "js_render_backend.h"
#include <emscripten.h>
#include <cstring>
#include <cmath>

using namespace indigo;

// ============================================================================
// EM_JS bindings: these call into JavaScript Canvas2D API
// ============================================================================

EM_JS(void, js_rb_createCanvas, (int w, int h), {
    if (typeof OffscreenCanvas !== 'undefined') {
        Module._rb_canvas = new OffscreenCanvas(w, h);
    } else {
        // Node.js fallback: create a simple object that tracks SVG commands
        Module._rb_canvas = { width: w, height: h };
    }
    Module._rb_ctx = Module._rb_canvas.getContext ? Module._rb_canvas.getContext('2d') : null;
    Module._rb_svgParts = [];
    Module._rb_width = w;
    Module._rb_height = h;
    Module._rb_mode = 0; // will be set
});

EM_JS(void, js_rb_destroyCanvas, (), {
    Module._rb_canvas = null;
    Module._rb_ctx = null;
    Module._rb_svgParts = null;
});

EM_JS(void, js_rb_beginPath, (), {
    if (Module._rb_ctx) Module._rb_ctx.beginPath();
});

EM_JS(void, js_rb_closePath, (), {
    if (Module._rb_ctx) Module._rb_ctx.closePath();
});

EM_JS(void, js_rb_moveTo, (float x, float y), {
    if (Module._rb_ctx) Module._rb_ctx.moveTo(x, y);
});

EM_JS(void, js_rb_lineTo, (float x, float y), {
    if (Module._rb_ctx) Module._rb_ctx.lineTo(x, y);
});

EM_JS(void, js_rb_curveTo, (float x1, float y1, float x2, float y2, float x3, float y3), {
    if (Module._rb_ctx) Module._rb_ctx.bezierCurveTo(x1, y1, x2, y2, x3, y3);
});

EM_JS(void, js_rb_arc, (float cx, float cy, float r, float a0, float a1, int ccw), {
    if (Module._rb_ctx) Module._rb_ctx.arc(cx, cy, r, a0, a1, !!ccw);
});

EM_JS(void, js_rb_rect, (float x, float y, float w, float h), {
    if (Module._rb_ctx) Module._rb_ctx.rect(x, y, w, h);
});

EM_JS(void, js_rb_fill, (), {
    if (Module._rb_ctx) Module._rb_ctx.fill();
});

EM_JS(void, js_rb_stroke, (), {
    if (Module._rb_ctx) Module._rb_ctx.stroke();
});

EM_JS(void, js_rb_paint, (), {
    if (Module._rb_ctx) {
        Module._rb_ctx.fillRect(0, 0, Module._rb_width, Module._rb_height);
    }
});

EM_JS(void, js_rb_setSourceRGB, (float r, float g, float b), {
    if (Module._rb_ctx) {
        var color = 'rgb(' + Math.round(r*255) + ',' + Math.round(g*255) + ',' + Math.round(b*255) + ')';
        Module._rb_ctx.fillStyle = color;
        Module._rb_ctx.strokeStyle = color;
    }
});

EM_JS(void, js_rb_setSourceRGBA, (float r, float g, float b, float a), {
    if (Module._rb_ctx) {
        var color = 'rgba(' + Math.round(r*255) + ',' + Math.round(g*255) + ',' + Math.round(b*255) + ',' + a + ')';
        Module._rb_ctx.fillStyle = color;
        Module._rb_ctx.strokeStyle = color;
    }
});

EM_JS(void, js_rb_setLineWidth, (float w), {
    if (Module._rb_ctx) Module._rb_ctx.lineWidth = w;
});

EM_JS(void, js_rb_setLineJoin, (int join), {
    if (Module._rb_ctx) {
        var joins = ['miter', 'round', 'bevel'];
        Module._rb_ctx.lineJoin = joins[join] || 'miter';
    }
});

EM_JS(void, js_rb_setDash, (const double* pattern, int count, double offset), {
    if (Module._rb_ctx) {
        var arr = [];
        for (var i = 0; i < count; i++) {
            arr.push(HEAPF64[(pattern >> 3) + i]);
        }
        Module._rb_ctx.setLineDash(arr);
        Module._rb_ctx.lineDashOffset = offset;
    }
});

EM_JS(void, js_rb_save, (), {
    if (Module._rb_ctx) Module._rb_ctx.save();
});

EM_JS(void, js_rb_restore, (), {
    if (Module._rb_ctx) Module._rb_ctx.restore();
});

EM_JS(void, js_rb_translate, (float dx, float dy), {
    if (Module._rb_ctx) Module._rb_ctx.translate(dx, dy);
});

EM_JS(void, js_rb_scale, (float sx, float sy), {
    if (Module._rb_ctx) Module._rb_ctx.scale(sx, sy);
});

EM_JS(void, js_rb_rotate, (float angle), {
    if (Module._rb_ctx) Module._rb_ctx.rotate(angle);
});

EM_JS(void, js_rb_setTransform, (double a, double b, double c, double d, double e, double f), {
    if (Module._rb_ctx) Module._rb_ctx.setTransform(a, b, c, d, e, f);
});

EM_JS(void, js_rb_getTransform, (double* out), {
    if (Module._rb_ctx) {
        var t = Module._rb_ctx.getTransform();
        HEAPF64[(out >> 3) + 0] = t.a;
        HEAPF64[(out >> 3) + 1] = t.b;
        HEAPF64[(out >> 3) + 2] = t.c;
        HEAPF64[(out >> 3) + 3] = t.d;
        HEAPF64[(out >> 3) + 4] = t.e;
        HEAPF64[(out >> 3) + 5] = t.f;
    }
});

EM_JS(void, js_rb_setFont, (const char* fontStr), {
    if (Module._rb_ctx) Module._rb_ctx.font = UTF8ToString(fontStr);
});

EM_JS(void, js_rb_fillText, (const char* text, float x, float y), {
    if (Module._rb_ctx) Module._rb_ctx.fillText(UTF8ToString(text), x, y);
});

EM_JS(float, js_rb_measureTextWidth, (const char* text), {
    if (Module._rb_ctx) {
        var m = Module._rb_ctx.measureText(UTF8ToString(text));
        return m.width;
    }
    return 0;
});

EM_JS(float, js_rb_measureTextAscent, (const char* text), {
    if (Module._rb_ctx) {
        var m = Module._rb_ctx.measureText(UTF8ToString(text));
        return m.actualBoundingBoxAscent || 0;
    }
    return 0;
});

EM_JS(float, js_rb_measureTextDescent, (const char* text), {
    if (Module._rb_ctx) {
        var m = Module._rb_ctx.measureText(UTF8ToString(text));
        return m.actualBoundingBoxDescent || 0;
    }
    return 0;
});

EM_JS(void, js_rb_setGlobalCompositeOp, (int op), {
    if (Module._rb_ctx) {
        // op=1 corresponds to CAIRO_OPERATOR_SOURCE -> 'copy' in Canvas2D
        Module._rb_ctx.globalCompositeOperation = op == 1 ? 'copy' : 'source-over';
    }
});

EM_JS(void, js_rb_setLinearGradient, (float x0, float y0, float x1, float y1,
                                       float r1, float g1, float b1, float r2, float g2, float b2), {
    if (Module._rb_ctx) {
        var grad = Module._rb_ctx.createLinearGradient(x0, y0, x1, y1);
        grad.addColorStop(0, 'rgb(' + Math.round(r1*255) + ',' + Math.round(g1*255) + ',' + Math.round(b1*255) + ')');
        grad.addColorStop(1, 'rgb(' + Math.round(r2*255) + ',' + Math.round(g2*255) + ',' + Math.round(b2*255) + ')');
        Module._rb_ctx.fillStyle = grad;
        Module._rb_ctx.strokeStyle = grad;
    }
});

// ============================================================================
// JSRenderBackend implementation
// ============================================================================

JSRenderBackend::JSRenderBackend()
    : _width(0), _height(0), _mode(0), _fontSize(12), _fontBold(false), _fontItalic(false), _curX(0), _curY(0)
{
    strcpy(_fontFamily, "Arial");
    _matrix[0] = 1;
    _matrix[1] = 0;
    _matrix[2] = 0;
    _matrix[3] = 1;
    _matrix[4] = 0;
    _matrix[5] = 0;
}

JSRenderBackend::~JSRenderBackend()
{
    js_rb_destroyCanvas();
}

// ---- Surface lifecycle ----

void JSRenderBackend::createSurface(int mode, int width, int height, void* /*output*/)
{
    _mode = mode;
    _width = width;
    _height = height;
    js_rb_createCanvas(width, height);
}

void JSRenderBackend::closeSurface(int mode, bool discard, void* /*output*/)
{
    // TODO: for PNG mode, extract canvas pixels and encode to PNG
    // TODO: for SVG mode, build SVG string from recorded commands
    js_rb_destroyCanvas();
}

void JSRenderBackend::createContext()
{
    // Context is created together with canvas in JS
}

void JSRenderBackend::destroyContext()
{
    // Context is destroyed together with canvas in JS
}

// ---- Path operations ----

void JSRenderBackend::beginPath()
{
    js_rb_beginPath();
}

void JSRenderBackend::closePath()
{
    js_rb_closePath();
}

void JSRenderBackend::moveTo(float x, float y)
{
    _curX = x;
    _curY = y;
    js_rb_moveTo(x, y);
}

void JSRenderBackend::lineTo(float x, float y)
{
    _curX = x;
    _curY = y;
    js_rb_lineTo(x, y);
}

void JSRenderBackend::relMoveTo(float dx, float dy)
{
    _curX += dx;
    _curY += dy;
    js_rb_moveTo(_curX, _curY);
}

void JSRenderBackend::relLineTo(float dx, float dy)
{
    _curX += dx;
    _curY += dy;
    js_rb_lineTo(_curX, _curY);
}

void JSRenderBackend::curveTo(float x1, float y1, float x2, float y2, float x3, float y3)
{
    _curX = x3;
    _curY = y3;
    js_rb_curveTo(x1, y1, x2, y2, x3, y3);
}

void JSRenderBackend::arc(float cx, float cy, float r, float a0, float a1)
{
    js_rb_arc(cx, cy, r, a0, a1, 0);
}

void JSRenderBackend::arcNegative(float cx, float cy, float r, float a0, float a1)
{
    js_rb_arc(cx, cy, r, a0, a1, 1);
}

void JSRenderBackend::rect(float x, float y, float w, float h)
{
    js_rb_rect(x, y, w, h);
}

// ---- Drawing operations ----

void JSRenderBackend::fill()
{
    js_rb_fill();
}

void JSRenderBackend::stroke()
{
    js_rb_stroke();
}

void JSRenderBackend::paint()
{
    js_rb_paint();
}

// ---- Style ----

void JSRenderBackend::setSourceRGB(float r, float g, float b)
{
    js_rb_setSourceRGB(r, g, b);
}

void JSRenderBackend::setSourceRGBA(float r, float g, float b, float a)
{
    js_rb_setSourceRGBA(r, g, b, a);
}

void JSRenderBackend::setLineWidth(float w)
{
    js_rb_setLineWidth(w);
}

void JSRenderBackend::setLineJoin(int join)
{
    js_rb_setLineJoin(join);
}

void JSRenderBackend::setDash(const double* pattern, int count, double offset)
{
    js_rb_setDash(pattern, count, offset);
}

void JSRenderBackend::setOperator(int op)
{
    js_rb_setGlobalCompositeOp(op);
}

void JSRenderBackend::setAntialias(int /*mode*/)
{
    // Canvas2D always anti-aliases; nothing to do
}

// ---- Transform ----

void JSRenderBackend::save()
{
    js_rb_save();
}

void JSRenderBackend::restore()
{
    js_rb_restore();
}

void JSRenderBackend::translate(float dx, float dy)
{
    js_rb_translate(dx, dy);
}

void JSRenderBackend::scale(float sx, float sy)
{
    js_rb_scale(sx, sy);
}

void JSRenderBackend::rotate(float angle)
{
    js_rb_rotate(angle);
}

void JSRenderBackend::getMatrix(double m[6])
{
    js_rb_getTransform(m);
}

void JSRenderBackend::setMatrix(const double m[6])
{
    js_rb_setTransform(m[0], m[1], m[2], m[3], m[4], m[5]);
}

void JSRenderBackend::initIdentityMatrix(double m[6])
{
    m[0] = 1;
    m[1] = 0;
    m[2] = 0;
    m[3] = 1;
    m[4] = 0;
    m[5] = 0;
}

void JSRenderBackend::userToDevice(double& x, double& y)
{
    double m[6];
    js_rb_getTransform(m);
    double nx = m[0] * x + m[2] * y + m[4];
    double ny = m[1] * x + m[3] * y + m[5];
    x = nx;
    y = ny;
}

void JSRenderBackend::deviceToUser(double& x, double& y)
{
    double m[6];
    js_rb_getTransform(m);
    double det = m[0] * m[3] - m[1] * m[2];
    if (fabs(det) < 1e-12)
        return;
    double dx = x - m[4];
    double dy = y - m[5];
    double nx = (m[3] * dx - m[2] * dy) / det;
    double ny = (-m[1] * dx + m[0] * dy) / det;
    x = nx;
    y = ny;
}

// ---- Extents ----

void JSRenderBackend::strokeExtents(double& x1, double& y1, double& x2, double& y2)
{
    // Canvas2D doesn't have path extents API. Return large bounds.
    // This is used for bounding box calculation which happens in a separate idle pass.
    x1 = 0;
    y1 = 0;
    x2 = _width;
    y2 = _height;
}

void JSRenderBackend::pathExtents(double& x1, double& y1, double& x2, double& y2)
{
    x1 = 0;
    y1 = 0;
    x2 = _width;
    y2 = _height;
}

// ---- Text ----

static void _buildFontString(char* out, int maxLen, float size, bool italic, bool bold, const char* family)
{
    snprintf(out, maxLen, "%s%s%.1fpx %s", italic ? "italic " : "", bold ? "bold " : "", size, family);
}

void JSRenderBackend::selectFontFace(const char* family, bool italic, bool bold)
{
    _fontItalic = italic;
    _fontBold = bold;
    strncpy(_fontFamily, family, sizeof(_fontFamily) - 1);
    _fontFamily[sizeof(_fontFamily) - 1] = '\0';

    char fontStr[512];
    _buildFontString(fontStr, sizeof(fontStr), _fontSize, _fontItalic, _fontBold, _fontFamily);
    js_rb_setFont(fontStr);
}

void JSRenderBackend::setFontSize(float size)
{
    _fontSize = size;
    char fontStr[512];
    _buildFontString(fontStr, sizeof(fontStr), _fontSize, _fontItalic, _fontBold, _fontFamily);
    js_rb_setFont(fontStr);
}

void JSRenderBackend::textExtents(const char* text, float& width, float& height, float& x_bearing, float& y_bearing)
{
    width = js_rb_measureTextWidth(text);
    float ascent = js_rb_measureTextAscent(text);
    float descent = js_rb_measureTextDescent(text);
    height = ascent + descent;
    x_bearing = 0;
    y_bearing = -ascent;
}

void JSRenderBackend::fontExtents(double& height)
{
    // Measure "M" as a representative character
    float w = js_rb_measureTextWidth("M");
    float asc = js_rb_measureTextAscent("M");
    float desc = js_rb_measureTextDescent("M");
    height = asc + desc;
    if (height < 1.0)
        height = _fontSize; // fallback
}

void JSRenderBackend::showText(const char* text)
{
    js_rb_fillText(text, _curX, _curY);
}

void JSRenderBackend::textPath(const char* text)
{
    // Canvas2D doesn't have textPath. We just draw the text directly.
    // For bounding box calculation, this is called in idle mode and the result is discarded.
    js_rb_fillText(text, _curX, _curY);
}

// ---- Font options ----

void JSRenderBackend::createFontOptions()
{
    // No-op in JS — browser handles font rendering options
}

void JSRenderBackend::destroyFontOptions()
{
    // No-op
}

void JSRenderBackend::setFontOptionsAntialias(int /*mode*/)
{
    // No-op — browser always anti-aliases
}

void JSRenderBackend::applyFontOptions()
{
    // No-op
}

// ---- Gradient ----

void JSRenderBackend::setLinearGradient(float x0, float y0, float x1, float y1, float r1, float g1, float b1, float r2, float g2, float b2)
{
    js_rb_setLinearGradient(x0, y0, x1, y1, r1, g1, b1, r2, g2, b2);
}

void JSRenderBackend::clearPattern()
{
    // Reset to black
    js_rb_setSourceRGB(0, 0, 0);
}

// ---- Image ----

void JSRenderBackend::drawPngImage(const void* /*data*/, int /*dataLen*/, float /*x*/, float /*y*/, float /*w*/, float /*h*/)
{
    // TODO: decode PNG in JS and draw to canvas
    // For now, skip embedded images (rare in molecule rendering)
}

void JSRenderBackend::writeSurfaceToPng(void* /*output*/)
{
    // TODO: extract canvas pixels and encode to PNG
}

// ---- Path debugging ----

bool JSRenderBackend::isPathEmpty()
{
    // Canvas2D doesn't provide path inspection.
    // Always return false to avoid assertion failures.
    return false;
}

// ---- Surface source ----

void JSRenderBackend::setSourceSurface(float /*x*/, float /*y*/)
{
    // Used for compositing PNG surfaces — not needed in JS backend
}

#endif // __EMSCRIPTEN__
