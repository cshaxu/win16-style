#ifndef WIN2K_EDIT_PAINT_H
#define WIN2K_EDIT_PAINT_H
#include "../common/palette.h"
typedef enum win2k_edit_kind {WIN2K_EDIT_NONE,WIN2K_EDIT_STANDARD,WIN2K_EDIT_NATIVE_CLIENT} win2k_edit_kind;
/* Standard child classes; rich/hotkey client formats and key names stay native. */
win2k_edit_kind win2k_edit_class(HWND);
/* Existing NC allocation, borrowed print DC in window coordinates or NULL for
 * an acquired window DC. No buffer/text/format/font ownership is transferred. */
BOOL win2k_edit_border(HWND,HDC,const win2k_palette *,style_line_profile);
/* Called after the native parent CTLCOLOR result, only for the attached EDIT.
 * Map standard brush roles; custom brush/text/background choices stay owned. */
HBRUSH win2k_edit_brush(HDC,HBRUSH,const win2k_palette *);
#endif
