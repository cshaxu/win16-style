#define UNICODE
#include <windows.h>
static HINSTANCE owner;
static LRESULT CALLBACK proc(HWND w,UINT m,WPARAM p,LPARAM l){if(m==WM_HSCROLL){SCROLLINFO s={.cbSize=sizeof(s),.fMask=SIF_POS};GetScrollInfo(w,SB_HORZ,&s);s.nPos+=LOWORD(p)==SB_LINERIGHT?1:LOWORD(p)==SB_LINELEFT?-1:0;SetScrollInfo(w,SB_HORZ,&s,TRUE);return 0;}if(m==WM_PAINT || m==WM_PRINTCLIENT){PAINTSTRUCT ps;HDC dc=m==WM_PAINT?BeginPaint(w,&ps):(HDC)p;RECT r={20,20,60,60};HBRUSH b=CreateSolidBrush(RGB(220,40,25));FillRect(dc,&r,b);DeleteObject(b);if(m==WM_PAINT)EndPaint(w,&ps);return 0;}return DefWindowProcW(w,m,p,l);}
__declspec(dllexport) BOOL WINAPI FixtureRegister(void){WNDCLASSW c={.hInstance=owner,.lpfnWndProc=proc,.lpszClassName=L"TStringGrid"};if(!RegisterClassW(&c))return FALSE;c.lpszClassName=L"TDrawGrid";return RegisterClassW(&c)!=0;}
__declspec(dllexport) HWND WINAPI FixtureCreate(HWND parent,BOOL draw,BOOL valid){DWORD style=WS_CHILD|WS_VISIBLE|WS_HSCROLL;HWND w=CreateWindowExW(valid?WS_EX_CLIENTEDGE:0,draw?L"TDrawGrid":L"TStringGrid",L"",style,10,10,220,120,parent,NULL,owner,NULL);if(w){SCROLLINFO i={.cbSize=sizeof(i),.fMask=SIF_RANGE|SIF_PAGE|SIF_POS,.nMin=0,.nMax=100,.nPage=10,.nPos=0};SetScrollInfo(w,SB_HORZ,&i,TRUE);}return w;}
__declspec(dllexport) BOOL WINAPI FixtureUnregister(void){return UnregisterClassW(L"TStringGrid",owner)&&UnregisterClassW(L"TDrawGrid",owner);}
BOOL WINAPI DllMain(HINSTANCE h,DWORD reason,LPVOID reserved){(void)reserved;if(reason==DLL_PROCESS_ATTACH)owner=h;return TRUE;}
