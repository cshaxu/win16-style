#include "unwind.h"
static _Thread_local ntw16s_unwind_scope *active;

static void close_scope(ntw16s_unwind_scope *scope,BOOL abnormal)
{
    DWORD error=GetLastError();
    if(active!=scope){RaiseFailFastException(NULL,NULL,0);__builtin_unreachable();}
    /* Pop before callback: a callback that raises must not run twice. */
    active=scope->previous;
#ifndef _WIN64
    if(!abnormal)__asm__ __volatile__("movl %0,%%fs:0"::"r"(scope->record.previous):"memory");
#else
    (void)abnormal;
#endif
    scope->cleanup(scope->data);
    SetLastError(error);
}
#ifdef _WIN64
EXCEPTION_DISPOSITION NTAPI ntw16s_unwind_handler(EXCEPTION_RECORD *exception,
    void *frame,CONTEXT *context,DISPATCHER_CONTEXT *dispatcher)
{
    (void)context;(void)dispatcher;
    if((exception->ExceptionFlags&(EXCEPTION_UNWINDING|EXCEPTION_EXIT_UNWIND)) &&
        active && active->frame==frame)close_scope(active,TRUE);
    return ExceptionContinueSearch;
}
#else
static EXCEPTION_DISPOSITION NTAPI unwind32(EXCEPTION_RECORD *exception,
    void *frame,CONTEXT *context,void *dispatcher)
{
    (void)context;(void)dispatcher;
    if(exception->ExceptionFlags&(EXCEPTION_UNWINDING|EXCEPTION_EXIT_UNWIND))
        close_scope(frame,TRUE);
    return ExceptionContinueSearch;
}
#endif
void ntw16s_unwind_begin(ntw16s_unwind_scope *scope,void *frame,void (*cleanup)(void *),void *data)
{
    DWORD error=GetLastError();
    scope->previous=active;scope->frame=frame;scope->cleanup=cleanup;scope->data=data;
    active=scope;
#ifndef _WIN64
    scope->record.handler=unwind32;
    __asm__ __volatile__("movl %%fs:0,%0":"=r"(scope->record.previous)::"memory");
    __asm__ __volatile__("movl %0,%%fs:0"::"r"(&scope->record):"memory");
#endif
    SetLastError(error);
}
void ntw16s_unwind_end(ntw16s_unwind_scope *scope)
{close_scope(scope,FALSE);}
