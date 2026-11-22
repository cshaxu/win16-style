#ifndef STYLE_RASTER_FONT_H
#define STYLE_RASTER_FONT_H
#include "v4_font.h"
#include "lib-win32-style/application/graphics/font_cache.h"
/* Private, GUI-thread-owned System glyph variants with the source font's
 * cell height and per-character advances. Stock/System inputs stay borrowed.
 * A successful acquire must be released after all native users restore font. */
HFONT style_raster_font_acquire(HFONT source,unsigned points);
HFONT style_raster_font_acquire_isolated(HFONT source,unsigned points);
BOOL style_raster_font_try_retain(HFONT font);
extern const style_font_ops style_raster_font_isolated_operations;
/* VB client text keeps thin strokes when reducing to the native cell. */
HFONT style_raster_font_acquire_stroke_fit(HFONT source,unsigned points);
extern const style_font_ops style_raster_font_stroke_operations;
/* Private menu lease realized from a theme descriptor at window DPI. */
HFONT style_menu_font_acquire_with_bitmap_default(UINT dpi,const msstyles_font *defined);
/* Single-calendar text at the witnessed 96-DPI size. */
HFONT style_raster_font_acquire_at_density(unsigned raster_dpi,unsigned points);
void style_raster_font_release(HFONT font);
void style_raster_font_retain(HFONT font);
HFONT style_raster_font_source(HFONT font);
#endif
