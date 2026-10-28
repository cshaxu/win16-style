#define _WIN32_WINNT 0x0A00
#include <windows.h>
typedef void (*other_t)(HDC,int,int);static other_t other;
static HINSTANCE instance;static int mode;static HWND switch_parent;static BOOL reenter;
static void draw(HWND w,HDC dc){RECT r;GetClientRect(w,&r);POINT a[3]={{0,r.bottom-1},{0,0},{r.right-1,0}},b[3]={{r.right-1,0},{r.right-1,r.bottom-1},{-1,r.bottom-1}};
 if(mode==0){return;} if(mode==4 && other){other(dc,r.right,r.bottom);return;} HPEN first=CreatePen(PS_SOLID,1,GetSysColor(COLOR_BTNHIGHLIGHT)),second=CreatePen(PS_SOLID,1,mode==2?RGB(15,24,33):GetSysColor(COLOR_BTNSHADOW));HPEN old=SelectObject(dc,first);Polyline(dc,a,3);
 if(reenter){reenter=FALSE;InvalidateRect(w,NULL,TRUE);SendMessageW(w,WM_PAINT,0,0);}if(switch_parent){HWND next=switch_parent;switch_parent=NULL;SetParent(w,next);}SelectObject(dc,second);if(mode!=3)Polyline(dc,b,3);SelectObject(dc,old);DeleteObject(first);DeleteObject(second);}
static LRESULT CALLBACK proc(HWND w,UINT m,WPARAM p,LPARAM l){if(m==WM_PAINT){PAINTSTRUCT ps;HDC dc=BeginPaint(w,&ps);draw(w,dc);EndPaint(w,&ps);return 0;}return DefWindowProcW(w,m,p,l);}
__declspec(dllexport) BOOL WINAPI Reg(void){WNDCLASSW c={.hInstance=instance,.lpfnWndProc=DefWindowProcW,.lpszClassName=L"PanelFixtureParent"};if(!RegisterClassW(&c))return FALSE;c.lpfnWndProc=proc;c.lpszClassName=L"TPanel";return RegisterClassW(&c)!=0;}
__declspec(dllexport) HWND WINAPI Parent(void){return CreateWindowW(L"PanelFixtureParent",L"",WS_OVERLAPPEDWINDOW,10,10,400,300,NULL,NULL,instance,NULL);}
__declspec(dllexport) HWND WINAPI Panel(HWND p){return CreateWindowW(L"TPanel",L"",WS_CHILD,5,5,250,120,p,NULL,instance,NULL);}
__declspec(dllexport) void WINAPI Mode(int v){mode=v;}
__declspec(dllexport) void WINAPI Switch(HWND p){switch_parent=p;}
__declspec(dllexport) void WINAPI SetOther(other_t callback){other=callback;}
__declspec(dllexport) void WINAPI Reenter(BOOL yes){reenter=yes;}
__declspec(dllexport) void WINAPI Outside(HWND w){HDC dc=GetDC(w);draw(w,dc);ReleaseDC(w,dc);}
__declspec(dllexport) BOOL WINAPI Unreg(void){return UnregisterClassW(L"TPanel",instance)&&UnregisterClassW(L"PanelFixtureParent",instance);}
BOOL WINAPI DllMain(HINSTANCE h,DWORD m,void *p){(void)p;if(m==DLL_PROCESS_ATTACH)instance=h;return TRUE;}
