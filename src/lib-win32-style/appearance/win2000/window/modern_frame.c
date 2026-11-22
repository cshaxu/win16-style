#include "modern_frame.h"
#include "lib-win32-style/appearance/msstyles/modern_frame_style.h"
#ifndef STYLE_THEME_EXTERNAL_ONLY
#include "caption.h"
#endif

BOOL win2k_uwp_frame_apply(HWND owner,HWND title,const style_uwp_frame_action *actions,unsigned count,
    const classic_theme_mapping *source)
{
    if(!title){SetLastError(ERROR_INVALID_PARAMETER);return FALSE;}
#ifdef STYLE_THEME_EXTERNAL_ONLY
    if(!source || !source->resources){SetLastError(ERROR_RESOURCE_DATA_NOT_FOUND);return FALSE;}
#endif
    style_uwp_observed_theme theme={.create_font=classic_modern_caption_font};
    if(source)classic_modern_observed_theme(&theme,source);
#ifndef STYLE_THEME_EXTERNAL_ONLY
    else{
        theme.default_buttons=win2k_caption_system_buttons;
        theme.default_grip_width=16;
    }
#endif
    return style_uwp_observed_frame_apply(owner,title,actions,count,&theme);
}
