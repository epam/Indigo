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

#include "render_context.h"
#include "base_cpp/array.h"
#include "base_cpp/obj_array.h"
#include "base_cpp/output.h"
#include "molecule/molecule.h"
#include "reaction/reaction.h"

#include <limits.h>

using namespace indigo;

RenderContext::TextLock RenderContext::_tlock;

IMPL_ERROR(RenderContext, "render context");

#ifdef _WIN32

#define NOMINMAX
#include "cairo-win32.h"
#include <windows.h>

cairo_surface_t* RenderContext::createWin32PrintingSurfaceForHDC()
{
    cairo_surface_t* surface = cairo_win32_printing_surface_create((HDC)opt.hdc);
    cairoCheckStatus();
    return surface;
}

cairo_surface_t* RenderContext::createWin32Surface()
{
    cairo_surface_t* surface = cairo_win32_surface_create((HDC)opt.hdc);
    cairoCheckStatus();
    return surface;
}

static void _init_language_pack()
{
    typedef BOOL(WINAPI * gdi_init_lang_pack_func_t)(int);
    gdi_init_lang_pack_func_t gdi_init_lang_pack;
    HMODULE module;

    if (GetModuleHandleA("LPK.DLL"))
        return;

    module = GetModuleHandleA("GDI32.DLL");
    if (module)
    {
        gdi_init_lang_pack = (gdi_init_lang_pack_func_t)GetProcAddress(module, "GdiInitializeLanguagePack");
        if (gdi_init_lang_pack)
            gdi_init_lang_pack(0);
    }
}

cairo_surface_t* RenderContext::createWin32PrintingSurfaceForMetafile(bool& isLarge)
{
    HDC dc = GetDC(NULL);
    int hr = GetDeviceCaps(dc, HORZRES);
    int hs = GetDeviceCaps(dc, HORZSIZE);
    int vr = GetDeviceCaps(dc, VERTRES);
    int vs = GetDeviceCaps(dc, VERTSIZE);
    // physical display size in millimeters, divided over the resolution and
    //    multiplied by 100, as metafile dimensions are specified in 0.01mm units
    float cfx = hs * 100.0f / hr;
    float cfy = vs * 100.0f / vr; // it may differ for x and y
    int w = (int)(_width * cfx);
    int h = (int)(_height * cfy);
    RECT rc = {0, 0, w, h}, crc;
    _init_language_pack();
    GetClipBox(dc, &crc);
    isLarge = (_width >= crc.right || _height >= crc.bottom);
    _meta_hdc = CreateEnhMetaFileA(dc, 0, &rc, "Indigo Render2D\0\0");
    ReleaseDC(NULL, dc);
    cairo_surface_t* s = cairo_win32_printing_surface_create((HDC)_meta_hdc);
    cairoCheckStatus();
    StartPage((HDC)_meta_hdc);
    return s;
}

void RenderContext::storeAndDestroyMetafile(bool discard)
{
    cairo_surface_show_page(_surface);
    cairoCheckStatus();
    EndPage((HDC)_meta_hdc);
    cairo_surface_destroy(_surface);
    cairoCheckStatus();
    _surface = NULL;
    HENHMETAFILE hemf = CloseEnhMetaFile((HDC)_meta_hdc);
    if (!discard)
    {
        int size = GetEnhMetaFileBits(hemf, 0, NULL);
        Array<char> buf;
        buf.resize(size);
        GetEnhMetaFileBits(hemf, size, (BYTE*)(buf.ptr()));
        opt.output->writeArray(buf);
    }
    DeleteEnhMetaFile(hemf);
}

#endif

CP_DEF(RenderContext);

RenderContext::RenderContext(const RenderOptions& ropt, float sf, float lwf)
    : CP_INIT, TL_CP_GET(_fontfamily), TL_CP_GET(transforms), metafileFontsToCurves(false), _cr(NULL), _surface(NULL), _meta_hdc(NULL), opt(ropt),
      _pattern(NULL)
{
    _settings.init(sf, lwf);
    bprintf(_fontfamily, "Arial");
    bbmin.x = bbmin.y = 1;
    bbmax.x = bbmax.y = -1;
    _defaultScale = 0.0f;
}

void RenderContext::bbIncludePoint(const Vec2f& v)
{
    double x = v.x, y = v.y;
    cairo_user_to_device(_cr, &x, &y);
    Vec2f u((float)x, (float)y);
    if (bbmin.x > bbmax.x)
    { // init
        bbmin.x = bbmax.x = u.x;
        bbmin.y = bbmax.y = u.y;
    }
    else
    {
        bbmin.min(u);
        bbmax.max(u);
    }
}

void RenderContext::_bbVecToUser(Vec2f& d, const Vec2f& s)
{
    double x = s.x, y = s.y;
    cairo_device_to_user(_cr, &x, &y);
    d.set((float)x, (float)y);
}

