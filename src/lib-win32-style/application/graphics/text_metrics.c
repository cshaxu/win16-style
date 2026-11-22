#include "text_metrics.h"

BOOL style_text_fit(HDC target,HFONT font,const WCHAR *text,const RECT *bounds,COLORREF face,COLORREF ink,SIZE limit,SIZE *drawn)
{
    RECT measure={0,0,0,0};
    if(!DrawTextW(target,text,-1,&measure,DT_CALCRECT|DT_SINGLELINE))return FALSE;
    int width=measure.right,height=measure.bottom,available_width=bounds->right-bounds->left,available_height=bounds->bottom-bounds->top;
    if(width<=0 || height<=0 || width>limit.cx || height>limit.cy || available_width<=0 || available_height<=0 ||
       (width<=available_width && height<=available_height))return FALSE;
    int out_width=width,out_height=height;
    if(out_width>available_width){out_height=MulDiv(out_height,available_width,out_width);out_width=available_width;}
    if(out_height>available_height){out_width=MulDiv(out_width,available_height,out_height);out_height=available_height;}
    if(out_width<=0 || out_height<=0)return FALSE;
    HDC memory=CreateCompatibleDC(target);if(!memory)return FALSE;
    HBITMAP bitmap=CreateCompatibleBitmap(target,width,height);if(!bitmap){DeleteDC(memory);return FALSE;}
    HGDIOBJ old=SelectObject(memory,bitmap),old_font=SelectObject(memory,font);BOOL ok=FALSE;
    if(old && old!=HGDI_ERROR && old_font && old_font!=HGDI_ERROR){
        RECT r={0,0,width,height};SetDCBrushColor(memory,face);FillRect(memory,&r,(HBRUSH)GetStockObject(DC_BRUSH));
        SetBkMode(memory,TRANSPARENT);SetTextColor(memory,ink);
        if(DrawTextW(memory,text,-1,&r,DT_SINGLELINE)){
            int saved=SaveDC(target);
            if(saved){SetStretchBltMode(target,COLORONCOLOR);
                int x=bounds->left+(available_width-out_width)/2,y=bounds->top+(available_height-out_height)/2;
                ok=StretchBlt(target,x,y,out_width,out_height,memory,0,0,width,height,SRCCOPY);
                RestoreDC(target,saved);
            }
        }
    }
    if(old_font && old_font!=HGDI_ERROR)SelectObject(memory,old_font);
    if(old && old!=HGDI_ERROR)SelectObject(memory,old);
    DeleteObject(bitmap);DeleteDC(memory);
    if(ok){drawn->cx=out_width;drawn->cy=out_height;}return ok;
}

BOOL style_text_ascii(LPCWSTR text,int count)
{
    for(int i=0;i<count;++i)if(text[i]>0x7f)return FALSE;
    return TRUE;
}

BOOL style_text_single_byte_wide(HDC dc,const WCHAR *text,int count,BOOL active,UINT code_page)
{
    if(!active)return TRUE;
    if(count<0 || count>4096 || (GetTextAlign(dc)&TA_RTLREADING))return FALSE;
    if(!count)return TRUE;
    BOOL used=FALSE;char buffer[4096];
    return text && WideCharToMultiByte(code_page,WC_NO_BEST_FIT_CHARS,text,count,buffer,sizeof(buffer),NULL,&used)==count && !used;
}
BOOL style_text_single_byte_ansi(HDC dc,const char *text,int count,BOOL active,UINT code_page)
{
    if(!active)return TRUE;
    if(!text || (GetTextAlign(dc)&TA_RTLREADING))return FALSE;
    if(count==-1){count=0;while(count<=4096 && text[count])++count;}
    if(count<0 || count>4096)return FALSE;
    if(GetACP()!=code_page)for(int i=0;i<count;++i)if((unsigned char)text[i]>=128)return FALSE;
    return TRUE;
}

BOOL style_text_glyph_run_allowed(HDC dc,UINT options,UINT count)
{return count<=4096 && !(options&ETO_RTLREADING) && !(GetTextAlign(dc)&TA_RTLREADING);}

HFONT style_text_static_source(HDC dc,LPCWSTR text,int count,UINT flags,const void *address,HWND window,HMODULE native_module)
{
    HMODULE caller=NULL;HWND from;
    if(!dc || !text || (flags&(DT_CALCRECT|DT_RTLREADING|DT_MODIFYSTRING)) ||
       (GetLayout(dc)&LAYOUT_RTL) || !GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|
       GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,(LPCWSTR)address,&caller) || caller!=native_module)return NULL;
    from=WindowFromDC(dc);if(from && from!=window)return NULL;
    if(count< -1)return NULL;
    if(count==-1){count=0;while(count<4096 && text[count])++count;if(count==4096)return NULL;}
    if(count>4096)return NULL;
    return style_text_ascii(text,count)?(HFONT)GetCurrentObject(dc,OBJ_FONT):NULL;
}
