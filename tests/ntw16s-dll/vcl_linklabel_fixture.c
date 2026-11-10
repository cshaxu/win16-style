#define UNICODE
#include <windows.h>
#include <commctrl.h>
#include "native_thunk.h"
/* Test-only DLL: its class is owned outside the EXE. The synthetic delayed
 * native initialization exercises creation-scope admission; it does not
 * claim to reproduce a Delphi runtime's actual creation order. */
#define EDIT_CLASS L"TLinkLabel"
#include "vcl_edit_fixture.h"
__declspec(dllexport) void WINAPI FixtureSetMode(int value){fake=value;}
__declspec(dllexport) BOOL WINAPI FixtureRegister(int impostor)
{
    WNDCLASSEXW n={.cbSize=sizeof(n)};
    INITCOMMONCONTROLSEX init={.dwSize=sizeof(init),.dwICC=ICC_LINK_CLASS};
    if(!InitCommonControlsEx(&init) || !GetClassInfoExW(NULL,WC_LINK,&n))return FALSE;
    return register_native(&n,impostor);
}
__declspec(dllexport) HWND WINAPI FixtureCreate(HWND parent,BOOL multiline)
{
    DWORD style=WS_CHILD|WS_VISIBLE|WS_TABSTOP;
#ifdef T18_MEMO
    multiline=TRUE;
#endif
    LPCWSTR initial_text=multiline?L"":L"<a id=\"probe\">Open</a>";
    HWND w=CreateWindowExW(0,EDIT_CLASS,initial_text,style,20,20,220,40,parent,NULL,owner,NULL);
    if(w){CREATESTRUCTW cs={0};cs.hwndParent=parent;cs.hInstance=owner;cs.style=GetWindowLongPtrW(w,GWL_STYLE);
        cs.dwExStyle=GetWindowLongPtrW(w,GWL_EXSTYLE);cs.lpszClass=EDIT_CLASS;cs.lpszName=initial_text;cs.cx=220;cs.cy=40;
        enabled=TRUE;CallWindowProcW(native_edit,w,WM_NCCREATE,0,(LPARAM)&cs);CallWindowProcW(native_edit,w,WM_CREATE,0,(LPARAM)&cs);SendMessageW(w,WM_SETFONT,(WPARAM)GetStockObject(DEFAULT_GUI_FONT),FALSE);}
#ifdef T18_MEMO
    if(w)SendMessageW(w,EM_SETLIMITTEXT,0,0);
#endif
    return w;
}
