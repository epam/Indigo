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
#include "base_cpp/output.h"
#include <cmath>
#include <cstring>
#include <emscripten.h>
#include <vector>

using namespace indigo;

// ============================================================================
// EM_JS: SVG builder - all drawing operations build SVG XML in JavaScript
// ===================================================

EM_JS(void, js_rb_init, (int w, int h, int mode), {
    Module._rb = {
        w : w,
        h : h,
        mode : mode,
        // State
        s : {
            ctm : [ 1, 0, 0, 1, 0, 0 ],
            r : 0,
            g : 0,
            b : 0,
            a : 1,
            lw : 1,
            lj : 'miter',
            dash : [],
            doff : 0,
            fsz : 12,
            ffam : 'Noto Sans',
            fbold : false,
            fital : false
        },
        stack : [],
        // Path
        pd : '',
        cx : 0,
        cy : 0,
        pathEmpty : true,
        // Bounding box tracking (in final pixel coords)
        bbx0 : 1e9,
        bby0 : 1e9,
        bbx1 : -1e9,
        bby1 : -1e9,
        // SVG output
        elems : [],
        defs : '',
        gradCnt : 0,
        gradId : null,
        out : null
    };
});

EM_JS(void, js_rb_destroy, (), { Module._rb = null; });

// -- Path operations --
EM_JS(void, js_rb_beginPath, (), {
    var r = Module._rb;
    r.pd ='';
    r.pathEmpty = true;
});
EM_JS(void, js_rb_closePath, (), { Module._rb.pd += 'Z '; });
EM_JS(void, js_rb_moveTo, (double x, double y), {
    var r = Module._rb;
    r.pd += 'M' + x + ' ' + y + ' ';
    r.cx = x;
    r.cy = y;
    r.pathEmpty = false;
});
EM_JS(void, js_rb_lineTo, (double x, double y), {
    var r = Module._rb;
    r.pd += 'L' + x + ' ' + y + ' ';
    r.cx = x;
    r.cy = y;
    r.pathEmpty = false;
});
EM_JS(void, js_rb_curveTo, (double x1, double y1, double x2, double y2, double x3, double y3), {
    var r = Module._rb;
    r.pd += 'C' + x1 + ' ' + y1 + ' ' + x2 + ' ' + y2 + ' ' + x3 + ' ' + y3 + ' ';
    r.cx = x3;
    r.cy = y3;
    r.pathEmpty = false;
});
EM_JS(void, js_rb_arc, (double cx, double cy, double rad, double a0, double a1, int ccw), {
    // Convert arc to SVG path segments
    var r = Module._rb;
    if (rad < 0.001)
        return;
    var step = ccw ? -0.5 : 0.5;
    var angle = a0;
    var sx = cx + rad * Math.cos(a0), sy = cy + rad * Math.sin(a0);
    if (r.pathEmpty)
    {
        r.pd += 'M' + sx + ' ' + sy + ' ';
        r.pathEmpty = false;
    }
    else
    {
        r.pd += 'L' + sx + ' ' + sy + ' ';
    }
    // Use SVG arc command
    var da = a1 - a0;
    if (ccw && da > 0)
        da -= 2 * Math.PI;
    if (!ccw && da < 0)
        da += 2 * Math.PI;
    var large = Math.abs(da) > Math.PI ? 1 : 0;
    var sweep = da > 0 ? 1 : 0;
    var ex = cx + rad * Math.cos(a1), ey = cy + rad * Math.sin(a1);
    r.pd += 'A' + rad + ' ' + rad + ' 0 ' + large + ' ' + sweep + ' ' + ex + ' ' + ey + ' ';
    r.cx = ex;
    r.cy = ey;
});
EM_JS(void, js_rb_rect, (double x, double y, double w, double h), {
    var r = Module._rb;
    r.pd += 'M' + x + ' ' + y + 'L' + (x + w) + ' ' + y + 'L' + (x + w) + ' ' + (y + h) + 'L' + x + ' ' + (y + h) + 'Z ';
    r.pathEmpty = false;
});

