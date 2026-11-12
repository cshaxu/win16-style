#ifndef STYLE_COMMON_DIALOG_HOOK_H
#define STYLE_COMMON_DIALOG_HOOK_H
#include <windows.h>
#include "lib-win32/process-hook/unwind.h"

/* The caller supplies scope ownership and finish policy. The original call's
 * error survives cleanup and finish; exceptions unwind the scope unchanged. */
#define STYLE_DIALOG_HOOK(NAME,ARGS,CALL,RET,FAIL,MODE,SCOPE,BEGIN,PREPARE,CLEANUP,FINISH) \
static __typeof__(&NAME) real_##NAME; \
static PROCESS_HOOK_UNWIND_FUNCTION RET WINAPI hook_##NAME ARGS \
{ SCOPE state;process_hook_unwind_scope unwind;RET result=FAIL; \
  DWORD incoming=GetLastError(),error=ERROR_SUCCESS; \
  if(!BEGIN(&state,incoming))return FAIL; \
  PROCESS_HOOK_UNWIND_BEGIN(unwind,CLEANUP,&state); \
  { PREPARE(&state,incoming);result=real_##NAME CALL;error=GetLastError(); } \
  PROCESS_HOOK_UNWIND_END(unwind); \
  FINISH(&state,MODE,result,error,FAIL);SetLastError(error);return result; }
#endif
