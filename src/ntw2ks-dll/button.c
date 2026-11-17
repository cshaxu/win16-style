/* Windows classic button primitive. The caller owns the DC and rectangle. */
#include "lib-win32-style/appearance/win2000/control/button_paint.h"

__declspec(dllexport) BOOL WINAPI Ntw2ksDrawButton(HDC dc, RECT *bounds, UINT state)
{
    if(!dc || !bounds)return FALSE;
    return win2k_push_button_frame(dc,bounds,DFCS_BUTTONPUSH|state,FALSE,&win2k_control_standard_palette);
}