// -- Helpers --
EM_JS(void, js_rb_emitPath, (int doFill, int doStroke), {
    var r = Module._rb, s = r.s;
    if (!r.pd)
        return;
    var m = s.ctm;
    // Track bbox: parse path coords and transform
    var nums = r.pd.match(/-?[0-9]*\.?[0-9]+/g);
    if (nums)
    {
        for (var i = 0; i + 1 < nums.length; i += 2)
        {
            var px = parseFloat(nums[i]);
            var py = parseFloat(nums[i + 1]);
            var tx = m[0] * px + m[2] * py + m[4];
            var ty = m[1] * px + m[3] * py + m[5];
            if (tx < r.bbx0)
                r.bbx0 = tx;
            if (ty < r.bby0)
                r.bby0 = ty;
            if (tx > r.bbx1)
                r.bbx1 = tx;
            if (ty > r.bby1)
                r.bby1 = ty;
        }
    }
    var e = '<path d="' + r.pd + '" transform="matrix(' + m[0] + ',' + m[1] + ',' + m[2] + ',' + m[3] + ',' + m[4] + ',' + m[5] + ')"';
    if (doFill)
    {
        var fc = s.gradId ? 'url(#' + s.gradId + ')' : 'rgb(' + Math.round(s.r * 255) + ',' + Math.round(s.g * 255) + ',' + Math.round(s.b * 255) + ')';
        e += ' fill="' + fc + '"';
        if (s.a < 1 && !s.gradId)
            e += ' fill-opacity="' + s.a + '"';
    }
    else
    {
        e += ' fill="none"';
    }
    if (doStroke)
    {
        e += ' stroke="rgb(' + Math.round(s.r * 255) + ',' + Math.round(s.g * 255) + ',' + Math.round(s.b * 255) + ')"';
        if (s.a < 1)
            e += ' stroke-opacity="' + s.a + '"';
        e += ' stroke-width="' + s.lw + '" stroke-linejoin="' + s.lj + '"';
        if (s.dash.length)
            e += ' stroke-dasharray="' + s.dash.join(',') + '" stroke-dashoffset="' + s.doff + '"';
    }
    else if (!doFill)
    {
        e += ' stroke="none"';
    }
    e += '/>';
    r.elems.push(e);
    r.pd = '';
    r.pathEmpty = true;
});

// -- Drawing --
EM_JS(void, js_rb_fill, (), { js_rb_emitPath(1, 0); });
EM_JS(void, js_rb_stroke, (), { js_rb_emitPath(0, 1); });
EM_JS(void, js_rb_paint, (), {
    var r = Module._rb, s = r.s;
    var c = 'rgb(' + Math.round(s.r * 255) + ',' + Math.round(s.g * 255) + ',' + Math.round(s.b * 255) + ')';
    r.elems.push('<rect x="0" y="0" width="' + r.w + '" height="' + r.h + '" fill="' + c + '"/>');
});

// -- Style --
EM_JS(void, js_rb_setColor, (double cr, double cg, double cb, double ca), {
    var s = Module._rb.s;
    s.r = cr;
    s.g = cg;
    s.b = cb;
    s.a = ca;
    s.gradId = null;
});
EM_JS(void, js_rb_setLineWidth, (double w), { Module._rb.s.lw = w; });
EM_JS(void, js_rb_setLineJoin, (int j), { Module._rb.s.lj = [ 'miter', 'round', 'bevel' ][j] || 'miter'; });
EM_JS(void, js_rb_setDash, (const double* p, int n, double off), {
    var s = Module._rb.s;
    s.dash = [];
    for (var i = 0; i < n; i++)
        s.dash.push(HEAPF64[(p >> 3) + i]);
    s.doff = off;
});

// -- Transform --
EM_JS(void, js_rb_save, (), {
    var r = Module._rb, s = r.s;
    r.stack.push({
        ctm : s.ctm.slice(),
        r : s.r,
        g : s.g,
        b : s.b,
        a : s.a,
        lw : s.lw,
        lj : s.lj,
        dash : s.dash.slice(),
        doff : s.doff,
        fsz : s.fsz,
        ffam : s.ffam,
        fbold : s.fbold,
        fital : s.fital,
        gradId : s.gradId
    });
});
EM_JS(void, js_rb_restore, (), {
    var r = Module._rb;
    if (r.stack.length)
        r.s = r.stack.pop();
});

