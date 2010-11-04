/* -*- Mode: c; c-basic-offset: 4; indent-tabs-mode: t; tab-width: 8; -*- */
/*
 * Copyright © 2004,2006 Red Hat, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * Red Hat, Inc. not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior
 * permission. Red Hat, Inc. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * RED HAT, INC. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS, IN NO EVENT SHALL RED HAT, INC. BE LIABLE FOR ANY SPECIAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR
 * IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author: Carl D. Worth <cworth@cworth.org>
 */

#include "cairo-boilerplate.h"
#include "cairo-boilerplate-glitz-private.h"

#include <cairo-glitz.h>

static const cairo_user_data_key_t glitz_closure_key;

typedef struct _glitz_target_closure_base {
    int width;
    int height;
    cairo_content_t content;
} glitz_target_closure_base_t;

#if CAIRO_CAN_TEST_GLITZ_GLX_SURFACE
#include <glitz-glx.h>

typedef struct _glitz_glx_target_closure {
    glitz_target_closure_base_t base;
    Display        *dpy;
    int             scr;
    Window          win;
} glitz_glx_target_closure_t;

static glitz_surface_t *
_cairo_boilerplate_glitz_glx_create_surface_internal (glitz_format_name_t		 formatname,
						      int				 width,
						      int				 height,
						      glitz_glx_target_closure_t	*closure)
{
    Display                 * dpy = closure->dpy;
    int                       scr = closure->scr;
    glitz_drawable_format_t   templ;
    glitz_drawable_format_t * dformat = NULL;
    unsigned long             mask;
    glitz_drawable_t        * drawable = NULL;
    glitz_format_t          * format;
    glitz_surface_t         * sr;

    XSizeHints                xsh;
    XSetWindowAttributes      xswa;
    XVisualInfo             * vinfo;

    memset(&templ, 0, sizeof(templ));
    templ.color.red_size = 8;
    templ.color.green_size = 8;
    templ.color.blue_size = 8;
    templ.color.alpha_size = 8;
    templ.color.fourcc = GLITZ_FOURCC_RGB;
    templ.samples = 1;

    glitz_glx_init (NULL);

    mask = GLITZ_FORMAT_SAMPLES_MASK | GLITZ_FORMAT_FOURCC_MASK |
	GLITZ_FORMAT_RED_SIZE_MASK | GLITZ_FORMAT_GREEN_SIZE_MASK |
	GLITZ_FORMAT_BLUE_SIZE_MASK;
    if (formatname == GLITZ_STANDARD_ARGB32)
	mask |= GLITZ_FORMAT_ALPHA_SIZE_MASK;

    /* Try for a pbuffer first */
    if (!getenv("CAIRO_TEST_FORCE_GLITZ_WINDOW"))
	dformat = glitz_glx_find_pbuffer_format (dpy, scr, mask, &templ, 0);

    if (dformat) {
	closure->win = None;

	drawable = glitz_glx_create_pbuffer_drawable (dpy, scr, dformat,
						      width, height);
	if (!drawable)
	    goto FAIL;
    } else {
	/* No pbuffer, try window */
	dformat = glitz_glx_find_window_format (dpy, scr, mask, &templ, 0);

	if (!dformat)
	    goto FAIL;

	vinfo = glitz_glx_get_visual_info_from_format(dpy,
						      DefaultScreen(dpy),
						      dformat);

	if (!vinfo)
	    goto FAIL;

	xsh.flags = PSize;
	xsh.x = 0;
	xsh.y = 0;
	xsh.width = width;
	xsh.height = height;

	xswa.colormap = XCreateColormap (dpy, RootWindow(dpy, scr),
					 vinfo->visual, AllocNone);
	closure->win = XCreateWindow (dpy, RootWindow(dpy, scr),
				      xsh.x, xsh.y, xsh.width, xsh.height,
				      0, vinfo->depth, CopyFromParent,
				      vinfo->visual, CWColormap, &xswa);
	XFree (vinfo);

	drawable =
	    glitz_glx_create_drawable_for_window (dpy, scr,
						  dformat, closure->win,
						  width, height);

	if (!drawable)
	    goto DESTROY_WINDOW;
    }

    format = glitz_find_standard_format (drawable, formatname);
    if (!format)
	goto DESTROY_DRAWABLE;

    sr = glitz_surface_create (drawable, format, width, height, 0, NULL);
    if (!sr)
	goto DESTROY_DRAWABLE;

    if (closure->win == None || dformat->doublebuffer) {
	glitz_surface_attach (sr, drawable, GLITZ_DRAWABLE_BUFFER_BACK_COLOR);
    } else {
	XMapWindow (closure->dpy, closure->win);
	glitz_surface_attach (sr, drawable, GLITZ_DRAWABLE_BUFFER_FRONT_COLOR);
    }

    glitz_drawable_destroy (drawable);

    return sr;

 DESTROY_DRAWABLE:
    glitz_drawable_destroy (drawable);
 DESTROY_WINDOW:
    if (closure->win)
	XDestroyWindow (dpy, closure->win);
 FAIL:
    return NULL;
}

