#include "creation_call.h"
#include "lib-win32/process-hook/unwind.h"

BOOL style_creation_call_begin(style_creation_call *call,BOOL modal,style_creation_context *context)
{
    ZeroMemory(call,sizeof(*call));
    if(!context->active())return TRUE;
    if(!InitOnceExecuteOnce(&context->once,context->prepare,NULL,NULL))return FALSE;
    call->enabled=TRUE;
    call->creation=style_creation_begin(TRUE,modal,context->operations);
    return call->creation!=NULL;
}
void style_creation_call_end(void *data)
{
    style_creation_call *call=data;style_creation_result result={0};
    if(!call->creation)return;
    if(!style_creation_end(call->creation,&result))call->failure=GetLastError();
    else call->failure=result.failure;
}
PROCESS_HOOK_UNWIND_FUNCTION HRESULT style_creation_call_invoke(style_creation_context *context,
    HRESULT (*callback)(void *),void *data)
{
    process_hook_unwind_scope unwind;style_creation_call call;HRESULT result=E_FAIL;
    if(!callback)return E_INVALIDARG;
    if(!style_creation_call_begin(&call,TRUE,context))return HRESULT_FROM_WIN32(GetLastError());
    PROCESS_HOOK_UNWIND_BEGIN(unwind,style_creation_call_end,&call);
    result=callback(data);
    PROCESS_HOOK_UNWIND_END(unwind);
    return call.failure?HRESULT_FROM_WIN32(call.failure):result;
}
