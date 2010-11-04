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
#include "cairo-boilerplate-test-surfaces-private.h"

#include <test-fallback-surface.h>
#include <test-meta-surface.h>
#include <test-paginated-surface.h>

#include <assert.h>

cairo_surface_t *
_cairo_boilerplate_test_fallback_create_surface (const char			 *name,
						 cairo_content_t		  content,
						 int				  width,
						 int				  height,
						 int				  max_width,
						 int				  max_height,
						 cairo_boilerplate_mode_t	  mode,
						 int                              id,
						 void				**closure)
{
    *closure = NULL;
    return _cairo_test_fallback_surface_create (content, width, height);
}

cairo_surface_t *
_cairo_boilerplate_test_meta_create_surface (const char			 *name,
					     cairo_content_t		  content,
					     int			  width,
					     int			  height,
					     int			  max_width,
					     int			  max_height,
					     cairo_boilerplate_mode_t	  mode,
					     int                          id,
					     void			**closure)
{
    *closure = NULL;
    return _cairo_test_meta_surface_create (content, width, height);
}

static const cairo_user_data_key_t test_paginated_closure_key;

typedef struct {
    unsigned char *data;
    cairo_content_t content;
    int width;
    int height;
    int stride;
} test_paginated_closure_t;

cairo_surface_t *
_cairo_boilerplate_test_paginated_create_surface (const char			 *name,
						  cairo_content_t		  content,
						  int				  width,
						  int				  height,
						  int				  max_width,
						  int				  max_height,
						  cairo_boilerplate_mode_t	  mode,
						  int                             id,
						  void				**closure)
{
    test_paginated_closure_t *tpc;
    cairo_format_t format;
    cairo_surface_t *surface;
    cairo_status_t status;

    *closure = tpc = xmalloc (sizeof (test_paginated_closure_t));

    format = cairo_boilerplate_format_from_content (content);

    tpc->content = content;
    tpc->width = width;
    tpc->height = height;
    tpc->stride = cairo_format_stride_for_width (format, width);
    tpc->data = xcalloc (tpc->stride, height);

    surface = _cairo_test_paginated_surface_create_for_data (tpc->data,
							     tpc->content,
							     tpc->width,
							     tpc->height,
							     tpc->stride);
    if (cairo_surface_status (surface))
	goto CLEANUP;

    status = cairo_surface_set_user_data (surface,
					  &test_paginated_closure_key,
					  tpc, NULL);
    if (status == CAIRO_STATUS_SUCCESS)
	return surface;

    cairo_surface_destroy (surface);
    surface = cairo_boilerplate_surface_create_in_error (status);

  CLEANUP:
    free (tpc->data);
    free (tpc);
    return surface;
}

/* The only reason we go through all these machinations to write a PNG
 * image is to _really ensure_ that the data actually landed in our
 * buffer through the paginated surface to the test_paginated_surface.
 *
 * If we didn't implement this function then the default
 * cairo_surface_write_to_png would result in the paginated_surface's
 * acquire_source_image function replaying the meta-surface to an
 * intermediate image surface. And in that case the
 * test_paginated_surface would not be involved and wouldn't be
 * tested.
 */
cairo_status_t
_cairo_boilerplate_test_paginated_surface_write_to_png (cairo_surface_t	*surface,
						        const char	*filename)
{
    cairo_surface_t *image;
    cairo_format_t format;
    test_paginated_closure_t *tpc;
    cairo_status_t status;

    /* show page first.  the automatic show_page is too late for us */
    cairo_surface_show_page (surface);
    status = cairo_surface_status (surface);
    if (status)
	return status;

    tpc = cairo_surface_get_user_data (surface, &test_paginated_closure_key);

    format = cairo_boilerplate_format_from_content (tpc->content);

    image = cairo_image_surface_create_for_data (tpc->data,
						 format,
						 tpc->width,
						 tpc->height,
						 tpc->stride);

    status = cairo_surface_write_to_png (image, filename);
    cairo_surface_destroy (image);

    return status;
}

cairo_surface_t *
_cairo_boilerplate_test_paginated_get_image_surface (cairo_surface_t *surface,
						     int page,
						     int width,
						     int height)
{
    cairo_format_t format;
    test_paginated_closure_t *tpc;
    cairo_status_t status;

    /* XXX separate finish as per PDF */
    if (page != 0)
	return cairo_boilerplate_surface_create_in_error (CAIRO_STATUS_SURFACE_TYPE_MISMATCH);

    /* show page first.  the automatic show_page is too late for us */
    cairo_surface_show_page (surface);
    status = cairo_surface_status (surface);
    if (status)
	return cairo_boilerplate_surface_create_in_error (status);

    tpc = cairo_surface_get_user_data (surface, &test_paginated_closure_key);

    format = cairo_boilerplate_format_from_content (tpc->content);

    if (0) {
	return cairo_image_surface_create_for_data (tpc->data + tpc->stride * (tpc->height - height) + 4 * (tpc->width - width), /* hide the device offset */
						    format,
						    width,
						    height,
						    tpc->stride);
    } else {
	cairo_surface_t *image, *surface;

	image = cairo_image_surface_create_for_data (tpc->data,
						     format,
						     tpc->width,
						     tpc->height,
						     tpc->stride);
	cairo_surface_set_device_offset (image,
					 tpc->width - width,
					 tpc->height - height);
	surface = _cairo_boilerplate_get_image_surface (image, 0, width, height);
	cairo_surface_destroy (image);
	return surface;
    }
}

void
_cairo_boilerplate_test_paginated_cleanup (void *closure)
{
    test_paginated_closure_t *tpc = closure;

    free (tpc->data);
    free (tpc);
}
