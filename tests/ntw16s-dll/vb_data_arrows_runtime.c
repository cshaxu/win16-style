/* Self-owned runtime model. This is not Microsoft VB or a DAO substitute. */
#include <windows.h>
static HINSTANCE module;
typedef struct state {int pressed;unsigned clicks,cancels;BOOL clipped;} state;
static void paint(HWND w,HDC dc,state *s)
{
 RECT r;GetClientRect(w,&r);int saved=SaveDC(dc);if(s->clipped)IntersectClipRect(dc,0,0,9,r.bottom);FillRect(dc,&r,GetStockObject(WHITE_BRUSH));
 HDC mem=CreateCompatibleDC(dc);if(!mem)return;
 const int xs[4]={4,24,r.right-33,r.right-15};
 for(int n=0;n<4;n++){
  int width=n==0 || n==3?11:8;HBITMAP bitmap=CreateCompatibleBitmap(dc,width,13);if(!bitmap)continue;
  HGDIOBJ old=SelectObject(mem,bitmap);RECT fill={0,0,width,13};FillRect(mem,&fill,GetStockObject(WHITE_BRUSH));
  /* Native first/last marker: exactly two columns of eleven black pixels. */
  if(n==0 || n==3){RECT mark={n==0?1:8,1,n==0?3:10,12};FillRect(mem,&mark,GetStockObject(BLACK_BRUSH));}
  else {RECT mark={2,3,6,10};FillRect(mem,&mark,GetStockObject(BLACK_BRUSH));}
  int delta=s->pressed==n?1:0;StretchBlt(dc,xs[n]+delta,4+delta,width,13,mem,0,0,width,13,SRCCOPY);
  SelectObject(mem,old);DeleteObject(bitmap);
 }
 DeleteDC(mem);if(saved)RestoreDC(dc,saved);
}
static LRESULT CALLBACK proc(HWND w,UINT m,WPARAM a,LPARAM b)
{
 state *s=(void*)GetWindowLongPtrW(w,GWLP_USERDATA);
 if(m==WM_NCCREATE){s=HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,sizeof(*s));if(!s)return FALSE;s->pressed=-1;SetWindowLongPtrW(w,GWLP_USERDATA,(LONG_PTR)s);}
 if(!s)return DefWindowProcW(w,m,a,b);
 if(m==WM_PAINT){PAINTSTRUCT ps;HDC dc=BeginPaint(w,&ps);paint(w,dc,s);EndPaint(w,&ps);return 0;}
 if(m==WM_GETFONT)return (LRESULT)GetStockObject(DEFAULT_GUI_FONT);
 if(m==WM_APP+91){s->clipped=a!=0;return 0;}
 if(m==WM_LBUTTONDOWN){RECT r;GetClientRect(w,&r);int x=(short)LOWORD(b);s->pressed=x<18?0:x<35?1:x>=r.right-18?3:2;SetCapture(w);InvalidateRect(w,NULL,TRUE);return 0;}
 if(m==WM_CANCELMODE){if(s->pressed>=0){s->pressed=-1;++s->cancels;if(GetCapture()==w)ReleaseCapture();InvalidateRect(w,NULL,TRUE);}return 0;}
 if(m==WM_LBUTTONUP){if(s->pressed>=0){s->pressed=-1;++s->clicks;if(GetCapture()==w)ReleaseCapture();SendMessageW(GetParent(w),WM_COMMAND,MAKEWPARAM(GetDlgCtrlID(w),BN_CLICKED),(LPARAM)w);InvalidateRect(w,NULL,TRUE);}return 0;}
 if(m==WM_NCDESTROY){LRESULT result=DefWindowProcW(w,m,a,b);SetWindowLongPtrW(w,GWLP_USERDATA,0);HeapFree(GetProcessHeap(),0,s);return result;}
 return DefWindowProcW(w,m,a,b);
}
__declspec(dllexport) BOOL WINAPI VbDataRegister(void){WNDCLASSW c={.hInstance=module,.lpfnWndProc=proc,.lpszClassName=L"ThunderRT6Data",.hbrBackground=GetStockObject(WHITE_BRUSH)};return RegisterClassW(&c)!=0;}
__declspec(dllexport) BOOL WINAPI VbDataInspect(HWND w,BOOL *pressed,unsigned *clicks,unsigned *cancels){state *s=(void*)GetWindowLongPtrW(w,GWLP_USERDATA);if(!s)return FALSE;*pressed=s->pressed>=0;*clicks=s->clicks;*cancels=s->cancels;return TRUE;}
__declspec(dllexport) BOOL WINAPI VbDataRelease(void){return UnregisterClassW(L"ThunderRT6Data",module);}
BOOL WINAPI DllMain(HINSTANCE h,DWORD why,LPVOID ignored){(void)ignored;if(why==DLL_PROCESS_ATTACH)module=h;return TRUE;}
