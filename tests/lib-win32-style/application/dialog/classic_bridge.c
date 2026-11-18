#include "lib-win32-style/application/dialog/classic_com_show.h"
#include "lib-win32-style/application/dialog/classic_browse.h"
#include "lib-win32/process-hook/unwind.h"
#include <setjmp.h>
static BOOL active,raises,caught;
#define TEST_EXCEPTION 0xe0424242u
static unsigned native_calls,scope_calls;
static FILEOPENDIALOGOPTIONS options;
static HRESULT options_result;
static BOOL session_active(void) { return active; }
static HRESULT STDMETHODCALLTYPE native_show(IFileDialog *self,HWND window)
{ ++native_calls;if(raises)RaiseException(TEST_EXCEPTION,0,0,NULL);return self && window==(HWND)(ULONG_PTR)42 ? E_ABORT : E_FAIL; }
static HRESULT STDMETHODCALLTYPE native_options(IFileDialog *self,FILEOPENDIALOGOPTIONS *value)
{ (void)self;*value=options;return options_result; }
static HRESULT scope(HRESULT (*call)(void *),void *context)
{ ++scope_calls;return call(context); }
/* Use the same native outer catcher as the existing unwind contract test.
 * The production Show facade must unwind its own reference before this catches. */
#ifndef _WIN64
typedef struct catcher {process_hook_unwind_record record;jmp_buf saved;} catcher;
static EXCEPTION_DISPOSITION NTAPI catch32(EXCEPTION_RECORD *exception,
    void *frame,CONTEXT *context,void *dispatcher)
{
    (void)context;(void)dispatcher;
    if(!(exception->ExceptionFlags&(EXCEPTION_UNWINDING|EXCEPTION_EXIT_UNWIND)) &&
       exception->ExceptionCode==TEST_EXCEPTION){
        caught=TRUE;RtlUnwind(frame,NULL,exception,NULL);longjmp(((catcher *)frame)->saved,1);
    }
    return ExceptionContinueSearch;
}
#else
static LONG __attribute__((used)) filter(EXCEPTION_POINTERS *exception,void *frame)
{
    (void)frame;
    if(exception->ExceptionRecord->ExceptionCode!=TEST_EXCEPTION)return EXCEPTION_CONTINUE_SEARCH;
    caught=TRUE;return EXCEPTION_EXECUTE_HANDLER;
}
#endif
static PROCESS_HOOK_UNWIND_FUNCTION BOOL exception_show(style_classic_dialog_proxy *proxy,
    const style_classic_com_show_services *services)
{
    raises=TRUE;caught=FALSE;
#ifndef _WIN64
    catcher c;c.record.handler=catch32;
    __asm__ __volatile__("movl %%fs:0,%0":"=r"(c.record.previous)::"memory");
    __asm__ __volatile__("movl %0,%%fs:0"::"r"(&c.record):"memory");
    if(!setjmp(c.saved))style_classic_com_show(proxy,(HWND)(ULONG_PTR)42,services);
    __asm__ __volatile__("movl %0,%%fs:0"::"r"(c.record.previous):"memory");
#else
    __asm__ __volatile__(".Lshow_catch_start:\n.seh_handler __C_specific_handler, @except\n"
        ".seh_handlerdata\n.long 1\n.rva .Lshow_catch_start,.Lshow_catch_end,filter,.Lshow_catch_end\n.text\n":::"memory");
    style_classic_com_show(proxy,(HWND)(ULONG_PTR)42,services);
    __asm__ __volatile__("nop\n.Lshow_catch_end: nop\n":::"memory");
#endif
    raises=FALSE;return caught;
}
int main(void)
{
    IFileDialogVtbl vtable={.Show=native_show,.GetOptions=native_options};
    IFileDialog dialog={.lpVtbl=&vtable};
    style_classic_dialog_proxy proxy={.native=&dialog,.completed=TRUE};
    const style_classic_com_show_services services={.session_active=session_active,.scope=scope};
    if(style_classic_com_show(&proxy,(HWND)(ULONG_PTR)42,&services)!=E_ABORT ||
       proxy.completed || native_calls!=1 || scope_calls!=1)return 1;
    active=TRUE;options_result=E_FAIL;
    if(style_classic_com_show(&proxy,(HWND)(ULONG_PTR)42,&services)!=E_ABORT ||
       native_calls!=2 || scope_calls!=2)return 4;
    options_result=S_OK;proxy.unsupported=TRUE;
    if(style_classic_com_show(&proxy,(HWND)(ULONG_PTR)42,&services)!=E_ABORT ||
       native_calls!=3 || scope_calls!=3)return 5;
    proxy.unsupported=FALSE;options=FOS_ALLOWMULTISELECT;
    if(style_classic_com_show(&proxy,(HWND)(ULONG_PTR)42,&services)!=E_ABORT ||
       native_calls!=4 || scope_calls!=4)return 6;
    style_classic_com_show_services experimental=services;experimental.experimental_events=TRUE;
    proxy.refs=1;proxy.classic_show_aborted=TRUE;
    if(style_classic_com_show(&proxy,(HWND)(ULONG_PTR)42,&experimental)!=E_UNEXPECTED ||
       proxy.refs!=1 || scope_calls!=4)return 7;
    proxy.classic_show_aborted=FALSE;proxy.classic_close_pending=TRUE;options_result=E_FAIL;
    if(style_classic_com_show(&proxy,(HWND)(ULONG_PTR)42,&experimental)!=E_ABORT ||
       proxy.refs!=1 || proxy.classic_close_pending || proxy.classic_show_aborted || scope_calls!=5)return 8;
    if(!exception_show(&proxy,&experimental) || proxy.refs!=1 || !proxy.classic_show_aborted ||
       proxy.completed || native_calls!=6 || scope_calls!=6)return 9;
    if(style_classic_com_show(&proxy,(HWND)(ULONG_PTR)42,&experimental)!=E_UNEXPECTED ||
       native_calls!=6 || scope_calls!=6)return 10;
    style_classic_com_show_services explorer=services;explorer.explorer_files=TRUE;
    options_result=S_OK;options=0;
    proxy.classic_show_aborted=FALSE;
    if(style_classic_com_show(&proxy,(HWND)(ULONG_PTR)42,&explorer)!=E_ABORT ||
       native_calls!=7 || scope_calls!=7)return 11;
    UINT flags=BIF_RETURNONLYFSDIRS|BIF_NEWDIALOGSTYLE;
    style_classic_browse_plan p=style_classic_plan_browse(flags,FALSE);
    if(p.mode!=STYLE_CLASSIC_BROWSE_REWRITE || p.flags!=BIF_RETURNONLYFSDIRS) return 2;
    p=style_classic_plan_browse(flags,TRUE);
    if(p.mode!=STYLE_CLASSIC_BROWSE_NATIVE || p.flags!=flags) return 3;
    return 0;
}
