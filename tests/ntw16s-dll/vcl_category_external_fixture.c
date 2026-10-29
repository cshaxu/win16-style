#define _WIN32_WINNT 0x0A00
#include <windows.h>
static HINSTANCE inst;
static LRESULT CALLBACK proc(HWND w,UINT m,WPARAM p,LPARAM l){WCHAR name[80];if(m==WM_PAINT && GetClassNameW(w,name,80) && !lstrcmpW(name,L"TCategoryButtons")){PAINTSTRUCT ps;HDC dc=BeginPaint(w,&ps);RECT r;GetClientRect(w,&r);SetDCBrushColor(dc,RGB(15,145,88));FillRect(dc,&r,GetStockObject(DC_BRUSH));EndPaint(w,&ps);return 0;}return DefWindowProcW(w,m,p,l);}
BOOL WINAPI DllMain(HINSTANCE h,DWORD reason,LPVOID r){(void)r;if(reason==DLL_PROCESS_ATTACH)inst=h;return TRUE;}
__declspec(dllexport) BOOL WINAPI FixtureRegister(void){WNDCLASSW wc={0};wc.hInstance=inst;wc.lpfnWndProc=proc;wc.lpszClassName=L"CategoryOwnedParent";if(!RegisterClassW(&wc))return FALSE;wc.lpszClassName=L"TCategoryButtons";return RegisterClassW(&wc)!=0;}
__declspec(dllexport) BOOL WINAPI FixtureUnregister(void){return UnregisterClassW(L"TCategoryButtons",inst)&&UnregisterClassW(L"CategoryOwnedParent",inst);}
