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

#ifndef __render_backend_h__
#define __render_backend_h__

#include <string>

namespace indigo
{

    // Output format modes (mirrors existing MODE_* constants)
    enum RenderBackendMode
    {
        RBMODE_PNG,
        RBMODE_SVG,
        RBMODE_PDF,
        RBMODE_HDC,
        RBMODE_PRN,
        RBMODE_EMF
    };

    // Abstract rendering backend interface.
    // Decouples render2d drawing logic from a specific graphics library.
    // Desktop builds use CairoRenderBackend; WASM builds use JSRenderBackend.
    class IRenderBackend
    {
    public:
        virtual ~IRenderBackend() = default;

        // ---- Surface lifecycle ----
        // Create the drawing surface. `output` is an opaque pointer to an Output* (used by cairo backend).
        virtual void createSurface(int mode, int width, int height, void* output) = 0;
        // Finalize the surface and write to output. For JS backend, may produce base64 PNG or SVG string.
        virtual void closeSurface(int mode, bool discard, void* output) = 0;
        // Create cairo context (or JS canvas context)
        virtual void createContext() = 0;
        // Destroy the context
        virtual void destroyContext() = 0;

        // ---- Path operations ----
        virtual void beginPath() = 0;
        virtual void closePath() = 0;
        virtual void moveTo(double x, double y) = 0;
        virtual void lineTo(double x, double y) = 0;
        virtual void relMoveTo(double dx, double dy) = 0;
        virtual void relLineTo(double dx, double dy) = 0;
        virtual void curveTo(double x1, double y1, double x2, double y2, double x3, double y3) = 0;
        virtual void arc(double cx, double cy, double r, double a0, double a1) = 0;
        virtual void arcNegative(double cx, double cy, double r, double a0, double a1) = 0;
        virtual void rect(double x, double y, double w, double h) = 0;

        // ---- Drawing operations ----
        virtual void fill() = 0;
        virtual void stroke() = 0;
        virtual void paint() = 0;

        // ---- Style ----
        virtual void setSourceRGB(double r, double g, double b) = 0;
        virtual void setSourceRGBA(double r, double g, double b, double a) = 0;
        virtual void setLineWidth(double w) = 0;
        virtual void setLineJoin(int join) = 0; // 0=miter, 1=round, 2=bevel
        virtual void setDash(const double* pattern, int count, double offset) = 0;
        virtual void setOperator(int op) = 0; // CAIRO_OPERATOR_SOURCE = 1
        virtual void setAntialias(int mode) = 0;

        // ---- Transform ----
        virtual void save() = 0;
        virtual void restore() = 0;
        virtual void translate(double dx, double dy) = 0;
        virtual void scale(double sx, double sy) = 0;
        virtual void rotate(double angle) = 0;

        // Matrix operations: get/set current transform matrix as 6 doubles [xx, yx, xy, yy, x0, y0]
        virtual void getMatrix(double m[6]) = 0;
        virtual void setMatrix(const double m[6]) = 0;
        virtual void initIdentityMatrix(double m[6]) = 0;

        // Coordinate transform
        virtual void userToDevice(double& x, double& y) = 0;
        virtual void deviceToUser(double& x, double& y) = 0;

        // ---- Extents ----
        virtual void strokeExtents(double& x1, double& y1, double& x2, double& y2) = 0;
        virtual void pathExtents(double& x1, double& y1, double& x2, double& y2) = 0;

        // ---- Text ----
        virtual void selectFontFace(const char* family, bool italic, bool bold) = 0;
        virtual void setFontSize(double size) = 0;
        virtual void textExtents(const char* text, double& width, double& height, double& x_bearing, double& y_bearing) = 0;
        virtual void fontExtents(double& height) = 0;
        virtual void showText(const char* text) = 0;
        virtual void textPath(const char* text) = 0;

        // ---- Font options ----
        virtual void createFontOptions() = 0;
        virtual void destroyFontOptions() = 0;
        virtual void setFontOptionsAntialias(int mode) = 0;
        virtual void applyFontOptions() = 0;

        // ---- Gradient ----
        virtual void setLinearGradient(double x0, double y0, double x1, double y1, double r1, double g1, double b1, double r2, double g2, double b2) = 0;
        virtual void clearPattern() = 0;

        // ---- Image operations ----
        // Draw a PNG image from raw data into the given bounding box
        virtual void drawPngImage(const void* data, int dataLen, double x, double y, double w, double h) = 0;
        // Write surface to PNG stream
        virtual void writeSurfaceToPng(void* output) = 0;

        // ---- Path debugging ----
        virtual bool isPathEmpty() = 0;

        // ---- Surface source ----
        virtual void setSourceSurface(double x, double y) = 0;
    };

} // namespace indigo

#endif
