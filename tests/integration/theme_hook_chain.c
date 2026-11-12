#include <windows.h>
#include <uxtheme.h>
#include <stdio.h>
#include <setjmp.h>
#include "lib-win32/process-hook/startup_hooks.h"
#include "lib-win32/process-hook/late_batch.h"
#include "lib-win32/process-hook/unwind.h"
#include "lib-style/common/theme_state.h"
#define TEST_EXCEPTION 0xe0424242u

static __typeof__(&SetWindowTheme) first_original,second_original;
static unsigned first_calls,second_calls;
static BOOL request_nested,request_exception;
static HWND exception_window;
static WCHAR restored[64];
static HRESULT first_native(HWND window,LPCWSTR application,LPCWSTR ids)
{
    if(request_exception){request_exception=FALSE;RaiseException(TEST_EXCEPTION,0,0,NULL);}
    if(request_nested){request_nested=FALSE;SetWindowTheme(window,L"Nested",NULL);}
    return first_original(window,application,ids);
}
static HRESULT second_native(HWND window,LPCWSTR application,LPCWSTR ids)
{return second_original(window,application,ids);}
static HRESULT observe(HWND window,LPCWSTR application,LPCWSTR ids)
{(void)window;(void)ids;lstrcpynW(restored,application?application:L"<null>",ARRAYSIZE(restored));return S_OK;}
static HRESULT WINAPI first_hook(HWND window,LPCWSTR application,LPCWSTR ids)
{++first_calls;return style_theme_intercept(window,application,ids,first_native);}
static HRESULT WINAPI second_hook(HWND window,LPCWSTR application,LPCWSTR ids)
{++second_calls;return style_theme_intercept(window,application,ids,second_native);}
#if !defined(_WIN64)
typedef struct catcher {process_hook_unwind_record record;jmp_buf saved;} catcher;
static EXCEPTION_DISPOSITION NTAPI catch32(EXCEPTION_RECORD *exception,void *frame,CONTEXT *context,void *dispatcher)
{
    (void)context;(void)dispatcher;
    if(!(exception->ExceptionFlags&(EXCEPTION_UNWINDING|EXCEPTION_EXIT_UNWIND)) && exception->ExceptionCode==TEST_EXCEPTION){
        RtlUnwind(frame,NULL,exception,NULL);longjmp(((catcher *)frame)->saved,1);
    }
    return ExceptionContinueSearch;
}
#else
static LONG __attribute__((used)) filter(EXCEPTION_POINTERS *exception,void *frame)
{(void)frame;return exception->ExceptionRecord->ExceptionCode==TEST_EXCEPTION?EXCEPTION_EXECUTE_HANDLER:EXCEPTION_CONTINUE_SEARCH;}
#endif
static PROCESS_HOOK_UNWIND_FUNCTION void catch_theme_exception(void)
{
#if !defined(_WIN64)
    catcher c;c.record.handler=catch32;
    __asm__ __volatile__("movl %%fs:0,%0":"=r"(c.record.previous)::"memory");
    __asm__ __volatile__("movl %0,%%fs:0"::"r"(&c.record):"memory");
    if(!setjmp(c.saved))SetWindowTheme(exception_window,L"Raise",NULL);
    __asm__ __volatile__("movl %0,%%fs:0"::"r"(c.record.previous):"memory");
#else
    __asm__ __volatile__(".Lchain_catch_start:\n.seh_handler __C_specific_handler, @except\n"
        ".seh_handlerdata\n.long 1\n.rva .Lchain_catch_start,.Lchain_catch_end,filter,.Lchain_catch_end\n.text\n":::"memory");
    SetWindowTheme(exception_window,L"Raise",NULL);
    __asm__ __volatile__("nop\n.Lchain_catch_end: nop\n":::"memory");
#endif
}
int main(void)
{
    first_original=SetWindowTheme;
    DWORD error=process_hook_startup_begin();
    if(!error)error=process_hook_startup_attach(&first_original,(void *)first_hook);
    if(!error)error=process_hook_startup_commit();else process_hook_startup_abort();
    if(error){printf("THEME CHAIN startup=%lu\n",error);return 1;}
    process_hook_late_patch patch[1]={0};process_hook_late_patch *items[1]={0};
    process_hook_late_batch batch={0};second_original=SetWindowTheme;
    error=process_hook_late_batch_begin(&batch,patch,items,1);
    if(!error)error=process_hook_late_batch_prepare(&batch,&second_original,(void *)second_hook);
    error=process_hook_late_batch_commit(&batch,error,100,1);
    if(error){printf("THEME CHAIN late=%lu\n",error);return 2;}
    HWND window=CreateWindowW(L"STATIC",L"",WS_POPUP,0,0,40,40,NULL,NULL,GetModuleHandleW(NULL),NULL);
    int owner=0;
    BOOL captured=window && style_theme_capture(window) && style_theme_suppress(window,&owner,observe);
    first_calls=second_calls=0;
    HRESULT result=captured?SetWindowTheme(window,L"Explorer",NULL):E_FAIL;
    if(captured)style_theme_remove(window,&owner,observe);
    BOOL preserved=!lstrcmpW(restored,L"Explorer");
    BOOL ok=SUCCEEDED(result) && first_calls==1 && second_calls==1 && preserved;
    BOOL nested=window && style_theme_capture(window) && style_theme_suppress(window,&owner,observe);
    if(nested){request_nested=TRUE;nested=SUCCEEDED(SetWindowTheme(window,L"Explorer",NULL));
        style_theme_remove(window,&owner,observe);nested=nested && !lstrcmpW(restored,L"Nested");}
    BOOL exception=window && style_theme_capture(window) && style_theme_suppress(window,&owner,observe);
    if(exception){exception_window=window;request_exception=TRUE;catch_theme_exception();
        exception=!request_exception && SUCCEEDED(SetWindowTheme(window,L"After",NULL));
        style_theme_remove(window,&owner,observe);exception=exception && !lstrcmpW(restored,L"After");}
    ok=ok && nested && exception;
    printf("THEME CHAIN bits=%u startup=%u late=%u result=%08lx preserved=%d nested=%d exception=%d\n",
           (unsigned)(sizeof(void *)*8),first_calls,second_calls,(unsigned long)result,preserved,nested,exception);
    if(window)DestroyWindow(window);
    return ok?0:3;
}
