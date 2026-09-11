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
        void moveTo(double x, double y) override;
        void lineTo(double x, double y) override;
        void relMoveTo(double dx, double dy) override;
        void relLineTo(double dx, double dy) override;
        void curveTo(double x1, double y1, double x2, double y2, double x3, double y3) override;
        void arc(double cx, double cy, double r, double a0, double a1) override;
        void arcNegative(double cx, double cy, double r, double a0, double a1) override;
        void rect(double x, double y, double w, double h) override;

        // ---- Drawing operations ----
        void fill() override;
        void stroke() override;
        void paint() override;

        // ---- Style ----
        void setSourceRGB(double r, double g, double b) override;
        void setSourceRGBA(double r, double g, double b, double a) override;
        void setLineWidth(double w) override;
        void setLineJoin(int join) override;
        void setDash(const double* pattern, int count, double offset) override;
        void setOperator(int op) override;
        void setAntialias(int mode) override;

        // ---- Transform ----
        void save() override;
        void restore() override;
        void translate(double dx, double dy) override;
        void scale(double sx, double sy) override;
        void rotate(double angle) override;
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
        void setFontSize(double size) override;
        void textExtents(const char* text, double& width, double& height, double& x_bearing, double& y_bearing) override;
        void fontExtents(double& height) override;
        void showText(const char* text) override;
        void textPath(const char* text) override;

        // ---- Font options ----
        void createFontOptions() override;
        void destroyFontOptions() override;
        void setFontOptionsAntialias(int mode) override;
        void applyFontOptions() override;

        // ---- Gradient ----
        void setLinearGradient(double x0, double y0, double x1, double y1, double r1, double g1, double b1, double r2, double g2, double b2) override;
        void clearPattern() override;

        // ---- Image ----
        void drawPngImage(const void* data, int dataLen, double x, double y, double w, double h) override;
        void writeSurfaceToPng(void* output) override;

        // ---- Path debugging ----
        bool isPathEmpty() override;

        // ---- Surface source ----
        void setSourceSurface(double x, double y) override;

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
