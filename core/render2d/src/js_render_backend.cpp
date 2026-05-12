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
#include <cstdint>
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
        // Per-path bbox (reset on each emitPath)
        pbbx0 : 1e9,
        pbby0 : 1e9,
        pbbx1 : -1e9,
        pbby1 : -1e9,
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

// Helper: transform point through current CTM and track per-path bbox
// m = [a, b, c, d, e, f] => x' = a*x + c*y + e, y' = b*x + d*y + f
EM_JS(void, js_rb_moveTo, (double x, double y), {
    var r = Module._rb, m = r.s.ctm;
    var tx = m[0] * x + m[2] * y + m[4];
    var ty = m[1] * x + m[3] * y + m[5];
    r.pd += 'M' + tx + ' ' + ty + ' ';
    r.cx = tx;
    r.cy = ty;
    r.pathEmpty = false;
    if (tx < r.pbbx0)
        r.pbbx0 = tx;
    if (ty < r.pbby0)
        r.pbby0 = ty;
    if (tx > r.pbbx1)
        r.pbbx1 = tx;
    if (ty > r.pbby1)
        r.pbby1 = ty;
});
EM_JS(void, js_rb_lineTo, (double x, double y), {
    var r = Module._rb, m = r.s.ctm;
    var tx = m[0] * x + m[2] * y + m[4];
    var ty = m[1] * x + m[3] * y + m[5];
    r.pd += 'L' + tx + ' ' + ty + ' ';
    r.cx = tx;
    r.cy = ty;
    r.pathEmpty = false;
    if (tx < r.pbbx0)
        r.pbbx0 = tx;
    if (ty < r.pbby0)
        r.pbby0 = ty;
    if (tx > r.pbbx1)
        r.pbbx1 = tx;
    if (ty > r.pbby1)
        r.pbby1 = ty;
});
EM_JS(void, js_rb_curveTo, (double x1, double y1, double x2, double y2, double x3, double y3), {
    var r = Module._rb, m = r.s.ctm;
    var tx1 = m[0] * x1 + m[2] * y1 + m[4], ty1 = m[1] * x1 + m[3] * y1 + m[5];
    var tx2 = m[0] * x2 + m[2] * y2 + m[4], ty2 = m[1] * x2 + m[3] * y2 + m[5];
    var tx3 = m[0] * x3 + m[2] * y3 + m[4], ty3 = m[1] * x3 + m[3] * y3 + m[5];
    r.pd += 'C' + tx1 + ' ' + ty1 + ' ' + tx2 + ' ' + ty2 + ' ' + tx3 + ' ' + ty3 + ' ';
    r.cx = tx3;
    r.cy = ty3;
    r.pathEmpty = false;
    var pts = [ tx1, ty1, tx2, ty2, tx3, ty3 ];
    for (var k = 0; k < 6; k += 2)
    {
        if (pts[k] < r.pbbx0)
            r.pbbx0 = pts[k];
        if (pts[k + 1] < r.pbby0)
            r.pbby0 = pts[k + 1];
        if (pts[k] > r.pbbx1)
            r.pbbx1 = pts[k];
        if (pts[k + 1] > r.pbby1)
            r.pbby1 = pts[k + 1];
    }
});
EM_JS(void, js_rb_arc, (double cx, double cy, double rad, double a0, double a1, int ccw), {
    var r = Module._rb, m = r.s.ctm;
    if (rad < 0.001)
        return;

    var da = a1 - a0;
    if (ccw && da > 0)
        da -= 2 * Math.PI;
    if (!ccw && da < 0)
        da += 2 * Math.PI;
    if (Math.abs(da) < 1e-12)
        return;

    function transformPoint(x, y)
    {
        return {x : m[0] * x + m[2] * y + m[4], y : m[1] * x + m[3] * y + m[5]};
    }

    function includePoint(p)
    {
        if (p.x < r.pbbx0)
            r.pbbx0 = p.x;
        if (p.y < r.pbby0)
            r.pbby0 = p.y;
        if (p.x > r.pbbx1)
            r.pbbx1 = p.x;
        if (p.y > r.pbby1)
            r.pbby1 = p.y;
    }

    var ax = m[0] * rad, ay = m[1] * rad;
    var bx = m[2] * rad, by = m[3] * rad;
    var q00 = ax * ax + bx * bx;
    var q01 = ax * ay + bx * by;
    var q11 = ay * ay + by * by;
    var trace = q00 + q11;
    var diff = q00 - q11;
    var root = Math.sqrt(Math.max(0, diff * diff + 4 * q01 * q01));
    var rx = Math.sqrt(Math.max(0, (trace + root) / 2));
    var ry = Math.sqrt(Math.max(0, (trace - root) / 2));
    var rotation = 0.5 * Math.atan2(2 * q01, diff) * 180 / Math.PI;
    var det = m[0] * m[3] - m[1] * m[2];

    if (rx < 1e-9 || ry < 1e-9)
    {
        var first = transformPoint(cx + rad * Math.cos(a0), cy + rad * Math.sin(a0));
        var end = transformPoint(cx + rad * Math.cos(a1), cy + rad * Math.sin(a1));
        if (r.pathEmpty)
        {
            r.pd += 'M' + first.x + ' ' + first.y + ' ';
            r.pathEmpty = false;
        }
        else
        {
            r.pd += 'L' + first.x + ' ' + first.y + ' ';
        }
        r.pd += 'L' + end.x + ' ' + end.y + ' ';
        r.cx = end.x;
        r.cy = end.y;
        includePoint(first);
        includePoint(end);
        return;
    }

    var first = transformPoint(cx + rad * Math.cos(a0), cy + rad * Math.sin(a0));
    if (r.pathEmpty)
    {
        r.pd += 'M' + first.x + ' ' + first.y + ' ';
        r.pathEmpty = false;
    }
    else
    {
        r.pd += 'L' + first.x + ' ' + first.y + ' ';
    }

    var segments = Math.max(1, Math.ceil(Math.abs(da) / Math.PI));
    var prev = a0;
    for (var si = 1; si <= segments; si++)
    {
        var next = a0 + da * (si / segments);
        var delta = next - prev;
        var end = transformPoint(cx + rad * Math.cos(next), cy + rad * Math.sin(next));
        var largeArc = Math.abs(delta) > Math.PI ? 1 : 0;
        var sweep = (delta >= 0) == (det >= 0) ? 1 : 0;
        r.pd += 'A' + rx + ' ' + ry + ' ' + rotation + ' ' + largeArc + ' ' + sweep + ' ' + end.x + ' ' + end.y + ' ';
        r.cx = end.x;
        r.cy = end.y;
        prev = next;
    }

    var steps = Math.max(8, Math.ceil(Math.abs(da) / 0.02));
    for (var i = 0; i <= steps; i++)
    {
        var t = a0 + da * (i / steps);
        includePoint(transformPoint(cx + rad * Math.cos(t), cy + rad * Math.sin(t)));
    }
});
EM_JS(void, js_rb_rect, (double x, double y, double w, double h), {
    var r = Module._rb, m = r.s.ctm;
    var x0 = x, y0 = y, x1 = x + w, y1 = y + h;
    var t00x = m[0] * x0 + m[2] * y0 + m[4], t00y = m[1] * x0 + m[3] * y0 + m[5];
    var t10x = m[0] * x1 + m[2] * y0 + m[4], t10y = m[1] * x1 + m[3] * y0 + m[5];
    var t11x = m[0] * x1 + m[2] * y1 + m[4], t11y = m[1] * x1 + m[3] * y1 + m[5];
    var t01x = m[0] * x0 + m[2] * y1 + m[4], t01y = m[1] * x0 + m[3] * y1 + m[5];
    r.pd += 'M' + t00x + ' ' + t00y + 'L' + t10x + ' ' + t10y + 'L' + t11x + ' ' + t11y + 'L' + t01x + ' ' + t01y + 'Z ';
    r.pathEmpty = false;
    var pts = [ t00x, t00y, t10x, t10y, t11x, t11y, t01x, t01y ];
    for (var k = 0; k < 8; k += 2)
    {
        if (pts[k] < r.pbbx0)
            r.pbbx0 = pts[k];
        if (pts[k + 1] < r.pbby0)
            r.pbby0 = pts[k + 1];
        if (pts[k] > r.pbbx1)
            r.pbbx1 = pts[k];
        if (pts[k + 1] > r.pbby1)
            r.pbby1 = pts[k + 1];
    }
});

