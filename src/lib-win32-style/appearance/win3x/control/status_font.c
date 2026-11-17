#include "status_font.h"
#include "../common/font.h"
const style_font_ops win16_status_font_operations={win16_style_system_acquire_isolated,
    win16_style_system_try_retain,win16_style_system_release};
BOOL win16_control_status_text_supported(LPCWSTR text,int count)
{for(int i=0;i<count;++i)if(text[i]>0x7f)return FALSE;return TRUE;}
