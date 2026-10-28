/* Test-only runtime-owned model of a Data host, not Microsoft VB. */
#include <windows.h>
static HINSTANCE module;
typedef struct state {BOOL pressed;unsigned clicks,cancels;} state;
static void paint(HWND w,HDC dc,state *s){RECT r;GetClientRect(w,&r);FillRect(dc,&r,GetStockObject(WHITE_BRUSH));SetBkMode(dc,TRANSPARENT);SetTextColor(dc,IsWindowEnabled(w)?RGB(0,0,0):RGB(128,128,128));TextOutW(dc,6,4,s->pressed?L"< pressed >":L"< native >",10);}
static LRESULT CALLBACK data_proc(HWND w,UINT m,WPARAM a,LPARAM b){
 state *s=(void *)GetWindowLongPtrW(w,GWLP_USERDATA);
 if(m==WM_NCCREATE){s=HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,sizeof(*s));if(!s)return FALSE;SetWindowLongPtrW(w,GWLP_USERDATA,(LONG_PTR)s);}
 if(!s)return DefWindowProcW(w,m,a,b);
 if(m==WM_PAINT){PAINTSTRUCT p;HDC dc=BeginPaint(w,&p);paint(w,dc,s);EndPaint(w,&p);return 0;}
 if(m==WM_PRINTCLIENT){paint(w,(HDC)a,s);return 0;}
 if(m==WM_GETFONT)return (LRESULT)GetStockObject(DEFAULT_GUI_FONT);
 if(m==WM_LBUTTONDOWN){s->pressed=TRUE;SetCapture(w);InvalidateRect(w,NULL,TRUE);return 0;}
 if(m==WM_CANCELMODE){if(s->pressed){s->pressed=FALSE;++s->cancels;if(GetCapture()==w)ReleaseCapture();InvalidateRect(w,NULL,TRUE);}return 0;}
 if(m==WM_LBUTTONUP){if(s->pressed){s->pressed=FALSE;++s->clicks;if(GetCapture()==w)ReleaseCapture();SendMessageW(GetParent(w),WM_COMMAND,MAKEWPARAM(GetDlgCtrlID(w),BN_CLICKED),(LPARAM)w);InvalidateRect(w,NULL,TRUE);}return 0;}
 if(m==WM_NCDESTROY){LRESULT result=DefWindowProcW(w,m,a,b);SetWindowLongPtrW(w,GWLP_USERDATA,0);HeapFree(GetProcessHeap(),0,s);return result;}
 return DefWindowProcW(w,m,a,b);
}
__declspec(dllexport) BOOL WINAPI VbDataRegister(void){WNDCLASSW c={.hInstance=module,.lpfnWndProc=data_proc,.lpszClassName=L"ThunderRT6Data",.hbrBackground=GetStockObject(WHITE_BRUSH)};return RegisterClassW(&c)!=0;}
__declspec(dllexport) BOOL WINAPI VbDataInspect(HWND w,BOOL *pressed,unsigned *clicks,unsigned *cancels){state *s=(void *)GetWindowLongPtrW(w,GWLP_USERDATA);if(!s)return FALSE;*pressed=s->pressed;*clicks=s->clicks;*cancels=s->cancels;return TRUE;}
__declspec(dllexport) BOOL WINAPI VbDataRelease(void){return UnregisterClassW(L"ThunderRT6Data",module);}
BOOL WINAPI DllMain(HINSTANCE h,DWORD reason,LPVOID ignored){(void)ignored;if(reason==DLL_PROCESS_ATTACH)module=h;return TRUE;}
