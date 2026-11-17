/* Exercise the production ownership root with real current-thread hooks;
 * the one leaf failure is injected without changing system policy. */
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include "lib-win32/process-hook/unwind.h"
#include "lib-win32-style/application/dialog/observation.h"
#define TEST_EXCEPTION 0xe0424253u
static unsigned live,observed;
static BOOL fail_hook,session_active,fail_prepare,caught;
static HHOOK WINAPI tracked_hook(int type,HOOKPROC callback,HINSTANCE module,DWORD thread)
{
    if(fail_hook){fail_hook=FALSE;SetLastError(ERROR_ACCESS_DENIED);return NULL;}
    HHOOK hook=SetWindowsHookExW(type,callback,module,thread);if(hook)++live;return hook;
}
static BOOL WINAPI tracked_unhook(HHOOK hook)
{BOOL result=UnhookWindowsHookEx(hook);if(result){if(!live)abort();--live;}return result;}
#define SetWindowsHookExW tracked_hook
#define UnhookWindowsHookEx tracked_unhook
#include "lib-win32-style/application/dialog/observation.c"
#undef SetWindowsHookExW
#undef UnhookWindowsHookEx
static BOOL enabled(void){return session_active;}
static BOOL CALLBACK prepare(PINIT_ONCE once,void *parameter,void **result)
{(void)once;(void)parameter;(void)result;if(fail_prepare){SetLastError(ERROR_BAD_FORMAT);return FALSE;}return TRUE;}
static BOOL allow(HWND window,DWORD style){(void)window;(void)style;return TRUE;}
static BOOL attached(HWND window){(void)window;return FALSE;}
static void ignore(HWND window){(void)window;}
static const style_creation_ops operations={allow,allow,attached,ignore,ignore,FALSE};
static style_creation_context context={.once=INIT_ONCE_STATIC_INIT,.prepare=prepare,.active=enabled,.operations=&operations};
static LRESULT CALLBACK observe(int code,WPARAM wp,LPARAM lp)
{
    const CWPRETSTRUCT *message=style_message_dialog_initialized(code,lp);
    if(message){++observed;style_dialog_observation_found(message->hwnd);}
    return CallNextHookEx(NULL,code,wp,lp);
}
static LRESULT CALLBACK untracked(int code,WPARAM wp,LPARAM lp)
{return CallNextHookEx(NULL,code,wp,lp);}
static PROCESS_HOOK_UNWIND_FUNCTION void call_with_exception(void)
{
    style_dialog_observation scope;process_hook_unwind_scope unwind;
    if(!style_dialog_observation_begin(&scope,&context,observe,TRUE))abort();
    PROCESS_HOOK_UNWIND_BEGIN(unwind,style_dialog_observation_end,&scope);
    RaiseException(TEST_EXCEPTION,0,0,NULL);
    PROCESS_HOOK_UNWIND_END(unwind);
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
    if(!setjmp(c.saved))call_with_exception();
    __asm__ __volatile__("movl %0,%%fs:0"::"r"(c.record.previous):"memory");
#else
    __asm__ __volatile__(".Lcall_catch_start:\n.seh_handler __C_specific_handler, @except\n"
        ".seh_handlerdata\n.long 1\n.rva .Lcall_catch_start,.Lcall_catch_end,filter,.Lcall_catch_end\n.text\n":::"memory");
    call_with_exception();
    __asm__ __volatile__("nop\n.Lcall_catch_end: nop\n":::"memory");
#endif
}
#define CHECK(x) do{if(!(x)){fprintf(stderr,"DIALOG_OBSERVATION_FAIL line=%d error=%lu hooks=%u\n",__LINE__,GetLastError(),live);return 1;}}while(0)
int main(void)
{
    HWND first=CreateWindowW(L"STATIC",L"",WS_OVERLAPPEDWINDOW,0,0,100,60,NULL,NULL,GetModuleHandleW(NULL),NULL);
    HWND second=CreateWindowW(L"STATIC",L"",WS_OVERLAPPEDWINDOW,0,0,100,60,NULL,NULL,GetModuleHandleW(NULL),NULL);CHECK(first && second);
    style_dialog_observation outer,inner,plain;
    CHECK(style_dialog_observation_begin(&outer,&context,observe,TRUE) && !outer.creation.enabled && !live);
    style_dialog_observation_found(first);CHECK(style_dialog_observation_current()==first);style_dialog_observation_end(&outer);
    session_active=TRUE;fail_prepare=TRUE;
    CHECK(!style_dialog_observation_begin(&outer,&context,observe,TRUE) && GetLastError()==ERROR_BAD_FORMAT && !live && !style_dialog_observation_current());
    CHECK(style_dialog_observation_begin(&outer,NULL,observe,FALSE) && outer.creation.enabled && !outer.creation.creation && live==1);
    unsigned native_before=observed;SendMessageW(first,WM_INITDIALOG,0,0);CHECK(observed>native_before);
    style_dialog_observation_end(&outer);CHECK(!live);
    fail_prepare=FALSE;
    CHECK(style_dialog_observation_begin(&outer,&context,observe,TRUE) && live==1);
    SendMessageW(first,WM_INITDIALOG,0,0);CHECK(observed && style_dialog_observation_current()==first);
    CHECK(style_dialog_observation_begin(&plain,&context,untracked,FALSE) && live==2 && style_dialog_observation_current()==first);
    style_dialog_observation_end(&plain);CHECK(live==1 && style_dialog_observation_current()==first);
    session_active=FALSE;CHECK(style_dialog_observation_begin(&inner,&context,observe,TRUE) && live==1 && !style_dialog_observation_current());
    style_dialog_observation_end(&inner);session_active=TRUE;CHECK(style_dialog_observation_current()==first);
    CHECK(style_dialog_observation_begin(&inner,&context,observe,TRUE) && live==2 && !style_dialog_observation_current());
    SendMessageW(second,WM_INITDIALOG,0,0);CHECK(style_dialog_observation_current()==second);
    style_dialog_observation_end(&inner);CHECK(live==1 && style_dialog_observation_current()==first);
    style_dialog_observation_end(&outer);CHECK(!live && !style_dialog_observation_current());
    unsigned before=observed;SendMessageW(first,WM_INITDIALOG,0,0);CHECK(observed==before);
    fail_hook=TRUE;CHECK(style_dialog_observation_begin(&outer,&context,observe,TRUE) && !live && !outer.hook);
    SendMessageW(first,WM_INITDIALOG,0,0);CHECK(observed==before);style_dialog_observation_end(&outer);
    catch_call();CHECK(caught && !live && !style_dialog_observation_current());
    SendMessageW(second,WM_INITDIALOG,0,0);CHECK(observed==before);
    CHECK(style_dialog_observation_begin(&outer,&context,observe,TRUE));SendMessageW(first,WM_INITDIALOG,0,0);
    CHECK(style_dialog_observation_current()==first && observed>before);style_dialog_observation_end(&outer);
    CHECK(!live && DestroyWindow(second) && DestroyWindow(first));
    printf("DIALOG_OBSERVATION_OK bits=%u native_hook=1 observer_only=1 tracked_nested=1 untracked=1 inactive_barrier=1 prepare_failure=1 advisory_hook_failure=1 native_unwind=1 reuse=1\n",(unsigned)(sizeof(void *)*8));return 0;
}
