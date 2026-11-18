#include "native.h"
#include <commctrl.h>

static BOOL tab_changes(UINT m)
{
    switch(m){
    case WM_SETFONT:case WM_SETFOCUS:case WM_KILLFOCUS:case WM_ENABLE:case WM_SIZE:
    case WM_STYLECHANGED:case WM_UPDATEUISTATE:case WM_THEMECHANGED:case WM_MOUSEMOVE:case WM_MOUSELEAVE:
    case WM_LBUTTONDOWN:case WM_LBUTTONUP:case WM_KEYDOWN:case WM_KEYUP:
    case TCM_INSERTITEMA:case TCM_INSERTITEMW:case TCM_DELETEITEM:case TCM_DELETEALLITEMS:
    case TCM_SETCURSEL:case TCM_SETCURFOCUS:case TCM_SETITEMA:case TCM_SETITEMW:
    case TCM_SETPADDING:case TCM_SETITEMSIZE:case TCM_SETIMAGELIST:case TCM_SETMINTABWIDTH:
    case TCM_HIGHLIGHTITEM:case TCM_DESELECTALL:case TCM_SETEXTENDEDSTYLE:return TRUE;
    default:return FALSE;
    }
}
static BOOL header_changes(UINT m)
{
    switch(m){
    case WM_SIZE:case WM_SETFONT:case WM_STYLECHANGED:case WM_ENABLE:case WM_THEMECHANGED:
    case WM_MOUSEMOVE:case WM_LBUTTONDOWN:case WM_LBUTTONUP:case WM_LBUTTONDBLCLK:
    case WM_CANCELMODE:case WM_CAPTURECHANGED:case WM_SETFOCUS:case WM_KILLFOCUS:
    case WM_KEYDOWN:case WM_KEYUP:case WM_UPDATEUISTATE:
    case HDM_INSERTITEMA:case HDM_INSERTITEMW:case HDM_SETITEMA:case HDM_SETITEMW:case HDM_DELETEITEM:
    case HDM_SETORDERARRAY:case HDM_SETIMAGELIST:case HDM_SETBITMAPMARGIN:case HDM_SETUNICODEFORMAT:
        return TRUE;
    default:return FALSE;
    }
}
static BOOL progress_changes(UINT m)
{
    return m==PBM_SETRANGE || m==PBM_SETRANGE32 || m==PBM_SETPOS || m==PBM_DELTAPOS ||
        m==PBM_SETSTEP || m==PBM_STEPIT || m==PBM_SETBARCOLOR || m==PBM_SETBKCOLOR ||
        m==PBM_SETSTATE || m==PBM_SETMARQUEE || m==WM_STYLECHANGED || m==WM_SIZE || m==WM_ENABLE;
}
void style_control_native_init(style_control_native_state *state)
{*state=(style_control_native_state){.pressed=-1};}
BOOL style_control_native_changes(style_control_native_kind kind,UINT message)
{
    if(kind==STYLE_CONTROL_TAB)return tab_changes(message);
    if(kind==STYLE_CONTROL_HEADER)return header_changes(message);
    return kind==STYLE_CONTROL_PROGRESS && progress_changes(message);
}
void style_control_native_observe(style_control_native_state *state,style_control_native_kind kind,UINT message)
{if(style_control_native_changes(kind,message))++state->epoch;}
LRESULT style_control_native_message(HWND w,style_instance *instance,style_control_native_state *state,style_control_native_kind kind,
    UINT m,WPARAM a,LPARAM b,BOOL (*supported)(HWND),const style_control_draw *draw)
{
    LRESULT result=0;
    if(kind==STYLE_CONTROL_HEADER){
        if(m==WM_LBUTTONDOWN){
            HDHITTESTINFO hit={.pt={(short)LOWORD(b),(short)HIWORD(b)}};
            LRESULT item=SendMessageW(w,HDM_HITTEST,0,(LPARAM)&hit);
            if(instance->detached)return 0;
            state->pressed=(item>=0 && (hit.flags&HHT_ONHEADER))?(int)item:-1;
        }
        if(m==WM_LBUTTONUP || m==WM_CANCELMODE || m==WM_CAPTURECHANGED)state->pressed=-1;
    }
    BOOL classic=kind==STYLE_CONTROL_PROGRESS?supported(w):
        ((m==WM_PAINT || m==WM_PRINTCLIENT) && supported(w));
    if(m==WM_PAINT && classic){PAINTSTRUCT ps;HDC dc=BeginPaint(w,&ps);draw->render(w,draw->context,dc,TRUE);EndPaint(w,&ps);}
    else if(m==WM_PRINTCLIENT && classic)draw->render(w,draw->context,(HDC)a,FALSE);
    else if(kind==STYLE_CONTROL_PROGRESS && m==WM_PRINT && classic && (b&PRF_CLIENT)){
        int saved=SaveDC((HDC)a);if(saved){result=DefSubclassProc(w,m,a,b&~PRF_CLIENT);RestoreDC((HDC)a,saved);}
        if(!instance->detached)draw->render(w,draw->context,(HDC)a,FALSE);
    }else result=DefSubclassProc(w,m,a,b);
    if(!instance->detached && style_control_native_changes(kind,m))
        RedrawWindow(w,NULL,NULL,RDW_INVALIDATE|(instance->calls==1?RDW_UPDATENOW:0));
    return result;
}

void style_control_native_paint(HWND w,style_instance *instance,style_control_state *state,void *context,
    BOOL (*paint)(HWND,void *,HDC,HFONT),HDC dc)
{
    HFONT font=state?state->font:NULL;
    const style_control_fonts *fonts=state?state->fonts:NULL;
    if(fonts)fonts->retain(font);
    if(!paint(w,context,dc,font) && !instance->detached)
        DefSubclassProc(w,WM_PRINTCLIENT,(WPARAM)dc,PRF_CLIENT);
    if(fonts)fonts->release(font);
}
