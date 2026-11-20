#ifndef WIN2K_PAINT_H
#define WIN2K_PAINT_H
#include "palette.h"

enum { WIN2K_CHOICE_SIZE=13 }; /* inctlpan.c's native cached glyph dimensions */
/* Square DrawBox operation; font and temporary GDI objects belong to this call.
 * Uses the system Marlett: four 13px state outlines match original Win2000
 * screenshot crops. This is not a claim to ship the original font file. */
struct classic_surface_colors;
BOOL win2k_choice_glyph(HDC,const RECT *,UINT state,const win2k_palette *,const struct classic_surface_colors *);
/* Native DrawState content with standard disabled emboss or mixed monochrome.
 * All images/text/DC remain borrowed; temporary brushes belong to this call. */
BOOL win2k_draw_content(HDC,LPARAM,WPARAM,int,int,int,int,UINT,DRAWSTATEPROC,BOOL,BOOL,const win2k_palette *);
#endif
