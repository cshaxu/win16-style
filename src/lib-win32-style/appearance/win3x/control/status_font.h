#ifndef WIN16_CONTROL_STATUS_FONT_H
#define WIN16_CONTROL_STATUS_FONT_H
#include "lib-win32-style/application/graphics/font_cache.h"
extern const style_font_ops win16_status_font_operations;
BOOL win16_control_status_text_supported(LPCWSTR,int);
#endif
