#include "intercept.h"
#include "startup_hooks.h"
#include "unwind.h"
#include "ntw16s-dll/ntw16s.h"
#include "ntw16s-dll/native_api.h"
#include "ntw16s-dll/vb_text.h"
#include "ntw16s-dll/updown_paint.h"
#include "ntw16s-dll/move.h"
#include <wchar.h>
#include <uxtheme.h>

/* The original trampoline leaves are bound privately to this same DLL.
 * No renderer LoadLibrary/GetProcAddress, GUI allocation or binding in DllMain. */
static INIT_ONCE title_once=INIT_ONCE_STATIC_INIT;
static LONG style_ready;
static __typeof__(&GetScrollInfo) real_get_scroll;
static __typeof__(&SetScrollInfo) real_set_scroll;
static __typeof__(&SetWindowTheme) real_set_theme;
static BOOL CALLBACK bind_titles(PINIT_ONCE once,void *parameter,void **context)
{
    (void)once;(void)parameter;(void)context;
    ntw16s_native_api api={sizeof(api),real_get_scroll,real_set_scroll,real_set_theme};
    if(!ntw16s_bind_native_api(&api))return FALSE;
    ntw16s_updown_api_enable();
    InterlockedExchange(&style_ready,1);return TRUE;
}
typedef struct title_scope { ntw16s_creation_scope *creation;DWORD failure; } title_scope;
static BOOL begin_titles(title_scope *scope,BOOL modal)
{
    (void)modal;ZeroMemory(scope,sizeof(*scope));
    if(!InitOnceExecuteOnce(&title_once,bind_titles,NULL,NULL))return FALSE;
    scope->creation=Ntw16sCreationBegin(TRUE);return scope->creation!=NULL;
}
static void end_titles(void *data)
{
    title_scope *scope=data;
    ntw16s_creation_result result={0};
    if(!Ntw16sCreationEnd(scope->creation,&result))scope->failure=GetLastError();
    else scope->failure=result.failure;
}

