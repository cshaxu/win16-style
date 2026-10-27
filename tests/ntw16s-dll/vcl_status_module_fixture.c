#define UNICODE
#include <windows.h>
#include <commctrl.h>
#include <string.h>
/* A DLL-owned superclass forwards native construction and painting without
 * depending on a Delphi installation; real framework evidence is separate. */
static HINSTANCE owner;
static WNDPROC native_status;
static BOOL fake,enabled;
static void *thunk;
static LRESULT CALLBACK dispatch(HWND w,UINT m,WPARAM p,LPARAM l)
{ return (!enabled || (fake && m==WM_PAINT))?DefWindowProcW(w,m,p,l):CallWindowProcW(native_status,w,m,p,l);}
static LRESULT CALLBACK initial(HWND w,UINT m,WPARAM p,LPARAM l)
{LRESULT result=dispatch(w,m,p,l);if(m==WM_NCCREATE)SetWindowLongPtrW(w,GWLP_WNDPROC,(LONG_PTR)thunk);return result;}
__declspec(dllexport) BOOL WINAPI FixtureRegister(BOOL impostor)
{
    INITCOMMONCONTROLSEX icc={sizeof(icc),ICC_BAR_CLASSES};WNDCLASSEXW n={.cbSize=sizeof(n)},c={.cbSize=sizeof(c)};DWORD old;
    if(!InitCommonControlsEx(&icc) || !GetClassInfoExW(NULL,STATUSCLASSNAMEW,&n))return FALSE;
    native_status=n.lpfnWndProc;fake=impostor;enabled=TRUE;
    thunk=VirtualAlloc(NULL,4096,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);if(!thunk)return FALSE;
#ifdef _WIN64
    unsigned char code[]={0x48,0xb8,0,0,0,0,0,0,0,0,0xff,0xe0};void *target=(void*)dispatch;memcpy(code+2,&target,sizeof(target));
#else
    unsigned char code[]={0xb8,0,0,0,0,0xff,0xe0};void *target=(void*)dispatch;memcpy(code+1,&target,sizeof(target));
#endif
    memcpy(thunk,code,sizeof(code));
    if(!VirtualProtect(thunk,4096,PAGE_EXECUTE_READ,&old)){VirtualFree(thunk,0,MEM_RELEASE);thunk=NULL;return FALSE;}
    FlushInstructionCache(GetCurrentProcess(),thunk,sizeof(code));
    c.style=CS_DBLCLKS|CS_HREDRAW|CS_VREDRAW;c.cbWndExtra=n.cbWndExtra;c.hCursor=LoadCursorW(NULL,IDC_ARROW);
    c.hInstance=owner;c.lpszClassName=L"TStatusBar";c.lpfnWndProc=initial;
    if(RegisterClassExW(&c))return TRUE;
    VirtualFree(thunk,0,MEM_RELEASE);thunk=NULL;return FALSE;
}
__declspec(dllexport) HWND WINAPI FixtureCreate(HWND parent,BOOL ignored)
{(void)ignored;return CreateWindowW(L"TStatusBar",L"",WS_CHILD|WS_VISIBLE|CCS_BOTTOM,0,0,400,30,parent,(HMENU)(INT_PTR)9,owner,NULL);}
__declspec(dllexport) HWND WINAPI FixtureCreateGrip(HWND parent,BOOL ignored)
{(void)ignored;return CreateWindowW(L"TStatusBar",L"",WS_CHILD|WS_VISIBLE|CCS_BOTTOM|SBARS_SIZEGRIP,0,0,400,30,parent,(HMENU)(INT_PTR)9,owner,NULL);}
__declspec(dllexport) BOOL WINAPI FixtureUnregister(void)
{BOOL ok=UnregisterClassW(L"TStatusBar",owner);if(thunk){ok=VirtualFree(thunk,0,MEM_RELEASE)&&ok;thunk=NULL;}return ok;}
BOOL WINAPI DllMain(HINSTANCE h,DWORD reason,LPVOID unused)
{(void)unused;if(reason==DLL_PROCESS_ATTACH)owner=h;return TRUE;}
