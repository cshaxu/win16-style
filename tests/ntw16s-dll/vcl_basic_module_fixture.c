#define UNICODE
#include <windows.h>
#include <commctrl.h>
#include "native_thunk.h"
/* A DLL-owned superclass forwards native construction and painting without
 * depending on a Delphi installation; real framework evidence is separate. */
#if defined(VCL_DATE_FIXTURE)
#define FIXTURE_CLASS L"TDateTimePicker"
#define NATIVE_CLASS DATETIMEPICK_CLASSW
#define FIXTURE_ICC ICC_DATE_CLASSES
#elif defined(VCL_STATUS_FIXTURE)
#define FIXTURE_CLASS L"TStatusBar"
#define NATIVE_CLASS STATUSCLASSNAMEW
#define FIXTURE_ICC ICC_BAR_CLASSES
#elif defined(VCL_TRACK_FIXTURE)
#define FIXTURE_CLASS L"TTrackBar"
#define NATIVE_CLASS TRACKBAR_CLASSW
#define FIXTURE_ICC ICC_BAR_CLASSES
#elif defined(VCL_DRIVE_FIXTURE)
#define FIXTURE_CLASS L"TDriveComboBox"
#define NATIVE_CLASS L"COMBOBOX"
#else
#define FIXTURE_CLASS L"TColorBox"
#define NATIVE_CLASS L"COMBOBOX"
#endif
static HINSTANCE owner;
static WNDPROC native_proc;
static BOOL fake,enabled;
#ifdef VCL_DATE_FIXTURE
static BOOL raise_print,reenter_info;
#endif
static void *thunk;
static LRESULT CALLBACK dispatch(HWND w,UINT m,WPARAM p,LPARAM l)
{
#ifdef VCL_DATE_FIXTURE
    if(raise_print && m==WM_PRINTCLIENT){raise_print=FALSE;RaiseException(0xe0425342u,0,0,NULL);}
    if(reenter_info && m==DTM_GETDATETIMEPICKERINFO){reenter_info=FALSE;SendMessageW(GetParent(w),WM_APP+42,0,(LPARAM)w);}
#endif
    return (!enabled || (fake && m==WM_PAINT))?DefWindowProcW(w,m,p,l):CallWindowProcW(native_proc,w,m,p,l);
}
static LRESULT CALLBACK initial(HWND w,UINT m,WPARAM p,LPARAM l)
{LRESULT result=dispatch(w,m,p,l);if(m==WM_NCCREATE)SetWindowLongPtrW(w,GWLP_WNDPROC,(LONG_PTR)thunk);return result;}
__declspec(dllexport) BOOL WINAPI FixtureRegister(BOOL impostor)
{
    WNDCLASSEXW n={.cbSize=sizeof(n)},c={.cbSize=sizeof(c)};
#ifdef FIXTURE_ICC
    INITCOMMONCONTROLSEX icc={sizeof(icc),FIXTURE_ICC};
    if(!InitCommonControlsEx(&icc))return FALSE;
#endif
    if(!GetClassInfoExW(NULL,NATIVE_CLASS,&n))return FALSE;
    native_proc=n.lpfnWndProc;fake=impostor;enabled=TRUE;
    thunk=test_create_native_thunk((void *)dispatch);if(!thunk)return FALSE;
    c.style=CS_DBLCLKS|CS_HREDRAW|CS_VREDRAW;c.cbWndExtra=n.cbWndExtra;c.hCursor=LoadCursorW(NULL,IDC_ARROW);
    c.hInstance=owner;c.lpszClassName=FIXTURE_CLASS;c.lpfnWndProc=initial;
    if(RegisterClassExW(&c))return TRUE;
    VirtualFree(thunk,0,MEM_RELEASE);thunk=NULL;return FALSE;
}
__declspec(dllexport) HWND WINAPI FixtureCreate(HWND parent,BOOL option)
{
#if defined(VCL_DATE_FIXTURE)
    DWORD style=WS_CHILD|WS_VISIBLE|WS_TABSTOP|DTS_SHORTDATEFORMAT;
    if(option)style|=DTS_SHOWNONE;
    return CreateWindowExW(0,FIXTURE_CLASS,L"",style,20,20,220,24,parent,NULL,owner,NULL);
#elif defined(VCL_STATUS_FIXTURE)
    (void)option;
    return CreateWindowW(FIXTURE_CLASS,L"",WS_CHILD|WS_VISIBLE|CCS_BOTTOM,0,0,400,30,parent,(HMENU)(INT_PTR)9,owner,NULL);
#elif defined(VCL_TRACK_FIXTURE)
    (void)option;
    DWORD style=WS_CHILD|WS_VISIBLE|WS_TABSTOP|TBS_AUTOTICKS|TBS_ENABLESELRANGE;
    return CreateWindowExW(0,FIXTURE_CLASS,L"",style,20,20,220,42,parent,NULL,owner,NULL);
#else
    (void)option;
    DWORD style=WS_CHILD|WS_VISIBLE|WS_TABSTOP|CBS_DROPDOWNLIST|CBS_OWNERDRAWFIXED|CBS_HASSTRINGS|CBS_AUTOHSCROLL;
    return CreateWindowExW(0,FIXTURE_CLASS,L"",style,20,20,220,180,parent,NULL,owner,NULL);
#endif
}
#ifdef VCL_DATE_FIXTURE
__declspec(dllexport) void WINAPI FixtureRaiseNextPrint(void){raise_print=TRUE;}
__declspec(dllexport) void WINAPI FixtureReenterInfo(void){reenter_info=TRUE;}
#endif
#ifdef VCL_STATUS_FIXTURE
__declspec(dllexport) HWND WINAPI FixtureCreateGrip(HWND parent,BOOL ignored)
{(void)ignored;return CreateWindowW(FIXTURE_CLASS,L"",WS_CHILD|WS_VISIBLE|CCS_BOTTOM|SBARS_SIZEGRIP,0,0,400,30,parent,(HMENU)(INT_PTR)9,owner,NULL);}
#endif
__declspec(dllexport) BOOL WINAPI FixtureUnregister(void)
{BOOL ok=UnregisterClassW(FIXTURE_CLASS,owner);if(thunk){ok=VirtualFree(thunk,0,MEM_RELEASE)&&ok;thunk=NULL;}return ok;}
BOOL WINAPI DllMain(HINSTANCE h,DWORD reason,LPVOID unused)
{(void)unused;if(reason==DLL_PROCESS_ATTACH)owner=h;return TRUE;}
