#ifndef WIN2K_WINDOW_CAPTION_H
#define WIN2K_WINDOW_CAPTION_H
#include "lib-win32-style/appearance/msstyles/native_caption_style.h"

BOOL win2k_caption_paint(HDC dc,const RECT *bounds,PCWSTR title,HFONT font,
                         const style_native_caption_colors *colors);
BOOL win2k_caption_system_buttons(const RECT *title,UINT dpi,BOOL tool,
                          const BOOL present[4],RECT bounds[4]);
BOOL win2k_caption_buttons_with_group(const RECT *title,UINT dpi,BOOL tool,
    const BOOL present[4],const classic_button_group *group,const classic_caption_metrics *metrics,RECT bounds[4]);
void win2k_caption_render(HDC dc,const style_native_caption_layout *layout,PCWSTR title,HICON icon,
                         HFONT font,UINT dpi,BOOL active,BOOL maximized,
                         const UINT button_flags[STYLE_NATIVE_CAPTION_BUTTONS]);
#endif
