#include "lib-style/common/control_protocol.h"
int main(void)
{
    HWND parent=CreateWindowW(L"STATIC",L"",WS_POPUP,0,0,40,40,NULL,NULL,GetModuleHandleW(NULL),NULL);
    if(!parent) return 1;
    HWND child=CreateWindowW(L"STATIC",L"",WS_CHILD,0,0,20,20,parent,NULL,GetModuleHandleW(NULL),NULL);
    if(!child){DestroyWindow(parent);return 2;}
    BOOL ok=!style_control_protocol_bind(child) && !style_control_protocol_acquire(child) &&
        !style_control_protocol_is(child,L"BUTTON");
    style_control_protocol_release(NULL);DestroyWindow(parent);return ok?0:3;
}
