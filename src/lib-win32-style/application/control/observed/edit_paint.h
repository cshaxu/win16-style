#ifndef STYLE_OBSERVED_EDIT_PAINT_H
#define STYLE_OBSERVED_EDIT_PAINT_H
#include "lib-win32-style/application/graphics/paint_frame.h"
typedef enum style_observed_edit_kind {STYLE_OBSERVED_EDIT_NONE,STYLE_OBSERVED_EDIT_STANDARD,STYLE_OBSERVED_EDIT_NATIVE_CLIENT} style_observed_edit_kind;
/* Standard child classes; rich/hotkey client formats and key names stay native. */
style_observed_edit_kind style_observed_edit_class(HWND);
/* Existing NC allocation, borrowed print DC in window coordinates or NULL for
 * an acquired window DC. No buffer/text/format/font ownership is transferred. */
BOOL style_observed_edit_border(HWND,HDC,const style_relief_palette *,style_line_profile);
/* Called after the native parent CTLCOLOR result, only for the attached EDIT.
 * Map standard brush roles; custom brush/text/background choices stay owned. */
HBRUSH style_observed_edit_brush(HDC,HBRUSH,const style_relief_palette *);
#endif
