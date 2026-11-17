#include "dispatch.h"
#include "lib-win32/process-hook/unwind.h"
#include <commctrl.h>
typedef struct dispatch_call {const style_dispatch_ops *ops;BOOL completed;} dispatch_call;
static void finish_call(void *context)
{dispatch_call *call=context;call->ops->finish(call->ops->finish_context,call->completed);}
LRESULT PROCESS_HOOK_UNWIND_FUNCTION style_dispatch_message(HWND w,UINT m,WPARAM p,LPARAM l,const style_dispatch_ops *ops)
{
    DWORD entry=GetLastError(),error;dispatch_call call={.ops=ops};process_hook_unwind_scope unwind;
    PROCESS_HOOK_UNWIND_BEGIN(unwind,finish_call,&call);
    SetLastError(entry);LRESULT result=ops->invoke?ops->invoke(ops->call_context,w,m,p,l):DefSubclassProc(w,m,p,l);
    error=GetLastError();call.completed=TRUE;PROCESS_HOOK_UNWIND_END(unwind);SetLastError(error);return result;
}
