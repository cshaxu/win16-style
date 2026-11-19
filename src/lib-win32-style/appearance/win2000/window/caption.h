#ifndef WIN2K_WINDOW_CAPTION_H
#define WIN2K_WINDOW_CAPTION_H
#include <windows.h>
#include "lib-win32-style/application/window/caption_slots.h"

typedef struct win2k_caption_colors {
    COLORREF left, right, text;
    BOOL gradient;
} win2k_caption_colors;

/* The caller owns the DC and font. The rectangle is in DC coordinates. */
BOOL win2k_caption_paint(HDC dc,const RECT *bounds,PCWSTR title,HFONT font,
                         const win2k_caption_colors *colors);
win2k_caption_colors win2k_caption_palette(BOOL active);
HFONT win2k_caption_font(UINT dpi,BOOL tool);
enum win2k_caption_part { WIN2K_MINIMIZE, WIN2K_MAXIMIZE, WIN2K_HELP, WIN2K_CLOSE, WIN2K_CAPTION_PARTS };
typedef struct win2k_caption_layout {
    RECT band, icon, buttons[WIN2K_CAPTION_PARTS];
} win2k_caption_layout;

int win2k_caption_height(UINT dpi,BOOL tool);
BOOL win2k_caption_buttons(const RECT *title,UINT dpi,BOOL tool,
                          const BOOL present[4],RECT bounds[4]);
BOOL win2k_caption_buttons_with_group(const RECT *title,UINT dpi,BOOL tool,
    const BOOL present[4],const classic_button_group *group,RECT bounds[4]);
void win2k_caption_render(HDC dc,const win2k_caption_layout *layout,PCWSTR title,HICON icon,
                         HFONT font,UINT dpi,BOOL active,BOOL maximized,
                         const UINT button_flags[WIN2K_CAPTION_PARTS]);
void win2k_caption_render_with_colors(HDC dc,const win2k_caption_layout *layout,PCWSTR title,HICON icon,
    HFONT font,UINT dpi,win2k_caption_colors colors,BOOL maximized,const UINT button_flags[WIN2K_CAPTION_PARTS]);
void win2k_window_border_paint(HDC dc,int width,int height,int thickness);
#endif