#define WINDOW_HOOK(S,T) \
static __typeof__(&CreateWindowEx##S) create_##S; \
static NTW16S_UNWIND_FUNCTION HWND WINAPI window_##S(DWORD ex,T cls,T text,DWORD style,int x,int y,int cx,int cy,HWND parent,HMENU menu,HINSTANCE instance,void *param) \
{ ntw16s_unwind_scope unwind;title_scope scope;HWND result=NULL;DWORD error=ERROR_SUCCESS; \
  if(!begin_titles(&scope,FALSE))return NULL; \
  NTW16S_UNWIND_BEGIN(unwind,end_titles,&scope); { result=create_##S(ex,cls,text,style,x,y,cx,cy,parent,menu,instance,param);error=GetLastError(); \
  } NTW16S_UNWIND_END(unwind); \
  if(scope.failure){if(result)DestroyWindow(result);result=NULL;error=scope.failure;} \
  SetLastError(error);return result; }
WINDOW_HOOK(A,LPCSTR)
WINDOW_HOOK(W,LPCWSTR)

#define DIALOG_HOOK(NAME,TYPE,RET,MODAL,FAIL) \
static __typeof__(&NAME) real_##NAME; \
static NTW16S_UNWIND_FUNCTION RET WINAPI hook_##NAME(HINSTANCE instance,TYPE name,HWND parent,DLGPROC proc,LPARAM param) \
{ ntw16s_unwind_scope unwind;title_scope scope;RET result=FAIL;DWORD error=ERROR_SUCCESS; \
  if(!begin_titles(&scope,MODAL))return FAIL; \
  NTW16S_UNWIND_BEGIN(unwind,end_titles,&scope); { result=real_##NAME(instance,name,parent,proc,param);error=GetLastError(); \
  } NTW16S_UNWIND_END(unwind); \
  if(scope.failure){if(!(MODAL)){if(result)DestroyWindow((HWND)result);}result=FAIL;error=scope.failure;} \
  SetLastError(error);return result; }
DIALOG_HOOK(CreateDialogParamA,LPCSTR,HWND,FALSE,NULL)
DIALOG_HOOK(CreateDialogParamW,LPCWSTR,HWND,FALSE,NULL)
DIALOG_HOOK(CreateDialogIndirectParamA,LPCDLGTEMPLATEA,HWND,FALSE,NULL)
DIALOG_HOOK(CreateDialogIndirectParamW,LPCDLGTEMPLATEW,HWND,FALSE,NULL)
DIALOG_HOOK(DialogBoxParamA,LPCSTR,INT_PTR,TRUE,-1)
DIALOG_HOOK(DialogBoxParamW,LPCWSTR,INT_PTR,TRUE,-1)
DIALOG_HOOK(DialogBoxIndirectParamA,LPCDLGTEMPLATEA,INT_PTR,TRUE,-1)
DIALOG_HOOK(DialogBoxIndirectParamW,LPCDLGTEMPLATEW,INT_PTR,TRUE,-1)

/* MessageBox creates its own dialog inside USER rather than necessarily
 * calling an exported DialogBox entry. Keep its API/return contract and use
 * the same bounded creation scope as the other modal entry points. */
#define MESSAGE_HOOK(NAME,PARAMETERS,ARGUMENTS) \
static __typeof__(&NAME) real_##NAME; \
static NTW16S_UNWIND_FUNCTION int WINAPI hook_##NAME PARAMETERS \
{ ntw16s_unwind_scope unwind;title_scope scope;int result=0;DWORD error=ERROR_SUCCESS; \
  if(!begin_titles(&scope,TRUE))return 0; \
  NTW16S_UNWIND_BEGIN(unwind,end_titles,&scope); { result=real_##NAME ARGUMENTS;error=GetLastError(); \
  } NTW16S_UNWIND_END(unwind); \
  if(scope.failure){result=0;error=scope.failure;} SetLastError(error);return result; }
MESSAGE_HOOK(MessageBoxA,(HWND owner,LPCSTR text,LPCSTR caption,UINT type),(owner,text,caption,type))
MESSAGE_HOOK(MessageBoxW,(HWND owner,LPCWSTR text,LPCWSTR caption,UINT type),(owner,text,caption,type))
MESSAGE_HOOK(MessageBoxExA,(HWND owner,LPCSTR text,LPCSTR caption,UINT type,WORD language),(owner,text,caption,type,language))
MESSAGE_HOOK(MessageBoxExW,(HWND owner,LPCWSTR text,LPCWSTR caption,UINT type,WORD language),(owner,text,caption,type,language))
MESSAGE_HOOK(MessageBoxIndirectA,(const MSGBOXPARAMSA *parameters),(parameters))
MESSAGE_HOOK(MessageBoxIndirectW,(const MSGBOXPARAMSW *parameters),(parameters))
#undef MESSAGE_HOOK

/* The DLL uses the three original native leaves bound at load, so application
 * reentry is never accidentally bypassed by an ambient recursion guard. */
static __typeof__(&SetScrollPos) real_set_pos;
static __typeof__(&SetScrollRange) real_set_range;
static __typeof__(&ShowScrollBar) real_show_scroll;
static __typeof__(&EnableScrollBar) real_enable_scroll;
static HRESULT WINAPI hook_set_theme(HWND w,LPCWSTR application,LPCWSTR ids)
{
    if(!InterlockedCompareExchange(&style_ready,0,0))return real_set_theme(w,application,ids);
    return Ntw16sSetWindowTheme(w,application,ids);
}
static BOOL eligible_scroll(HWND w)
{
    DWORD saved=GetLastError();
    BOOL eligible=InterlockedCompareExchange(&style_ready,0,0) && Ntw16sWindowAttached(w);
    SetLastError(saved);return eligible;
}
static BOOL WINAPI hook_get_scroll(HWND w,int bar,LPSCROLLINFO info)
{
    if(!eligible_scroll(w))return real_get_scroll(w,bar,info);
    return win16_style_get_scroll_info(w,bar,info);
}
static int WINAPI hook_set_scroll(HWND w,int bar,LPCSCROLLINFO info,BOOL redraw)
{
    if(!eligible_scroll(w))return real_set_scroll(w,bar,info,redraw);
    return win16_style_set_scroll_info(w,bar,info,redraw);
}
static int WINAPI hook_set_pos(HWND w,int bar,int position,BOOL redraw)
{
    int result;DWORD error;
    if(!eligible_scroll(w))return real_set_pos(w,bar,position,redraw);
    result=real_set_pos(w,bar,position,FALSE);error=GetLastError();if(redraw)Ntw16sScrollChanged(w);
    SetLastError(error);return result;
}
static BOOL WINAPI hook_set_range(HWND w,int bar,int first,int last,BOOL redraw)
{
    BOOL result;DWORD error;
    if(!eligible_scroll(w))return real_set_range(w,bar,first,last,redraw);
    result=real_set_range(w,bar,first,last,FALSE);error=GetLastError();if(result && redraw)Ntw16sScrollChanged(w);
    SetLastError(error);return result;
}
static BOOL WINAPI hook_show_scroll(HWND w,int bar,BOOL show)
{
    BOOL result;DWORD error;
    if(!eligible_scroll(w))return real_show_scroll(w,bar,show);
    result=real_show_scroll(w,bar,show);error=GetLastError();if(result)Ntw16sScrollChanged(w);
    SetLastError(error);return result;
}
static BOOL WINAPI hook_enable_scroll(HWND w,UINT bar,UINT arrows)
{
    BOOL result;DWORD error;
    if(!eligible_scroll(w))return real_enable_scroll(w,bar,arrows);
    result=real_enable_scroll(w,bar,arrows);error=GetLastError();Ntw16sScrollChanged(w);
    SetLastError(error);return result;
}

/* Process-local native text leaves. Outside a verified VB6/report paint scope these
 * are transparent trampolines. The temporary font is restored before any
 * lease can be released; no application HFONT/property/layout is modified. */
static _Thread_local unsigned vb_text_busy;
static BOOL vb_text_caller(const void *address)
{
    HMODULE caller=NULL;
    if(!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            (const WCHAR *)address,&caller))return FALSE;
    return ntw16s_report_paint_caller(caller) || caller==GetModuleHandleW(L"MSVBVM60.DLL") || caller==GetModuleHandleW(L"USER32.DLL") ||
        caller==GetModuleHandleW(L"LPK.DLL") || caller==GetModuleHandleW(L"USP10.DLL") ||
        caller==GetModuleHandleW(L"GDI32FULL.DLL");
}
/* The report adapter promises only whole, reversible CP1252 runs. Keep
 * measurements and unsupported language/RTL runs on the native font. */
