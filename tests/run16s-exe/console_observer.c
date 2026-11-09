#include <windows.h>
#include <stdint.h>

typedef BOOL (__cdecl *frame_fn)(HWND);
__declspec(dllexport) LRESULT CALLBACK ObserveConsole(int code,WPARAM wp,LPARAM lp)
{
    if(code>=0 && lp){
        const CWPSTRUCT *message=(const CWPSTRUCT *)lp;
        if(message->message==RegisterWindowMessageW(L"Run16s.Console.Test.v1")){
            HMODULE module=GetModuleHandleW(L"ntw16s64.dll");
            frame_fn fn=module?(frame_fn)(uintptr_t)GetProcAddress(module,
                message->wParam==1?"Ntw16sWindowAttached":"Ntw16sWindowRemove"):NULL;
            BOOL result=fn && fn(message->hwnd);
            SetPropW(message->hwnd,L"Run16s.Console.Test.Result.v1",(HANDLE)(uintptr_t)(result?2:1));
        }
    }
    return CallNextHookEx(NULL,code,wp,lp);
}
