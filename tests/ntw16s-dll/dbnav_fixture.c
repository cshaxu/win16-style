#define _WIN32_WINNT 0x0A00
#include <windows.h>
#include <uxtheme.h>
__declspec(dllexport) HRESULT WINAPI FixtureDraw(HTHEME theme,HDC dc,int part,int state,const RECT *r,const RECT *clip){return DrawThemeBackground(theme,dc,part,state,r,clip);}

static HINSTANCE instance;static int media_state;
static LRESULT CALLBACK parent_proc(HWND w,UINT m,WPARAM p,LPARAM l){return DefWindowProcW(w,m,p,l);}
static LRESULT CALLBACK media_proc(HWND w,UINT m,WPARAM p,LPARAM l){if(m==WM_PAINT){PAINTSTRUCT ps;HDC dc=BeginPaint(w,&ps);HTHEME t=OpenThemeData(NULL,L"button");if(t){for(int i=0;i<4;i++){RECT r={i*50,0,(i+1)*50,24},glyph={i*50+20,8,i*50+28,16};int state=media_state?media_state:(i==0?1:i==1?3:i==2?4:5);DrawThemeBackground(t,dc,1,state,&r,NULL);FillRect(dc,&glyph,(HBRUSH)GetStockObject(BLACK_BRUSH));}CloseThemeData(t);}EndPaint(w,&ps);return 0;}return DefWindowProcW(w,m,p,l);}
__declspec(dllexport) void WINAPI MediaState(int state){media_state=state;}
__declspec(dllexport) BOOL WINAPI MediaRegister(void){WNDCLASSW c={0};c.hInstance=instance;c.lpfnWndProc=parent_proc;c.lpszClassName=L"MediaOwnerParent";if(!RegisterClassW(&c))return FALSE;c.lpfnWndProc=media_proc;c.lpszClassName=L"TMediaPlayer";return RegisterClassW(&c)!=0;}
__declspec(dllexport) HWND WINAPI MediaParent(HWND parent){return CreateWindowW(L"MediaOwnerParent",L"",WS_CHILD|WS_VISIBLE,0,0,220,40,parent,NULL,instance,NULL);}
__declspec(dllexport) HWND WINAPI MediaChild(HWND parent){return CreateWindowW(L"TMediaPlayer",L"",WS_CHILD|WS_VISIBLE,0,0,200,24,parent,NULL,instance,NULL);}
__declspec(dllexport) BOOL WINAPI MediaUnregister(void){return UnregisterClassW(L"TMediaPlayer",instance)&&UnregisterClassW(L"MediaOwnerParent",instance);}
BOOL WINAPI DllMain(HINSTANCE h,DWORD reason,void *reserved){(void)reserved;if(reason==DLL_PROCESS_ATTACH)instance=h;return TRUE;}
