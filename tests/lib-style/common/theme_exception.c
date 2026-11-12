#include <windows.h>
#include <stdio.h>
#include <setjmp.h>
#include <string.h>
#include "lib-style/common/theme_state.h"
#include "lib-win32/process-hook/unwind.h"
#define TEST_EXCEPTION 0xe0424242u

static HWND window;
static int owner;
static unsigned calls;
static HRESULT raise_theme(HWND w,LPCWSTR application,LPCWSTR ids)
{(void)w;(void)application;(void)ids;RaiseException(TEST_EXCEPTION,0,0,NULL);return E_FAIL;}
static HRESULT observe(HWND w,LPCWSTR application,LPCWSTR ids)
{(void)w;(void)application;(void)ids;++calls;return S_OK;}
static void raise_intercept(void)
{(void)style_theme_intercept(window,L"Explorer",NULL,raise_theme);}
static void raise_suppress(void)
{(void)style_theme_suppress(window,&owner,raise_theme);}
static void raise_remove(void)
{style_theme_remove(window,&owner,raise_theme);}
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
static PROCESS_HOOK_UNWIND_FUNCTION void catch_exception(void (*callback)(void))
{
#if !defined(_WIN64)
    catcher c;c.record.handler=catch32;
    __asm__ __volatile__("movl %%fs:0,%0":"=r"(c.record.previous)::"memory");
    __asm__ __volatile__("movl %0,%%fs:0"::"r"(&c.record):"memory");
    if(!setjmp(c.saved))callback();
    __asm__ __volatile__("movl %0,%%fs:0"::"r"(c.record.previous):"memory");
#else
    __asm__ __volatile__(".Ltheme_catch_start:\n.seh_handler __C_specific_handler, @except\n"
        ".seh_handlerdata\n.long 1\n.rva .Ltheme_catch_start,.Ltheme_catch_end,filter,.Ltheme_catch_end\n.text\n":::"memory");
    callback();
    __asm__ __volatile__("nop\n.Ltheme_catch_end: nop\n":::"memory");
#endif
}
int main(int argc,char **argv)
{
    window=CreateWindowW(L"STATIC",L"",WS_POPUP,0,0,40,40,NULL,NULL,GetModuleHandleW(NULL),NULL);
    if(!window || !style_theme_capture(window))return 1;
    if(argc==2 && (!strcmp(argv[1],"--expect-v6") || !strcmp(argv[1],"--expect-v5"))){
        typedef struct {DWORD size,major,minor,build,platform;} version_info;
        HMODULE controls=GetModuleHandleW(L"comctl32.dll");
        HRESULT (CALLBACK *version)(version_info *)=controls?(void *)GetProcAddress(controls,"DllGetVersion"):NULL;
        version_info info={.size=sizeof(info)};
        DWORD expected=!strcmp(argv[1],"--expect-v6")?6:5;
        if(!version || FAILED(version(&info)) || info.major!=expected)return 3;
        printf("THEME COMCTL VERSION bits=%u major=%lu\n",(unsigned)(sizeof(void *)*8),(unsigned long)info.major);
    }
    catch_exception(raise_intercept);
    BOOL after_intercept=SUCCEEDED(style_theme_intercept(window,L"Explorer",NULL,observe)) && calls==1;
    catch_exception(raise_suppress);
    BOOL after_suppress=!style_theme_retained(window) && style_theme_terminal(window,NULL) &&
        style_theme_capture(window) && style_theme_suppress(window,&owner,observe);
    catch_exception(raise_remove);
    BOOL removed=!style_theme_retained(window) && style_theme_capture(window) &&
        style_theme_suppress(window,&owner,observe);
    style_theme_remove(window,&owner,observe);
    removed=removed && !style_theme_retained(window);
    DestroyWindow(window);
    printf("THEME EXCEPTION bits=%u intercept=%d suppress=%d removed=%d\n",
           (unsigned)(sizeof(void *)*8),after_intercept,after_suppress,removed);
    return after_intercept && after_suppress && removed?0:2;
}
