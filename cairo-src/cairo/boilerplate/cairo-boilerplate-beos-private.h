#ifndef _CAIRO_BOILERPLATE_BEOS_PRIVATE_H_
#define _CAIRO_BOILERPLATE_BEOS_PRIVATE_H_

/* Two functions: One for a real window, one for a bitmap */

#include <cairo.h>

CAIRO_BEGIN_DECLS

extern cairo_surface_t *
_cairo_boilerplate_beos_create_surface (const char              *name,
					cairo_content_t          content,
					int                      width,
					int                      height,
					int                      max_width,
					int                      max_height,
					cairo_boilerplate_mode_t mode,
					int                      id,
					void                   **closure);

extern void
_cairo_boilerplate_beos_cleanup (void* closure);

extern cairo_surface_t *
_cairo_boilerplate_beos_create_surface_for_bitmap (const char              *name,
						   cairo_content_t          content,
						   int                      width,
						   int                      height,
						   int                      max_width,
						   int                      max_height,
						   cairo_boilerplate_mode_t mode,
						   int                      id,
						   void                   **closure);

extern void
_cairo_boilerplate_beos_cleanup_bitmap (void* closure);

CAIRO_END_DECLS

#endif
