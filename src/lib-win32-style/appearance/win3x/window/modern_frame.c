#include "modern_frame.h"
#include "lib-win32-style/appearance/msstyles/modern_frame_style.h"

extern IMAGE_DOS_HEADER __ImageBase;

BOOL win16_uwp_frame_apply_actions(HWND owner,HWND title,const style_uwp_frame_action *actions,unsigned count,
    const classic_theme_mapping *source)
{
    if(!title){SetLastError(ERROR_INVALID_PARAMETER);return FALSE;}
#ifndef STYLE_THEME_EXTERNAL_ONLY
    if(!source)return classic_modern_client_current((HMODULE)&__ImageBase,owner,title,actions,count);
#else
    if(!source || !source->resources){SetLastError(ERROR_RESOURCE_DATA_NOT_FOUND);return FALSE;}
#endif
    return classic_modern_client_frame(owner,title,actions,count,source);
}

BOOL win16_uwp_frame_apply_nonclient(HWND owner)
{return classic_modern_client_current((HMODULE)&__ImageBase,owner,NULL,NULL,0);}