void RenderContext::bbGetMin(Vec2f& v)
{
    _bbVecToUser(v, bbmin);
}

void RenderContext::bbGetMax(Vec2f& v)
{
    _bbVecToUser(v, bbmax);
}

void RenderContext::bbIncludePoint(double x, double y)
{
    Vec2f v((float)x, (float)y);
    bbIncludePoint(v);
}

void RenderContext::bbIncludePath(bool stroke)
{
    double x1, x2, y1, y2;
    if (stroke)
        cairo_stroke_extents(_cr, &x1, &y1, &x2, &y2);
    else
        cairo_path_extents(_cr, &x1, &y1, &x2, &y2);
    bbIncludePoint(x1, y1);
    bbIncludePoint(x2, y2);
}

void RenderContext::setDefaultScale(float scale)
{
    _defaultScale = scale;
}

void RenderContext::setFontFamily(const char* ff)
{
    bprintf(_fontfamily, "%s", ff);
}

int RenderContext::getMaxPageSize() const
{
    if (opt.mode == MODE_PDF)
        return 14400;
    return INT_MAX;
}

cairo_status_t RenderContext::writer(void* closure, const unsigned char* data, unsigned int length)
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

void RenderContext::createSurface(cairo_write_func_t writer, Output* output, int width, int height)
{
    int mode = opt.mode;
    if (writer == NULL && (mode == MODE_HDC || mode == MODE_PRN))
        mode = MODE_PDF;
    switch (mode)
    {
    case MODE_NONE:
        throw Error("mode not set");
    case MODE_PDF:
        _surface = cairo_pdf_surface_create_for_stream(writer, opt.output, _width, _height);
        cairoCheckSurfaceStatus();
        break;
    case MODE_SVG:
        _surface = cairo_svg_surface_create_for_stream(writer, opt.output, _width, _height);
        cairoCheckSurfaceStatus();
        break;
    case MODE_PNG:
        _surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, _width, _height);
        cairoCheckSurfaceStatus();
        break;
    case MODE_HDC:
#ifdef _WIN32
        _surface = createWin32Surface();
#else
        throw Error("mode \"HDC\" is not supported on this platform");
#endif
        break;
    case MODE_PRN:
#ifdef _WIN32
        _surface = createWin32PrintingSurfaceForHDC();
#else
        throw Error("mode \"PRN\" is not supported on this platform");
#endif
        break;
    case MODE_EMF:
#ifdef _WIN32
        bool isLarge;
        _surface = createWin32PrintingSurfaceForMetafile(isLarge);
        if (isLarge)
            metafileFontsToCurves = true;
#else
        throw Error("mode \"EMF\" is not supported on this platform");
#endif
        break;
    default:
        throw Error("unknown mode: %d", mode);
    }
}

void RenderContext::cairoCheckSurfaceStatus() const
{
    cairo_status_t s;
    s = cairo_surface_status(_surface);
    if (s != CAIRO_STATUS_SUCCESS)
        throw Error("Cairo error: %s\n", cairo_status_to_string(s));
}

