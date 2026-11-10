#include "ntw16s-dll/global/vb_text.h"
#include "lib-win32/module-import/import_scan.h"
#include "lib-win32/win16-control/vb_text.h"

static INIT_ONCE originals_once=INIT_ONCE_STATIC_INIT;
static int (WINAPI *original_draw)(HDC,LPCSTR,int,LPRECT,UINT);
static int (WINAPI *original_draw_ex)(HDC,LPSTR,int,LPRECT,UINT,LPDRAWTEXTPARAMS);
static BOOL (WINAPI *original_text)(HDC,int,int,LPCSTR,int);
static BOOL (WINAPI *original_text_ex)(HDC,int,int,UINT,const RECT *,LPCSTR,UINT,const INT *);
static _Thread_local unsigned text_depth;

static BOOL CALLBACK originals_init(PINIT_ONCE once,void *parameter,void **context)
{
    HMODULE user=GetModuleHandleW(L"user32.dll"),gdi=GetModuleHandleW(L"gdi32.dll");
    (void)once;(void)parameter;(void)context;
    if(!user || !gdi)return FALSE;
    original_draw=(void *)GetProcAddress(user,"DrawTextA");
    original_draw_ex=(void *)GetProcAddress(user,"DrawTextExA");
    original_text=(void *)GetProcAddress(gdi,"TextOutA");
    original_text_ex=(void *)GetProcAddress(gdi,"ExtTextOutA");
    return original_draw && original_draw_ex && original_text && original_text_ex;
}

static HGDIOBJ begin_text(HDC dc)
{
    HFONT font=NULL;
    if(!text_depth++ && ntw16s_vb_direct_text_allowed() && ntw16s_vb_paint_active())font=ntw16s_vb_paint_font(dc);
    HGDIOBJ previous=font?SelectObject(dc,font):NULL;
    return previous==HGDI_ERROR?NULL:previous;
}
static void end_text(HDC dc,HGDIOBJ previous)
{if(previous)SelectObject(dc,previous);--text_depth;}

#define TEXT_HOOK(name,type,parameters,arguments) \
static type WINAPI global_##name parameters \
{ \
    DWORD entry=GetLastError();HGDIOBJ old=begin_text(dc);SetLastError(entry); \
    type result=original_##name arguments;DWORD error=GetLastError(); \
    end_text(dc,old);SetLastError(error);return result; \
}
TEXT_HOOK(draw,int,(HDC dc,LPCSTR text,int count,LPRECT rect,UINT format),(dc,text,count,rect,format))
TEXT_HOOK(draw_ex,int,(HDC dc,LPSTR text,int count,LPRECT rect,UINT format,LPDRAWTEXTPARAMS params),(dc,text,count,rect,format,params))
TEXT_HOOK(text,BOOL,(HDC dc,int x,int y,LPCSTR text,int count),(dc,x,y,text,count))
TEXT_HOOK(text_ex,BOOL,(HDC dc,int x,int y,UINT options,const RECT *rect,LPCSTR text,UINT count,const INT *dx),(dc,x,y,options,rect,text,count,dx))
#undef TEXT_HOOK

DWORD ntw16s_global_vb_text(HWND window,BOOL *changed)
{
    static const win32_module_import_query queries[]={
        {"USER32.dll","DrawTextA"},{"USER32.dll","DrawTextExA"},
        {"GDI32.dll","TextOutA"},{"GDI32.dll","ExtTextOutA"}};
    HMODULE runtime=NULL;DWORD process=0,error=ERROR_NOT_SUPPORTED;WCHAR name[64];
    void **slots[ARRAYSIZE(queries)]={0};unsigned count=0;
    if(changed)*changed=FALSE;
    if(!window || !changed)return ERROR_INVALID_PARAMETER;
    if(GetWindowThreadProcessId(window,&process)!=GetCurrentThreadId() || process!=GetCurrentProcessId() ||
       !GetClassNameW(window,name,ARRAYSIZE(name)) || lstrcmpW(name,L"ThunderRT6FormDC"))return ERROR_NOT_SUPPORTED;
    if(!GetModuleHandleExW(0,L"MSVBVM60.DLL",&runtime))return ERROR_NOT_SUPPORTED;
    if((HMODULE)GetClassLongPtrW(window,GCLP_HMODULE)!=runtime)goto done;
    if(!InitOnceExecuteOnce(&originals_once,originals_init,NULL,NULL)){error=ERROR_PROC_NOT_FOUND;goto done;}
    error=win32_module_import_slots(runtime,queries,ARRAYSIZE(queries),slots);if(error)goto done;
    win32_module_import_change changes[]={
        {slots[0],(void *)original_draw,(void *)global_draw},
        {slots[1],(void *)original_draw_ex,(void *)global_draw_ex},
        {slots[2],(void *)original_text,(void *)global_text},
        {slots[3],(void *)original_text_ex,(void *)global_text_ex}};
    error=win32_module_import_apply(runtime,changes,ARRAYSIZE(changes),&count);
    if(!error)*changed=count!=0;
done:
    FreeLibrary(runtime);return error;
}
