#ifndef NTW16S_TEST_EXCEPTION_PROBE_H
#define NTW16S_TEST_EXCEPTION_PROBE_H
#include <setjmp.h>
#include "../../src/lib-win32/process-hook/unwind.h"

#ifndef NTW16S_TEST_EXCEPTION_CODE
#define NTW16S_TEST_EXCEPTION_CODE 0xe0425342u
#endif

static BOOL exception_seen;
#ifndef _WIN64
typedef struct catcher {process_hook_unwind_record record;jmp_buf saved;} catcher;
static EXCEPTION_DISPOSITION NTAPI catch_exception(EXCEPTION_RECORD *e,void *frame,CONTEXT *c,void *dispatcher)
{(void)c;(void)dispatcher;if(!(e->ExceptionFlags&(EXCEPTION_UNWINDING|EXCEPTION_EXIT_UNWIND)) && e->ExceptionCode==NTW16S_TEST_EXCEPTION_CODE){exception_seen=TRUE;RtlUnwind(frame,NULL,e,NULL);longjmp(((catcher*)frame)->saved,1);}return ExceptionContinueSearch;}
#else
static LONG __attribute__((used)) rich_filter(EXCEPTION_POINTERS *e,void *frame)
{(void)frame;if(e->ExceptionRecord->ExceptionCode==NTW16S_TEST_EXCEPTION_CODE){exception_seen=TRUE;return EXCEPTION_EXECUTE_HANDLER;}return EXCEPTION_CONTINUE_SEARCH;}
#endif
static PROCESS_HOOK_UNWIND_FUNCTION BOOL test_unwind(HWND w,UINT message,WPARAM wparam,LPARAM lparam)
{exception_seen=FALSE;
#ifndef _WIN64
 catcher c;c.record.handler=catch_exception;__asm__ __volatile__("movl %%fs:0,%0":"=r"(c.record.previous)::"memory");__asm__ __volatile__("movl %0,%%fs:0"::"r"(&c.record):"memory");if(!setjmp(c.saved))SendMessageW(w,message,wparam,lparam);__asm__ __volatile__("movl %0,%%fs:0"::"r"(c.record.previous):"memory");
#else
 __asm__ __volatile__(".Lrich_start:\n.seh_handler __C_specific_handler, @except\n.seh_handlerdata\n.long 1\n.rva .Lrich_start,.Lrich_end,rich_filter,.Lrich_end\n.text\n":::"memory");SendMessageW(w,message,wparam,lparam);__asm__ __volatile__("nop\n.Lrich_end: nop\n":::"memory");
#endif
 return exception_seen;}
#endif
