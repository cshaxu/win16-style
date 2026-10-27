#define UNICODE
#include <windows.h>
#include <string.h>
/* Test-only DLL: its class is owned outside the EXE. The synthetic delayed
 * native initialization exercises creation-scope admission; it does not
 * claim to reproduce a Delphi runtime's actual creation order. */
static HINSTANCE owner;
static WNDPROC native_edit;
static BOOL fake,enabled;
static void *thunk;
static LRESULT CALLBACK dispatch(HWND w,UINT m,WPARAM p,LPARAM l)
{return (!enabled || (fake && m==WM_PAINT))?DefWindowProcW(w,m,p,l):CallWindowProcW(native_edit,w,m,p,l);}
static LRESULT CALLBACK initial(HWND w,UINT m,WPARAM p,LPARAM l)
{LRESULT result=dispatch(w,m,p,l);if(m==WM_NCCREATE)SetWindowLongPtrW(w,GWLP_WNDPROC,(LONG_PTR)thunk);return result;}
__declspec(dllexport) BOOL WINAPI FixtureRegister(BOOL impostor)
{
    WNDCLASSEXW n={.cbSize=sizeof(n)},c={.cbSize=sizeof(c)};DWORD old;
    if(!GetClassInfoExW(NULL,L"EDIT",&n))return FALSE;
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
    c.hInstance=owner;c.lpszClassName=L"TEdit";c.lpfnWndProc=initial;
    if(RegisterClassExW(&c))return TRUE;
    VirtualFree(thunk,0,MEM_RELEASE);thunk=NULL;return FALSE;
}
__declspec(dllexport) HWND WINAPI FixtureCreate(HWND parent,BOOL multiline)
{
    DWORD style=WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS|WS_TABSTOP|ES_AUTOHSCROLL|ES_AUTOVSCROLL;
    if(multiline)style|=ES_MULTILINE|WS_VSCROLL|WS_HSCROLL;
    HWND w=CreateWindowExW(WS_EX_CLIENTEDGE,L"TEdit",L"probe",style,20,20,220,multiline?70:28,parent,NULL,owner,NULL);
    if(w){CREATESTRUCTW cs={0};cs.hwndParent=parent;cs.hInstance=owner;cs.style=GetWindowLongPtrW(w,GWL_STYLE);
        cs.dwExStyle=GetWindowLongPtrW(w,GWL_EXSTYLE);cs.lpszClass=L"TEdit";cs.lpszName=L"probe";cs.cx=220;cs.cy=28;
        enabled=TRUE;CallWindowProcW(native_edit,w,WM_NCCREATE,0,(LPARAM)&cs);CallWindowProcW(native_edit,w,WM_CREATE,0,(LPARAM)&cs);}
    return w;
}
__declspec(dllexport) BOOL WINAPI FixtureUnregister(void)
{BOOL ok=UnregisterClassW(L"TEdit",owner);if(thunk){ok=VirtualFree(thunk,0,MEM_RELEASE)&&ok;thunk=NULL;}return ok;}
BOOL WINAPI DllMain(HINSTANCE h,DWORD reason,LPVOID unused)
{(void)unused;if(reason==DLL_PROCESS_ATTACH)owner=h;return TRUE;}
