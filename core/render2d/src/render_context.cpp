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
#include "base_cpp/output.h"

#ifdef __EMSCRIPTEN__
#include "js_render_backend.h"
#else
#include "cairo_render_backend.h"
#endif
#include "molecule/meta_commons.h"

#include <limits.h>

using namespace indigo;

std::mutex RenderContext::_mutex;

IMPL_ERROR(RenderContext, "render context");

#ifdef _WIN32

#include "cairo-win32.h"
#include <windows.h>

void* RenderContext::createWin32PrintingSurfaceForHDC()
{
    cairo_surface_t* surface = cairo_win32_printing_surface_create((HDC)opt.hdc);
    backendCheckStatus();
    return surface;
}

void* RenderContext::createWin32Surface()
{
    cairo_surface_t* surface = cairo_win32_surface_create((HDC)opt.hdc);
    backendCheckStatus();
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

void* RenderContext::createWin32PrintingSurfaceForMetafile(bool& isLarge)
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
    backendCheckStatus();
    StartPage((HDC)_meta_hdc);
    return s;
}

void RenderContext::storeAndDestroyMetafile(bool discard)
{
    auto* cairoBackend = dynamic_cast<CairoRenderBackend*>(_backend.get());
    cairo_surface_t* surface = cairoBackend ? cairoBackend->getSurface() : nullptr;
    if (surface)
    {
        cairo_surface_show_page(surface);
        backendCheckStatus();
    }
    EndPage((HDC)_meta_hdc);
    if (surface)
    {
        cairo_surface_destroy(surface);
        backendCheckStatus();
    }
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

#ifdef _MSC_VER
#undef min
#undef max
#endif

#endif

CP_DEF(RenderContext);

RenderContext::RenderContext(const RenderOptions& ropt, float relativeThickness, float bondLineWidthFactor)
    : CP_INIT, TL_CP_GET(_fontfamily), TL_CP_GET(transforms), metafileFontsToCurves(false), _meta_hdc(NULL), opt(ropt), _settings()
{
    AcsOptions acs;
    if (ropt.fontSize > 0)
        acs.fontSizeAngstrom = UnitsOfMeasure::convertToPx(ropt.fontSize, ropt.fontSizeUnit, ropt.ppi) / ropt.bond_length_px;
    if (ropt.fontSizeSub > 0)
        acs.fontSizeSubAngstrom = UnitsOfMeasure::convertToPx(ropt.fontSizeSub, ropt.fontSizeSubUnit, ropt.ppi) / ropt.bond_length_px;
    if (ropt.bondThickness > 0)
        acs.bondThicknessAngstrom = UnitsOfMeasure::convertToPx(ropt.bondThickness, ropt.bondThicknessUnit, ropt.ppi) / ropt.bond_length_px;
    if (ropt.stereoBondWidth > 0)
        acs.stereoBondWidthAngstrom = UnitsOfMeasure::convertToPx(ropt.stereoBondWidth, ropt.stereoBondWidthUnit, ropt.ppi) / ropt.bond_length_px;
    if (ropt.hashSpacing > 0)
        acs.hashSpacingAngstrom = UnitsOfMeasure::convertToPx(ropt.hashSpacing, ropt.hashSpacingUnit, ropt.ppi) / ropt.bond_length_px;
    if (ropt.bondSpacing > 0)
        acs.bondSpacing = ropt.bondSpacing;
    _settings.init(relativeThickness, bondLineWidthFactor, &acs);

    bprintf(_fontfamily, "Arial");
    bbmin.x = bbmin.y = 1;
    bbmax.x = bbmax.y = -1;
    _defaultScale = 0.0f;

#ifdef __EMSCRIPTEN__
    _backend = std::make_unique<JSRenderBackend>();
#else
    _backend = std::make_unique<CairoRenderBackend>();
#endif
}

void RenderContext::bbIncludePoint(const Vec2f& v)
{
    double x = v.x, y = v.y;
    _backend->userToDevice(x, y);
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
    _backend->deviceToUser(x, y);
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
        _backend->strokeExtents(x1, y1, x2, y2);
    else
        _backend->pathExtents(x1, y1, x2, y2);
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

void RenderContext::createSurface(int /*width*/, int /*height*/)
{
    int rbMode = RBMODE_PNG;
    switch (opt.mode)
    {
    case MODE_NONE:
        throw Error("mode not set");
    case MODE_PDF:
        rbMode = RBMODE_PDF;
        break;
    case MODE_SVG:
        rbMode = RBMODE_SVG;
        break;
    case MODE_PNG:
        rbMode = RBMODE_PNG;
        break;
    default:
        rbMode = RBMODE_PNG;
        break;
    }
    _backend->createSurface(rbMode, _width, _height, opt.output);
}

void RenderContext::backendCheckStatus() const
{
    // Status checking is handled internally by each backend
}

void RenderContext::init()
{
    fontsInit();
    _backend->selectFontFace(_fontfamily.ptr(), false, false);
    _backend->setFontSize(_settings.fzz[FONT_SIZE_ATTR]);
    _backend->setAntialias(0);

    _currentLineWidth = _settings.bondLineWidth;
}

void RenderContext::fillBackground()
{
    _backend->setSourceRGB(opt.backgroundColor.x, opt.backgroundColor.y, opt.backgroundColor.z);
    backendCheckStatus();
    _backend->paint();
    backendCheckStatus();
}

void RenderContext::initNullContext()
{
    _width = 10;
    _height = 10;
    // Backend manages its own surface/context state
    createSurface(1, 1);
    _backend->createContext();
    scale(_defaultScale);
}

void RenderContext::initContext(int width, int height)
{
    _width = width;
    _height = height;
    if (opt.mode != MODE_HDC && opt.mode != MODE_PRN && opt.output == NULL)
        throw Error("output not set");
    // Backend manages its own surface/context state

    createSurface(_width, _height);
    _backend->createContext();
    if (opt.backgroundColor.x >= 0 && opt.backgroundColor.y >= 0 && opt.backgroundColor.z >= 0)
        fillBackground();
}

void RenderContext::closeContext(bool discard)
{
    _backend->destroyContext();

    int rbMode = RBMODE_PNG;
    switch (opt.mode)
    {
    case MODE_PDF:
        rbMode = RBMODE_PDF;
        break;
    case MODE_SVG:
        rbMode = RBMODE_SVG;
        break;
    case MODE_PNG:
        rbMode = RBMODE_PNG;
        break;
    default:
        rbMode = RBMODE_PNG;
        break;
    }
    _backend->closeSurface(rbMode, discard, opt.output);

    bbmin.x = bbmin.y = 1;
    bbmax.x = bbmax.y = -1;

    fontsDispose();
}

void RenderContext::translate(float dx, float dy)
{
    _backend->translate(dx, dy);
    backendCheckStatus();
}

void RenderContext::scale(float s)
{
    _backend->scale(s, s);
    backendCheckStatus();
}

void RenderContext::storeTransform()
{
    RenderMatrix& t = transforms.push();
    _backend->getMatrix(t.m);
    backendCheckStatus();
}

void RenderContext::restoreTransform()
{
    std::lock_guard<std::mutex> _lock(_mutex);
    RenderMatrix& t = transforms.top();
    _backend->setMatrix(t.m);
    backendCheckStatus();
}

void RenderContext::removeStoredTransform()
{
    transforms.pop();
}

void RenderContext::resetTransform()
{
    RenderMatrix t;
    _backend->initIdentityMatrix(t.m);
    _backend->setMatrix(t.m);
    backendCheckStatus();
}

void RenderContext::setLineWidth(double width)
{
    _currentLineWidth = (float)width;
    _backend->setLineWidth(width);
    backendCheckStatus();
}

void RenderContext::drawRectangle(const Vec2f& p, const Vec2f& sz)
{
    _backend->rect(p.x, p.y, sz.x, sz.y);
    backendCheckStatus();
    checkPathNonEmpty();
    _backend->fill();
    backendCheckStatus();
}

void RenderContext::drawEllipse(const Vec2f& v1, const Vec2f& v2)
{
    Rect2f bbox(v1, v2);
    auto width = bbox.width();
    auto height = bbox.height();
    RenderMatrix save_matrix;
    _backend->getMatrix(save_matrix.m);
    _backend->translate(bbox.center().x, bbox.center().y);
    _backend->scale(1, height / width);
    _backend->translate(-bbox.center().x, -bbox.center().y);
    _backend->arc(bbox.center().x, bbox.center().y, width / 2.0, 0, 2 * M_PI);
    _backend->setMatrix(save_matrix.m);
    checkPathNonEmpty();
    bbIncludePath(true);
    _backend->stroke();
    backendCheckStatus();
}

void RenderContext::drawItemBackground(const RenderItem& item)
{
    _backend->rect(item.bbp.x, item.bbp.y, item.bbsz.x, item.bbsz.y);
    backendCheckStatus();
    if (opt.backgroundColor.x >= 0 && opt.backgroundColor.y >= 0 && opt.backgroundColor.z >= 0)
    {
        setSingleSource(opt.backgroundColor);
        checkPathNonEmpty();
        _backend->fill();
        backendCheckStatus();
    }
    else
    {
        /*
         * By default, we use transparent background
         * Fill the rectangle with the transparent color, invalidating it and
         * erasing everything underneath
         */
        _backend->save();
        backendCheckStatus();
        _backend->setSourceRGBA(0, 0, 0, 0);
        backendCheckStatus();
        _backend->setOperator(1);
        backendCheckStatus();
        _backend->fill();
        backendCheckStatus();
        _backend->restore();
        backendCheckStatus();
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
    TextItem ti_mod(ti);
    if (!ti_mod.bold)
        ti_mod.bold = ti.highlighted && opt.highlightThicknessEnable;
    ti_mod.bbp.y += ti.script_type == 0 ? 0 : (ti.script_type == 1 ? -ti_mod.relpos.y / 2 : ti_mod.relpos.y / 2);
    fontsSetFont(ti_mod);
    fontsDrawText(ti_mod, color, idle);
}

void RenderContext::drawPng(const std::string& pngData, const Rect2f& bbox)
{
    _backend->drawPngImage(pngData.data(), static_cast<int>(pngData.size()), bbox.left(), bbox.bottom(), bbox.width(), bbox.height());
    bbIncludePoint(bbox.leftTop());
    bbIncludePoint(bbox.rightBottom());
}

void RenderContext::drawLine(const Vec2f& v0, const Vec2f& v1)
{
    moveTo(v0);
    lineTo(v1);
    checkPathNonEmpty();
    bbIncludePath(true);
    {
        std::lock_guard<std::mutex> _lock(_mutex);
        _backend->stroke();
    }
    backendCheckStatus();
}

void RenderContext::drawPoly(const Array<Vec2f>& v)
{
    moveTo(v[0]);
    for (int i = 1; i < v.size(); ++i)
        lineTo(v[i]);
    lineTo(v[0]);
    checkPathNonEmpty();
    bbIncludePath(true);
    _backend->stroke();
    backendCheckStatus();
}

void RenderContext::checkPathNonEmpty() const
{
#ifdef DEBUG
    if (_backend->isPathEmpty())
        throw Error("Empty path");
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
    _backend->fill();
    backendCheckStatus();
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
    _backend->fill();
    backendCheckStatus();
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
    _backend->stroke();
    backendCheckStatus();
}

void RenderContext::fillQuadStripesSpacing(const Vec2f& v0r, const Vec2f& v0l, const Vec2f& v1r, const Vec2f& v1l, float spacing)
{
    Vec2f r(v0r), dr;
    Vec2f l(v0l), dl;
    Vec2f v;
    dr.diff(v1r, v0r);
    dl.diff(v1l, v0l);
    v.diff(v1l, v1r);
    float dr_len = dr.lengthSqr();
    float dl_len = dl.lengthSqr();

    dr.normalize();
    dr.scale(std::fabs(dr.vsin(v)) * spacing);
    dl.normalize();
    dl.scale(std::fabs(dl.vsin(v)) * spacing);

    while (true)
    {
        r.add(dr);
        l.add(dl);
        if (Vec2f::distSqr(v0r, r) > dr_len || Vec2f::distSqr(v0l, l) > dl_len)
            break;
        moveTo(r);
        lineTo(l);
    }
    checkPathNonEmpty();
    bbIncludePath(true);
    _backend->stroke();
    backendCheckStatus();
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
    _backend->fill();
    backendCheckStatus();
}

void RenderContext::drawQuad(const Vec2f& v0, const Vec2f& v1, const Vec2f& v2, const Vec2f& v3)
{
    moveTo(v0);
    lineTo(v1);
    lineTo(v2);
    lineTo(v3);
    _backend->closePath();
    backendCheckStatus();
    checkPathNonEmpty();
    bbIncludePath(true);
    _backend->stroke();
    backendCheckStatus();
}

void RenderContext::drawTriangleZigzag(const Vec2f& v0, const Vec2f& v1, const Vec2f& v2, int cnt)
{
    Vec2f r(v0), dr;
    Vec2f l(v0), dl;
    dr.diff(v1, v0);
    dr.scale(1.0f / cnt);
    dl.diff(v2, v0);
    dl.scale(1.0f / cnt);

    _backend->setLineJoin(0);
    backendCheckStatus();

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
    _backend->stroke();
    backendCheckStatus();
    _backend->setLineJoin(2);
    backendCheckStatus();
}

void RenderContext::drawCircle(const Vec2f& center, const float r)
{
    _backend->beginPath();
    _arc(center.x, center.y, r, 0, 2 * M_PI);
    backendCheckStatus();
    checkPathNonEmpty();
    bbIncludePath(true);
    _backend->stroke();
    backendCheckStatus();
    _backend->beginPath();
}

void RenderContext::fillCircle(const Vec2f& center, const float r)
{
    _arc(center.x, center.y, r, 0, 2 * M_PI);
    backendCheckStatus();
    checkPathNonEmpty();
    bbIncludePath(false);
    _backend->fill();
    backendCheckStatus();
}

void RenderContext::drawArc(const Vec2f& center, const float r, const float a0, const float a1)
{
    _backend->beginPath();
    backendCheckStatus();
    _arc(center.x, center.y, r, a0, a1);
    backendCheckStatus();
    checkPathNonEmpty();
    bbIncludePath(true);
    _backend->stroke();
    backendCheckStatus();
}

void RenderContext::_arc(double xc, double yc, double radius, double angle1, double angle2)
{
#ifdef __EMSCRIPTEN__
    // In the WASM build this workaround fixes function signature issues with cairo_arc.
    while (angle2 < angle1)
        angle2 += 2 * M_PI;
    const double arc_parts = 36;
    double diff = angle2 - angle1;
    double phi = angle1;
    double step = diff / arc_parts;
    for (int i = 0; i <= arc_parts; ++i)
    {
        Vec2f p(radius * cos(phi) + xc, radius * sin(phi) + yc);
        if (i)
            _backend->lineTo(p.x, p.y);
        else
            _backend->moveTo(p.x, p.y);
        phi += step;
    }
#else
    _backend->arc(xc, yc, radius, angle1, angle2);
#endif
}

void RenderContext::setFontSize(double fontSize)
{
    _backend->setFontSize(fontSize);
    backendCheckStatus();
}

double RenderContext::getFontExtentHeight()
{
    double h;
    _backend->fontExtents(h);
    return h;
}

void RenderContext::setTextItemSize(TextItem& ti)
{
    if (!ti.bold)
        ti.bold = ti.highlighted && opt.highlightThicknessEnable;
    fontsSetFont(ti);
    fontsGetTextExtents(ti.text.ptr(), ti.fontsize, ti.bbsz.x, ti.bbsz.y, ti.relpos.x, ti.relpos.y);
}

void RenderContext::setTextItemSize(TextItem& ti, const Vec2f& c)
{
    setTextItemSize(ti);
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
    setLineWidth(_settings.bondLineWidth);
    moveTo(ri.p0);
    lineTo(ri.p1);
    checkPathNonEmpty();
    bbIncludePath(false);
    _backend->stroke();
    backendCheckStatus();

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
        _backend->curveTo(q.x, q.y, r.x, r.y, p.x, p.y);
    }
    checkPathNonEmpty();
    bbIncludePath(false);
    _backend->stroke();
    backendCheckStatus();

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
        _arc(v0.x, v0.y, _settings.graphItemDotRadius, 0, 2 * M_PI);
        backendCheckStatus();
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
    _backend->fill();
    backendCheckStatus();
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
    _backend->stroke();
    backendCheckStatus();
}

void RenderContext::fillRect(double x, double y, double w, double h)
{
    _backend->rect(x, y, w, h);
    backendCheckStatus();
    checkPathNonEmpty();
    _backend->fill();
    backendCheckStatus();
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
    _backend->stroke();
    backendCheckStatus();
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
    _backend->stroke();
    backendCheckStatus();
}

void RenderContext::drawHalfEllipse(const Vec2f& v1, const Vec2f& v2, const float height, const bool is_negative)
{
    float h = std::fabs(height);
    double angle1 = height > 0 ? -M_PI : 0;
    double angle2 = height > 0 ? 0 : M_PI;
    Vec2f d;
    d.diff(v2, v1);
    float width = d.length();
    RenderMatrix save_matrix;
    _backend->getMatrix(save_matrix.m);
    _backend->translate((v1.x + v2.x) / 2.0, (v1.y + v2.y) / 2.0);
    _backend->rotate(atan2(d.y, d.x));
    _backend->scale(1, 2 * h / width);
    _backend->translate(-(v1.x + v2.x) / 2.0, -(v1.y + v2.y) / 2.0);
    if (is_negative)
        _backend->arcNegative((v1.x + v2.x) / 2.0, (v1.y + v2.y) / 2.0, width / 2.0, angle1, angle2);
    else
        _backend->arc((v1.x + v2.x) / 2.0, (v1.y + v2.y) / 2.0, width / 2.0, angle1, angle2);
    _backend->setMatrix(save_matrix.m);
}

void RenderContext::drawTriangleArrowHeader(const Vec2f& v, const Vec2f& dir, const float /*width*/, const float headwidth, const float headsize)
{
    Vec2f n(dir), p(v), d(dir);
    n.rotate(1, 0);
    d.negate();
    auto arr_wc = headwidth / 2;
    auto arr_hyp = std::hypot(arr_wc, headsize);
    auto cs = headsize / arr_hyp;
    auto si = arr_wc / arr_hyp;
    Vec2f back_vector(d);
    back_vector.rotate(si, cs);
    moveTo(p);
    p.addScaled(back_vector, arr_hyp);
    lineTo(p);
    p.addScaled(n, headwidth);
    lineTo(p);
    lineTo(v);
}

void RenderContext::drawHalfArrowHeader(const Vec2f& v, const Vec2f& dir, const float width, const float headwidth, const float headsize,
                                        const ArrowType arrow_type)
{
    Vec2f n(dir), p(v), d(dir);
    n.rotate(1, 0);
    p.addScaled(n, width / 2);
    Vec2f header(p);
    moveTo(p);
    d.negate();
    auto arr_wc = headwidth / 2 + width / 2;
    auto arr_hyp = std::hypot(arr_wc, headsize);
    auto cs = headsize / arr_hyp;
    auto si = arr_wc / arr_hyp;
    auto arr_h = (arr_wc * headsize) / arr_hyp;
    auto inner_w = arr_wc * (arr_h - width) / arr_h;
    auto inner_h = inner_w * headsize / arr_wc;
    auto inner_hyp = std::hypot(inner_w, inner_h);
    d.rotate(si, cs);
    p.addScaled(d, arr_hyp);
    lineTo(p);
    if (arrow_type == ArrowType::ETriangleArrow)
    {
        p.addScaled(n, arr_wc);
    }
    else
    {
        p.addScaled(n, width / cs);
        if (arrow_type == ArrowType::EOpenArrow)
            lineTo(p);
        d.negate();
        p.addScaled(d, inner_hyp);
    }
    lineTo(p);
    lineTo(header);
}

void RenderContext::drawArrowHeader(const Vec2f& v, const Vec2f& dir, const float width, const float headwidth, const float headsize, bool is_bow)
{
    Vec2f n(dir), p(v), d(dir);
    n.rotate(1, 0);
    d.negate();
    auto arr_wc = headwidth / 2;
    auto arr_hyp = std::hypot(arr_wc, headsize);
    auto arr_h = (arr_wc * headsize) / arr_hyp;
    auto inner_w = arr_wc * (arr_h - width) / arr_h;
    auto inner_h = inner_w * headsize / arr_wc;
    auto inner_hyp = std::hypot(inner_w, inner_h);
    auto cs = headsize / arr_hyp;
    auto si = arr_wc / arr_hyp;
    Vec2f back_vector(d);
    back_vector.rotate(si, cs);
    moveTo(p);
    p.addScaled(back_vector, arr_hyp);
    lineTo(p);
    p.addScaled(n, arr_wc - inner_w);
    if (!is_bow)
        lineTo(p);
    back_vector.negate();
    p.addScaled(back_vector, inner_hyp);
    lineTo(p);
    back_vector.set(d.x, d.y);
    back_vector.rotate(-si, cs);
    p.addScaled(back_vector, inner_hyp);
    if (!is_bow)
        lineTo(p);
    p.addScaled(n, arr_wc - inner_w);
    lineTo(p);
    back_vector.negate();
    p.addScaled(back_vector, arr_hyp);
    lineTo(p);
}

void RenderContext::drawEllipticalArrow(const Vec2f& p1, const Vec2f& p2, const float width, const float headwidth, const float headsize, const float height,
                                        int arrow_type)
{
    float h_sign = height > 0 ? 1.f : -1.f;
    Vec2f d, n_orig, pa(p1), pb(p2);
    d.diff(p2, p1);
    d.normalize();
    n_orig.copy(d);
    n_orig.rotate(-h_sign, 0);
    pb.addScaled(n_orig, width * 2); // margin for headsize
    d.diff(pb, pa);
    d.normalize();
    Vec2f n(d);
    n.rotate(-h_sign, 0);

    // float len = d.length();

    n_orig.negate();
    switch (arrow_type)
    {
    case ReactionArrowObject::EEllipticalArcFilledBow:
        drawArrowHeader(p2, n_orig, width, headwidth, headsize, true);
        break;

    case ReactionArrowObject::EEllipticalArcFilledTriangle:
        drawTriangleArrowHeader(p2, n_orig, width, headwidth, headsize);
        break;
    case ReactionArrowObject::EEllipticalArcOpenAngle:
        drawArrowHeader(p2, n_orig, width, headwidth, headsize);
        break;
    case ReactionArrowObject::EEllipticalArcOpenHalfAngle:
        drawHalfArrowHeader(p2, n_orig, width, headwidth, headsize);
        break;
    }
    _backend->fill();
    pb.addScaled(d, width / 2); // go forward to outer ellipse
    d.negate();                 // backward
    pa.addScaled(d, width / 2); // back to outer ellipse
    drawHalfEllipse(pa, pb, height);
    moveTo(pb);
    d.negate();
    pa.addScaled(d, width);
    d.negate();
    pb.addScaled(d, width);
    lineTo(pb);
    drawHalfEllipse(pb, pa, height - width * h_sign, true);
    checkPathNonEmpty();
    bbIncludePath(false);
    _backend->fill();
    backendCheckStatus();
}

void RenderContext::drawBothEndsArrow(const Vec2f& p1, const Vec2f& p2, const float width, const float headwidth, const float headsize)
{
    Vec2f d, n, p(p1);
    d.diff(p2, p1);
    float len = d.length();
    d.normalize();
    n.copy(d);
    n.rotate(1, 0);
    auto arr_wc = headwidth / 2;
    auto arr_hyp = std::hypot(arr_wc, headsize);
    auto cs = headsize / arr_hyp;
    auto si = arr_wc / arr_hyp;
    moveTo(p);
    Vec2f arrow_side1(d), arrow_side2(d);
    arrow_side1.rotate(-si, cs);
    arrow_side2.rotate(si, cs);
    p.addScaled(arrow_side1, arr_hyp);
    lineTo(p);
    p.addScaled(n, arr_wc - width / 2);
    lineTo(p);
    p.addScaled(d, len - headsize * 2);
    lineTo(p);
    n.negate();
    p.addScaled(n, arr_wc - width / 2);
    lineTo(p);
    p.addScaled(arrow_side2, arr_hyp);
    lineTo(p);
    arrow_side1.negate();
    p.addScaled(arrow_side1, arr_hyp);
    lineTo(p);
    p.addScaled(n, arr_wc - width / 2);
    lineTo(p);
    d.negate();
    p.addScaled(d, len - headsize * 2);
    lineTo(p);
    n.negate();
    p.addScaled(n, arr_wc - width / 2);
    lineTo(p);
    arrow_side2.negate();
    p.addScaled(arrow_side2, arr_hyp);
    lineTo(p);
    checkPathNonEmpty();
    bbIncludePath(false);
    _backend->fill();
    backendCheckStatus();
}

void RenderContext::drawDashedArrow(const Vec2f& p1, const Vec2f& p2, const float width, const float headwidth, const float headsize)
{
    Vec2f d, n, p(p1);
    d.diff(p2, p1);
    float len = d.length();
    d.normalize();
    n.copy(d);
    n.rotate(1, 0);
    double brick_size = 0.3;
    double filled_part = brick_size * 0.7;
    int whole_bricks = static_cast<int>(floor(len / brick_size));
    double brick_part = len - whole_bricks * brick_size;
    if (brick_part > filled_part)
        brick_part = filled_part;
    bool is_last = false;
    for (int i = 0; i <= whole_bricks; ++i)
    {
        if (i == whole_bricks)
        {
            if (brick_part < width)
                break;
            brick_part -= width;
            is_last = true;
        }
        moveTo(p);
        p.addScaled(n, width / 2);
        lineTo(p);
        p.addScaled(d, static_cast<float>(is_last ? brick_part : filled_part));
        lineTo(p);
        n.negate();
        p.addScaled(n, width);
        lineTo(p);
        d.negate();
        p.addScaled(d, static_cast<float>(is_last ? brick_part : filled_part));
        lineTo(p);
        n.negate();
        d.negate();
        p.addScaled(n, width / 2);
        lineTo(p);
        p.addScaled(d, static_cast<float>(brick_size));
    }
    drawArrowHeader(p2, d, width, headwidth, headsize, false);
    checkPathNonEmpty();
    bbIncludePath(false);
    _backend->fill();
    backendCheckStatus();
}

void RenderContext::drawBar(const Vec2f& p1, const Vec2f& p2, const float width, const float margin)
{
    Vec2f d, n, p(p1);
    d.diff(p2, p1);
    float len = d.length() - margin;
    d.normalize();
    n.copy(d);
    n.rotate(1, 0);
    moveTo(p);
    p.addScaled(n, width / 2);
    lineTo(p);
    p.addScaled(d, len);
    lineTo(p);
    n.negate();
    p.addScaled(n, width);
    lineTo(p);
    d.negate();
    p.addScaled(d, len);
    lineTo(p);
    n.negate();
    d.negate();
    p.addScaled(n, width / 2);
    lineTo(p);
}

void RenderContext::drawEquillibriumHalf(const Vec2f& p1, const Vec2f& p2, const float width, float headwidth, const float headsize, const ArrowType arrow_type,
                                         const bool is_large, const bool is_unbalanced)
{
    float margin = arrow_type == ArrowType::ETriangleArrow ? headsize : static_cast<float>(width * 1.5);
    float width_scale = is_large ? 1.5f : 1.f;
    Vec2f d, n, pa(p1);
    d.diff(p2, p1);
    float len = d.length();
    d.normalize();
    n.copy(d);
    n.rotate(-1, 0);
    pa.addScaled(n, headwidth / 2);
    Vec2f pb(pa);
    pb.addScaled(d, len);
    drawHalfArrowHeader(pb, d, width, headwidth * width_scale, headsize, arrow_type);
    drawBar(pa, pb, width, margin);
    n.negate();
    pa.addScaled(n, headwidth);
    pb.addScaled(n, headwidth);
    const float default_shift = headsize * 2.0f;
    float shift = default_shift;
    if ((len - shift * 2.0f) < default_shift)
    {
        if (len < default_shift * 2.0f)
            shift = len / 4.0f; // too short arrow - shift will be 1/4 of arrow length, short part - 1/2
        else
            shift = (len - default_shift) / 2.0f; // short part of arrow will be headsize*2
    }
    if (is_unbalanced)
        pa.addScaled(d, shift);
    d.negate();
    if (is_unbalanced)
        pb.addScaled(d, shift);
    drawHalfArrowHeader(pa, d, width, headwidth * width_scale, headsize, arrow_type);
    drawBar(pb, pa, width, margin);
    checkPathNonEmpty();
    bbIncludePath(false);
    _backend->fill();
    backendCheckStatus();
}

void RenderContext::drawEquillibriumFilledTriangle(const Vec2f& p1, const Vec2f& p2, const float width, const float headwidth, const float headsize)
{
    Vec2f d, n, pa(p1);
    d.diff(p2, p1);
    float len = d.length();
    d.normalize();
    n.copy(d);
    n.rotate(-1, 0);
    pa.addScaled(n, headwidth / 2);
    Vec2f pb(pa);
    pb.addScaled(d, len);
    drawCustomArrow(pa, pb, width, headwidth, headsize, false);
    n.negate();
    pa.addScaled(n, headwidth);
    pb.addScaled(n, headwidth);
    drawCustomArrow(pb, pa, width, headwidth, headsize, false);
    checkPathNonEmpty();
    bbIncludePath(false);
    _backend->fill();
    backendCheckStatus();
}

void RenderContext::drawCustomArrow(const Vec2f& p1, const Vec2f& p2, const float width, const float headwidth, const float headsize, const bool is_bow)
{
    Vec2f d, n, p(p1);
    d.diff(p2, p1);
    float len = d.length();
    d.normalize();
    n.copy(d);
    n.rotate(1, 0);
    p.addScaled(n, width / 2);
    moveTo(p);
    auto arr_wc = headwidth / 2;
    auto arr_hyp = std::hypot(arr_wc, headsize);
    auto arr_h = (arr_wc * headsize) / arr_hyp;
    auto inner_w = arr_wc * (arr_h - width) / arr_h - width / 2;
    auto inner_h = inner_w * headsize / arr_wc;
    p.addScaled(d, len - headsize + inner_h);
    lineTo(p);
    Vec2f back_vector(-d.x, -d.y);
    auto cs = headsize / arr_hyp;
    auto si = arr_wc / arr_hyp;
    back_vector.rotate(-si, cs);
    p.addScaled(back_vector, inner_h / cs);
    if (!is_bow)
        lineTo(p);
    p.addScaled(n, arr_wc - inner_w - width / 2);
    lineTo(p);
    back_vector.negate();
    p.addScaled(back_vector, arr_hyp);
    lineTo(p);
    back_vector.set(-d.x, -d.y);
    back_vector.rotate(si, cs);
    p.addScaled(back_vector, arr_hyp);
    lineTo(p);
    p.addScaled(n, arr_wc - inner_w - width / 2);
    if (!is_bow)
        lineTo(p);
    back_vector.negate();
    p.addScaled(back_vector, inner_h / cs);
    lineTo(p);
    back_vector.set(-d.x, -d.y);
    p.addScaled(back_vector, len - headsize + inner_h);
    lineTo(p);
    p.addScaled(n, width);
    lineTo(p);
}

void RenderContext::drawRetroSynthArrow(const Vec2f& p1, const Vec2f& p2, const float width, const float headwidth, const float headsize)
{
    Vec2f d, n, pa(p1);
    d.diff(p2, p1);
    float len = d.length();
    d.normalize();
    n.copy(d);
    n.rotate(-1, 0);

    pa.addScaled(n, headwidth / 2);
    Vec2f pb(pa);
    pb.addScaled(d, len);
    drawBar(pa, pb, width, headwidth);
    n.negate();
    pa.addScaled(n, headwidth);
    pb.addScaled(n, headwidth);
    drawBar(pa, pb, width, headwidth);

    n.negate();
    pb.addScaled(n, headwidth / 2);

    drawArrowHeader(pb, d, width, (headwidth + width) * 2, headsize * 2);
    checkPathNonEmpty();
    bbIncludePath(false);
    _backend->fill();
    backendCheckStatus();
}

void RenderContext::drawCustomArrow(const Vec2f& p1, const Vec2f& p2, const float width, const float headwidth, const float headsize, const bool is_bow,
                                    const bool is_failed)
{
    Vec2f d, n, p(p1);
    d.diff(p2, p1);
    float len = d.length();
    d.normalize();
    drawCustomArrow(p1, p2, width, headwidth, headsize, is_bow);
    if (is_failed)
    {
        _backend->fill();
        for (int arr_ind = 0; arr_ind < 2; ++arr_ind)
        {
            p.set(p1.x, p1.y);
            p.addScaled(d, len / 2); // set to middle
            Vec2f d45(d);
            d45.rotate(static_cast<float>((arr_ind ? 1 : -1) * sqrt(2) / 2.), static_cast<float>(sqrt(2) / 2.)); // rotate 45 degrees clockwise
            auto len_cross = len / 10;
            p.addScaled(d45, len_cross / 2);
            Vec2f n90(d45);
            n90.rotate(1, 0);
            p.addScaled(n90, width / 2);
            moveTo(p);
            n90.rotate(1, 0);
            p.addScaled(n90, len_cross);
            lineTo(p);
            n90.rotate(1, 0);
            p.addScaled(n90, width);
            lineTo(p);
            n90.rotate(1, 0);
            p.addScaled(n90, len_cross);
            lineTo(p);
            n90.rotate(1, 0);
            p.addScaled(n90, width);
            lineTo(p);
        }
    }
    checkPathNonEmpty();
    bbIncludePath(false);
    _backend->fill();
    backendCheckStatus();
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
    _backend->fill();
    backendCheckStatus();
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
    _backend->setSourceRGB(v.x, v.y, v.z);
    backendCheckStatus();
}

void RenderContext::setSingleSource(const Vec3f& color)
{
    _backend->setSourceRGB(color.x, color.y, color.z);
    backendCheckStatus();
}

void RenderContext::setGradientSource(const Vec3f& color1, const Vec3f& color2, const Vec2f& pos1, const Vec2f& pos2)
{
    _backend->setLinearGradient(pos1.x, pos1.y, pos2.x, pos2.y, color1.x, color1.y, color1.z, color2.x, color2.y, color2.z);

    backendCheckStatus();
}

void RenderContext::clearPattern()
{
    _backend->clearPattern();
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
    _backend->setDash(dash.ptr(), dash.size(), _getDashedLineAlignmentOffset(length));
    backendCheckStatus();
}

void RenderContext::resetDash()
{
    _backend->setDash(nullptr, 0, 0);
    backendCheckStatus();
}

void RenderContext::lineTo(const Vec2f& v)
{
    _backend->lineTo(v.x, v.y);
    backendCheckStatus();
}

void RenderContext::lineToRel(float x, float y)
{
    _backend->relLineTo(x, y);
    backendCheckStatus();
}

void RenderContext::lineToRel(const Vec2f& v)
{
    _backend->relLineTo(v.x, v.y);
    backendCheckStatus();
}

void RenderContext::moveTo(const Vec2f& v)
{
    _backend->moveTo(v.x, v.y);
    backendCheckStatus();
}

void RenderContext::moveToRel(float x, float y)
{
    _backend->relMoveTo(x, y);
    backendCheckStatus();
}

void RenderContext::moveToRel(const Vec2f& v)
{
    _backend->relMoveTo(v.x, v.y);
    backendCheckStatus();
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
        {0.60, 0.12, 0.83},  {0.50, 0.12, 0.83}, {0.70, 0.12, 0.63}, {0.70, 0.12, 0.83},  {0.70, 0.32, 0.83},  {0.60, 0.12, 0.83},  {0.70, 0.12, 0.43},
        {0.0, 0.0, 0.0},     {0.0, 0.0, 0.0},    {0.0, 0.0, 0.0},    {0.0, 0.0, 0.0},     {0.0, 0.0, 0.0},     {0.0, 0.0, 0.0},     {0.0, 0.0, 0.0},
        {0.0, 0.0, 0.0},     {0.0, 0.0, 0.0},    {0.0, 0.0, 0.0},    {0.0, 0.0, 0.0}};

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
