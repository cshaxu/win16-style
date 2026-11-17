#include "runtime_text_imports.h"
#include "runtime_text.h"
#include "../../graphics/font_scope.h"
#include "lib-win32/module-import/import_scan.h"

static INIT_ONCE originals_once=INIT_ONCE_STATIC_INIT;
static int (WINAPI *original_draw)(HDC,LPCSTR,int,LPRECT,UINT);
static int (WINAPI *original_draw_ex)(HDC,LPSTR,int,LPRECT,UINT,LPDRAWTEXTPARAMS);
static BOOL (WINAPI *original_text)(HDC,int,int,LPCSTR,int);
static BOOL (WINAPI *original_text_ex)(HDC,int,int,UINT,const RECT *,LPCSTR,UINT,const INT *);
static _Thread_local style_font_gate text_gate;
static style_text_font_provider font_provider;

static BOOL CALLBACK originals_init(PINIT_ONCE once,void *parameter,void **context)
{
    HMODULE user=GetModuleHandleW(L"user32.dll"),gdi=GetModuleHandleW(L"gdi32.dll");
    (void)once;(void)context;
    font_provider=*(style_text_font_provider *)parameter;
    if(!user || !gdi)return FALSE;
    original_draw=(void *)GetProcAddress(user,"DrawTextA");
    original_draw_ex=(void *)GetProcAddress(user,"DrawTextExA");
    original_text=(void *)GetProcAddress(gdi,"TextOutA");
    original_text_ex=(void *)GetProcAddress(gdi,"ExtTextOutA");
    return original_draw && original_draw_ex && original_text && original_text_ex;
}

#define TEXT_HOOK(name,type,parameters,arguments) \
static type WINAPI global_##name parameters \
{ \
    DWORD entry=GetLastError();style_font_scope scope; \
    if(style_font_gate_begin(&text_gate,&scope,dc) && style_runtime_text_direct_allowed() && style_runtime_text_active())style_font_scope_select(&scope,font_provider(dc)); \
    SetLastError(entry); \
    type result=original_##name arguments;DWORD error=GetLastError(); \
    style_font_scope_end(&scope);style_font_gate_end(&text_gate);SetLastError(error);return result; \
}
TEXT_HOOK(draw,int,(HDC dc,LPCSTR text,int count,LPRECT rect,UINT format),(dc,text,count,rect,format))
TEXT_HOOK(draw_ex,int,(HDC dc,LPSTR text,int count,LPRECT rect,UINT format,LPDRAWTEXTPARAMS params),(dc,text,count,rect,format,params))
TEXT_HOOK(text,BOOL,(HDC dc,int x,int y,LPCSTR text,int count),(dc,x,y,text,count))
TEXT_HOOK(text_ex,BOOL,(HDC dc,int x,int y,UINT options,const RECT *rect,LPCSTR text,UINT count,const INT *dx),(dc,x,y,options,rect,text,count,dx))
#undef TEXT_HOOK

DWORD style_runtime_text_imports(HMODULE runtime,style_text_font_provider font,BOOL *changed)
{
    static const win32_module_import_query queries[]={
        {"USER32.dll","DrawTextA"},{"USER32.dll","DrawTextExA"},
        {"GDI32.dll","TextOutA"},{"GDI32.dll","ExtTextOutA"}};
    DWORD error;void **slots[ARRAYSIZE(queries)]={0};unsigned count=0;
    if(changed)*changed=FALSE;
    if(!runtime || !font || !changed)return ERROR_INVALID_PARAMETER;
    if(!InitOnceExecuteOnce(&originals_once,originals_init,&font,NULL))return ERROR_PROC_NOT_FOUND;
    error=win32_module_import_slots(runtime,queries,ARRAYSIZE(queries),slots);if(error)return error;
    win32_module_import_change changes[]={
        {slots[0],(void *)original_draw,(void *)global_draw},
        {slots[1],(void *)original_draw_ex,(void *)global_draw_ex},
        {slots[2],(void *)original_text,(void *)global_text},
        {slots[3],(void *)original_text_ex,(void *)global_text_ex}};
    error=win32_module_import_apply(runtime,changes,ARRAYSIZE(changes),&count);
    if(!error)*changed=count!=0;
    return error;
}