// Matrix multiply: result = current * [A,B,C,D,E,F]
EM_JS(void, js_rb_mmul, (double A, double B, double C, double D, double E, double F), {
    var m = Module._rb.s.ctm;
    var a = m[0], b = m[1], c = m[2], d = m[3], e = m[4], f = m[5];
    m[0] = a * A + c * B;
    m[1] = b * A + d * B;
    m[2] = a * C + c * D;
    m[3] = b * C + d * D;
    m[4] = a * E + c * F + e;
    m[5] = b * E + d * F + f;
});

EM_JS(void, js_rb_setMatrix, (double a, double b, double c, double d, double e, double f), {
    var m = Module._rb.s.ctm;
    m[0] = a;
    m[1] = b;
    m[2] = c;
    m[3] = d;
    m[4] = e;
    m[5] = f;
});
EM_JS(void, js_rb_getMatrix, (double* out), {
    var m = Module._rb.s.ctm;
    HEAPF64[(out >> 3)] = m[0];
    HEAPF64[(out >> 3) + 1] = m[1];
    HEAPF64[(out >> 3) + 2] = m[2];
    HEAPF64[(out >> 3) + 3] = m[3];
    HEAPF64[(out >> 3) + 4] = m[4];
    HEAPF64[(out >> 3) + 5] = m[5];
});

// -- Text --
EM_JS(void, js_rb_setFont, (const char* fam, double sz, int bold, int ital), {
    var s = Module._rb.s;
    s.ffam = UTF8ToString(fam);
    s.fsz = sz;
    s.fbold = !!bold;
    s.fital = !!ital;
});
EM_JS(void, js_rb_fillText, (const char* text, double x, double y), {
    var r = Module._rb;
    var s = r.s;
    var m = s.ctm;
    var t = UTF8ToString(text);
    // Escape XML special chars
    var esc = '';
    for (var i = 0; i < t.length; i++)
    {
        var c = t.charAt(i);
        if (c == '&')
            esc += '&amp;';
        else if (c == '<')
            esc += '&lt;';
        else if (c == '>')
            esc += '&gt;';
        else if (c == '"')
            esc += '&quot;';
        else
            esc += c;
    }
    t = esc;
    // Track bbox: transform text anchor through CTM
    var tx = m[0] * x + m[2] * y + m[4];
    var ty = m[1] * x + m[3] * y + m[5];
    // Estimate text extent (use char count * fontSize * scale as rough width)
    var tw = t.length * s.fsz * 0.6 * Math.abs(m[0]);
    var th = s.fsz * Math.abs(m[3]);
    if (tx < r.bbx0)
        r.bbx0 = tx;
    if (ty - th < r.bby0)
        r.bby0 = ty - th;
    if (tx + tw > r.bbx1)
        r.bbx1 = tx + tw;
    if (ty > r.bby1)
        r.bby1 = ty;
    var fc = 'rgb(' + Math.round(s.r * 255) + ',' + Math.round(s.g * 255) + ',' + Math.round(s.b * 255) + ')';
    var e = '<text x="' + x + '" y="' + y + '" font-family="' + s.ffam + '"';
    e += ' font-size="' + s.fsz + '" fill="' + fc + '"';
    if (s.fbold)
        e += ' font-weight="bold"';
    if (s.fital)
        e += ' font-style="italic"';
    if (s.a < 1)
        e += ' fill-opacity="' + s.a + '"';
    e += ' transform="matrix(' + m[0] + ' ' + m[1] + ' ' + m[2] + ' ' + m[3] + ' ' + m[4] + ' ' + m[5] + ')"';
    e += '>' + t + '</text>';
    r.elems.push(e);
});

// Text metrics approximation (works without Canvas2D)
EM_JS(double, js_rb_measureWidth, (const char* text, double fontSize), {
    var t = UTF8ToString(text);
    // Try browser Canvas2D if available
    if (typeof document != 'undefined')
    {
        try
        {
            var c = document.createElement('canvas').getContext('2d');
            c.font = fontSize + 'px ' + Module._rb.s.ffam;
            return c.measureText(t).width;
        }
        catch (e)
        {
        }
    }
    // Fallback: approximate char widths for sans-serif (NotoSans-like)
    var w = 0;
    for (var i = 0; i < t.length; i++)
    {
        var ch = t.charCodeAt(i);
        if (ch >= 48 && ch <= 57)
            w += 0.60; // digits
        else if (ch >= 65 && ch <= 90)
            w += 0.72; // uppercase
        else if (ch == 77 || ch == 87)
            w += 0.88; // M, W
        else if (ch == 73 || ch == 108)
            w += 0.33; // I, l
        else if (ch >= 97 && ch <= 122)
            w += 0.55; // lowercase
        else if (ch == 32)
            w += 0.30; // space
        else if (ch == 43 || ch == 45)
            w += 0.55; // +, -
        else
            w += 0.60;
    }
    return w * fontSize;
});