static BOOL report_wide_text(HDC dc,const WCHAR *text,int count)
{
    if(!ntw16s_report_paint_active())return TRUE;
    if(count<0 || count>4096 || (GetTextAlign(dc)&TA_RTLREADING))return FALSE;
    if(!count)return TRUE;
    BOOL used=FALSE;char buffer[4096];
    return text && WideCharToMultiByte(1252,WC_NO_BEST_FIT_CHARS,text,count,buffer,sizeof(buffer),NULL,&used)==count && !used;
}
static BOOL report_ansi_text(HDC dc,const char *text,int count)
{
    if(!ntw16s_report_paint_active())return TRUE;
    if(!text || (GetTextAlign(dc)&TA_RTLREADING))return FALSE;
    if(count==-1){count=0;while(count<=4096 && text[count])++count;}
    if(count<0 || count>4096)return FALSE;
    if(GetACP()!=1252)for(int i=0;i<count;++i)if((unsigned char)text[i]>=128)return FALSE;
    return TRUE;
}
#define VB_TEXT_HOOK(NAME,TYPE,PARAMS,ARGS,ALLOW) \
static __typeof__(&NAME) real_##NAME; \
static TYPE WINAPI hook_##NAME PARAMS \
{ HFONT font=NULL,old=NULL;DWORD entry=GetLastError(),error;TYPE result; \
  if(!vb_text_busy++ && (ALLOW) && ntw16s_vb_paint_active() && vb_text_caller(__builtin_return_address(0))){font=ntw16s_vb_paint_font(dc);if(font){old=SelectObject(dc,font);if(old==HGDI_ERROR)old=NULL;}} \
  SetLastError(entry);result=real_##NAME ARGS;error=GetLastError(); \
  if(old){SelectObject(dc,old);}--vb_text_busy;SetLastError(error);return result; }
