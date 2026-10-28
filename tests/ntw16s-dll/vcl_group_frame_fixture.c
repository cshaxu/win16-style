#define UNICODE
#include <windows.h>
#include <uxtheme.h>
static HINSTANCE instance; static BOOL background=TRUE,armed;static HWND reparent_target;
static void draw(HWND w,HDC dc){RECT c,t;GetClientRect(w,&c);RECT border={0,12,c.right,c.bottom};RECT title={8,0,87,25};WCHAR str[100];int n=GetWindowTextW(w,str,100);HTHEME theme=OpenThemeData(w,L"BUTTON");if(!theme)return;if(background){if(!n)border.top=0;DrawThemeBackground(theme,dc,4,1,&border,NULL);if(reparent_target)armed=TRUE;}if(n){t=title;DrawThemeTextEx(theme,dc,4,1,str,n,0,&t,NULL);}CloseThemeData(theme);}
static LRESULT CALLBACK proc(HWND w,UINT m,WPARAM p,LPARAM l){if(m==WM_GETTEXT && armed){armed=FALSE;HWND target=reparent_target;reparent_target=NULL;SetParent(w,target);}if(m==WM_PAINT){PAINTSTRUCT ps;HDC dc=BeginPaint(w,&ps);draw(w,dc);EndPaint(w,&ps);return 0;}return DefWindowProcW(w,m,p,l);}
__declspec(dllexport) BOOL WINAPI RegisterFixture(void){WNDCLASSW c={0};c.hInstance=instance;c.lpfnWndProc=DefWindowProcW;c.lpszClassName=L"GroupFixtureParent";if(!RegisterClassW(&c))return FALSE;c.lpfnWndProc=proc;c.lpszClassName=L"TGroupBox";return RegisterClassW(&c)!=0;}
__declspec(dllexport) HWND WINAPI MakeParent(void){return CreateWindowW(L"GroupFixtureParent",L"parent",WS_OVERLAPPEDWINDOW|WS_VISIBLE,10,10,350,200,NULL,NULL,instance,NULL);}
__declspec(dllexport) HWND WINAPI MakeGroup(HWND p){return CreateWindowW(L"TGroupBox",L"GroupBox",WS_CHILD|WS_VISIBLE,5,5,250,120,p,NULL,instance,NULL);}
__declspec(dllexport) void WINAPI SetBackground(BOOL v){background=v;}
__declspec(dllexport) void WINAPI ReparentDuringText(HWND parent){reparent_target=parent;armed=FALSE;}
__declspec(dllexport) void WINAPI OutsidePaint(HWND w){HDC dc=GetDC(w);BOOL saved=background;background=TRUE;draw(w,dc);background=saved;ReleaseDC(w,dc);}
__declspec(dllexport) BOOL WINAPI ReleaseFixture(void){return UnregisterClassW(L"TGroupBox",instance)&&UnregisterClassW(L"GroupFixtureParent",instance);}
BOOL WINAPI DllMain(HINSTANCE h,DWORD reason,LPVOID p){(void)p;if(reason==DLL_PROCESS_ATTACH)instance=h;return TRUE;}
