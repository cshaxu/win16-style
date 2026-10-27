#define UNICODE
#include <windows.h>
#include <commctrl.h>
#include <uxtheme.h>
static HINSTANCE owner;
static WNDPROC native_button;
static BOOL trigger_retire,trigger_raise;
static LRESULT CALLBACK window_proc(HWND w,UINT m,WPARAM p,LPARAM l){return CallWindowProcW(native_button,w,m,p,l);}
__declspec(dllexport) BOOL WINAPI FixtureRegister(void)
{WNDCLASSEXW native={.cbSize=sizeof(native)};if(!GetClassInfoExW(NULL,L"BUTTON",&native))return FALSE;native_button=native.lpfnWndProc;native.hInstance=owner;native.lpszClassName=L"TBitBtn";native.lpfnWndProc=window_proc;return RegisterClassExW(&native)!=0;}
__declspec(dllexport) HWND WINAPI FixtureCreate(HWND parent)
{return CreateWindowW(L"TBitBtn",L"Fixture",WS_CHILD|WS_VISIBLE|WS_TABSTOP|BS_OWNERDRAW,20,20,180,40,parent,(HMENU)(INT_PTR)17,owner,NULL);}
__declspec(dllexport) void WINAPI FixtureRetireNext(void){trigger_retire=TRUE;}
__declspec(dllexport) void WINAPI FixtureRaiseNext(void){trigger_raise=TRUE;}
__declspec(dllexport) BOOL WINAPI FixtureDraw(const DRAWITEMSTRUCT *item)
{if(trigger_raise){trigger_raise=FALSE;RaiseException(0xe0425342u,0,0,NULL);}if(trigger_retire){trigger_retire=FALSE;SendMessageW(GetParent(item->hwndItem),WM_APP+17,0,(LPARAM)item->hwndItem);}HTHEME theme=OpenThemeData(item->hwndItem,L"BUTTON");if(!theme)return FALSE;HRESULT hr=DrawThemeBackground(theme,item->hDC,1,(item->itemState&ODS_SELECTED)?3:1,&item->rcItem,NULL);CloseThemeData(theme);if(FAILED(hr))return FALSE;HBRUSH brush=CreateSolidBrush(RGB(220,40,25));RECT r={40,10,50,20};FillRect(item->hDC,&r,brush);DeleteObject(brush);return TRUE;}
__declspec(dllexport) BOOL WINAPI FixtureUnregister(void){return UnregisterClassW(L"TBitBtn",owner);}
BOOL WINAPI DllMain(HINSTANCE h,DWORD reason,LPVOID unused){(void)unused;if(reason==DLL_PROCESS_ATTACH)owner=h;return TRUE;}