VB_TEXT_HOOK(DrawTextA,int,(HDC dc,LPCSTR text,int count,LPRECT rect,UINT format),(dc,text,count,rect,format),(!ntw16s_report_paint_active() || !(format&DT_CALCRECT)) && report_ansi_text(dc,text,count))
VB_TEXT_HOOK(DrawTextExA,int,(HDC dc,LPSTR text,int count,LPRECT rect,UINT format,LPDRAWTEXTPARAMS params),(dc,text,count,rect,format,params),(!ntw16s_report_paint_active() || !(format&DT_CALCRECT)) && report_ansi_text(dc,text,count))
VB_TEXT_HOOK(TextOutA,BOOL,(HDC dc,int x,int y,LPCSTR text,int count),(dc,x,y,text,count),report_ansi_text(dc,text,count))
VB_TEXT_HOOK(ExtTextOutA,BOOL,(HDC dc,int x,int y,UINT options,const RECT *rect,LPCSTR text,UINT count,const INT *dx),(dc,x,y,options,rect,text,count,dx),(!ntw16s_report_paint_active() || !(options&(ETO_GLYPH_INDEX|ETO_RTLREADING))) && report_ansi_text(dc,text,(int)count))
VB_TEXT_HOOK(TextOutW,BOOL,(HDC dc,int x,int y,LPCWSTR text,int count),(dc,x,y,text,count),report_wide_text(dc,text,count))

#undef VB_TEXT_HOOK

/* USER's EDIT can submit shaped glyph IDs. Those IDs belong to its source
 * font, not the replacement FNT. Reverse only verified CP1252 glyphs; any
 * missing/complex glyph leaves the whole native run unchanged. Keep its dx,
 * ETO_PDY, clipping and visual order, and do not reshape the recovered run. */
static __typeof__(&ExtTextOutW) real_ExtTextOutW;
static BOOL WINAPI hook_ExtTextOutW(HDC dc,int x,int y,UINT options,const RECT *rect,LPCWSTR text,UINT count,const INT *dx)
{
    DWORD entry=GetLastError(),error;HFONT font=NULL,old=NULL;WCHAR *decoded=NULL;char *encoded=NULL;BOOL result;LPCWSTR original_text=text;UINT original_options=options;
    if(!vb_text_busy++ && ntw16s_vb_paint_active() && vb_text_caller(__builtin_return_address(0))){
        font=ntw16s_vb_paint_font(dc);
        if(font && (options&ETO_GLYPH_INDEX)){
            BOOL valid=count<=4096 && !(options&ETO_RTLREADING) && !(GetTextAlign(dc)&TA_RTLREADING);
            if(valid && count)decoded=HeapAlloc(GetProcessHeap(),0,count*sizeof(WCHAR));
            if(count && !decoded)valid=FALSE;
            if(valid)valid=ntw16s_vb_decode_glyphs(dc,text,count,decoded);
            if(valid && count){
                BOOL used_default=FALSE;encoded=HeapAlloc(GetProcessHeap(),0,count);
                if(!encoded || WideCharToMultiByte(1252,WC_NO_BEST_FIT_CHARS,decoded,(int)count,encoded,(int)count,NULL,&used_default)!=(int)count || used_default)valid=FALSE;
            }
            if(valid){text=decoded;options&=~ETO_GLYPH_INDEX;}
            else font=NULL;
        }
        if(font && !(options&ETO_GLYPH_INDEX) &&
            (!report_wide_text(dc,text,(int)count) || (ntw16s_report_paint_active() && (options&ETO_RTLREADING))))font=NULL;
        if(font)old=SelectObject(dc,font);
        if(!old || old==HGDI_ERROR){
            old=NULL;text=original_text;options=original_options;
            if(encoded){HeapFree(GetProcessHeap(),0,encoded);encoded=NULL;}
        }
    }
    SetLastError(entry);
    if(old && encoded)result=real_ExtTextOutA(dc,x,y,options,rect,encoded,count,dx);
    else result=real_ExtTextOutW(dc,x,y,options,rect,text,count,dx);
    error=GetLastError();
    if(old)SelectObject(dc,old);
    if(decoded)HeapFree(GetProcessHeap(),0,decoded);
    if(encoded)HeapFree(GetProcessHeap(),0,encoded);
    --vb_text_busy;SetLastError(error);return result;
}

