#include <windows.h>
#include <stdint.h>
__declspec(dllexport) LRESULT CALLBACK fixture_hook(int code,WPARAM wp,LPARAM lp)
{
    if(code>=0 && lp){const CWPSTRUCT *message=(const CWPSTRUCT *)lp;
        if(message->message==RegisterWindowMessageW(L"StyleDispatch.Test.Message")){
            if(message->wParam==1)SetPropW(message->hwnd,L"StyleDispatch.Test.Result",(HANDLE)(uintptr_t)1);
            if(message->wParam==2)SetPropW(message->hwnd,L"StyleDispatch.Test.Result",(HANDLE)(uintptr_t)(ERROR_ACCESS_DENIED+1));
        }}
    return CallNextHookEx(NULL,code,wp,lp);
}
