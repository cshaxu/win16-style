#ifndef WIN16_STYLE_FONT_H
#define WIN16_STYLE_FONT_H
#include "api.h"
/* Legacy explicit API, no longer the default frame/control typography.
 * Same-GUI-thread references to a process-private, 20-pixel System bitmap
 * font. No guest font, disk asset or global font installation. Release only
 * after every borrowing HWND/DC has restored its previous font. */
NTW16S_API HFONT win16_style_font_acquire(void);
NTW16S_API void win16_style_font_release(HFONT font);
/* Private, GUI-thread-owned System glyph variants with the source font's
 * cell height and per-character advances. Stock/System inputs stay borrowed.
 * A successful acquire must be released after all native users restore font. */
HFONT win16_style_system_acquire(HFONT source);
HFONT win16_style_system_acquire_isolated(HFONT source);
BOOL win16_style_system_try_retain(HFONT font);
/* VB client text keeps thin strokes when reducing to the native cell. */
HFONT win16_style_vb_font_acquire(HFONT source);
/* Private menu lease: System16 uniformly scaled by window DPI. */
HFONT win16_style_menu_font_acquire(UINT dpi);
/* Single-calendar text at the witnessed 96-DPI size. */
HFONT win16_style_calendar_font_acquire(void);
void win16_style_system_release(HFONT font);
void win16_style_system_retain(HFONT font);
HFONT win16_style_system_source(HFONT font);
#endif