static __typeof__(&DrawFrameControl) real_DrawFrameControl;
static __typeof__(&DrawThemeBackground) real_DrawThemeBackground;
static __typeof__(&CreateCompatibleDC) real_CreateCompatibleDC;
static __typeof__(&DeleteDC) real_DeleteDC;
static BOOL WINAPI hook_DrawFrameControl(HDC dc,LPRECT rect,UINT type,UINT state)
{
    DWORD entry=GetLastError();BOOL drawn=ntw16s_updown_draw_frame(dc,rect,type,state,__builtin_return_address(0));
    SetLastError(entry);return drawn?TRUE:real_DrawFrameControl(dc,rect,type,state);
}
static HRESULT WINAPI hook_DrawThemeBackground(HTHEME theme,HDC dc,int part,int state,const RECT *rect,const RECT *clip)
{
    DWORD entry=GetLastError();BOOL drawn=ntw16s_updown_draw_theme(theme,dc,part,state,rect,clip,__builtin_return_address(0));
    SetLastError(entry);return drawn?S_OK:real_DrawThemeBackground(theme,dc,part,state,rect,clip);
}
static HDC WINAPI hook_CreateCompatibleDC(HDC source)
{
    HDC result=real_CreateCompatibleDC(source);DWORD error=GetLastError();
    ntw16s_updown_dc_created(source,result,__builtin_return_address(0));SetLastError(error);return result;
}
static BOOL WINAPI hook_DeleteDC(HDC dc)
{
    BOOL result=real_DeleteDC(dc);DWORD error=GetLastError();if(result)ntw16s_updown_dc_deleted(dc);
    SetLastError(error);return result;
}

#define DEFAULT_WINDOW_HOOK(S) \
static __typeof__(&DefWindowProc##S) real_DefWindowProc##S; \
static LRESULT WINAPI hook_DefWindowProc##S(HWND w,UINT msg,WPARAM wp,LPARAM lp) \
{ DWORD entry=GetLastError(); \
  if(msg==WM_SYSCOMMAND && win16_style_default_move(w,wp,lp)){SetLastError(entry);return 0;} \
  SetLastError(entry);return real_DefWindowProc##S(w,msg,wp,lp); }
DEFAULT_WINDOW_HOOK(A)
DEFAULT_WINDOW_HOOK(W)

LONG ntw16s_attach_titles(void)
{
    LONG error=NO_ERROR;
    if(!(ntw16s_process_context.flags&NTW16S_POLICY_STYLE))return error;
    create_A=CreateWindowExA;create_W=CreateWindowExW;
    real_get_scroll=GetScrollInfo;real_set_scroll=SetScrollInfo;real_set_theme=SetWindowTheme;
    real_set_pos=SetScrollPos;real_set_range=SetScrollRange;
    real_show_scroll=ShowScrollBar;real_enable_scroll=EnableScrollBar;
    error=(LONG)ntw16s_hooks_attach(&create_A,(void *)window_A);
    if(!error)error=(LONG)ntw16s_hooks_attach(&create_W,(void *)window_W);
    if(!error)error=(LONG)ntw16s_hooks_attach(&real_get_scroll,(void *)hook_get_scroll);
    if(!error)error=(LONG)ntw16s_hooks_attach(&real_set_scroll,(void *)hook_set_scroll);
    if(!error)error=(LONG)ntw16s_hooks_attach(&real_set_pos,(void *)hook_set_pos);
    if(!error)error=(LONG)ntw16s_hooks_attach(&real_set_range,(void *)hook_set_range);
    if(!error)error=(LONG)ntw16s_hooks_attach(&real_show_scroll,(void *)hook_show_scroll);
    if(!error)error=(LONG)ntw16s_hooks_attach(&real_enable_scroll,(void *)hook_enable_scroll);
    if(!error)error=(LONG)ntw16s_hooks_attach(&real_set_theme,(void *)hook_set_theme);
#define ATTACH(NAME) do { real_##NAME=NAME; if(!error)error=(LONG)ntw16s_hooks_attach(&real_##NAME,(void *)hook_##NAME); } while(0)
    ATTACH(CreateDialogParamA);ATTACH(CreateDialogParamW);
    ATTACH(CreateDialogIndirectParamA);ATTACH(CreateDialogIndirectParamW);
    ATTACH(DialogBoxParamA);ATTACH(DialogBoxParamW);
    ATTACH(DialogBoxIndirectParamA);ATTACH(DialogBoxIndirectParamW);
    ATTACH(MessageBoxA);ATTACH(MessageBoxW);
    ATTACH(MessageBoxExA);ATTACH(MessageBoxExW);
    ATTACH(MessageBoxIndirectA);ATTACH(MessageBoxIndirectW);
    ATTACH(DrawTextA);ATTACH(DrawTextExA);ATTACH(TextOutA);ATTACH(ExtTextOutA);
    ATTACH(TextOutW);ATTACH(ExtTextOutW);
    ATTACH(DrawFrameControl);ATTACH(DrawThemeBackground);ATTACH(CreateCompatibleDC);ATTACH(DeleteDC);
    ATTACH(DefWindowProcA);ATTACH(DefWindowProcW);
#undef ATTACH
    return error;
}
