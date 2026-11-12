#include "lib-style/common/message_observer.h"
static unsigned seen;
static unsigned nested_seen;
static LRESULT CALLBACK observe(int code,WPARAM wp,LPARAM lp)
{
    if(code>=0 && lp && ((const CWPRETSTRUCT *)lp)->message==WM_APP+19)++seen;
    return CallNextHookEx(NULL,code,wp,lp);
}
static LRESULT CALLBACK observe_before(int code,WPARAM wp,LPARAM lp)
{
    if(code>=0 && lp && ((const CWPSTRUCT *)lp)->message==WM_APP+19)++seen;
    return CallNextHookEx(NULL,code,wp,lp);
}
static LRESULT CALLBACK observe_nested(int code,WPARAM wp,LPARAM lp)
{
    if(code>=0 && lp && ((const CWPRETSTRUCT *)lp)->message==WM_APP+19)++nested_seen;
    return CallNextHookEx(NULL,code,wp,lp);
}
int main(void)
{
    CWPRETSTRUCT message={.message=WM_INITDIALOG};
    if(style_message_dialog_initialized(-1,(LPARAM)&message) ||
       style_message_dialog_initialized(0,0) ||
       style_message_dialog_initialized(0,(LPARAM)&message)!=&message)return 9;
    message.message=WM_CREATE;
    if(style_message_dialog_initialized(0,(LPARAM)&message))return 10;
    HWND window=CreateWindowW(L"STATIC",L"",WS_POPUP,0,0,10,10,NULL,NULL,GetModuleHandleW(NULL),NULL);
    if(!window)return 1;
    HHOOK hook=style_message_observer_begin(observe);
    if(!hook){DestroyWindow(window);return 2;}
    SendMessageW(window,WM_APP+19,0,0);
    if(seen!=1){style_message_observer_end(hook);DestroyWindow(window);return 3;}
    HHOOK nested=style_message_observer_begin(observe_nested);
    if(!nested){style_message_observer_end(hook);DestroyWindow(window);return 11;}
    SendMessageW(window,WM_APP+19,0,0);
    if(seen!=2 || nested_seen!=1){style_message_observer_end(nested);style_message_observer_end(hook);DestroyWindow(window);return 12;}
    style_message_observer_end(nested);
    SendMessageW(window,WM_APP+19,0,0);
    if(seen!=3 || nested_seen!=1){style_message_observer_end(hook);DestroyWindow(window);return 13;}
    style_message_observer_end(hook);
    SendMessageW(window,WM_APP+19,0,0);
    if(seen!=3 || nested_seen!=1){DestroyWindow(window);return 4;}
    style_message_observer_end(NULL);
    hook=SetWindowsHookExW(WH_CALLWNDPROC,observe_before,NULL,GetCurrentThreadId());
    if(!hook){DestroyWindow(window);return 5;}
    SendMessageW(window,WM_APP+19,0,0);UnhookWindowsHookEx(hook);
    if(seen!=4){DestroyWindow(window);return 6;}
    hook=SetWindowsHookExW(WH_CALLWNDPROC,observe_before,GetModuleHandleW(NULL),GetCurrentThreadId());
    if(!hook){DestroyWindow(window);return 7;}
    SendMessageW(window,WM_APP+19,0,0);UnhookWindowsHookEx(hook);
    DestroyWindow(window);return seen==5?0:8;
}
