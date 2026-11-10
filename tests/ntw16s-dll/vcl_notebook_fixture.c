#define UNICODE
#include <windows.h>
#include <commctrl.h>
#include "native_thunk.h"
#if defined(VCL_TAB_FIXTURE)
#define VCL_FIXTURE_CLASS L"TTabControl"
#elif defined(VCL_PAGE_FIXTURE)
#define VCL_FIXTURE_CLASS L"TPageControl"
#else
#define VCL_FIXTURE_CLASS L"TTabbedNotebook"
#endif
/* Test-only DLL: its class is owned outside the EXE. The synthetic delayed
 * native initialization exercises creation-scope admission; it does not
 * claim to reproduce a Delphi runtime's actual creation order. */
static HINSTANCE owner;
static WNDPROC native_tab;
static BOOL fake,enabled;
static void *thunk;
static LRESULT CALLBACK dispatch(HWND w,UINT m,WPARAM p,LPARAM l)
{return (!enabled || (fake && m==WM_PAINT))?DefWindowProcW(w,m,p,l):CallWindowProcW(native_tab,w,m,p,l);}
static LRESULT CALLBACK initial(HWND w,UINT m,WPARAM p,LPARAM l)
{LRESULT result=dispatch(w,m,p,l);if(m==WM_NCCREATE)SetWindowLongPtrW(w,GWLP_WNDPROC,(LONG_PTR)thunk);return result;}
__declspec(dllexport) BOOL WINAPI FixtureRegister(BOOL impostor)
{
    WNDCLASSEXW n={.cbSize=sizeof(n)},c={.cbSize=sizeof(c)};
    if(!GetClassInfoExW(NULL,WC_TABCONTROLW,&n))return FALSE;
    native_tab=n.lpfnWndProc;fake=impostor;enabled=FALSE;
    thunk=test_create_native_thunk((void *)dispatch);if(!thunk)return FALSE;
    c.style=CS_DBLCLKS|CS_HREDRAW|CS_VREDRAW;c.cbWndExtra=n.cbWndExtra;c.hCursor=LoadCursorW(NULL,IDC_IBEAM);
    c.hInstance=owner;c.lpszClassName=VCL_FIXTURE_CLASS;c.lpfnWndProc=initial;
    if(RegisterClassExW(&c))return TRUE;
    VirtualFree(thunk,0,MEM_RELEASE);thunk=NULL;return FALSE;
}
__declspec(dllexport) HWND WINAPI FixtureCreate(HWND parent,BOOL multiline)
{
    DWORD style=WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS|WS_TABSTOP;
    if(multiline)style|=TCS_OWNERDRAWFIXED;
    HWND w=CreateWindowExW(0,VCL_FIXTURE_CLASS,L"probe",style,20,20,220,120,parent,NULL,owner,NULL);
    if(w){CREATESTRUCTW cs={0};cs.hwndParent=parent;cs.hInstance=owner;cs.style=GetWindowLongPtrW(w,GWL_STYLE);
        cs.dwExStyle=GetWindowLongPtrW(w,GWL_EXSTYLE);cs.lpszClass=VCL_FIXTURE_CLASS;cs.lpszName=L"probe";cs.cx=220;cs.cy=120;
        enabled=TRUE;CallWindowProcW(native_tab,w,WM_NCCREATE,0,(LPARAM)&cs);CallWindowProcW(native_tab,w,WM_CREATE,0,(LPARAM)&cs);}
    if(w){TCITEMW item={.mask=TCIF_TEXT,.pszText=L"One"};SendMessageW(w,TCM_INSERTITEMW,0,(LPARAM)&item);item.pszText=L"Two";SendMessageW(w,TCM_INSERTITEMW,1,(LPARAM)&item);}return w;
}
__declspec(dllexport) BOOL WINAPI FixtureUnregister(void)
{BOOL ok=UnregisterClassW(VCL_FIXTURE_CLASS,owner);if(thunk){ok=VirtualFree(thunk,0,MEM_RELEASE)&&ok;thunk=NULL;}return ok;}
BOOL WINAPI DllMain(HINSTANCE h,DWORD reason,LPVOID unused)
{(void)unused;if(reason==DLL_PROCESS_ATTACH)owner=h;return TRUE;}
