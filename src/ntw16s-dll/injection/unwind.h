/* Project-authored C cleanup scopes for the native Windows unwinder.
 * Private to the injection facade; never catches or translates exceptions.
 * Compile facade functions with frame pointers, no sibling calls, and no LTO.
 * One live scope per facade frame. Cleanup must release/pop its state before
 * calling application code. Keep protected calls between BEGIN and END.
 */
#ifndef NTW16S_UNWIND_H
#define NTW16S_UNWIND_H
#include <windows.h>
#if !defined(__GNUC__) || (!defined(__i386__) && !defined(__x86_64__))
#error This private cleanup implementation requires x86/x64 MinGW GCC
#endif
typedef struct ntw16s_unwind_record {
    struct ntw16s_unwind_record *previous;
    PEXCEPTION_ROUTINE handler;
} ntw16s_unwind_record;
typedef struct ntw16s_unwind_scope {
    ntw16s_unwind_record record;
    struct ntw16s_unwind_scope *previous;
    void *frame;
    void (*cleanup)(void *);
    void *data;
} ntw16s_unwind_scope;
void ntw16s_unwind_begin(ntw16s_unwind_scope *,void *,void (*)(void *),void *);
void ntw16s_unwind_end(ntw16s_unwind_scope *);
#ifdef _WIN64
EXCEPTION_DISPOSITION NTAPI ntw16s_unwind_handler(EXCEPTION_RECORD *,void *,CONTEXT *,DISPATCHER_CONTEXT *);
#define NTW16S_UNWIND_METADATA() \
    __asm__ __volatile__(".seh_handler ntw16s_unwind_handler, @unwind\n":::"memory")
#else
#define NTW16S_UNWIND_METADATA() ((void)0)
#endif
#define NTW16S_UNWIND_FUNCTION __attribute__((noinline,noclone,section(".text")))
#define NTW16S_UNWIND_BEGIN(scope,cleanup,data) do { \
    ntw16s_unwind_begin(&(scope),__builtin_frame_address(0),(cleanup),(data)); \
    NTW16S_UNWIND_METADATA(); \
} while(0)
#define NTW16S_UNWIND_END(scope) ntw16s_unwind_end(&(scope))
#endif
