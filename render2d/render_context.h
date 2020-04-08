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

#ifndef __render_context_h__
#define __render_context_h__

#include <cairo-pdf.h>
#include <cairo-svg.h>
#include <cairo.h>

#include "render_common.h"

namespace indigo
{

    class RenderContext
    {
    public:
        DECL_ERROR;

        void checkPathNonEmpty() const;

        RenderContext(const RenderOptions& opt, float sf, float lwf);
        void setDefaultScale(float scale);
        void setHDC(PVOID hdc);
        int getMaxPageSize() const;
        void setLineWidth(double width);
        void setFontFamily(const char* ff);
        void setOutput(Output* output);
        void createSurface(cairo_write_func_t writer, Output* output, int width, int height);
        void init();
        void fillBackground();
        void initNullContext();
        void initContext(int width, int height);
        void closeContext(bool discard);
        void translate(float dx, float dy);
        void scale(float s);
        void storeTransform();
        void restoreTransform();
        void resetTransform();
        void removeStoredTransform();
        void drawRectangle(const Vec2f& p, const Vec2f& sz);
        void drawItemBackground(const RenderItem& item);
        void drawTextItemText(const TextItem& ti, bool idle);
        void drawTextItemText(const TextItem& ti, const Vec3f& color, bool idle);
        void drawTextItemText(const TextItem& ti, const Vec3f& color, bool bold, bool idle);
        void drawBracket(RenderItemBracket& bracket);
        void drawAttachmentPoint(RenderItemAttachmentPoint& ri, bool idle);
        void drawRSiteAttachmentIndex(RenderItemRSiteAttachmentIndex& ri);
        void drawLine(const Vec2f& v0, const Vec2f& v1);
        void fillHex(const Vec2f& v0, const Vec2f& v1, const Vec2f& v2, const Vec2f& v3, const Vec2f& v4, const Vec2f& v5);
        void fillQuad(const Vec2f& v0, const Vec2f& v1, const Vec2f& v2, const Vec2f& v3);
        void fillQuadStripes(const Vec2f& v0r, const Vec2f& v0l, const Vec2f& v1r, const Vec2f& v1l, int cnt);
        void fillPentagon(const Vec2f& v0, const Vec2f& v1, const Vec2f& v2, const Vec2f& v3, const Vec2f& v4);
        void drawQuad(const Vec2f& v0, const Vec2f& v1, const Vec2f& v2, const Vec2f& v3);
        void drawTriangleZigzag(const Vec2f& v0, const Vec2f& v1, const Vec2f& v2, int cnt);
        void drawCircle(const Vec2f& center, const float r);
        void fillCircle(const Vec2f& center, const float r);
        void drawArc(const Vec2f& center, const float r, const float a0, const float a1);
        void drawPoly(const Array<Vec2f>& v);
        void setFontSize(double fontSize);
        void setTextItemSize(TextItem& ti);
        void setTextItemSize(TextItem& ti, const Vec2f& c);
        void setGraphItemSizeDot(GraphItem& gi);
        void setGraphItemSizeCap(GraphItem& gi);
        void setGraphItemSizeSign(GraphItem& gi, GraphItem::TYPE type);
        void drawGraphItem(GraphItem& gi);
        void drawGraphItem(GraphItem& gi, const Vec3f& color);
        void fillRect(double x, double y, double w, double h);
        void getColor(float& r, float& g, float& b, int c);
        int getElementColor(int label);
        void getColorVec(Vec3f& v, int color);
        void setSingleSource(int color);
        void setSingleSource(const Vec3f& color);
        void setGradientSource(const Vec3f& color1, const Vec3f& color2, const Vec2f& pos1, const Vec2f& pos2);
        void clearPattern();
        float _getDashedLineAlignmentOffset(float length);
        void setDash(const Array<double>& dash, float offset = 0);
        void resetDash();
        void drawPlus(const Vec2f& pos, const float linewidth, const float size);
        void drawEquality(const Vec2f& pos, const float linewidth, const float size, const float interval);
        void drawArrow(const Vec2f& p1, const Vec2f& p2, const float width, const float headwidth, const float headsize);
        float highlightedBondLineWidth() const;
        float currentLineWidth() const;
        void setHighlight();
        void resetHighlight();
        void resetHighlightThickness();

        const RenderSettings& getRenderSettings() const
        {
            return _settings;
        }
        int getWidth() const
        {
            return _width;
        }
        int getHeight() const
        {
            return _height;
        }

        void cairoCheckStatus() const;
        void cairoCheckSurfaceStatus() const;

#ifdef _WIN32
        cairo_surface_t* createWin32Surface();
        cairo_surface_t* createWin32PrintingSurfaceForHDC();
        cairo_surface_t* createWin32PrintingSurfaceForMetafile(bool& isLarge);
        void storeAndDestroyMetafile(bool discard);
#endif

        void fontsClear();
        void fontsInit();
        void fontsDispose();
        double fontGetSize(FONT_SIZE size);
        void fontsSetFont(cairo_t* cr, FONT_SIZE size, bool bold);
        void fontsGetTextExtents(cairo_t* cr, const char* text, int size, float& dx, float& dy, float& rx, float& ry);
        void fontsDrawText(const TextItem& ti, const Vec3f& color, bool bold, bool idle);

        void bbIncludePoint(const Vec2f& v);
        void bbIncludePoint(double x, double y);
        void bbIncludePath(bool stroke);
        void bbGetMin(Vec2f& v);
        void bbGetMax(Vec2f& v);
        void _bbVecToUser(Vec2f& d, const Vec2f& s);

        Vec2f bbmin, bbmax;

    private:
        static cairo_status_t writer(void* closure, const unsigned char* data, unsigned int length);

        void _drawGraphItem(GraphItem& gi);
        void lineTo(const Vec2f& v);
        void lineToRel(float x, float y);
        void lineToRel(const Vec2f& v);
        void moveTo(const Vec2f& v);
        void moveToRel(float x, float y);
        void moveToRel(const Vec2f& v);

        int _width;
        int _height;
        float _defaultScale;
        Vec3f _backColor;
        Vec3f _baseColor;
        float _currentLineWidth;
        cairo_pattern_t* _pattern;

        class TextLock
        {
        public:
            TextLock()
            {
#ifdef _WIN32
                osMutexCreate(&_mutex);
#endif
            }

            ~TextLock()
            {
#ifdef _WIN32
                osMutexDelete(&_mutex);
#endif
            }

            void lock()
            {
#ifdef _WIN32
                osMutexLock(&_mutex);
#endif
            }

            void unlock()
            {
#ifdef _WIN32
                osMutexUnlock(&_mutex);
#endif
            }

        private:
#ifdef _WIN32
            os_mutex _mutex;
#endif
        };

        static TextLock _tlock;

        CP_DECL;
        TL_CP_DECL(Array<char>, _fontfamily);
        TL_CP_DECL(Array<cairo_matrix_t>, transforms);
#ifdef _WIN32
        void* _h_fonts[FONT_SIZE_COUNT * 2];
#endif

        cairo_font_face_t *cairoFontFaceRegular, *cairoFontFaceBold;
        cairo_matrix_t fontScale, fontCtm;
        cairo_font_options_t* fontOptions;
        cairo_scaled_font_t* _scaled_fonts[FONT_SIZE_COUNT * 2];

        bool metafileFontsToCurves;
        cairo_t* _cr;
        cairo_surface_t* _surface;
        void* _meta_hdc;

    public:
        RenderSettings _settings;
        const RenderOptions& opt;
    };

} // namespace indigo

#endif
