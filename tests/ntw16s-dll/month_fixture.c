#define _WIN32_WINNT 0x0A00
#include <windows.h>
#include <commctrl.h>
static HINSTANCE instance;static WNDPROC native_proc;static BOOL fake,raise_paint;static HWND switch_parent,query_parent;static int query_mode;typedef LRESULT (WINAPI *forwardfn)(WNDPROC,HWND,UINT,WPARAM,LPARAM);static forwardfn foreign_forward;
static LRESULT CALLBACK parent_proc(HWND w,UINT m,WPARAM p,LPARAM l){return DefWindowProcW(w,m,p,l);}
static LRESULT CALLBACK hotkey_proc(HWND w,UINT m,WPARAM p,LPARAM l){if(m==MCM_GETCALENDARCOUNT && query_mode){int action=query_mode;query_mode=0;if(action==1)SetParent(w,query_parent);else if(action==2)RaiseException(0xe0425342u,0,0,NULL);else if(action==3)SendMessageW(w,WM_PAINT,0,0);}if(fake&&m==WM_PAINT)return DefWindowProcW(w,m,p,l);if(foreign_forward&&m==WM_PAINT)return foreign_forward(native_proc,w,m,p,l);LRESULT result=CallWindowProcW(native_proc,w,m,p,l);if(switch_parent&&m==WM_PAINT){HWND parent=switch_parent;switch_parent=NULL;SetParent(w,parent);}if(raise_paint&&m==WM_PAINT){raise_paint=FALSE;RaiseException(0xe0425342u,0,0,NULL);}return result;}
__declspec(dllexport) void WINAPI FixtureQuery(int mode,HWND parent){query_mode=mode;query_parent=parent;}
__declspec(dllexport) void WINAPI FixtureChangeParent(HWND p){switch_parent=p;}
__declspec(dllexport) void WINAPI FixtureFake(BOOL value){fake=value;}
__declspec(dllexport) void WINAPI FixtureForward(forwardfn f){foreign_forward=f;}
__declspec(dllexport) void WINAPI FixtureRaise(void){raise_paint=TRUE;}
__declspec(dllexport) BOOL WINAPI FixtureRegister(void){INITCOMMONCONTROLSEX init={.dwSize=sizeof(init),.dwICC=ICC_DATE_CLASSES};WNDCLASSEXW r={.cbSize=sizeof(r)},p={.cbSize=sizeof(p)};if(!InitCommonControlsEx(&init)||!GetClassInfoExW(GetModuleHandleW(L"comctl32.dll"),L"SysMonthCal32",&r))return FALSE;native_proc=r.lpfnWndProc;r.hInstance=instance;r.lpszClassName=L"TMonthCalendar";r.lpfnWndProc=hotkey_proc;p.hInstance=instance;p.lpszClassName=L"MonthFixtureParent";p.lpfnWndProc=parent_proc;return RegisterClassExW(&p)&&RegisterClassExW(&r);}
__declspec(dllexport) HWND WINAPI FixtureParent(HWND parent){return CreateWindowExW(0,L"MonthFixtureParent",L"",WS_CHILD|WS_VISIBLE,0,0,400,150,parent,NULL,instance,NULL);}
__declspec(dllexport) HWND WINAPI FixtureCreate(HWND parent,BOOL impostor){fake=impostor;return CreateWindowExW(0,L"TMonthCalendar",L"",WS_CHILD|WS_VISIBLE|WS_TABSTOP,0,0,225,187,parent,(HMENU)7,instance,NULL);}
__declspec(dllexport) HWND WINAPI FixtureMulti(HWND parent){fake=FALSE;return CreateWindowExW(0,L"TMonthCalendar",L"",WS_CHILD|WS_VISIBLE|MCS_MULTISELECT,0,0,225,187,parent,(HMENU)8,instance,NULL);}
__declspec(dllexport) BOOL WINAPI FixtureUnregister(void){return UnregisterClassW(L"TMonthCalendar",instance)&&UnregisterClassW(L"MonthFixtureParent",instance);}
BOOL WINAPI DllMain(HINSTANCE h,DWORD reason,LPVOID reserved){(void)reserved;if(reason==DLL_PROCESS_ATTACH)instance=h;return TRUE;}