// -- Gradient --
EM_JS(void, js_rb_setGradient, (double x0, double y0, double x1, double y1, double r1, double g1, double b1, double r2, double g2, double b2), {
    var r = Module._rb;
    var id = 'grad' + (r.gradCnt++);
    var c1 = 'rgb(' + Math.round(r1 * 255) + ',' + Math.round(g1 * 255) + ',' + Math.round(b1 * 255) + ')';
    var c2 = 'rgb(' + Math.round(r2 * 255) + ',' + Math.round(g2 * 255) + ',' + Math.round(b2 * 255) + ')';
    r.defs += '<linearGradient id="' + id + '" x1="' + x0 + '" y1="' + y0 + '" x2="' + x1 + '" y2="' + y1 +
              '" gradientUnits="userSpaceOnUse"><stop offset="0" stop-color="' + c1 + '"/><stop offset="1" stop-color="' + c2 + '"/></linearGradient>';
    r.s.gradId = id;
});

// -- Finalize: build SVG string and optionally convert to PNG --
EM_JS(int, js_rb_finalize, (int mode), {
    var r = Module._rb;
    if (!r)
        return 0;
    // Build SVG with viewBox from tracked bounding box
    var vx = 0;
    var vy = 0;
    var vw = r.w;
    var vh = r.h;
    if (r.bbx1 > r.bbx0 && r.bby1 > r.bby0)
    {
        var pad = 2;
        vx = r.bbx0 - pad;
        vy = r.bby0 - pad;
        vw = r.bbx1 - r.bbx0 + 2 * pad;
        vh = r.bby1 - r.bby0 + 2 * pad;
    }
    var svg = '<svg xmlns="http://www.w3.org/2000/svg" width="' + Math.ceil(vw) + '" height="' + Math.ceil(vh) + '" viewBox="' + vx + ' ' + vy + ' ' + vw +
              ' ' + vh + '">';
    if (r.defs)
        svg += '<defs>' + r.defs + '</defs>';
    svg += r.elems.join('');
    svg += '</svg>';

    // Helper: encode string to Uint8Array
    function toBytes(s)
    {
        if (typeof TextEncoder != 'undefined')
            return new TextEncoder().encode(s);
        // Node.js fallback
        var buf = new Uint8Array(s.length);
        for (var i = 0; i < s.length; i++)
            buf[i] = s.charCodeAt(i) & 0xFF;
        return buf;
    }

    if (mode == 1)
    { // SVG - output SVG XML directly
        r.out = toBytes(svg);
        return r.out.length;
    }
    else if (mode == 0)
    { // PNG - rasterize SVG
        // Node.js: use sharp (npm dependency) for SVG→PNG
        if (typeof require != 'undefined')
        {
            try
            {
                var sharp = require('sharp');
                var fs = require('fs');
                var os = require('os');
                var svgBuf = Buffer.from(svg);
                var pngPath = os.tmpdir() + '/_indigo_render.png';
                // sharp supports sync-like via deasync or execSync
                var cp = require('child_process');
                var svgPath = os.tmpdir() + '/_indigo_render.svg';
                fs.writeFileSync(svgPath, svgBuf);
                cp.execSync('node -e "require(\'sharp\')(\'' + svgPath + '\').png().toFile(\'' + pngPath + '\').then(()=>process.exit(0))"');
                var pngData = fs.readFileSync(pngPath);
                r.out = new Uint8Array(pngData);
                return r.out.length;
            }
            catch (e)
            {
            }
        }
        // Fallback: output SVG (browser will handle conversion)
        r.out = toBytes(svg);
        return r.out.length;
    }
    else
    { // PDF - stub
        r.out = toBytes('%PDF-1.4 stub');
        return r.out.length;
    }
});