cairo_surface_t *
_cairo_boilerplate_glitz_glx_create_surface (const char			 *name,
					     cairo_content_t 		  content,
					     int			  width,
					     int			  height,
					     int			  max_width,
					     int			  max_height,
					     cairo_boilerplate_mode_t	  mode,
					     int                          id,
					     void			**closure)
{
    glitz_glx_target_closure_t *gxtc;
    glitz_surface_t  * glitz_surface;
    cairo_surface_t  * surface = NULL;
    cairo_status_t status;

    *closure = gxtc = xmalloc (sizeof (glitz_glx_target_closure_t));

    if (width == 0)
	width = 1;
    if (height == 0)
	height = 1;

    gxtc->dpy = XOpenDisplay (getenv("CAIRO_TEST_GLITZ_DISPLAY"));
    if (!gxtc->dpy) {
	CAIRO_BOILERPLATE_LOG ("Failed to open display: %s\n", XDisplayName(0));
	goto FAIL;
    }

    XSynchronize (gxtc->dpy, 1);

    gxtc->scr = DefaultScreen(gxtc->dpy);

    switch (content) {
    case CAIRO_CONTENT_COLOR:
	glitz_surface = _cairo_boilerplate_glitz_glx_create_surface_internal (GLITZ_STANDARD_RGB24, width, height, gxtc);
	break;
    case CAIRO_CONTENT_COLOR_ALPHA:
	glitz_surface = _cairo_boilerplate_glitz_glx_create_surface_internal (GLITZ_STANDARD_ARGB32, width, height, gxtc);
	break;
    case CAIRO_CONTENT_ALPHA:
    default:
	CAIRO_BOILERPLATE_LOG ("Invalid content for glitz-glx test: %d\n", content);
	goto FAIL_CLOSE_DISPLAY;
    }
    if (!glitz_surface) {
	CAIRO_BOILERPLATE_LOG ("Failed to create glitz-glx surface\n");
	goto FAIL_CLOSE_DISPLAY;
    }

    surface = cairo_glitz_surface_create (glitz_surface);
    glitz_surface_destroy (glitz_surface);

    if (cairo_surface_status (surface))
	goto FAIL_CLOSE_DISPLAY;

    gxtc->base.width = width;
    gxtc->base.height = height;
    gxtc->base.content = content;
    status = cairo_surface_set_user_data (surface,
					  &glitz_closure_key, gxtc, NULL);
    if (status == CAIRO_STATUS_SUCCESS)
	return surface;

    cairo_surface_destroy (surface);
    surface = cairo_boilerplate_surface_create_in_error (status);

 FAIL_CLOSE_DISPLAY:
    glitz_glx_fini ();
    XCloseDisplay (gxtc->dpy);
 FAIL:
    free (gxtc);
    return surface;
}

void
_cairo_boilerplate_glitz_glx_cleanup (void *closure)
{
    glitz_glx_target_closure_t *gxtc = closure;

    glitz_glx_fini ();

    if (gxtc->win)
	XDestroyWindow (gxtc->dpy, gxtc->win);

    XCloseDisplay (gxtc->dpy);

    free (gxtc);
}

#endif /* CAIRO_CAN_TEST_GLITZ_GLX_SURFACE */

#if CAIRO_CAN_TEST_GLITZ_AGL_SURFACE
#include <glitz-agl.h>

typedef struct _glitz_agl_target_closure {
    glitz_target_closure_base_t base;
} glitz_agl_target_closure_t;

