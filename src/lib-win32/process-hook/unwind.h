/* Project-authored C cleanup scopes for the native Windows unwinder.
 * Shared native cleanup support; never catches or translates exceptions.
 * Compile facade functions with frame pointers, no sibling calls, and no LTO.
 * One live scope per facade frame. Cleanup must release/pop its state before
 * calling application code. Keep protected calls between BEGIN and END.
 */
#ifndef PROCESS_HOOK_UNWIND_H
#define PROCESS_HOOK_UNWIND_H
#include <windows.h>
#if !defined(__GNUC__) || (!defined(__i386__) && !defined(__x86_64__))
#error This cleanup implementation requires x86/x64 MinGW GCC
#endif
typedef struct process_hook_unwind_record {
    struct process_hook_unwind_record *previous;
    PEXCEPTION_ROUTINE handler;
} process_hook_unwind_record;
typedef struct process_hook_unwind_scope {
    process_hook_unwind_record record;
    struct process_hook_unwind_scope *previous;
    void *frame;
    void (*cleanup)(void *);
    void *data;
} process_hook_unwind_scope;
void process_hook_unwind_begin(process_hook_unwind_scope *,void *,void (*)(void *),void *);
void process_hook_unwind_end(process_hook_unwind_scope *);
#ifdef _WIN64
EXCEPTION_DISPOSITION NTAPI process_hook_unwind_handler(EXCEPTION_RECORD *,void *,CONTEXT *,DISPATCHER_CONTEXT *);
#define PROCESS_HOOK_UNWIND_METADATA() \
    __asm__ __volatile__(".seh_handler process_hook_unwind_handler, @unwind\n":::"memory")
#else
#define PROCESS_HOOK_UNWIND_METADATA() ((void)0)
#endif
#define PROCESS_HOOK_UNWIND_FUNCTION __attribute__((noinline,noclone,section(".text")))
#define PROCESS_HOOK_UNWIND_BEGIN(scope,cleanup,data) do { \
    process_hook_unwind_begin(&(scope),__builtin_frame_address(0),(cleanup),(data)); \
    PROCESS_HOOK_UNWIND_METADATA(); \
} while(0)
#define PROCESS_HOOK_UNWIND_END(scope) process_hook_unwind_end(&(scope))
#endif
