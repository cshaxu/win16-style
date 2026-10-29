#include <windows.h>
static HINSTANCE mod;
static LRESULT CALLBACK proc(HWND w,UINT m,WPARAM p,LPARAM l){if(GetWindowLongPtrW(w,GWL_EXSTYLE)&WS_EX_MDICHILD)return DefMDIChildProcW(w,m,p,l);HWND client=FindWindowExW(w,NULL,L"MDIClient",NULL);return DefFrameProcW(w,client,m,p,l);}
__declspec(dllexport) BOOL __cdecl reg(void){WNDCLASSW c={.hInstance=mod,.lpfnWndProc=proc,.lpszClassName=L"TForm",.hbrBackground=(HBRUSH)(COLOR_WINDOW+1)};return RegisterClassW(&c)!=0;}
__declspec(dllexport) BOOL __cdecl unreg(void){return UnregisterClassW(L"TForm",mod);}
__declspec(dllexport) HWND __cdecl host(void){return CreateWindowW(L"TForm",L"Fixture host",WS_OVERLAPPEDWINDOW,10,10,600,400,NULL,NULL,mod,NULL);}
__declspec(dllexport) HWND __cdecl child(HWND client){return CreateMDIWindowW(L"TForm",L"Fixture child",WS_VISIBLE|WS_OVERLAPPEDWINDOW,10,10,250,180,client,mod,0);}
BOOL WINAPI DllMain(HINSTANCE h,DWORD reason,LPVOID p){(void)p;if(reason==DLL_PROCESS_ATTACH)mod=h;return TRUE;}
