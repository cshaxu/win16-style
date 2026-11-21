/* Windows classic button primitive. The caller owns the DC and rectangle. */
#include "lib-win32-style/appearance/win2000/common/palette.h"

extern IMAGE_DOS_HEADER __ImageBase;

BOOL WINAPI Ntw2ksDrawButton(HDC dc, RECT *bounds, UINT state)
{
    if(!dc || !bounds)return FALSE;
    classic_theme_mapping source={0};
    DWORD error=classic_theme_attachment_layout((HMODULE)&__ImageBase,CLASSIC_WINDOW_PROVIDER_NONCLIENT,&source);
    if(error){SetLastError(error);return FALSE;}
    win2k_palette palette=classic_theme_relief_palette(source.resources?&source.model:NULL);
    classic_theme_close(&source);
    return style_paint_push_frame(dc,bounds,DFCS_BUTTONPUSH|state,FALSE,1,STYLE_PUSH_RELIEF,&palette);
}
