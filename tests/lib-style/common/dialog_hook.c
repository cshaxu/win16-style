#include <windows.h>
#include <setjmp.h>
#include <stdio.h>
#include "lib-style/common/dialog_hook.h"

#define TEST_EXCEPTION 0xe0424243u
typedef struct test_scope { DWORD failure; } test_scope;
static unsigned begins,ends,calls,finishes;
static BOOL fail_begin,fail_end,raise_call;
static DWORD entry_error;
static INT_PTR WINAPI MockDialog(HINSTANCE instance,LPCWSTR name,HWND parent,DLGPROC proc,LPARAM param)
{
    (void)instance;(void)name;(void)parent;(void)proc;(void)param;
    ++calls;entry_error=GetLastError();
    if(raise_call)RaiseException(TEST_EXCEPTION,0,0,NULL);
    SetLastError(1234);return 47;
}
static INT_PTR WINAPI MockParam(void *parameter)
{
    (void)parameter;++calls;entry_error=GetLastError();
    if(raise_call)RaiseException(TEST_EXCEPTION,0,0,NULL);
    SetLastError(1234);return 47;
}
static BOOL begin(test_scope *scope,DWORD incoming)
{
    (void)incoming;++begins;scope->failure=0;
    if(fail_begin){SetLastError(2345);return FALSE;}
    return TRUE;
}
static void cleanup(void *context)
{test_scope *scope=context;++ends;if(fail_end)scope->failure=3456;SetLastError(4567);}
#define PREPARE(scope,incoming) ((void)(scope),SetLastError(incoming))
#define FINISH(scope,modal,result,error,fail) do { \
    (void)(modal);++finishes;if((scope)->failure){(result)=(fail);(error)=(scope)->failure;} \
    SetLastError(5678); \
} while(0)
STYLE_DIALOG_HOOK(MockDialog,(HINSTANCE instance,LPCWSTR name,HWND parent,DLGPROC proc,LPARAM param),(instance,name,parent,proc,param),INT_PTR,-1,TRUE,test_scope,begin,PREPARE,cleanup,FINISH)
STYLE_DIALOG_HOOK(MockParam,(void *parameters),(parameters),INT_PTR,-1,TRUE,test_scope,begin,PREPARE,cleanup,FINISH)
static void call_dialog(void){hook_MockDialog(NULL,NULL,NULL,NULL,0);}
static void call_param(void){hook_MockParam(NULL);}

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
    __asm__ __volatile__(".Ldialog_hook_catch_start:\n.seh_handler __C_specific_handler, @except\n"
        ".seh_handlerdata\n.long 1\n.rva .Ldialog_hook_catch_start,.Ldialog_hook_catch_end,filter,.Ldialog_hook_catch_end\n.text\n":::"memory");
    callback();
    __asm__ __volatile__("nop\n.Ldialog_hook_catch_end: nop\n":::"memory");
#endif
}
int main(void)
{
    real_MockDialog=MockDialog;
    SetLastError(42);
    if(hook_MockDialog(NULL,NULL,NULL,NULL,0)!=47 || GetLastError()!=1234 || entry_error!=42 || ends!=1 || finishes!=1)return 1;
    fail_begin=TRUE;
    if(hook_MockDialog(NULL,NULL,NULL,NULL,0)!=-1 || GetLastError()!=2345 || calls!=1 || ends!=1)return 2;
    fail_begin=FALSE;fail_end=TRUE;
    if(hook_MockDialog(NULL,NULL,NULL,NULL,0)!=-1 || GetLastError()!=3456 || ends!=2)return 3;
    fail_end=FALSE;raise_call=TRUE;catch_exception(call_dialog);
    if(begins!=4 || calls!=3 || ends!=3 || finishes!=2)return 4;
    begins=ends=calls=finishes=0;raise_call=FALSE;real_MockParam=MockParam;
    SetLastError(42);
    if(hook_MockParam(NULL)!=47 || GetLastError()!=1234 || entry_error!=42 || ends!=1 || finishes!=1)return 5;
    fail_begin=TRUE;
    if(hook_MockParam(NULL)!=-1 || GetLastError()!=2345 || calls!=1 || ends!=1)return 6;
    fail_begin=FALSE;fail_end=TRUE;
    if(hook_MockParam(NULL)!=-1 || GetLastError()!=3456 || ends!=2)return 7;
    fail_end=FALSE;raise_call=TRUE;catch_exception(call_param);
    if(begins!=4 || calls!=3 || ends!=3 || finishes!=2)return 8;
    puts("DIALOG HOOK PASS ordinary=1 parameter=1 return=1 error=1 begin_failure=1 rollback=1 exception_cleanup=1");return 0;
}