// -- Helpers --
EM_JS(void, js_rb_emitPath, (int doFill, int doStroke), {
    var r = Module._rb, s = r.s;
    if (!r.pd)
        return;
    // Merge per-path bbox into global bbox (skip background-colored fill-only shapes)
    var isBgFill =
        doFill && !doStroke && r.bgColor && r.bgColor == 'rgb(' + Math.round(s.r * 255) + ',' + Math.round(s.g * 255) + ',' + Math.round(s.b * 255) + ')';
    if (!isBgFill)
    {
        if (r.pbbx0 < r.bbx0)
            r.bbx0 = r.pbbx0;
        if (r.pbby0 < r.bby0)
            r.bby0 = r.pbby0;
        if (r.pbbx1 > r.bbx1)
            r.bbx1 = r.pbbx1;
        if (r.pbby1 > r.bby1)
            r.bby1 = r.pbby1;
    }
    // Reset per-path bbox for next path
    r.pbbx0 = 1e9;
    r.pbby0 = 1e9;
    r.pbbx1 = -1e9;
    r.pbby1 = -1e9;
    var e = '<path d="' + r.pd + '"';
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
        // Scale stroke-width and dash through CTM since coordinates are pre-transformed
        var m = s.ctm;
        var scl = Math.sqrt(Math.abs(m[0] * m[3] - m[1] * m[2]));
        if (scl < 1e-9)
            scl = 1;
        var sw = s.lw * scl;
        e += ' stroke-width="' + sw + '" stroke-linejoin="' + s.lj + '"';
        if (s.dash.length)
        {
            var sd = [];
            for (var di = 0; di < s.dash.length; di++)
                sd.push(s.dash[di] * scl);
            e += ' stroke-dasharray="' + sd.join(',') + '" stroke-dashoffset="' + (s.doff * scl) + '"';
        }
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
    // Store background color; the rect will be emitted in finalize at viewBox coords
    r.bgColor = 'rgb(' + Math.round(s.r * 255) + ',' + Math.round(s.g * 255) + ',' + Math.round(s.b * 255) + ')';
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
    return w * fontSize * 1.15;
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

// -- Image --
EM_JS(void, js_rb_drawImage, (const uint8_t* data, int dataLen, double x, double y, double w, double h), {
    var r = Module._rb;
    if (!r || !data || dataLen <= 0 || w <= 0 || h <= 0)
        return;

    var ptr = data >>> 0;

    function bytesToBase64()
    {
        var chars = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/';
        var out = "";
        var i = 0;
        for (; i + 2 < dataLen; i += 3)
        {
            var n = (HEAPU8[ptr + i] << 16) | (HEAPU8[ptr + i + 1] << 8) | HEAPU8[ptr + i + 2];
            out += chars[(n >> 18) & 63] + chars[(n >> 12) & 63] + chars[(n >> 6) & 63] + chars[n & 63];
        }
        if (i < dataLen)
        {
            var b0 = HEAPU8[ptr + i];
            var b1 = i + 1 < dataLen ? HEAPU8[ptr + i + 1] : 0;
            var n = (b0 << 16) | (b1 << 8);
            out += chars[(n >> 18) & 63] + chars[(n >> 12) & 63] + (i + 1 < dataLen ? chars[(n >> 6) & 63] : '=') + '=';
        }
        return out;
    }

    function isSvg()
    {
        var i = 0;
        if (dataLen >= 3 && HEAPU8[ptr] == 0xEF && HEAPU8[ptr + 1] == 0xBB && HEAPU8[ptr + 2] == 0xBF)
            i = 3;
        while (i < dataLen)
        {
            var ch = HEAPU8[ptr + i];
            if (ch != 9 && ch != 10 && ch != 13 && ch != 32)
                break;
            i++;
        }
        var header = "";
        var end = Math.min(dataLen, i + 512);
        for (var j = i; j < end; j++)
        {
            var c = HEAPU8[ptr + j];
            if (c == 0)
                break;
            header += String.fromCharCode(c).toLowerCase();
        }
        return header.indexOf('<svg') == 0 || (header.indexOf('<?xml') == 0 && header.indexOf('<svg') != -1);
    }

    function includePoint(px, py)
    {
        if (px < r.bbx0)
            r.bbx0 = px;
        if (py < r.bby0)
            r.bby0 = py;
        if (px > r.bbx1)
            r.bbx1 = px;
        if (py > r.bby1)
            r.bby1 = py;
    }

    var m = r.s.ctm;
    var pts = [ x, y, x + w, y, x + w, y + h, x, y + h ];
    for (var k = 0; k < pts.length; k += 2)
    {
        includePoint(m[0] * pts[k] + m[2] * pts[k + 1] + m[4], m[1] * pts[k] + m[3] * pts[k + 1] + m[5]);
    }

    var mime = isSvg() ? 'image/svg+xml' : 'image/png';
    var href = 'data:' + mime + ';base64,' + bytesToBase64();
    var e = '<image x="' + x + '" y="' + y + '" width="' + w + '" height="' + h + '" preserveAspectRatio="none" href="' + href + '"';
    e += ' transform="matrix(' + m[0] + ' ' + m[1] + ' ' + m[2] + ' ' + m[3] + ' ' + m[4] + ' ' + m[5] + ')"';
    e += '/>';
    r.elems.push(e);
});

// -- Finalize: build SVG string and optionally convert to PNG --
EM_JS(int, js_rb_finalize, (int mode), {
    var r = Module._rb;
    if (!r)
        return 0;
    // Use canvas dimensions from createSurface as viewBox (same as Cairo surface size)
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
    // Emit background rect at viewBox coordinates
    if (r.bgColor)
        svg += '<rect x="' + vx + '" y="' + vy + '" width="' + vw + '" height="' + vh + '" fill="' + r.bgColor + '"/>';
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
        // Node.js: use sharp (npm dependency) for SVG to PNG
        if (typeof require != 'undefined')
        {
            try
            {
                var sharp = require('sharp');
                var fs = require('fs');
                var os = require('os');
                var svgBuf = Buffer.from(svg);
                var pngPath = os.tmpdir() + '/_indigo_render.png';
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
        // Browser fallback: output SVG (use renderAsync for real PNG in browser)
        r.out = toBytes(svg);
        return r.out.length;
    }
    else
    { // PDF - stub
        r.out = toBytes('%PDF-1.4 stub');
        return r.out.length;
    }
});

// clang-format off
EM_JS(void, js_rb_copyOutput, (uint8_t* dst, int len), {
    var r = Module._rb;
    if (!r || !r.out)
        return;
    HEAPU8.set(r.out.subarray(0, len), dst);
});
// clang-format on

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
    height = _fontSize * 0.75; // Fix line spacing
    x_bearing = 0;
    y_bearing = -_fontSize * 0.8; // Maintain perfect sub/superscripts
}

void JSRenderBackend::fontExtents(double& height)
{
    height = _fontSize * 1.2;
}

void JSRenderBackend::showText(const char* text)
{
    // Shift the text down vertically to emulate Cairo's bounding box placement
    js_rb_fillText(text, _curX, _curY + _fontSize * 0.85);
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
void JSRenderBackend::drawPngImage(const void* data, int dataLen, double x, double y, double w, double h)
{
    js_rb_drawImage(static_cast<const uint8_t*>(data), dataLen, x, y, w, h);
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
