#ifndef WIN16_CONTROL_STATUS_FONT_H
#define WIN16_CONTROL_STATUS_FONT_H
#include <windows.h>
typedef struct win16_control_status_font_cache {
    HFONT font,source;LOGFONTW log;TEXTMETRICW metrics;int widths[96];unsigned borrows;BOOL invalid;
} win16_control_status_font_cache;
HFONT win16_control_status_font_borrow(win16_control_status_font_cache *,HDC,void *,void **);
void win16_control_status_font_return(win16_control_status_font_cache *,HFONT);
#endif
