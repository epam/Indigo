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

#ifndef __cairo_render_backend_h__
#define __cairo_render_backend_h__

#ifndef __EMSCRIPTEN__

#include "render_backend.h"

#include <cairo-pdf.h>
#include <cairo-svg.h>
#include <cairo.h>
#include <mutex>

namespace indigo
{

    class CairoRenderBackend : public IRenderBackend
    {
    public:
        CairoRenderBackend();
        ~CairoRenderBackend() override;

        // ---- Surface lifecycle ----
        void createSurface(int mode, int width, int height, void* output) override;
        void closeSurface(int mode, bool discard, void* output) override;
        void createContext() override;
        void destroyContext() override;

        // ---- Path operations ----
        void beginPath() override;
        void closePath() override;
        void moveTo(float x, float y) override;
        void lineTo(float x, float y) override;
        void relMoveTo(float dx, float dy) override;
        void relLineTo(float dx, float dy) override;
        void curveTo(float x1, float y1, float x2, float y2, float x3, float y3) override;
        void arc(float cx, float cy, float r, float a0, float a1) override;
        void arcNegative(float cx, float cy, float r, float a0, float a1) override;
        void rect(float x, float y, float w, float h) override;

        // ---- Drawing operations ----
        void fill() override;
        void stroke() override;
        void paint() override;

        // ---- Style ----
        void setSourceRGB(float r, float g, float b) override;
        void setSourceRGBA(float r, float g, float b, float a) override;
        void setLineWidth(float w) override;
        void setLineJoin(int join) override;
        void setDash(const double* pattern, int count, double offset) override;
        void setOperator(int op) override;
        void setAntialias(int mode) override;

        // ---- Transform ----
        void save() override;
        void restore() override;
        void translate(float dx, float dy) override;
        void scale(float sx, float sy) override;
        void rotate(float angle) override;
        void getMatrix(double m[6]) override;
        void setMatrix(const double m[6]) override;
        void initIdentityMatrix(double m[6]) override;
        void userToDevice(double& x, double& y) override;
        void deviceToUser(double& x, double& y) override;

        // ---- Extents ----
        void strokeExtents(double& x1, double& y1, double& x2, double& y2) override;
        void pathExtents(double& x1, double& y1, double& x2, double& y2) override;

        // ---- Text ----
        void selectFontFace(const char* family, bool italic, bool bold) override;
        void setFontSize(float size) override;
        void textExtents(const char* text, float& width, float& height, float& x_bearing, float& y_bearing) override;
        void fontExtents(double& height) override;
        void showText(const char* text) override;
        void textPath(const char* text) override;

        // ---- Font options ----
        void createFontOptions() override;
        void destroyFontOptions() override;
        void setFontOptionsAntialias(int mode) override;
        void applyFontOptions() override;

        // ---- Gradient ----
        void setLinearGradient(float x0, float y0, float x1, float y1, float r1, float g1, float b1, float r2, float g2, float b2) override;
        void clearPattern() override;

        // ---- Image ----
        void drawPngImage(const void* data, int dataLen, float x, float y, float w, float h) override;
        void writeSurfaceToPng(void* output) override;

        // ---- Path debugging ----
        bool isPathEmpty() override;

        // ---- Surface source ----
        void setSourceSurface(float x, float y) override;

        // Access raw cairo objects (needed during transition)
        cairo_t* getCr()
        {
            return _cr;
        }
        cairo_surface_t* getSurface()
        {
            return _surface;
        }

    private:
        static cairo_status_t _writer(void* closure, const unsigned char* data, unsigned int length);

        cairo_t* _cr;
        cairo_surface_t* _surface;
        cairo_surface_t* _pngImage; // temp for drawPngImage
        cairo_pattern_t* _pattern;
        cairo_font_options_t* _fontOptions;

        static std::mutex _mutex;
    };

} // namespace indigo

#endif // !__EMSCRIPTEN__

#endif
