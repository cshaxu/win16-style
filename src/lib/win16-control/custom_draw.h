#ifndef WIN16_CONTROL_CUSTOM_DRAW_H
#define WIN16_CONTROL_CUSTOM_DRAW_H
#include "palette.h"
#include <commctrl.h>
/* The application keeps its custom-draw result; request private stages only
 * after it selects the default route. State belongs to the calling window. */
LRESULT win16_control_custom_draw_after(HWND window,const NMCUSTOMDRAW *draw,BOOL *active,BOOL internal,
    LRESULT result,const win16_style_palette *palette,BOOL (*supported)(HWND),
    BOOL (*paint_part)(HWND,const NMCUSTOMDRAW *,const win16_style_palette *));
#endif
