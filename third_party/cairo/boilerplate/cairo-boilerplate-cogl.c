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

#include <cairo-cogl.h>
#include <cogl/cogl2-experimental.h>

typedef struct _cogl_closure {
    cairo_device_t *device;
    cairo_surface_t *surface;
} cogl_closure_t;

static const cairo_user_data_key_t cogl_closure_key;

static void
_cairo_boilerplate_cogl_cleanup (void *abstract_closure)
{
    cogl_closure_t *closure = abstract_closure;

    cairo_device_finish (closure->device);
    cairo_device_destroy (closure->device);

    free (closure);
}

static cairo_surface_t *
_cairo_boilerplate_cogl_create_offscreen_color_surface (const char		*name,
							cairo_content_t		 content,
							double			 width,
							double			 height,
							double			 max_width,
							double			 max_height,
							cairo_boilerplate_mode_t mode,
							void		       **abstract_closure)
{
    CoglContext *context;
    cairo_device_t *device;
    cogl_closure_t *closure;
    cairo_status_t status;

    if (width < 1)
        width = 1;
    if (height < 1)
        height = 1;

    context = cogl_context_new (NULL, NULL);

    device = cairo_cogl_device_create (context);

    /* The device will take a reference on the context */
    cogl_object_unref (context);

    closure = malloc (sizeof (cogl_closure_t));
    *abstract_closure = closure;
    closure->device = device;
    closure->surface = cairo_cogl_offscreen_surface_create (device,
                                                            content,
                                                            width,
                                                            height);

    status = cairo_surface_set_user_data (closure->surface,
					  &cogl_closure_key, closure, NULL);
    if (status == CAIRO_STATUS_SUCCESS)
	return closure->surface;

    _cairo_boilerplate_cogl_cleanup (closure);
    return cairo_boilerplate_surface_create_in_error (status);
}

static cairo_surface_t *
_cairo_boilerplate_cogl_create_onscreen_color_surface (const char	       *name,
						       cairo_content_t		content,
						       double			width,
						       double			height,
						       double			max_width,
						       double			max_height,
						       cairo_boilerplate_mode_t mode,
						       void		      **abstract_closure)
{
    CoglContext *context;
    cairo_device_t *device;
    cogl_closure_t *closure;
    cairo_status_t status;

    if (width < 1)
        width = 1;
    if (height < 1)
        height = 1;

    if (content & CAIRO_CONTENT_ALPHA) {
	/* A hackish way to ensure that we get a framebuffer with
	 * an alpha component */
	CoglSwapChain *swap_chain;
	CoglOnscreenTemplate *onscreen_template;
	CoglRenderer *renderer;
	CoglDisplay *display;

        swap_chain = cogl_swap_chain_new ();
        cogl_swap_chain_set_has_alpha (swap_chain, TRUE);

        onscreen_template = cogl_onscreen_template_new (swap_chain);
        renderer = cogl_renderer_new ();
        display = cogl_display_new (renderer, onscreen_template);

        /* References will be taken on the swap chain, renderer, and
         * onscreen template by the constructors */
        cogl_object_unref (swap_chain);
        cogl_object_unref (renderer);
        cogl_object_unref (onscreen_template);

        context = cogl_context_new (display, NULL);

        /* The context will take a reference on the display */
        cogl_object_unref (display);
    } else {
        context = cogl_context_new (NULL, NULL);
    }

    device = cairo_cogl_device_create (context);

    /* The device will take a reference on the context */
    cogl_object_unref (context);

    closure = malloc (sizeof (cogl_closure_t));
    *abstract_closure = closure;
    closure->device = device;
    closure->surface = cairo_cogl_onscreen_surface_create (device,
                                                           content,
                                                           width,
                                                           height);

    status = cairo_surface_set_user_data (closure->surface,
					  &cogl_closure_key, closure, NULL);
    if (status == CAIRO_STATUS_SUCCESS)
	return closure->surface;

    _cairo_boilerplate_cogl_cleanup (closure);
    return cairo_boilerplate_surface_create_in_error (status);
}

static cairo_status_t
_cairo_boilerplate_cogl_finish (cairo_surface_t *surface)
{
    return cairo_cogl_surface_end_frame (surface);
}

static void
_cairo_boilerplate_cogl_synchronize (void *abstract_closure)
{
    cogl_closure_t *closure = abstract_closure;
    cairo_cogl_surface_synchronize (closure->surface);
}

static const cairo_boilerplate_target_t targets[] = {
    {
	"cogl-offscreen-color", "cogl", NULL, NULL,
	CAIRO_SURFACE_TYPE_COGL, CAIRO_CONTENT_COLOR_ALPHA, 1,
	"cairo_cogl_device_create",
	_cairo_boilerplate_cogl_create_offscreen_color_surface,
	cairo_surface_create_similar,
	NULL,
        _cairo_boilerplate_cogl_finish,
	_cairo_boilerplate_get_image_surface,
	cairo_surface_write_to_png,
	_cairo_boilerplate_cogl_cleanup,
	_cairo_boilerplate_cogl_synchronize,
        NULL,
	TRUE, FALSE, FALSE
    },
    {
	"cogl-onscreen-color", "cogl", NULL, NULL,
	CAIRO_SURFACE_TYPE_COGL, CAIRO_CONTENT_COLOR_ALPHA, 1,
	"cairo_cogl_device_create",
	_cairo_boilerplate_cogl_create_onscreen_color_surface,
	cairo_surface_create_similar,
	NULL,
	_cairo_boilerplate_cogl_finish,
	_cairo_boilerplate_get_image_surface,
	cairo_surface_write_to_png,
	_cairo_boilerplate_cogl_cleanup,
	_cairo_boilerplate_cogl_synchronize,
        NULL,
	TRUE, FALSE, FALSE
    }
};
CAIRO_BOILERPLATE (cogl, targets)
