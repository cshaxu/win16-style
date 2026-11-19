/* Same production controller, with native leaves captured to avoid UI.
 * Observer ownership uses real current-thread Windows hook handles. */
#include <windows.h>
#include <commdlg.h>
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include "lib-win32/process-hook/unwind.h"
#include "lib-win32-style/application/dialog/classic_file_controller.h"
#define TEST_EXCEPTION 0xe0424252u
static BOOL WINAPI fake_open_a(OPENFILENAMEA *);
static BOOL WINAPI fake_open_w(OPENFILENAMEW *);
static BOOL WINAPI fake_save_a(OPENFILENAMEA *);
static BOOL WINAPI fake_save_w(OPENFILENAMEW *);
static HHOOK hooks[16];static unsigned hook_live;static BOOL fail_hook;
static HHOOK WINAPI tracked_hook(int type,HOOKPROC callback,HINSTANCE module,DWORD thread)
{
    if(fail_hook){fail_hook=FALSE;SetLastError(ERROR_ACCESS_DENIED);return NULL;}
    HHOOK hook=SetWindowsHookExW(type,callback,module,thread);
    if(hook){if(hook_live==16)abort();hooks[hook_live++]=hook;}return hook;
}
static BOOL WINAPI tracked_unhook(HHOOK hook)
{
    BOOL result=UnhookWindowsHookEx(hook);
    if(result){for(unsigned i=0;i<hook_live;++i)if(hooks[i]==hook){hooks[i]=hooks[--hook_live];return TRUE;}abort();}
    return FALSE;
}
#define SetWindowsHookExW tracked_hook
#define UnhookWindowsHookEx tracked_unhook
#define GetOpenFileNameA fake_open_a
#define GetOpenFileNameW fake_open_w
#define GetSaveFileNameA fake_save_a
#define GetSaveFileNameW fake_save_w
#include "lib-win32-style/application/dialog/classic_file_controller.c"
#undef SetWindowsHookExW
#undef UnhookWindowsHookEx
#undef GetOpenFileNameA
#undef GetOpenFileNameW
#undef GetSaveFileNameA
#undef GetSaveFileNameW

typedef BOOL (WINAPI *file_a)(OPENFILENAMEA *);
typedef BOOL (WINAPI *file_w)(OPENFILENAMEW *);
static file_a open_a,save_a;static file_w open_w,save_w;
static unsigned attaches,fail_at,scopes,native_calls,errors;
static BOOL active=TRUE,native_result=TRUE,expect_eligible,expect_resolve,nested,raise_call,caught;
static HRESULT scope_result=S_OK;
static void *expected_input;
static LPOFNHOOKPROC expected_hook;
static OPENFILENAMEA exception_input;
static BOOL session(void){return active;}
static HRESULT scope(HRESULT (*callback)(void *),void *value)
{++scopes;SetLastError(777);return FAILED(scope_result)?scope_result:callback(value);}
static void chrome(HWND window){(void)window;}
static void witness(HWND window){(void)window;}
static DWORD capture(void *target,void *replacement)
{
    (void)target;if(++attaches==fail_at)return ERROR_ACCESS_DENIED;
    if(attaches==1)open_a=(file_a)replacement;else if(attaches==2)open_w=(file_w)replacement;
    else if(attaches==3)save_a=(file_a)replacement;else if(attaches==4)save_w=(file_w)replacement;
    else return ERROR_INVALID_PARAMETER;
    return 0;
}
static void observe(void *input,DWORD flags,LPOFNHOOKPROC hook)
{
    ++native_calls;
    if(GetLastError()!=42)++errors;
    if(expect_eligible){if(input==expected_input || (flags&OFN_EXPLORER) || !(flags&OFN_ENABLEHOOK) || hook!=old_dialog_hook)++errors;}
    else if(input!=expected_input || hook!=expected_hook)++errors;
    if(active){if(!active_file_dialog || active_file_dialog->resolve_links!=expect_resolve)++errors;}
    else if(active_file_dialog)++errors;
    if(raise_call)RaiseException(TEST_EXCEPTION,0,0,NULL);
}
static BOOL WINAPI fake_open_a(OPENFILENAMEA *input)
{
    observe(input,input->Flags,input->lpfnHook);
    if(nested){
        nested=FALSE;old_dialog_scope *outer=active_file_dialog;void *saved=expected_input;BOOL resolve=expect_resolve;
        WCHAR file[32]=L"inner";OPENFILENAMEW inner={.lStructSize=sizeof(inner),.lpstrFile=file,.nMaxFile=32,.Flags=OFN_EXPLORER};
        expected_input=&inner;expect_resolve=FALSE;SetLastError(42);
        if(!save_w(&inner) || inner.nFileOffset!=7 || active_file_dialog!=outer)++errors;
        expected_input=saved;expect_resolve=resolve;
    }
    input->nFileOffset=7;input->nFilterIndex=3;input->Flags|=OFN_READONLY;SetLastError(2468);return native_result;
}
static BOOL WINAPI fake_open_w(OPENFILENAMEW *input)
{observe(input,input->Flags,input->lpfnHook);input->nFileOffset=7;input->nFilterIndex=3;input->Flags|=OFN_READONLY;SetLastError(2468);return native_result;}
static BOOL WINAPI fake_save_a(OPENFILENAMEA *input){return fake_open_a(input);}
static BOOL WINAPI fake_save_w(OPENFILENAMEW *input){return fake_open_w(input);}
static UINT_PTR CALLBACK application_hook(HWND w,UINT message,WPARAM wp,LPARAM lp)
{(void)w;(void)message;(void)wp;(void)lp;return 0;}
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
    if(!setjmp(c.saved))open_a(&exception_input);
    __asm__ __volatile__("movl %0,%%fs:0"::"r"(c.record.previous):"memory");
