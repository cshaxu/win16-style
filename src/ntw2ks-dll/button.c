/* Windows classic button primitive. The caller owns the DC and rectangle. */
#include "lib-win32-style/appearance/win2000/common/paint.h"

BOOL WINAPI Ntw2ksDrawButton(HDC dc, RECT *bounds, UINT state)
{
    if(!dc || !bounds)return FALSE;
    return style_paint_push_frame(dc,bounds,DFCS_BUTTONPUSH|state,FALSE,1,STYLE_PUSH_RELIEF,&win2k_standard_palette);
}
