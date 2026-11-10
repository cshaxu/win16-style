/* Windows classic button primitive. The caller owns the DC and rectangle. */
#include <windows.h>

__declspec(dllexport) BOOL WINAPI Ntw2ksDrawButton(HDC dc, RECT *bounds, UINT state)
{
    if(!dc || !bounds)return FALSE;
    return DrawFrameControl(dc,bounds,DFC_BUTTON,DFCS_BUTTONPUSH | state);
}
