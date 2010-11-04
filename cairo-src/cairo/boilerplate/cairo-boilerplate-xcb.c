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
#include "cairo-boilerplate-xcb-private.h"

#include <cairo-xcb-xrender.h>

#include <xcb/xcb_renderutil.h>

typedef struct _xcb_target_closure
{
    xcb_connection_t *c;
    xcb_pixmap_t pixmap;
} xcb_target_closure_t;

void
_cairo_boilerplate_xcb_synchronize (void *closure)
{
    xcb_target_closure_t *xtc = closure;
    free (xcb_get_image_reply (xtc->c,
		xcb_get_image (xtc->c, XCB_IMAGE_FORMAT_Z_PIXMAP,
		    xtc->pixmap, 0, 0, 1, 1, /* AllPlanes */ ~0UL),
		0));
}

cairo_surface_t *
_cairo_boilerplate_xcb_create_surface (const char		 *name,
				       cairo_content_t		  content,
				       int			  width,
				       int			  height,
				       int			  max_width,
				       int			  max_height,
				       cairo_boilerplate_mode_t	  mode,
				       int                        id,
				       void			**closure)
{
    xcb_screen_t *root;
    xcb_target_closure_t *xtc;
    xcb_connection_t *c;
    xcb_render_pictforminfo_t *render_format;
    xcb_pict_standard_t format;

    *closure = xtc = xmalloc (sizeof (xcb_target_closure_t));

    if (width == 0)
	width = 1;
    if (height == 0)
	height = 1;

    xtc->c = c = xcb_connect(NULL,NULL);
    if (xcb_connection_has_error(c)) {
	CAIRO_BOILERPLATE_LOG ("Failed to connect to X server through XCB\n");
	return NULL;
    }

    root = xcb_setup_roots_iterator(xcb_get_setup(c)).data;

    xtc->pixmap = xcb_generate_id (c);
    xcb_create_pixmap (c, 32, xtc->pixmap, root->root,
			 width, height);

    switch (content) {
    case CAIRO_CONTENT_COLOR:
	format = XCB_PICT_STANDARD_RGB_24;
	break;
    case CAIRO_CONTENT_COLOR_ALPHA:
	format = XCB_PICT_STANDARD_ARGB_32;
	break;
    case CAIRO_CONTENT_ALPHA:  /* would be XCB_PICT_STANDARD_A_8 */
    default:
	CAIRO_BOILERPLATE_LOG ("Invalid content for XCB test: %d\n", content);
	return NULL;
    }

    render_format = xcb_render_util_find_standard_format (xcb_render_util_query_formats (c), format);
    if (render_format->id == 0)
	return NULL;

    return cairo_xcb_surface_create_with_xrender_format (c, xtc->pixmap, root,
							 render_format,
							 width, height);
}

void
_cairo_boilerplate_xcb_cleanup (void *closure)
{
    xcb_target_closure_t *xtc = closure;

    xcb_free_pixmap (xtc->c, xtc->pixmap);
    xcb_disconnect (xtc->c);
    free (xtc);
}
