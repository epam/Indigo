#ifndef CAIRO_TEST_DIRECTFB_H_
#define CAIRO_TEST_DIRECTFB_H_

/* Two functions: One for a real window, one for a bitmap */

#include <cairo.h>

CAIRO_BEGIN_DECLS

extern cairo_surface_t *
_cairo_boilerplate_directfb_create_surface (const char			 *name,
					    cairo_content_t		  content,
					    int				  width,
					    int				  height,
					    int				  max_width,
					    int				  max_height,
					    cairo_boilerplate_mode_t	  mode,
					    int                           id,
					    void			**closure);

extern void
_cairo_boilerplate_directfb_cleanup (void* closure);

CAIRO_END_DECLS

#endif
