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
        virtual void moveTo(float x, float y) = 0;
        virtual void lineTo(float x, float y) = 0;
        virtual void relMoveTo(float dx, float dy) = 0;
        virtual void relLineTo(float dx, float dy) = 0;
        virtual void curveTo(float x1, float y1, float x2, float y2, float x3, float y3) = 0;
        virtual void arc(float cx, float cy, float r, float a0, float a1) = 0;
        virtual void arcNegative(float cx, float cy, float r, float a0, float a1) = 0;
        virtual void rect(float x, float y, float w, float h) = 0;

        // ---- Drawing operations ----
        virtual void fill() = 0;
        virtual void stroke() = 0;
        virtual void paint() = 0;

        // ---- Style ----
        virtual void setSourceRGB(float r, float g, float b) = 0;
        virtual void setSourceRGBA(float r, float g, float b, float a) = 0;
        virtual void setLineWidth(float w) = 0;
        virtual void setLineJoin(int join) = 0; // 0=miter, 1=round, 2=bevel
        virtual void setDash(const double* pattern, int count, double offset) = 0;
        virtual void setOperator(int op) = 0; // CAIRO_OPERATOR_SOURCE = 1
        virtual void setAntialias(int mode) = 0;

        // ---- Transform ----
        virtual void save() = 0;
        virtual void restore() = 0;
        virtual void translate(float dx, float dy) = 0;
        virtual void scale(float sx, float sy) = 0;
        virtual void rotate(float angle) = 0;

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
        virtual void setFontSize(float size) = 0;
        virtual void textExtents(const char* text, float& width, float& height, float& x_bearing, float& y_bearing) = 0;
        virtual void fontExtents(double& height) = 0;
        virtual void showText(const char* text) = 0;
        virtual void textPath(const char* text) = 0;

        // ---- Font options ----
        virtual void createFontOptions() = 0;
        virtual void destroyFontOptions() = 0;
        virtual void setFontOptionsAntialias(int mode) = 0;
        virtual void applyFontOptions() = 0;

        // ---- Gradient ----
        virtual void setLinearGradient(float x0, float y0, float x1, float y1, float r1, float g1, float b1, float r2, float g2, float b2) = 0;
        virtual void clearPattern() = 0;

        // ---- Image operations ----
        // Draw a PNG image from raw data into the given bounding box
        virtual void drawPngImage(const void* data, int dataLen, float x, float y, float w, float h) = 0;
        // Write surface to PNG stream
        virtual void writeSurfaceToPng(void* output) = 0;

        // ---- Path debugging ----
        virtual bool isPathEmpty() = 0;

        // ---- Surface source ----
        virtual void setSourceSurface(float x, float y) = 0;
    };

} // namespace indigo

#endif