#else
    __asm__ __volatile__(".Lcall_catch_start:\n.seh_handler __C_specific_handler, @except\n"
        ".seh_handlerdata\n.long 1\n.rva .Lcall_catch_start,.Lcall_catch_end,filter,.Lcall_catch_end\n.text\n":::"memory");
    open_a(&exception_input);
    __asm__ __volatile__("nop\n.Lcall_catch_end: nop\n":::"memory");
#endif
}
#define CHECK(x) do{if(!(x)){fprintf(stderr,"FILE_CONTROLLER_FAIL line=%d error=%lu checks=%u hooks=%u\n",__LINE__,GetLastError(),errors,hook_live);return 1;}}while(0)
int main(void)
{
    MSG message;PeekMessageW(&message,NULL,0,0,PM_NOREMOVE);
    style_classic_file_options options={.active=session,.scope=scope,.chrome=chrome};
    CHECK(style_classic_file_attach(NULL,&options)==ERROR_INVALID_PARAMETER && !attaches);
    fail_at=3;CHECK(style_classic_file_attach(capture,&options)==ERROR_ACCESS_DENIED && attaches==3);
    attaches=fail_at=0;CHECK(!style_classic_file_attach(capture,&options) && attaches==4);
    char file_a_buffer[32]="outer";WCHAR file_w_buffer[32]=L"wide";
    OPENFILENAMEA a={.lStructSize=sizeof(a),.lpstrFile=file_a_buffer,.nMaxFile=32,.lpstrFilter="All\0*.*\0\0",.Flags=OFN_EXPLORER};
    OPENFILENAMEW w={.lStructSize=sizeof(w),.lpstrFile=file_w_buffer,.nMaxFile=32,.Flags=OFN_EXPLORER};
    active=FALSE;expected_input=&a;SetLastError(42);CHECK(open_a(&a) && GetLastError()==2468 && !scopes && !hook_live && !errors);
    active=TRUE;expect_eligible=TRUE;expect_resolve=TRUE;SetLastError(42);
    CHECK(open_a(&a) && GetLastError()==2468 && a.nFileOffset==7 && a.nFilterIndex==3 && (a.Flags&OFN_EXPLORER) && !(a.Flags&OFN_ENABLEHOOK) && !hook_live && !active_file_dialog && !errors);
    expected_input=&w;expect_resolve=FALSE;SetLastError(42);CHECK(open_w(&w) && GetLastError()==2468 && w.nFileOffset==7 && !hook_live && !errors);
    expected_input=&a;SetLastError(42);CHECK(save_a(&a) && !hook_live && !errors);
    expected_input=&w;SetLastError(42);CHECK(save_w(&w) && !hook_live && !errors);
    expected_input=&a;expect_eligible=FALSE;expected_hook=application_hook;a.Flags|=OFN_ENABLEHOOK;a.lpfnHook=application_hook;SetLastError(42);
    CHECK(open_a(&a) && GetLastError()==2468 && a.lpfnHook==application_hook && !hook_live && !errors);
    a.Flags=OFN_EXPLORER|OFN_ENABLETEMPLATE;SetLastError(42);
    CHECK(open_a(&a) && GetLastError()==2468 && !hook_live && !errors);
    a.Flags=OFN_EXPLORER|OFN_ALLOWMULTISELECT;a.lpfnHook=NULL;expected_hook=NULL;SetLastError(42);
    CHECK(open_a(&a) && GetLastError()==2468 && !hook_live && !errors);
    a.Flags=OFN_EXPLORER;expect_eligible=TRUE;expect_resolve=TRUE;native_result=FALSE;SetLastError(42);
    CHECK(!open_a(&a) && GetLastError()==2468 && !hook_live && !errors);native_result=TRUE;
    unsigned before=native_calls;scope_result=E_ACCESSDENIED;a.nFileOffset=99;SetLastError(42);
    CHECK(!open_a(&a) && GetLastError()==ERROR_ACCESS_DENIED && native_calls==before && a.nFileOffset==99 && !hook_live && !active_file_dialog);
    scope_result=S_OK;fail_hook=TRUE;SetLastError(42);CHECK(open_a(&a) && GetLastError()==2468 && !hook_live && !errors);
    nested=TRUE;SetLastError(42);CHECK(open_a(&a) && !hook_live && !active_file_dialog && !errors);
    for(unsigned mode=0;mode<2;++mode){
        options.subclass_observer=(BOOL)mode;attaches=0;CHECK(!style_classic_file_attach(capture,&options));
        CHECK(!style_creation_dialog_observer(witness));
        exception_input=a;expected_input=&exception_input;caught=FALSE;raise_call=TRUE;SetLastError(42);catch_call();raise_call=FALSE;
        CHECK(caught && !active_file_dialog && !hook_live && !errors);
        CHECK(style_creation_dialog_observer(NULL)==witness);
        expected_input=&a;SetLastError(42);CHECK(open_a(&a) && !active_file_dialog && !hook_live && !errors);
    }
    printf("CLASSIC_FILE_CONTROLLER_OK bits=%u ansi_unicode=1 passthrough=1 result_error=1 scope_failure=1 observer_failure=1 nested=1 native_unwind=1 subclass_restore=1 hooks=0\n",(unsigned)(sizeof(void *)*8));return 0;
}