glitz_surface_t *
_cairo_boilerplate_glitz_agl_create_surface_internal (glitz_format_name_t		 formatname,
						      int				 width,
						      int				 height,
						      glitz_agl_target_closure_t	*closure)
{
    glitz_drawable_format_t *dformat;
    glitz_drawable_format_t templ;
    glitz_drawable_t *gdraw;
    glitz_format_t *format;
    glitz_surface_t *sr = NULL;
    unsigned long mask;

    memset(&templ, 0, sizeof(templ));
    templ.color.red_size = 8;
    templ.color.green_size = 8;
    templ.color.blue_size = 8;
    templ.color.alpha_size = 8;
    templ.color.fourcc = GLITZ_FOURCC_RGB;
    templ.samples = 1;

    mask = GLITZ_FORMAT_SAMPLES_MASK | GLITZ_FORMAT_FOURCC_MASK |
	GLITZ_FORMAT_RED_SIZE_MASK | GLITZ_FORMAT_GREEN_SIZE_MASK |
	GLITZ_FORMAT_BLUE_SIZE_MASK;
    if (formatname == GLITZ_STANDARD_ARGB32)
	mask |= GLITZ_FORMAT_ALPHA_SIZE_MASK;

    dformat = glitz_agl_find_pbuffer_format (mask, &templ, 0);
    if (!dformat) {
	CAIRO_BOILERPLATE_LOG ("Glitz failed to find pbuffer format for template.");
	goto FAIL;
    }

    gdraw = glitz_agl_create_pbuffer_drawable (dformat, width, height);
    if (!gdraw) {
	CAIRO_BOILERPLATE_LOG ("Glitz failed to create pbuffer drawable.");
	goto FAIL;
    }

    format = glitz_find_standard_format (gdraw, formatname);
    if (!format) {
	CAIRO_BOILERPLATE_LOG ("Glitz failed to find standard format for drawable.");
	goto DESTROY_DRAWABLE;
    }

    sr = glitz_surface_create (gdraw, format, width, height, 0, NULL);
    if (!sr) {
	CAIRO_BOILERPLATE_LOG ("Glitz failed to create a surface.");
	goto DESTROY_DRAWABLE;
    }

    glitz_surface_attach (sr, gdraw, GLITZ_DRAWABLE_BUFFER_FRONT_COLOR);

 DESTROY_DRAWABLE:
    glitz_drawable_destroy (gdraw);
    return sr;

 FAIL:
    return NULL;
}

cairo_surface_t *
_cairo_boilerplate_glitz_agl_create_surface (const char			 *name,
					     cairo_content_t 		  content,
					     int			  width,
					     int			  height,
					     int			  max_width,
					     int			  max_height,
					     cairo_boilerplate_mode_t	  mode,
					     int                          id,
					     void			**closure)
{
    glitz_surface_t *glitz_surface;
    cairo_surface_t *surface = NULL;
    glitz_agl_target_closure_t *aglc;

    glitz_agl_init ();

    *closure = aglc = xmalloc (sizeof (glitz_agl_target_closure_t));

    switch (content) {
    case CAIRO_CONTENT_COLOR:
	glitz_surface = _cairo_boilerplate_glitz_agl_create_surface_internal (GLITZ_STANDARD_RGB24, width, height, NULL);
	break;
    case CAIRO_CONTENT_COLOR_ALPHA:
	glitz_surface = _cairo_boilerplate_glitz_agl_create_surface_internal (GLITZ_STANDARD_ARGB32, width, height, NULL);
	break;
    default:
	CAIRO_BOILERPLATE_LOG ("Invalid content for glitz-agl test: %d\n", content);
	goto FAIL;
    }

    if (!glitz_surface)
	goto FAIL;

    surface = cairo_glitz_surface_create (glitz_surface);
    glitz_surface_destroy (glitz_surface);

    if (cairo_surface_status (surface))
	goto FAIL;

    aglc->base.width = width;
    aglc->base.height = height;
    aglc->base.content = content;
    status = cairo_surface_set_user_data (surface,
					  &glitz_closure_key, aglc, NULL);
    if (status == CAIRO_STATUS_SUCCESS)
	return surface;

    cairo_surface_destroy (surface);
    surface = cairo_boilerplate_surface_create_in_error (status);

 FAIL:
    glitz_agl_fini ();
    return surface;
}

void
_cairo_boilerplate_glitz_agl_cleanup (void *closure)
{
    free (closure);
    glitz_agl_fini ();
}

#endif /* CAIRO_CAN_TEST_GLITZ_AGL_SURFACE */

#if CAIRO_CAN_TEST_GLITZ_WGL_SURFACE
#include <glitz-wgl.h>

typedef struct _glitz_wgl_target_closure {
    glitz_target_closure_base_t base;
} glitz_wgl_target_closure_t;

