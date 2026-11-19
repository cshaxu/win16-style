#ifndef WIN2K_PAINT_H
#define WIN2K_PAINT_H
#include "palette.h"

/* Source-derived rectangular USER edge. BF_SOFT/FLAT/MONO select the original
 * layer colors; BF_ADJUST publishes the inset. Diagonal/partial sides are
 * outside this operation. The caller retains the DC and palette. */
BOOL win2k_edge(HDC,RECT *,UINT edge,UINT flags,const win2k_palette *);
BOOL win2k_fill(HDC,const RECT *,COLORREF);
BOOL win2k_dither(HDC,const RECT *,const win2k_palette *);
BOOL win2k_push_button_frame(HDC,RECT *,UINT state,BOOL default_button,const win2k_palette *);
/* Original DFC_SCROLL frame and Marlett direction glyph, without BF_SOFT.
 * Borrowed rect/DC/palette; this operation owns only its temporary font. */
BOOL win2k_scroll_arrow_button(HDC,const RECT *,UINT state,const win2k_palette *);
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
