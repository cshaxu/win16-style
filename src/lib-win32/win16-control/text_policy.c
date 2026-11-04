#include "text_policy.h"
#include "../win16-common/font.h"

BOOL win16_control_vb_text_caller(const void *address,BOOL (*report_caller)(HMODULE))
{
    HMODULE caller=NULL;
    if(!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            (const WCHAR *)address,&caller))return FALSE;
    return report_caller(caller) || caller==GetModuleHandleW(L"MSVBVM60.DLL") || caller==GetModuleHandleW(L"MSVBVM50.DLL") || caller==GetModuleHandleW(L"USER32.DLL") ||
        caller==GetModuleHandleW(L"LPK.DLL") || caller==GetModuleHandleW(L"USP10.DLL") || caller==GetModuleHandleW(L"GDI32FULL.DLL");
}
BOOL win16_control_vb_label_caller(const void *address,BOOL direct)
{
    if(direct)return TRUE;
    HMODULE caller=NULL;
    return GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        (const WCHAR *)address,&caller) && (caller==GetModuleHandleW(L"MSVBVM50.DLL") || caller==GetModuleHandleW(L"MSVBVM60.DLL"));
}

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

BOOL win16_control_vb_glyph_prepare(HDC dc,LPCWSTR *text,UINT *options,UINT count,WCHAR **decoded,char **encoded,BOOL (*decode)(HDC,const WCHAR *,UINT,WCHAR *))
{
    BOOL valid=count<=4096 && !(*options&ETO_RTLREADING) && !(GetTextAlign(dc)&TA_RTLREADING);
    if(valid && count)*decoded=HeapAlloc(GetProcessHeap(),0,count*sizeof(WCHAR));
    if(count && !*decoded)valid=FALSE;
    if(valid)valid=decode(dc,*text,count,*decoded);
    if(valid && count){
        BOOL used_default=FALSE;*encoded=HeapAlloc(GetProcessHeap(),0,count);
        if(!*encoded || WideCharToMultiByte(1252,WC_NO_BEST_FIT_CHARS,*decoded,(int)count,*encoded,(int)count,NULL,&used_default)!=(int)count || used_default)valid=FALSE;
    }
    if(valid){*text=*decoded;*options&=~ETO_GLYPH_INDEX;}
    return valid;
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