glitz_surface_t *
_cairo_boilerplate_glitz_wgl_create_surface_internal (glitz_format_name_t		 formatname,
						      int				 width,
						      int				 height,
						      glitz_wgl_target_closure_t	*closure)
{
    glitz_drawable_format_t *dformat;
    glitz_drawable_format_t templ;
    glitz_drawable_t *gdraw;
    glitz_format_t *format;
    glitz_surface_t *sr = NULL;
    unsigned long mask;

    memset(&templ, 0, sizeof(templ));
    templ.color.red_size = 8;
    templ.color.green_size = 8;
    templ.color.blue_size = 8;
    templ.color.alpha_size = 8;
    templ.color.fourcc = GLITZ_FOURCC_RGB;
    templ.samples = 1;

    mask = GLITZ_FORMAT_SAMPLES_MASK | GLITZ_FORMAT_FOURCC_MASK |
	GLITZ_FORMAT_RED_SIZE_MASK | GLITZ_FORMAT_GREEN_SIZE_MASK |
	GLITZ_FORMAT_BLUE_SIZE_MASK;
    if (formatname == GLITZ_STANDARD_ARGB32)
	mask |= GLITZ_FORMAT_ALPHA_SIZE_MASK;

    dformat = glitz_wgl_find_pbuffer_format (mask, &templ, 0);
    if (!dformat) {
	CAIRO_BOILERPLATE_LOG ("Glitz failed to find pbuffer format for template.");
	goto FAIL;
    }

    gdraw = glitz_wgl_create_pbuffer_drawable (dformat, width, height);
    if (!gdraw) {
	CAIRO_BOILERPLATE_LOG ("Glitz failed to create pbuffer drawable.");
	goto FAIL;
    }

    format = glitz_find_standard_format (gdraw, formatname);
    if (!format) {
	CAIRO_BOILERPLATE_LOG ("Glitz failed to find standard format for drawable.");
	goto DESTROY_DRAWABLE;
    }

    sr = glitz_surface_create (gdraw, format, width, height, 0, NULL);
    if (!sr) {
	CAIRO_BOILERPLATE_LOG ("Glitz failed to create a surface.");
	goto DESTROY_DRAWABLE;
    }

    glitz_surface_attach (sr, gdraw, GLITZ_DRAWABLE_BUFFER_FRONT_COLOR);

 DESTROY_DRAWABLE:
    glitz_drawable_destroy (gdraw);

 FAIL:
    return sr; /* will be NULL unless we create it and attach */
}

cairo_surface_t *
_cairo_boilerplate_glitz_wgl_create_surface (const char			 *name,
					     cairo_content_t		  content,
					     int			  width,
					     int			  height,
					     int			  max_width,
					     int			  max_height,
					     cairo_boilerplate_mode_t	  mode,
					     int                          id,
					     void			**closure)
{
    glitz_surface_t *glitz_surface;
    cairo_surface_t *surface = NULL;
    glitz_wgl_target_closure_t *wglc;

    glitz_wgl_init (NULL);

    *closure = wglc = xmalloc (sizeof (glitz_wgl_target_closure_t));

    switch (content) {
    case CAIRO_CONTENT_COLOR:
	glitz_surface = _cairo_boilerplate_glitz_wgl_create_surface_internal (GLITZ_STANDARD_RGB24, width, height, NULL);
	break;
    case CAIRO_CONTENT_COLOR_ALPHA:
	glitz_surface = _cairo_boilerplate_glitz_wgl_create_surface_internal (GLITZ_STANDARD_ARGB32, width, height, NULL);
	break;
    default:
	CAIRO_BOILERPLATE_LOG ("Invalid content for glitz-wgl test: %d\n", content);
	goto FAIL;
    }

    if (!glitz_surface)
	goto FAIL;

    surface = cairo_glitz_surface_create (glitz_surface);
    glitz_surface_destroy (glitz_surface);

    if (cairo_surface_status (surface))
	goto FAIL;

    wglc->base.width = width;
    wglc->base.height = height;
    wglc->base.content = content;
    status = cairo_surface_set_user_data (surface,
					  &glitz_closure_key, wglc, NULL);
    if (status == CAIRO_STATUS_SUCCESS)
	return surface;

    cairo_surface_destroy (surface);
    surface = cairo_boilerplate_surface_create_in_error (status);

 FAIL:
    glitz_wgl_fini ();
    free (wglc);
    return surface;
}

void
_cairo_boilerplate_glitz_wgl_cleanup (void *closure)
{
    free (closure);
    glitz_wgl_fini ();
}

#endif /* CAIRO_CAN_TEST_GLITZ_WGL_SURFACE */