void RenderContext::init()
{
    fontsInit();

    cairo_text_extents_t te;
    cairo_select_font_face(_cr, _fontfamily.ptr(), CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairoCheckStatus();
    cairo_set_font_size(_cr, _settings.fzz[FONT_SIZE_ATTR]);
    cairoCheckStatus();
    _tlock.lock();
    cairo_text_extents(_cr, "N", &te);
    _tlock.unlock();
    cairoCheckStatus();

    cairo_set_antialias(_cr, CAIRO_ANTIALIAS_GRAY);
    cairoCheckStatus();

    _currentLineWidth = _settings.bondLineWidth;
}

void RenderContext::fillBackground()
{
    cairo_set_source_rgb(_cr, opt.backgroundColor.x, opt.backgroundColor.y, opt.backgroundColor.z);
    cairoCheckStatus();
    cairo_paint(_cr);
    cairoCheckStatus();
}

void RenderContext::initNullContext()
{
    _width = 10;
    _height = 10;
    if (_surface != NULL || _cr != NULL)
        throw Error("context is already open (or invalid)");
    createSurface(NULL, NULL, 1, 1);
    cairoCheckStatus();
    _cr = cairo_create(_surface);
    scale(_defaultScale);
}

void RenderContext::initContext(int width, int height)
{
    _width = width;
    _height = height;
    if (opt.mode != MODE_HDC && opt.mode != MODE_PRN && opt.output == NULL)
        throw Error("output not set");
    if (_surface != NULL || _cr != NULL)
        throw Error("context is already open (or invalid)");

    createSurface(writer, opt.output, _width, _height);
    _cr = cairo_create(_surface);
    if (opt.backgroundColor.x >= 0 && opt.backgroundColor.y >= 0 && opt.backgroundColor.z >= 0)
        fillBackground();
}

void RenderContext::closeContext(bool discard)
{
    if (_cr != NULL)
    {
        cairo_destroy(_cr);
        _cr = NULL;
    }

    switch (opt.mode)
    {
    case MODE_NONE:
        throw Error("mode not set");
    case MODE_PNG:
        if (!discard)
            cairo_surface_write_to_png_stream(_surface, writer, opt.output);
        break;
    case MODE_PDF:
    case MODE_SVG:
    case MODE_HDC:
    case MODE_PRN:
        break;
    case MODE_EMF:
#ifdef _WIN32
        storeAndDestroyMetafile(discard);
#endif
        break;
    default:
        throw Error("unknown mode: %d", opt.mode);
    }

    if (_surface != NULL)
    {
        cairo_surface_destroy(_surface);
        _surface = NULL;
    }

    bbmin.x = bbmin.y = 1;
    bbmax.x = bbmax.y = -1;

    fontsDispose();
}

void RenderContext::translate(float dx, float dy)
{
    cairo_translate(_cr, dx, dy);
    cairoCheckStatus();
}

void RenderContext::scale(float s)
{
    cairo_scale(_cr, s, s);
    cairoCheckStatus();
}

void RenderContext::storeTransform()
{
    cairo_matrix_t& t = transforms.push();
    cairo_get_matrix(_cr, &t);
    cairoCheckStatus();
}
void RenderContext::restoreTransform()
{
    cairo_matrix_t& t = transforms.top();
    cairo_set_matrix(_cr, &t);
    cairoCheckStatus();
}
void RenderContext::removeStoredTransform()
{
    transforms.pop();
}

void RenderContext::resetTransform()
{
    cairo_matrix_t t;
    cairo_matrix_init_identity(&t);
    cairo_set_matrix(_cr, &t);
    cairoCheckStatus();
}

void RenderContext::setLineWidth(double width)
{
    _currentLineWidth = (float)width;
    cairo_set_line_width(_cr, width);
    cairoCheckStatus();
}

void RenderContext::drawRectangle(const Vec2f& p, const Vec2f& sz)
{
    cairo_rectangle(_cr, p.x, p.y, sz.x, sz.y);
    cairoCheckStatus();
    checkPathNonEmpty();
    cairo_fill(_cr);
    cairoCheckStatus();
}

void RenderContext::drawItemBackground(const RenderItem& item)
{
    cairo_rectangle(_cr, item.bbp.x, item.bbp.y, item.bbsz.x, item.bbsz.y);
    cairoCheckStatus();
    if (opt.backgroundColor.x >= 0 && opt.backgroundColor.y >= 0 && opt.backgroundColor.z >= 0)
    {
        setSingleSource(opt.backgroundColor);
        checkPathNonEmpty();
        cairo_fill(_cr);
        cairoCheckStatus();
    }
    else
    {
        /*
         * By default, we use transparent background
         * Fill the rectangle with the transparent color, invalidating it and
         * erasing everything underneath
         */
        cairo_save(_cr);
        cairoCheckStatus();
        cairo_set_source_rgba(_cr, 0, 0, 0, 0);
        cairoCheckStatus();
        cairo_set_operator(_cr, CAIRO_OPERATOR_SOURCE);
        cairoCheckStatus();
        cairo_fill(_cr);
        cairoCheckStatus();
        cairo_restore(_cr);
        cairoCheckStatus();
        return;
    }
}

void RenderContext::drawTextItemText(const TextItem& ti, bool idle)
{
    Vec3f color;
    if (ti.ritype == RenderItem::RIT_AAM)
        color.copy(opt.aamColor);
    else if (ti.ritype == RenderItem::RIT_DATASGROUP)
        color.copy(opt.dataGroupColor);
    else if (ti.ritype == RenderItem::RIT_COMMENT)
        color.copy(opt.commentColor);
    else if (ti.ritype == RenderItem::RIT_TITLE)
        color.copy(opt.titleColor);
    else
    {
        getColorVec(color, ti.color);
        if (ti.highlighted && opt.highlightColorEnable)
            color.copy(opt.highlightColor);
    }
    drawTextItemText(ti, color, idle);
}

void RenderContext::drawTextItemText(const TextItem& ti, const Vec3f& color, bool idle)
{
    bool bold = ti.highlighted && opt.highlightThicknessEnable;
    drawTextItemText(ti, color, bold, idle);
}

void RenderContext::drawTextItemText(const TextItem& ti, const Vec3f& color, bool bold, bool idle)
{
    fontsSetFont(_cr, ti.fontsize, bold);
    fontsDrawText(ti, color, bold, idle);
}

void RenderContext::drawLine(const Vec2f& v0, const Vec2f& v1)
{
    moveTo(v0);
    lineTo(v1);
    checkPathNonEmpty();
    bbIncludePath(true);
    cairo_stroke(_cr);
    cairoCheckStatus();
}

void RenderContext::drawPoly(const Array<Vec2f>& v)
{
    moveTo(v[0]);
    for (int i = 1; i < v.size(); ++i)
        lineTo(v[i]);
    lineTo(v[0]);
    checkPathNonEmpty();
    bbIncludePath(true);
    cairo_stroke(_cr);
    cairoCheckStatus();
}

void RenderContext::checkPathNonEmpty() const
{
#ifdef DEBUG
    cairo_path_t* p = cairo_copy_path(_cr);
    cairoCheckStatus();
    if (p->num_data == 0)
        throw Error("Empty path");
    cairo_path_destroy(p);
    cairoCheckStatus();
#endif
}

void RenderContext::fillQuad(const Vec2f& v0, const Vec2f& v1, const Vec2f& v2, const Vec2f& v3)
{
    moveTo(v0);
    lineTo(v1);
    lineTo(v2);
    lineTo(v3);
    checkPathNonEmpty();
    bbIncludePath(false);
    cairo_fill(_cr);
    cairoCheckStatus();
}

void RenderContext::fillHex(const Vec2f& v0, const Vec2f& v1, const Vec2f& v2, const Vec2f& v3, const Vec2f& v4, const Vec2f& v5)
{
    moveTo(v0);
    lineTo(v1);
    lineTo(v2);
    lineTo(v3);
    lineTo(v4);
    lineTo(v5);
    checkPathNonEmpty();
    bbIncludePath(false);
    cairo_fill(_cr);
    cairoCheckStatus();
}

void RenderContext::fillQuadStripes(const Vec2f& v0r, const Vec2f& v0l, const Vec2f& v1r, const Vec2f& v1l, int cnt)
{
    Vec2f r(v0r), dr;
    Vec2f l(v0l), dl;
    dr.diff(v1r, v0r);
    dr.scale(1.0f / cnt);
    dl.diff(v1l, v0l);
    dl.scale(1.0f / cnt);

    if (cnt < 3)
        cnt = 3;
    for (int i = 0; i < cnt; ++i)
    {
        r.add(dr);
        l.add(dl);
        moveTo(r);
        lineTo(l);
    }
    checkPathNonEmpty();
    bbIncludePath(true);
    cairo_stroke(_cr);
    cairoCheckStatus();
}

void RenderContext::fillPentagon(const Vec2f& v0, const Vec2f& v1, const Vec2f& v2, const Vec2f& v3, const Vec2f& v4)
{
    moveTo(v0);
    lineTo(v1);
    lineTo(v2);
    lineTo(v3);
    lineTo(v4);
    checkPathNonEmpty();
    bbIncludePath(false);
    cairo_fill(_cr);
    cairoCheckStatus();
}

void RenderContext::drawQuad(const Vec2f& v0, const Vec2f& v1, const Vec2f& v2, const Vec2f& v3)
{
    moveTo(v0);
    lineTo(v1);
    lineTo(v2);
    lineTo(v3);
    cairo_close_path(_cr);
    cairoCheckStatus();
    checkPathNonEmpty();
    bbIncludePath(true);
    cairo_stroke(_cr);
    cairoCheckStatus();
}

void RenderContext::drawTriangleZigzag(const Vec2f& v0, const Vec2f& v1, const Vec2f& v2, int cnt)
{
    Vec2f r(v0), dr;
    Vec2f l(v0), dl;
    dr.diff(v1, v0);
    dr.scale(1.0f / cnt);
    dl.diff(v2, v0);
    dl.scale(1.0f / cnt);

    cairo_set_line_join(_cr, CAIRO_LINE_JOIN_MITER);
    cairoCheckStatus();

    moveTo(v0);
    if (cnt < 3)
        cnt = 3;
    for (int i = 0; i < cnt; ++i)
    {
        r.add(dr);
        l.add(dl);
        if (i & 1)
            lineTo(l);
        else
            lineTo(r);
    }
    checkPathNonEmpty();
    bbIncludePath(true);
    cairo_stroke(_cr);
    cairoCheckStatus();
    cairo_set_line_join(_cr, CAIRO_LINE_JOIN_BEVEL);
    cairoCheckStatus();
}

void RenderContext::drawCircle(const Vec2f& center, const float r)
{
    cairo_new_path(_cr);
    cairo_arc(_cr, center.x, center.y, r, 0, 2 * M_PI);
    cairoCheckStatus();
    checkPathNonEmpty();
    bbIncludePath(true);
    cairo_stroke(_cr);
    cairoCheckStatus();
    cairo_new_path(_cr);
}

void RenderContext::fillCircle(const Vec2f& center, const float r)
{
    cairo_arc(_cr, center.x, center.y, r, 0, 2 * M_PI);
    cairoCheckStatus();
    checkPathNonEmpty();
    bbIncludePath(false);
    cairo_fill(_cr);
    cairoCheckStatus();
}

void RenderContext::drawArc(const Vec2f& center, const float r, const float a0, const float a1)
{
    cairo_new_path(_cr);
    cairoCheckStatus();
    cairo_arc(_cr, center.x, center.y, r, a0, a1);
    cairoCheckStatus();
    checkPathNonEmpty();
    bbIncludePath(true);
    cairo_stroke(_cr);
    cairoCheckStatus();
}

void RenderContext::setFontSize(double fontSize)
{
    cairo_set_font_size(_cr, fontSize);
    cairoCheckStatus();
}

void RenderContext::setTextItemSize(TextItem& ti)
{
    bool bold = ti.highlighted && opt.highlightThicknessEnable;

    fontsSetFont(_cr, ti.fontsize, bold);
    fontsGetTextExtents(_cr, ti.text.ptr(), ti.fontsize, ti.bbsz.x, ti.bbsz.y, ti.relpos.x, ti.relpos.y);
}

void RenderContext::setTextItemSize(TextItem& ti, const Vec2f& c)
{
    setTextItemSize(ti);

    cairo_font_extents_t fe;
    cairo_font_extents(_cr, &fe);
    ti.bbp.x = c.x - ti.bbsz.x / 2;
    ti.bbp.y = c.y - ti.bbsz.y / 2;
}

void RenderContext::setGraphItemSizeDot(GraphItem& gi)
{
    gi.type = GraphItem::DOT;
    gi.bbsz.set(2 * _settings.graphItemDotRadius, 2 * _settings.graphItemDotRadius);
    gi.relpos.set(_settings.graphItemDotRadius, _settings.graphItemDotRadius);
}

void RenderContext::setGraphItemSizeCap(GraphItem& gi)
{
    gi.type = GraphItem::CAP;
    gi.bbsz.set(2 * _settings.graphItemCapWidth, _settings.graphItemCapWidth * _settings.graphItemCapSlope);
    gi.relpos.set(0, _settings.graphItemCapWidth * _settings.graphItemCapSlope);
}

void RenderContext::setGraphItemSizeSign(GraphItem& gi, GraphItem::TYPE type)
{
    gi.type = type;
    gi.bbsz.set(_settings.graphItemDigitWidth, _settings.graphItemDigitHeight);
    gi.relpos.set(0, 0);
}

void RenderContext::drawAttachmentPoint(RenderItemAttachmentPoint& ri, bool idle)
{
    setSingleSource(ri.color);
    if (ri.highlighted && opt.highlightColorEnable)
        setSingleSource(opt.highlightColor);
    setLineWidth(_settings.unit);
    moveTo(ri.p0);
    lineTo(ri.p1);
    checkPathNonEmpty();
    bbIncludePath(false);
    cairo_stroke(_cr);
    cairoCheckStatus();

    Vec2f n;
    n.copy(ri.dir);
    n.rotateL(1, 0);
    Vec2f p, q, r;
    int waveCnt = 10;
    float waveLength = 0.5f;
    float waveWidth = waveLength / waveCnt;
    float slopeFactor = 0.2f;
    p.lineCombin(ri.p1, n, -0.5f * waveLength);
    moveTo(p);
    float step = waveLength / waveCnt;
    for (int i = 0; i < waveCnt; ++i)
    {
        int turn = ((i & 1) ? 1 : -1);
        q.lineCombin(p, ri.dir, waveWidth * turn);
        q.addScaled(n, waveWidth * slopeFactor);
        p.addScaled(n, step);
        r.lineCombin(p, ri.dir, waveWidth * turn);
        r.addScaled(n, -waveWidth * slopeFactor);
        cairo_curve_to(_cr, q.x, q.y, r.x, r.y, p.x, p.y);
    }
    checkPathNonEmpty();
    bbIncludePath(false);
    cairo_stroke(_cr);
    cairoCheckStatus();

    QS_DEF(TextItem, ti);
    ti.clear();
    if (ri.number > 0)
    {
        bprintf(ti.text, "%d", ri.number);
        ti.fontsize = FONT_SIZE_ATTACHMENT_POINT_INDEX;
        setTextItemSize(ti, ri.p1);
        float sz = ti.bbsz.length();
        ti.bbp.addScaled(n, -(sz / 2 + _settings.unit));
        ti.bbp.addScaled(ri.dir, -(sz / 2 + waveWidth + _settings.unit));
        drawTextItemText(ti, idle);
    }
}

void RenderContext::drawRSiteAttachmentIndex(RenderItemRSiteAttachmentIndex& ri)
{
    setSingleSource(ri.color);
    setLineWidth(_settings.unit / 2);
    drawCircle(ri.bbp, ri.radius);
}

void RenderContext::drawGraphItem(GraphItem& gi)
{
    setSingleSource(gi.color);
    if (gi.highlighted && opt.highlightColorEnable)
        setSingleSource(opt.highlightColor);
    _drawGraphItem(gi);
}

void RenderContext::drawGraphItem(GraphItem& gi, const Vec3f& color)
{
    setSingleSource(color);
    _drawGraphItem(gi);
}

void RenderContext::_drawGraphItem(GraphItem& gi)
{
    Vec2f v0;
    v0.sum(gi.bbp, gi.relpos);
    switch (gi.type)
    {
    case GraphItem::CAP:
        moveTo(v0);
        lineToRel(_settings.graphItemCapWidth, _settings.graphItemCapSlope * -_settings.graphItemCapWidth);
        lineToRel(_settings.graphItemCapWidth, _settings.graphItemCapSlope * _settings.graphItemCapWidth);
        lineToRel(-_settings.graphItemCapBase, 0);
        lineToRel(_settings.graphItemCapBase - _settings.graphItemCapWidth,
                  _settings.graphItemCapSlope * (_settings.graphItemCapBase - _settings.graphItemCapWidth));
        lineToRel(_settings.graphItemCapBase - _settings.graphItemCapWidth,
                  _settings.graphItemCapSlope * (_settings.graphItemCapWidth - _settings.graphItemCapBase));
        break;
    case GraphItem::DOT:
        moveTo(v0);
        cairo_arc(_cr, v0.x, v0.y, _settings.graphItemDotRadius, 0, 2 * M_PI);
        cairoCheckStatus();
        break;
    case GraphItem::PLUS:
        moveTo(v0);
        moveToRel(0, (_settings.graphItemDigitHeight - _settings.graphItemSignLineWidth) / 2);
        lineToRel(_settings.graphItemPlusEdge, 0);
        lineToRel(0, -_settings.graphItemPlusEdge);
        lineToRel(_settings.graphItemSignLineWidth, 0);
        lineToRel(0, _settings.graphItemPlusEdge);
        lineToRel(_settings.graphItemPlusEdge, 0);
        lineToRel(0, _settings.graphItemSignLineWidth);
        lineToRel(-_settings.graphItemPlusEdge, 0);
        lineToRel(0, _settings.graphItemPlusEdge);
        lineToRel(-_settings.graphItemSignLineWidth, 0);
        lineToRel(0, -_settings.graphItemPlusEdge);
        lineToRel(-_settings.graphItemPlusEdge, 0);
        break;
    case GraphItem::MINUS:
        moveTo(v0);
        moveToRel(0, (_settings.graphItemDigitHeight - _settings.graphItemSignLineWidth) / 2);
        lineToRel(_settings.graphItemDigitWidth, 0);
        lineToRel(0, _settings.graphItemSignLineWidth);
        lineToRel(-_settings.graphItemDigitWidth, 0);
        break;
    }
    checkPathNonEmpty();
    bbIncludePath(false);
    cairo_fill(_cr);
    cairoCheckStatus();
}

void RenderContext::drawBracket(RenderItemBracket& bracket)
{
    setSingleSource(bracket.color);
    setLineWidth(_settings.unit);

    Vec2f p;
    moveTo(bracket.q0);
    lineTo(bracket.p0);
    lineTo(bracket.p1);
    lineTo(bracket.q1);

    checkPathNonEmpty();
    bbIncludePath(false);
    cairo_stroke(_cr);
    cairoCheckStatus();
}

void RenderContext::fillRect(double x, double y, double w, double h)
{
    cairo_rectangle(_cr, x, y, w, h);
    cairoCheckStatus();
    checkPathNonEmpty();
    cairo_fill(_cr);
    cairoCheckStatus();
}

void RenderContext::drawEquality(const Vec2f& pos, const float linewidth, const float size, const float interval)
{
    moveTo(pos);
    moveToRel(0, -interval / 2);
    lineToRel(size, 0);
    moveTo(pos);
    moveToRel(0, interval / 2);
    lineToRel(size, 0);
    setLineWidth(linewidth);
    checkPathNonEmpty();
    bbIncludePath(true);
    cairo_stroke(_cr);
    cairoCheckStatus();
}

void RenderContext::drawPlus(const Vec2f& pos, const float linewidth, const float size)
{
    float hsz = size / 2;
    moveTo(pos);

    moveToRel(-hsz, 0);
    lineToRel(2 * hsz, 0);
    moveToRel(-hsz, -hsz);
    lineToRel(0, 2 * hsz);
    setLineWidth(linewidth);
    checkPathNonEmpty();
    bbIncludePath(true);
    cairo_stroke(_cr);
    cairoCheckStatus();
}

void RenderContext::drawArrow(const Vec2f& p1, const Vec2f& p2, const float width, const float headwidth, const float headsize)
{
    Vec2f d, n, p(p1);
    d.diff(p2, p1);
    float len = d.length();
    d.normalize();
    n.copy(d);
    n.rotate(1, 0);

    p.addScaled(n, width / 2);
    moveTo(p);
    p.addScaled(d, len - headsize);
    lineTo(p);
    p.addScaled(n, (headwidth - width) / 2);
    lineTo(p);
    p.addScaled(n, -headwidth / 2);
    p.addScaled(d, headsize);
    lineTo(p);
    p.addScaled(n, -headwidth / 2);
    p.addScaled(d, -headsize);
    lineTo(p);
    p.addScaled(n, (headwidth - width) / 2);
    lineTo(p);
    p.addScaled(d, -len + headsize);
    lineTo(p);
    checkPathNonEmpty();
    bbIncludePath(false);
    cairo_fill(_cr);
    cairoCheckStatus();
}

float RenderContext::highlightedBondLineWidth() const
{
    return _settings.bondLineWidth * (opt.highlightThicknessEnable ? opt.highlightThicknessFactor : 1.0f);
}

float RenderContext::currentLineWidth() const
{
    return _currentLineWidth;
}

void RenderContext::setHighlight()
{
    if (opt.highlightColorEnable)
        setSingleSource(opt.highlightColor);
    if (opt.highlightThicknessEnable)
        setLineWidth(opt.highlightThicknessFactor * _settings.bondLineWidth);
}

void RenderContext::resetHighlightThickness()
{
    setLineWidth(_settings.bondLineWidth);
}

void RenderContext::resetHighlight()
{
    setSingleSource(CWC_BASE);
    resetHighlightThickness();
}

void RenderContext::getColorVec(Vec3f& v, int color)
{
    getColor(v.x, v.y, v.z, color);
    float y, ymax = 0.5f;
    if (color >= CWC_COUNT)
    {
        y = 0.299f * v.x + 0.587f * v.y + 0.114f * v.z;
        if (y > ymax)
            v.scale(ymax / y);
    }
}

void RenderContext::setSingleSource(int color)
{
    Vec3f v;
    getColorVec(v, color);
    cairo_set_source_rgb(_cr, v.x, v.y, v.z);
    cairoCheckStatus();
}

void RenderContext::setSingleSource(const Vec3f& color)
{
    cairo_set_source_rgb(_cr, color.x, color.y, color.z);
    cairoCheckStatus();
}

void RenderContext::setGradientSource(const Vec3f& color1, const Vec3f& color2, const Vec2f& pos1, const Vec2f& pos2)
{
    if (_pattern != NULL)
    {
        throw new Error("Pattern already initialized");
    }

    _pattern = cairo_pattern_create_linear(pos1.x, pos1.y, pos2.x, pos2.y);
    cairo_pattern_add_color_stop_rgb(_pattern, 0, color1.x, color1.y, color1.z);
    cairo_pattern_add_color_stop_rgb(_pattern, 1, color2.x, color2.y, color2.z);
    cairo_set_source(_cr, _pattern);

    cairoCheckStatus();
}

void RenderContext::clearPattern()
{
    if (_pattern != NULL)
    {
        cairo_pattern_destroy(_pattern);
        _pattern = NULL;
        cairoCheckStatus();
    }
}

float RenderContext::_getDashedLineAlignmentOffset(float length)
{
    float offset = 0;
    float delta = length - floorf(length / _settings.dashUnit);
    if (delta > 0.5)
        offset = 1 - delta - _settings.eps * _settings.dashUnit;
    else
        offset = -delta - _settings.eps * _settings.dashUnit;
    return offset;
}

void RenderContext::setDash(const Array<double>& dash, float length)
{
    cairo_set_dash(_cr, dash.ptr(), dash.size(), _getDashedLineAlignmentOffset(length));
    cairoCheckStatus();
}

void RenderContext::resetDash()
{
    cairo_set_dash(_cr, NULL, 0, 0);
    cairoCheckStatus();
}

void RenderContext::lineTo(const Vec2f& v)
{
    cairo_line_to(_cr, v.x, v.y);
    cairoCheckStatus();
}

void RenderContext::lineToRel(float x, float y)
{
    cairo_rel_line_to(_cr, x, y);
    cairoCheckStatus();
}

void RenderContext::lineToRel(const Vec2f& v)
{
    cairo_rel_line_to(_cr, v.x, v.y);
    cairoCheckStatus();
}

void RenderContext::moveTo(const Vec2f& v)
{
    cairo_move_to(_cr, v.x, v.y);
    cairoCheckStatus();
}

void RenderContext::moveToRel(float x, float y)
{
    cairo_rel_move_to(_cr, x, y);
    cairoCheckStatus();
}

void RenderContext::moveToRel(const Vec2f& v)
{
    cairo_rel_move_to(_cr, v.x, v.y);
    cairoCheckStatus();
}

int RenderContext::getElementColor(int label)
{
    return label - ELEM_H + CWC_COUNT;
}

void RenderContext::getColor(float& r, float& g, float& b, int c)
{
    static double colors[][3] = {
        {1.0f, 1.0f, 1.0f}, // WHITE
        {0.0f, 0.0f, 0.0f}, // BLACK
        {1.0f, 0.0f, 0.0f}, // RED
        {0.0f, 0.8f, 0.0f}, // GREEN
        {0.0f, 0.0f, 1.0f}, // BLUE
        {0.0f, 0.5f, 0.0f}, // DARKGREEN

        {0.0, 0.0, 0.0},  // ELEM_H
        {0.85, 1.0, 1.0}, // ....
        {0.80, 0.50, 1.0},   {0.76, 1.0, 0},     {1.0, 0.71, 0.71},  {0.0, 0.0, 0.0},     {0.19, 0.31, 0.97},  {1.0, 0.051, 0.051}, {0.56, 0.88, 0.31},
        {0.70, 0.89, 0.96},  {0.67, 0.36, 0.95}, {0.54, 1.0, 0},     {0.75, 0.65, 0.65},  {0.94, 0.78, 0.63},  {1.0, 0.50, 0},      {0.85, 0.65, 0.10},
        {0.12, 0.94, 0.12},  {0.50, 0.82, 0.89}, {0.56, 0.25, 0.83}, {0.24, 1.0, 0},      {0.90, 0.90, 0.90},  {0.75, 0.76, 0.78},  {0.65, 0.65, 0.67},
        {0.54, 0.60, 0.78},  {0.61, 0.48, 0.78}, {0.88, 0.40, 0.20}, {0.94, 0.56, 0.63},  {0.31, 0.82, 0.31},  {0.78, 0.50, 0.20},  {0.49, 0.50, 0.69},
        {0.76, 0.56, 0.56},  {0.40, 0.56, 0.56}, {0.74, 0.50, 0.89}, {1.0, 0.63, 0},      {0.65, 0.16, 0.16},  {0.36, 0.72, 0.82},  {0.44, 0.18, 0.69},
        {0, 1.0, 0},         {0.58, 1.0, 1.0},   {0.58, 0.88, 0.88}, {0.45, 0.76, 0.79},  {0.33, 0.71, 0.71},  {0.23, 0.62, 0.62},  {0.14, 0.56, 0.56},
        {0.039, 0.49, 0.55}, {0, 0.41, 0.52},    {0.75, 0.75, 0.75}, {1.0, 0.85, 0.56},   {0.65, 0.46, 0.45},  {0.40, 0.50, 0.50},  {0.62, 0.39, 0.71},
        {0.83, 0.48, 0},     {0.58, 0, 0.58},    {0.26, 0.62, 0.69}, {0.34, 0.090, 0.56}, {0, 0.79, 0},        {0.44, 0.83, 1.0},   {1.0, 1.0, 0.78},
        {0.85, 1.0, 0.78},   {0.78, 1.0, 0.78},  {0.64, 1.0, 0.78},  {0.56, 1.0, 0.78},   {0.38, 1.0, 0.78},   {0.27, 1.0, 0.78},   {0.19, 1.0, 0.78},
        {0.12, 1.0, 0.78},   {0, 1.0, 0.61},     {0, 0.90, 0.46},    {0, 0.83, 0.32},     {0, 0.75, 0.22},     {0, 0.67, 0.14},     {0.30, 0.76, 1.0},
        {0.30, 0.65, 1.0},   {0.13, 0.58, 0.84}, {0.15, 0.49, 0.67}, {0.15, 0.40, 0.59},  {0.090, 0.33, 0.53}, {0.82, 0.82, 0.88},  {1.0, 0.82, 0.14},
        {0.72, 0.72, 0.82},  {0.65, 0.33, 0.30}, {0.34, 0.35, 0.38}, {0.62, 0.31, 0.71},  {0.67, 0.36, 0},     {0.46, 0.31, 0.27},  {0.26, 0.51, 0.59},
        {0.26, 0, 0.40},     {0, 0.49, 0},       {0.44, 0.67, 0.98}, {0, 0.73, 1.0},      {0, 0.63, 1.0},      {0, 0.56, 1.0},      {0, 0.50, 1.0},
        {0, 0.42, 1.0},      {0.33, 0.36, 0.95}, {0.47, 0.36, 0.89}, {0.54, 0.31, 0.89},  {0.63, 0.21, 0.83},  {0.70, 0.12, 0.83},  {0.70, 0.22, 0.83},
        {0.60, 0.12, 0.83},  {0.50, 0.12, 0.83}, {0.70, 0.12, 0.63}, {0.70, 0.12, 0.83},  {0.70, 0.32, 0.83},  {0.60, 0.12, 0.83},  {0.70, 0.12, 0.43}};

    if (c == CWC_BASE)
    {
        r = (float)opt.baseColor.x;
        g = (float)opt.baseColor.y;
        b = (float)opt.baseColor.z;
        return;
    }

    if (c < 0 || c >= NELEM(colors))
        throw Error("unknown color: %d", c);

    r = (float)colors[c][0];
    g = (float)colors[c][1];
    b = (float)colors[c][2];
}
