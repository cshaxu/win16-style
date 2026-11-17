#include "lib-win32-style/application/lifecycle/creation_call.h"
#include "lib-win32/process-hook/unwind.h"
#include <stdio.h>
#include <setjmp.h>
#define TEST_EXCEPTION 0xe0424251u
static BOOL active,fail_prepare=TRUE,raise_call,caught;
static DWORD install_error;
static unsigned prepares,calls;
static HWND last_window;
static BOOL is_active(void){return active;}
static BOOL CALLBACK prepare(PINIT_ONCE once,void *parameter,void **result)
{(void)once;(void)parameter;(void)result;++prepares;if(fail_prepare){SetLastError(ERROR_ACCESS_DENIED);return FALSE;}return TRUE;}
static BOOL creation_prepare(HWND window,DWORD style){(void)window;(void)style;return TRUE;}
static BOOL install(HWND window,DWORD style)
{(void)style;if(install_error){SetLastError(install_error);return FALSE;}return SetPropW(window,L"Style.Test.CreationCall",(HANDLE)(UINT_PTR)1);}
static BOOL attached(HWND window){return GetPropW(window,L"Style.Test.CreationCall")!=NULL;}
static void discard(HWND window){(void)window;}
static const style_creation_ops operations={creation_prepare,install,attached,discard,discard,FALSE};
static style_creation_context context={.once=INIT_ONCE_STATIC_INIT,.prepare=prepare,.active=is_active,.operations=&operations};
static const WCHAR class_name[]=L"StyleCreationCallTest";
static HWND create(void)
{return CreateWindowW(class_name,L"scope",WS_OVERLAPPEDWINDOW,0,0,80,60,NULL,NULL,GetModuleHandleW(NULL),NULL);}
static HRESULT invoke(void *value)
{
    (void)value;++calls;last_window=create();if(!last_window)return E_FAIL;
    if(raise_call)RaiseException(TEST_EXCEPTION,0,0,NULL);
    return S_FALSE;
}
#ifndef _WIN64
typedef struct catcher {process_hook_unwind_record record;jmp_buf saved;} catcher;
static EXCEPTION_DISPOSITION NTAPI catch32(EXCEPTION_RECORD *exception,void *frame,CONTEXT *registers,void *dispatcher)
{
    (void)registers;(void)dispatcher;
    if(!(exception->ExceptionFlags&(EXCEPTION_UNWINDING|EXCEPTION_EXIT_UNWIND)) && exception->ExceptionCode==TEST_EXCEPTION){
        caught=TRUE;RtlUnwind(frame,NULL,exception,NULL);longjmp(((catcher *)frame)->saved,1);
    }
    return ExceptionContinueSearch;
}
#else
static LONG __attribute__((used)) filter(EXCEPTION_POINTERS *exception,void *frame)
{
    (void)frame;if(exception->ExceptionRecord->ExceptionCode!=TEST_EXCEPTION)return EXCEPTION_CONTINUE_SEARCH;
    caught=TRUE;return EXCEPTION_EXECUTE_HANDLER;
}
#endif
static PROCESS_HOOK_UNWIND_FUNCTION void catch_call(void)
{
#ifndef _WIN64
    catcher c;c.record.handler=catch32;
    __asm__ __volatile__("movl %%fs:0,%0":"=r"(c.record.previous)::"memory");
    __asm__ __volatile__("movl %0,%%fs:0"::"r"(&c.record):"memory");
    if(!setjmp(c.saved))style_creation_call_invoke(&context,invoke,NULL);
    __asm__ __volatile__("movl %0,%%fs:0"::"r"(c.record.previous):"memory");
#else
    __asm__ __volatile__(".Lcall_catch_start:\n.seh_handler __C_specific_handler, @except\n"
        ".seh_handlerdata\n.long 1\n.rva .Lcall_catch_start,.Lcall_catch_end,filter,.Lcall_catch_end\n.text\n":::"memory");
    style_creation_call_invoke(&context,invoke,NULL);
    __asm__ __volatile__("nop\n.Lcall_catch_end: nop\n":::"memory");
#endif
}
#define CHECK(x) do{if(!(x)){fprintf(stderr,"CREATION_CALL_FAIL line=%d error=%lu\n",__LINE__,GetLastError());return 1;}}while(0)
int main(void)
{
    WNDCLASSW cls={.lpfnWndProc=DefWindowProcW,.hInstance=GetModuleHandleW(NULL),.lpszClassName=class_name};CHECK(RegisterClassW(&cls));
    CHECK(style_creation_call_invoke(&context,NULL,NULL)==E_INVALIDARG && !prepares && !calls);
    style_creation_call state;CHECK(style_creation_call_begin(&state,FALSE,&context) && !state.enabled && !state.creation);
    style_creation_call_end(&state);CHECK(!state.failure && !prepares);
    CHECK(style_creation_call_invoke(&context,invoke,NULL)==S_FALSE && !prepares && calls==1 && !attached(last_window));CHECK(DestroyWindow(last_window));
    active=TRUE;
    CHECK(style_creation_call_invoke(&context,invoke,NULL)==HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED) && prepares==1 && calls==1);
    fail_prepare=FALSE;
    CHECK(style_creation_call_invoke(&context,invoke,NULL)==S_FALSE && prepares==2 && calls==2 && attached(last_window));CHECK(DestroyWindow(last_window));
    CHECK(style_creation_call_invoke(&context,invoke,NULL)==S_FALSE && prepares==2 && calls==3 && attached(last_window));CHECK(DestroyWindow(last_window));
    install_error=ERROR_BAD_FORMAT;
    CHECK(style_creation_call_invoke(&context,invoke,NULL)==HRESULT_FROM_WIN32(ERROR_BAD_FORMAT) && prepares==2 && !attached(last_window));CHECK(DestroyWindow(last_window));
    install_error=0;raise_call=TRUE;catch_call();raise_call=FALSE;
    CHECK(caught && prepares==2 && attached(last_window));CHECK(DestroyWindow(last_window));
    HWND outside=create();CHECK(outside && !attached(outside) && DestroyWindow(outside));
    CHECK(style_creation_call_invoke(&context,invoke,NULL)==S_FALSE && prepares==2 && attached(last_window));CHECK(DestroyWindow(last_window));
    CHECK(UnregisterClassW(class_name,cls.hInstance));
    printf("CREATION_CALL_OK bits=%u inactive=1 prepare_retry=1 once=1 native_result=1 failure_priority=1 exception_cleanup=1 reuse=1\n",(unsigned)(sizeof(void *)*8));return 0;
}
