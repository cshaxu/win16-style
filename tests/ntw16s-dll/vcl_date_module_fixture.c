#define UNICODE
#include <windows.h>
#include <commctrl.h>
#include <string.h>
/* A DLL-owned superclass forwards native construction and painting without
 * depending on a Delphi installation; real framework evidence is separate. */
static HINSTANCE owner;
static WNDPROC native_date;
static BOOL fake,enabled,raise_print,reenter_info;
static void *thunk;
static LRESULT CALLBACK dispatch(HWND w,UINT m,WPARAM p,LPARAM l)
{if(raise_print && m==WM_PRINTCLIENT){raise_print=FALSE;RaiseException(0xe0425342u,0,0,NULL);}
 if(reenter_info && m==DTM_GETDATETIMEPICKERINFO){reenter_info=FALSE;SendMessageW(GetParent(w),WM_APP+42,0,(LPARAM)w);}
 return (!enabled || (fake && m==WM_PAINT))?DefWindowProcW(w,m,p,l):CallWindowProcW(native_date,w,m,p,l);}
static LRESULT CALLBACK initial(HWND w,UINT m,WPARAM p,LPARAM l)
{LRESULT result=dispatch(w,m,p,l);if(m==WM_NCCREATE)SetWindowLongPtrW(w,GWLP_WNDPROC,(LONG_PTR)thunk);return result;}
__declspec(dllexport) BOOL WINAPI FixtureRegister(BOOL impostor)
{
    INITCOMMONCONTROLSEX icc={sizeof(icc),ICC_DATE_CLASSES};WNDCLASSEXW n={.cbSize=sizeof(n)},c={.cbSize=sizeof(c)};DWORD old;
    if(!InitCommonControlsEx(&icc) || !GetClassInfoExW(NULL,DATETIMEPICK_CLASSW,&n))return FALSE;
    native_date=n.lpfnWndProc;fake=impostor;enabled=TRUE;
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
    c.hInstance=owner;c.lpszClassName=L"TDateTimePicker";c.lpfnWndProc=initial;
    if(RegisterClassExW(&c))return TRUE;
    VirtualFree(thunk,0,MEM_RELEASE);thunk=NULL;return FALSE;
}
__declspec(dllexport) HWND WINAPI FixtureCreate(HWND parent,BOOL checked)
{
    DWORD style=WS_CHILD|WS_VISIBLE|WS_TABSTOP|DTS_SHORTDATEFORMAT;
    if(checked)style|=DTS_SHOWNONE;
    return CreateWindowExW(0,L"TDateTimePicker",L"",style,20,20,220,24,parent,NULL,owner,NULL);
}
__declspec(dllexport) void WINAPI FixtureRaiseNextPrint(void)
{raise_print=TRUE;}
__declspec(dllexport) void WINAPI FixtureReenterInfo(void)
{reenter_info=TRUE;}
__declspec(dllexport) BOOL WINAPI FixtureUnregister(void)
{BOOL ok=UnregisterClassW(L"TDateTimePicker",owner);if(thunk){ok=VirtualFree(thunk,0,MEM_RELEASE)&&ok;thunk=NULL;}return ok;}
BOOL WINAPI DllMain(HINSTANCE h,DWORD reason,LPVOID unused)
{(void)unused;if(reason==DLL_PROCESS_ATTACH)owner=h;return TRUE;}