EM_JS(void, js_rb_copyOutput, (uint8_t * dst, int len), {
    var r = Module._rb;
    if (!r || !r.out)
        return;
    HEAPU8.set(r.out.subarray(0, len), dst);
});

EM_JS(int, js_rb_isPathEmpty, (), { return Module._rb ? Module._rb.pathEmpty : 1; });

// ===================================================
// JSRenderBackend implementation
// ===================================================

JSRenderBackend::JSRenderBackend() : _width(0), _height(0), _mode(0), _fontSize(12), _fontBold(false), _fontItalic(false), _curX(0), _curY(0)
{
    strcpy(_fontFamily, "Noto Sans");
    _matrix[0] = 1;
    _matrix[1] = 0;
    _matrix[2] = 0;
    _matrix[3] = 1;
    _matrix[4] = 0;
    _matrix[5] = 0;
}

JSRenderBackend::~JSRenderBackend()
{
    js_rb_destroy();
}

void JSRenderBackend::createSurface(int mode, int width, int height, void*)
{
    _mode = mode;
    _width = width;
    _height = height;
    js_rb_init(width, height, mode);
}

void JSRenderBackend::closeSurface(int mode, bool discard, void* output)
{
    if (discard)
    {
        js_rb_destroy();
        return;
    }
    int rbMode = mode; // 0=PNG, 1=SVG, 2=PDF
    int len = js_rb_finalize(rbMode);
    if (len > 0 && output)
    {
        std::vector<char> buf(len);
        js_rb_copyOutput((uint8_t*)buf.data(), len);
        Output* out = static_cast<Output*>(output);
        out->write(buf.data(), len);
    }
    js_rb_destroy();
}

void JSRenderBackend::createContext()
{
}
void JSRenderBackend::destroyContext()
{
}

// Path
void JSRenderBackend::beginPath()
{
    js_rb_beginPath();
}
void JSRenderBackend::closePath()
{
    js_rb_closePath();
}
void JSRenderBackend::moveTo(double x, double y)
{
    _curX = x;
    _curY = y;
    js_rb_moveTo(x, y);
}
void JSRenderBackend::lineTo(double x, double y)
{
    _curX = x;
    _curY = y;
    js_rb_lineTo(x, y);
}
void JSRenderBackend::relMoveTo(double dx, double dy)
{
    _curX += dx;
    _curY += dy;
    js_rb_moveTo(_curX, _curY);
}
void JSRenderBackend::relLineTo(double dx, double dy)
{
    _curX += dx;
    _curY += dy;
    js_rb_lineTo(_curX, _curY);
}
void JSRenderBackend::curveTo(double x1, double y1, double x2, double y2, double x3, double y3)
{
    _curX = x3;
    _curY = y3;
    js_rb_curveTo(x1, y1, x2, y2, x3, y3);
}
void JSRenderBackend::arc(double cx, double cy, double r, double a0, double a1)
{
    js_rb_arc(cx, cy, r, a0, a1, 0);
}
void JSRenderBackend::arcNegative(double cx, double cy, double r, double a0, double a1)
{
    js_rb_arc(cx, cy, r, a0, a1, 1);
}
void JSRenderBackend::rect(double x, double y, double w, double h)
{
    js_rb_rect(x, y, w, h);
}

// Drawing
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

// Style
void JSRenderBackend::setSourceRGB(double r, double g, double b)
{
    js_rb_setColor(r, g, b, 1);
}
void JSRenderBackend::setSourceRGBA(double r, double g, double b, double a)
{
    js_rb_setColor(r, g, b, a);
}
void JSRenderBackend::setLineWidth(double w)
{
    js_rb_setLineWidth(w);
}
void JSRenderBackend::setLineJoin(int j)
{
    js_rb_setLineJoin(j);
}
void JSRenderBackend::setDash(const double* p, int n, double off)
{
    js_rb_setDash(p, n, off);
}
void JSRenderBackend::setOperator(int)
{
} // SVG doesn't need composite ops
void JSRenderBackend::setAntialias(int)
{
} // SVG always anti-aliases

