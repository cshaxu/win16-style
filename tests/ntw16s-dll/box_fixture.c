#define _WIN32_WINNT 0x0A00
#include <windows.h>
static HINSTANCE instance;static BOOL fake,raise_paint;static HWND switch_parent;typedef LRESULT(WINAPI *forwardfn)(HWND,UINT,WPARAM,LPARAM);static forwardfn foreign;
static LRESULT CALLBACK parent_proc(HWND w,UINT m,WPARAM p,LPARAM l){return DefWindowProcW(w,m,p,l);}
static LRESULT CALLBACK box_proc(HWND w,UINT m,WPARAM p,LPARAM l){if(m==WM_NCPAINT){if(switch_parent){HWND next=switch_parent;switch_parent=NULL;SetParent(w,next);}if(raise_paint){raise_paint=FALSE;RaiseException(0xe0425342u,0,0,NULL);}if(fake)return 0;if(foreign)return foreign(w,m,p,l);}if(m==WM_PAINT){PAINTSTRUCT ps;HDC dc=BeginPaint(w,&ps);FillRect(dc,&ps.rcPaint,GetStockObject(WHITE_BRUSH));EndPaint(w,&ps);return 0;}return DefWindowProcW(w,m,p,l);}
__declspec(dllexport) void WINAPI FixtureChangeParent(HWND p){switch_parent=p;}
__declspec(dllexport) void WINAPI FixtureFake(BOOL value){fake=value;}
__declspec(dllexport) void WINAPI FixtureForward(forwardfn f){foreign=f;}
__declspec(dllexport) void WINAPI FixtureRaise(void){raise_paint=TRUE;}
__declspec(dllexport) BOOL WINAPI FixtureRegister(void){WNDCLASSEXW b={.cbSize=sizeof(b)},p={.cbSize=sizeof(p)};b.hInstance=instance;b.lpszClassName=L"TScrollBox";b.lpfnWndProc=box_proc;b.hbrBackground=GetStockObject(WHITE_BRUSH);p.hInstance=instance;p.lpszClassName=L"BoxFixtureParent";p.lpfnWndProc=parent_proc;return RegisterClassExW(&p)&&RegisterClassExW(&b);}
__declspec(dllexport) HWND WINAPI FixtureParent(HWND parent){return CreateWindowExW(0,L"BoxFixtureParent",L"",WS_CHILD|WS_VISIBLE,0,0,400,200,parent,NULL,instance,NULL);}
__declspec(dllexport) HWND WINAPI FixtureCreate(HWND parent,BOOL impostor){fake=impostor;HWND w=CreateWindowExW(WS_EX_CLIENTEDGE,L"TScrollBox",L"",WS_CHILD|WS_VISIBLE|WS_HSCROLL|WS_VSCROLL,0,0,270,100,parent,(HMENU)7,instance,NULL);if(w){SCROLLINFO s={.cbSize=sizeof(s),.fMask=SIF_RANGE|SIF_PAGE|SIF_POS,.nMax=1000,.nPage=100,.nPos=0};SetScrollInfo(w,SB_HORZ,&s,TRUE);SetScrollInfo(w,SB_VERT,&s,TRUE);}return w;}
__declspec(dllexport) BOOL WINAPI FixtureUnregister(void){BOOL a=UnregisterClassW(L"TScrollBox",instance),b=UnregisterClassW(L"BoxFixtureParent",instance);return a&&b;}
BOOL WINAPI DllMain(HINSTANCE h,DWORD reason,LPVOID reserved){(void)reserved;if(reason==DLL_PROCESS_ATTACH)instance=h;return TRUE;}
