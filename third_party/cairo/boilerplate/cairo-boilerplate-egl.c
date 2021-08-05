/* Cairo - a vector graphics library with display and print output
 *
 * Copyright © 2009 Chris Wilson
 *
 * This library is free software; you can redistribute it and/or
 * modify it either under the terms of the GNU Lesser General Public
 * License version 2.1 as published by the Free Software Foundation
 * (the "LGPL") or, at your option, under the terms of the Mozilla
 * Public License Version 1.1 (the "MPL"). If you do not alter this
 * notice, a recipient may use your version of this file under either
 * the MPL or the LGPL.
 *
 * You should have received a copy of the LGPL along with this library
 * in the file COPYING-LGPL-2.1; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Suite 500, Boston, MA 02110-1335, USA
 * You should have received a copy of the MPL along with this library
 * in the file COPYING-MPL-1.1
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY
 * OF ANY KIND, either express or implied. See the LGPL or the MPL for
 * the specific language governing rights and limitations.
 *
 * The Original Code is the cairo graphics library.
 *
 * The Initial Developer of the Original Code is Chris Wilson.
 */

#include "cairo-boilerplate-private.h"

#include <cairo-gl.h>
#if CAIRO_HAS_GLESV3_SURFACE
#include <GLES3/gl3.h>
#include <EGL/eglext.h>
#elif CAIRO_HAS_GLESV2_SURFACE
#include <GLES2/gl2.h>
#elif CAIRO_HAS_GL_SURFACE
#include <GL/gl.h>
#endif

typedef struct _egl_target_closure {
    EGLDisplay dpy;
    EGLContext ctx;

    cairo_device_t *device;
    cairo_surface_t *surface;
} egl_target_closure_t;

static void
_cairo_boilerplate_egl_cleanup (void *closure)
{
    egl_target_closure_t *gltc = closure;

    cairo_device_finish (gltc->device);
    cairo_device_destroy (gltc->device);

    eglDestroyContext (gltc->dpy, gltc->ctx);
    eglMakeCurrent (gltc->dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglTerminate (gltc->dpy);

    free (gltc);
}

static cairo_surface_t *
_cairo_boilerplate_egl_create_surface (const char		 *name,
				       cairo_content_t		  content,
				       double			  width,
				       double			  height,
				       double			  max_width,
				       double			  max_height,
				       cairo_boilerplate_mode_t   mode,
				       void			**closure)
{
    egl_target_closure_t *gltc;
    cairo_surface_t *surface;
    int major, minor;
    EGLConfig config;
    EGLint numConfigs;
    EGLint config_attribs[] = {
	EGL_RED_SIZE, 8,
	EGL_GREEN_SIZE, 8,
	EGL_BLUE_SIZE, 8,
	EGL_ALPHA_SIZE, 8,
	EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
#if CAIRO_HAS_GLESV3_SURFACE
	EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT_KHR,
#elif CAIRO_HAS_GLESV2_SURFACE
	EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
#elif CAIRO_HAS_GL_SURFACE
	EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
#endif
	EGL_NONE
    };
    const EGLint ctx_attribs[] = {
#if CAIRO_HAS_GLESV3_SURFACE
	EGL_CONTEXT_CLIENT_VERSION, 3,
#elif CAIRO_HAS_GLESV2_SURFACE
	EGL_CONTEXT_CLIENT_VERSION, 2,
#endif
	EGL_NONE
    };

    gltc = xcalloc (1, sizeof (egl_target_closure_t));
    *closure = gltc;

    gltc->dpy = eglGetDisplay (EGL_DEFAULT_DISPLAY);

    if (! eglInitialize (gltc->dpy, &major, &minor)) {
	free (gltc);
	return NULL;
    }

    eglChooseConfig (gltc->dpy, config_attribs, &config, 1, &numConfigs);
#if CAIRO_HAS_GLESV3_SURFACE && CAIRO_HAS_GLESV2_SURFACE
    if (numConfigs == 0) {
        /* retry with ES2_BIT */
        config_attribs[11] = ES2_BIT;  /* FIXME: Ick */
        eglChooseConfig (gltc->dpy, config_attribs, &config, 1, &numConfigs);
    }
#endif
    if (numConfigs == 0) {
	free (gltc);
	return NULL;
    }

#if CAIRO_HAS_GLESV3_SURFACE || CAIRO_HAS_GLESV2_SURFACE
    eglBindAPI (EGL_OPENGL_ES_API);
#elif CAIRO_HAS_GL_SURFACE
    eglBindAPI (EGL_OPENGL_API);
#endif

    gltc->ctx = eglCreateContext (gltc->dpy, config, EGL_NO_CONTEXT,
				  ctx_attribs);
    if (gltc->ctx == EGL_NO_CONTEXT) {
	eglTerminate (gltc->dpy);
	free (gltc);
	return NULL;
    }

    gltc->device = cairo_egl_device_create (gltc->dpy, gltc->ctx);
    if (mode == CAIRO_BOILERPLATE_MODE_PERF)
	cairo_gl_device_set_thread_aware(gltc->device, FALSE);

    if (width < 1)
	width = 1;
    if (height < 1)
	height = 1;

    gltc->surface = surface = cairo_gl_surface_create (gltc->device,
						       content,
						       ceil (width),
						       ceil (height));
    if (cairo_surface_status (surface))
	_cairo_boilerplate_egl_cleanup (gltc);

    return surface;
}

static void
_cairo_boilerplate_egl_synchronize (void *closure)
{
    egl_target_closure_t *gltc = closure;

    if (cairo_device_acquire (gltc->device))
	return;

    glFinish ();

    cairo_device_release (gltc->device);
}

static const cairo_boilerplate_target_t targets[] = {
    {
	"egl", "gl", NULL, NULL,
	CAIRO_SURFACE_TYPE_GL, CAIRO_CONTENT_COLOR_ALPHA, 1,
	"cairo_egl_device_create",
	_cairo_boilerplate_egl_create_surface,
	cairo_surface_create_similar,
	NULL, NULL,
	_cairo_boilerplate_get_image_surface,
	cairo_surface_write_to_png,
	_cairo_boilerplate_egl_cleanup,
	_cairo_boilerplate_egl_synchronize,
        NULL,
	TRUE, FALSE, FALSE
    }
};
CAIRO_BOILERPLATE (egl, targets)