// Transform
void JSRenderBackend::save()
{
    js_rb_save();
}
void JSRenderBackend::restore()
{
    js_rb_restore();
}
void JSRenderBackend::translate(double dx, double dy)
{
    js_rb_mmul(1, 0, 0, 1, dx, dy);
}
void JSRenderBackend::scale(double sx, double sy)
{
    js_rb_mmul(sx, 0, 0, sy, 0, 0);
}
void JSRenderBackend::rotate(double a)
{
    js_rb_mmul(cos(a), sin(a), -sin(a), cos(a), 0, 0);
}
void JSRenderBackend::getMatrix(double m[6])
{
    js_rb_getMatrix(m);
}
void JSRenderBackend::setMatrix(const double m[6])
{
    js_rb_setMatrix(m[0], m[1], m[2], m[3], m[4], m[5]);
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
    js_rb_getMatrix(m);
    double nx = m[0] * x + m[2] * y + m[4];
    double ny = m[1] * x + m[3] * y + m[5];
    x = nx;
    y = ny;
}

void JSRenderBackend::deviceToUser(double& x, double& y)
{
    double m[6];
    js_rb_getMatrix(m);
    double det = m[0] * m[3] - m[1] * m[2];
    if (fabs(det) < 1e-12)
        return;
    double dx = x - m[4], dy = y - m[5];
    x = (m[3] * dx - m[2] * dy) / det;
    y = (-m[1] * dx + m[0] * dy) / det;
}

// Extents - approximate from path bounds
void JSRenderBackend::strokeExtents(double& x1, double& y1, double& x2, double& y2)
{
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

// Text
void JSRenderBackend::selectFontFace(const char* family, bool italic, bool bold)
{
    _fontItalic = italic;
    _fontBold = bold;
    strncpy(_fontFamily, family, sizeof(_fontFamily) - 1);
    _fontFamily[sizeof(_fontFamily) - 1] = '\0';
    js_rb_setFont(_fontFamily, _fontSize, bold ? 1 : 0, italic ? 1 : 0);
}

void JSRenderBackend::setFontSize(double size)
{
    _fontSize = size;
    js_rb_setFont(_fontFamily, _fontSize, _fontBold ? 1 : 0, _fontItalic ? 1 : 0);
}

void JSRenderBackend::textExtents(const char* text, double& width, double& height, double& x_bearing, double& y_bearing)
{
    width = js_rb_measureWidth(text, _fontSize);
    height = _fontSize;
    x_bearing = 0;
    y_bearing = -_fontSize * 0.8;
}

void JSRenderBackend::fontExtents(double& height)
{
    height = _fontSize * 1.2;
}

void JSRenderBackend::showText(const char* text)
{
    js_rb_fillText(text, _curX, _curY);
    // Advance current position
    _curX += js_rb_measureWidth(text, _fontSize);
}

void JSRenderBackend::textPath(const char* text)
{
    // SVG doesn't have textPath in the Cairo sense. Just render the text.
    js_rb_fillText(text, _curX, _curY);
}

// Font options - no-op for SVG
void JSRenderBackend::createFontOptions()
{
}
void JSRenderBackend::destroyFontOptions()
{
}
void JSRenderBackend::setFontOptionsAntialias(int)
{
}
void JSRenderBackend::applyFontOptions()
{
}

// Gradient
void JSRenderBackend::setLinearGradient(double x0, double y0, double x1, double y1, double r1, double g1, double b1, double r2, double g2, double b2)
{
    js_rb_setGradient(x0, y0, x1, y1, r1, g1, b1, r2, g2, b2);
}
void JSRenderBackend::clearPattern()
{
    js_rb_setColor(0, 0, 0, 1);
}

// Image
void JSRenderBackend::drawPngImage(const void*, int, double, double, double, double)
{
    // TODO: embed PNG as base64 data URI in SVG <image> element
}
void JSRenderBackend::writeSurfaceToPng(void* output)
{
    int len = js_rb_finalize(0); // PNG mode
    if (len > 0 && output)
    {
        std::vector<char> buf(len);
        js_rb_copyOutput((uint8_t*)buf.data(), len);
        Output* out = static_cast<Output*>(output);
        out->write(buf.data(), len);
    }
}

bool JSRenderBackend::isPathEmpty()
{
    return js_rb_isPathEmpty();
}
void JSRenderBackend::setSourceSurface(double, double)
{
}

#endif // __EMSCRIPTEN__
