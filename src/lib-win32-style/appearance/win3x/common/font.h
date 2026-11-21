#ifndef WIN16_STYLE_FONT_H
#define WIN16_STYLE_FONT_H
#include "control_api.h"
#include "lib-win32-style/appearance/msstyles/v4_font.h"
/* Standalone compatibility API. The theme-driven runtime does not export it.
 * Same-GUI-thread references to a process-private, 20-pixel System bitmap
 * font. No guest font, disk asset or global font installation. Release only
 * after every borrowing HWND/DC has restored its previous font. */
NTW16S_API HFONT win16_style_font_acquire(void);
NTW16S_API void win16_style_font_release(HFONT font);
/* Private, GUI-thread-owned System glyph variants with the source font's
 * cell height and per-character advances. Stock/System inputs stay borrowed.
 * A successful acquire must be released after all native users restore font. */
HFONT win16_style_system_acquire(HFONT source,unsigned points);
HFONT win16_style_system_acquire_isolated(HFONT source,unsigned points);
BOOL win16_style_system_try_retain(HFONT font);
/* VB client text keeps thin strokes when reducing to the native cell. */
HFONT win16_style_vb_font_acquire(HFONT source,unsigned points);
/* Private menu lease realized from a theme descriptor at window DPI. */
HFONT win16_style_menu_font_acquire(UINT dpi,const msstyles_font *defined);
/* Single-calendar text at the witnessed 96-DPI size. */
HFONT win16_style_calendar_font_acquire(unsigned raster_dpi,unsigned points);
void win16_style_system_release(HFONT font);
void win16_style_system_retain(HFONT font);
HFONT win16_style_system_source(HFONT font);
#endif
