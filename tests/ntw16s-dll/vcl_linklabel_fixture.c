#define UNICODE
#include <windows.h>
#include <commctrl.h>
#include <string.h>
/* Test-only DLL: its class is owned outside the EXE. The synthetic delayed
 * native initialization exercises creation-scope admission; it does not
 * claim to reproduce a Delphi runtime's actual creation order. */
#define EDIT_CLASS L"TLinkLabel"
typedef LRESULT (*foreign_fn)(WNDPROC,HWND,UINT,WPARAM,LPARAM);
static foreign_fn foreign_paint;
static HINSTANCE owner;
static WNDPROC native_edit;
static int fake;
static BOOL enabled;
static void *thunk;
static LRESULT CALLBACK dispatch(HWND w,UINT m,WPARAM p,LPARAM l)
{if (enabled && fake==2 && m==WM_PAINT && foreign_paint) return foreign_paint(native_edit,w,m,p,l);
 return (!enabled || (fake==1 && m==WM_PAINT))?DefWindowProcW(w,m,p,l):CallWindowProcW(native_edit,w,m,p,l);}
static LRESULT CALLBACK initial(HWND w,UINT m,WPARAM p,LPARAM l)
{LRESULT result=dispatch(w,m,p,l);if(m==WM_NCCREATE)SetWindowLongPtrW(w,GWLP_WNDPROC,(LONG_PTR)thunk);return result;}
__declspec(dllexport) void WINAPI FixtureSetForeign(foreign_fn fn){foreign_paint=fn;}
__declspec(dllexport) void WINAPI FixtureSetMode(int value){fake=value;}
__declspec(dllexport) BOOL WINAPI FixtureRegister(int impostor)
{
    WNDCLASSEXW n={.cbSize=sizeof(n)},c={.cbSize=sizeof(c)};DWORD old;
    INITCOMMONCONTROLSEX init={.dwSize=sizeof(init),.dwICC=ICC_LINK_CLASS};
    if(!InitCommonControlsEx(&init) || !GetClassInfoExW(NULL,WC_LINK,&n))return FALSE;
    native_edit=n.lpfnWndProc;fake=impostor;enabled=FALSE;
    thunk=VirtualAlloc(NULL,4096,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);if(!thunk)return FALSE;
#ifdef _WIN64
    unsigned char code[]={0x48,0xb8,0,0,0,0,0,0,0,0,0xff,0xe0};void *target=(void*)dispatch;memcpy(code+2,&target,sizeof(target));
#else
    unsigned char code[]={0xb8,0,0,0,0,0xff,0xe0};void *target=(void*)dispatch;memcpy(code+1,&target,sizeof(target));
#endif
    memcpy(thunk,code,sizeof(code));
    if(!VirtualProtect(thunk,4096,PAGE_EXECUTE_READ,&old)){VirtualFree(thunk,0,MEM_RELEASE);thunk=NULL;return FALSE;}
    FlushInstructionCache(GetCurrentProcess(),thunk,sizeof(code));
    c.style=CS_DBLCLKS|CS_HREDRAW|CS_VREDRAW;c.cbWndExtra=n.cbWndExtra;c.hCursor=LoadCursorW(NULL,IDC_IBEAM);
    c.hInstance=owner;c.lpszClassName=EDIT_CLASS;c.lpfnWndProc=initial;
    if(RegisterClassExW(&c))return TRUE;
    VirtualFree(thunk,0,MEM_RELEASE);thunk=NULL;return FALSE;
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
__declspec(dllexport) BOOL WINAPI FixtureUnregister(void)
{BOOL ok=UnregisterClassW(EDIT_CLASS,owner);if(thunk){ok=VirtualFree(thunk,0,MEM_RELEASE)&&ok;thunk=NULL;}return ok;}
BOOL WINAPI DllMain(HINSTANCE h,DWORD reason,LPVOID unused)
{(void)unused;if(reason==DLL_PROCESS_ATTACH)owner=h;return TRUE;}
