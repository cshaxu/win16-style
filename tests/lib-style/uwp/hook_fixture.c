#include "lib-style/uwp/hook.h"
#include <stdint.h>
static _Thread_local BOOL busy;
static BOOL accepts(WPARAM action,void *context){(void)context;return action==1 || action==2;}
static DWORD perform(HWND frame,HWND core,WPARAM action,void *context)
{
    (void)context;
    if(action==2)return ERROR_ACCESS_DENIED;
    SendMessageW(frame,RegisterWindowMessageW(L"StyleDispatch.Test.Message"),2,(LPARAM)core);
    return GetPropW(frame,L"StyleDispatch.Test.Result")?ERROR_INVALID_DATA:ERROR_SUCCESS;
}
__declspec(dllexport) LRESULT CALLBACK fixture_hook(int code,WPARAM wp,LPARAM lp)
{
    const style_uwp_hook_spec spec={L"StyleDispatch.Test.Message",L"StyleDispatch.Test.Result",accepts,perform,NULL};
    style_uwp_hook_handle(code,lp,&busy,&spec);return CallNextHookEx(NULL,code,wp,lp);
}
