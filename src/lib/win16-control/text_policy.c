#include "text_policy.h"
#include "font.h"

BOOL win16_control_report_wide_text(HDC dc,const WCHAR *text,int count,BOOL active)
{
    if(!active)return TRUE;
    if(count<0 || count>4096 || (GetTextAlign(dc)&TA_RTLREADING))return FALSE;
    if(!count)return TRUE;
    BOOL used=FALSE;char buffer[4096];
    return text && WideCharToMultiByte(1252,WC_NO_BEST_FIT_CHARS,text,count,buffer,sizeof(buffer),NULL,&used)==count && !used;
}
BOOL win16_control_report_ansi_text(HDC dc,const char *text,int count,BOOL active)
{
    if(!active)return TRUE;
    if(!text || (GetTextAlign(dc)&TA_RTLREADING))return FALSE;
    if(count==-1){count=0;while(count<=4096 && text[count])++count;}
    if(count<0 || count>4096)return FALSE;
    if(GetACP()!=1252)for(int i=0;i<count;++i)if((unsigned char)text[i]>=128)return FALSE;
    return TRUE;
}

HFONT win16_control_static_text_font(HDC dc,LPCWSTR text,int count,UINT flags,const void *address,HWND window,HMODULE native_module)
{
    HMODULE caller=NULL;HWND from;HFONT source;
    if(!dc || !text || (flags&(DT_CALCRECT|DT_RTLREADING|DT_MODIFYSTRING)) ||
       (GetLayout(dc)&LAYOUT_RTL) || !GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|
       GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,(LPCWSTR)address,&caller) || caller!=native_module)return NULL;
    from=WindowFromDC(dc);if(from && from!=window)return NULL;
    if(count< -1)return NULL;
    if(count==-1){count=0;while(count<4096 && text[count])++count;if(count==4096)return NULL;}
    if(count>4096)return NULL;
    for(int i=0;i<count;++i)if(text[i]>0x7f)return NULL;
    source=(HFONT)GetCurrentObject(dc,OBJ_FONT);return source?win16_style_system_acquire(source):NULL;
}
