#include "status_font.h"
#include "lib-win32-style/appearance/msstyles/raster_font.h"
const style_font_ops win16_status_font_operations={style_raster_font_acquire_isolated,
    style_raster_font_try_retain,style_raster_font_release};
BOOL win16_control_status_text_supported(LPCWSTR text,int count)
{for(int i=0;i<count;++i)if(text[i]>0x7f)return FALSE;return TRUE;}
