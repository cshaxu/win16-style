#define _WIN32_WINNT 0x0A00
#include <windows.h>
#include <commctrl.h>
#include <uxtheme.h>
static HINSTANCE instance;static int state=1;static BOOL fake,raise_paint,misplace,clip_button;static unsigned clicks;
static LRESULT CALLBACK parent_proc(HWND w,UINT m,WPARAM p,LPARAM l){return DefWindowProcW(w,m,p,l);}
static LRESULT CALLBACK child_proc(HWND w,UINT m,WPARAM p,LPARAM l){if(m==WM_PAINT){if(raise_paint){raise_paint=FALSE;RaiseException(0xe0425342u,0,0,NULL);}PAINTSTRUCT ps;HDC dc=BeginPaint(w,&ps);RECT r;GetClientRect(w,&r);FillRect(dc,&r,(HBRUSH)GetStockObject(WHITE_BRUSH));RECT button={r.right-17,0,r.right,r.bottom};if(misplace){button.left-=23;button.right-=23;}if(!fake){int saved=SaveDC(dc);if(clip_button)IntersectClipRect(dc,button.left,button.top,button.left+8,button.bottom);HTHEME theme=OpenThemeData(NULL,L"button");if(theme){DrawThemeBackground(theme,dc,1,state,&button,NULL);CloseThemeData(theme);}if(saved)RestoreDC(dc,saved);SetBkMode(dc,TRANSPARENT);SetTextColor(dc,RGB(0,0,0));TextOutW(dc,r.right-13,2,L"...",3);}EndPaint(w,&ps);return 0;}if(m==WM_LBUTTONUP)clicks++;return DefWindowProcW(w,m,p,l);}
__declspec(dllexport) BOOL WINAPI FixtureRegister(void){WNDCLASSEXW p={.cbSize=sizeof(p),.hInstance=instance,.lpfnWndProc=parent_proc,.lpszClassName=L"TValueListEditor"},c={.cbSize=sizeof(c),.hInstance=instance,.lpfnWndProc=child_proc,.lpszClassName=L"TInplaceEditList"};return RegisterClassExW(&p)&&RegisterClassExW(&c);}
__declspec(dllexport) HWND WINAPI FixtureParent(HWND parent){return CreateWindowW(L"TValueListEditor",L"",WS_CHILD|WS_VISIBLE,0,0,230,100,parent,NULL,instance,NULL);}
__declspec(dllexport) HWND WINAPI FixtureChild(HWND parent){return CreateWindowW(L"TInplaceEditList",L"",WS_CHILD|WS_VISIBLE|WS_TABSTOP,0,0,146,20,parent,(HMENU)1,instance,NULL);}
__declspec(dllexport) void WINAPI FixtureState(int value){state=value;}
__declspec(dllexport) void WINAPI FixtureClip(BOOL value){clip_button=value;}
__declspec(dllexport) void WINAPI FixtureMisplace(BOOL value){misplace=value;}
__declspec(dllexport) void WINAPI FixtureRaise(void){raise_paint=TRUE;}
__declspec(dllexport) void WINAPI FixtureFake(BOOL value){fake=value;}
__declspec(dllexport) unsigned WINAPI FixtureClicks(void){return clicks;}
__declspec(dllexport) BOOL WINAPI FixtureUnregister(void){return UnregisterClassW(L"TInplaceEditList",instance)&&UnregisterClassW(L"TValueListEditor",instance);}
BOOL WINAPI DllMain(HINSTANCE h,DWORD reason,void *reserved){(void)reserved;if(reason==DLL_PROCESS_ATTACH)instance=h;return TRUE;}
