#define UNICODE
#include <windows.h>
#include <uxtheme.h>
#include <string.h>
/* Test-only DLL: its class is owned outside the EXE. The synthetic delayed
 * native initialization exercises creation-scope admission; it does not
 * claim to reproduce a Delphi runtime's actual creation order. */
#define EDIT_CLASS L"TSpinEdit"
typedef LRESULT (*foreign_fn)(WNDPROC,HWND,UINT,WPARAM,LPARAM);
static foreign_fn foreign_paint;
static HINSTANCE owner;
static WNDPROC native_edit;
static int fake;
static BOOL enabled;
static void *thunk;
static LRESULT CALLBACK dispatch(HWND w,UINT m,WPARAM p,LPARAM l)
{
 if(enabled && fake==2 && m==WM_PAINT && foreign_paint)return foreign_paint(native_edit,w,m,p,l);
 if(!enabled || (fake==1 && m==WM_PAINT))return DefWindowProcW(w,m,p,l);
 LRESULT result=CallWindowProcW(native_edit,w,m,p,l);
 /* Model the observed VCL/native font update that widens the format rect. */
 if(m==WM_SETFONT && FindWindowExW(w,NULL,L"TSpinButton",NULL)){
    RECT r;CallWindowProcW(native_edit,w,EM_GETRECT,0,(LPARAM)&r);r.right=200;
    CallWindowProcW(native_edit,w,EM_SETRECTNP,0,(LPARAM)&r);
 }
 return result;
}
static LRESULT CALLBACK initial(HWND w,UINT m,WPARAM p,LPARAM l)
{LRESULT result=dispatch(w,m,p,l);if(m==WM_NCCREATE)SetWindowLongPtrW(w,GWLP_WNDPROC,(LONG_PTR)thunk);return result;}
__declspec(dllexport) void WINAPI FixtureSetForeign(foreign_fn fn){foreign_paint=fn;}
static int spin_mode;static COLORREF last_pixel;static HDC kept;static HBITMAP kept_bitmap;static HGDIOBJ kept_old;
__declspec(dllexport) void WINAPI SpinMode(int value){spin_mode=value;}
__declspec(dllexport) COLORREF WINAPI SpinLastPixel(void){return last_pixel;}
static LRESULT CALLBACK spin_proc(HWND w,UINT m,WPARAM p,LPARAM l)
{
 if(m==WM_PAINT || m==WM_PRINTCLIENT){
  PAINTSTRUCT ps;HDC dest=m==WM_PAINT?BeginPaint(w,&ps):(HDC)p;RECT full;GetClientRect(w,&full);
  HDC dc=CreateCompatibleDC(dest);HBITMAP bmp=CreateCompatibleBitmap(dest,full.right,full.bottom);HGDIOBJ old=SelectObject(dc,bmp);
  int source_clip=0;
  if(spin_mode==13 || spin_mode==14){HBRUSH brush=CreateSolidBrush(RGB(253,253,253));FillRect(dc,&full,brush);DeleteObject(brush);
      source_clip=SaveDC(dc);if(spin_mode==13)IntersectClipRect(dc,0,0,10,full.bottom);
      else ExcludeClipRect(dc,14,2,17,5);}
  if(spin_mode==1){HDC inner=CreateCompatibleDC(dest);if(inner)DeleteDC(inner);}
  if(spin_mode==2&&!kept){kept=CreateCompatibleDC(dest);kept_bitmap=CreateCompatibleBitmap(dest,full.right,full.bottom);kept_old=SelectObject(kept,kept_bitmap);}
  HTHEME theme=OpenThemeData(NULL,L"BUTTON");RECT r={0,0,full.right,full.bottom/2};
  if(spin_mode==3&&kept){DrawThemeBackground(theme,kept,1,1,&r,NULL);last_pixel=GetPixel(kept,2,2);SelectObject(kept,kept_old);DeleteObject(kept_bitmap);DeleteDC(kept);kept=NULL;kept_bitmap=NULL;kept_old=NULL;}
  if(spin_mode==5){HDC other=CreateCompatibleDC(dest);HBITMAP ob=CreateCompatibleBitmap(dest,full.right,full.bottom);HGDIOBJ oo=SelectObject(other,ob);DrawThemeBackground(theme,other,1,1,&r,NULL);last_pixel=GetPixel(other,3,3);SelectObject(other,oo);DeleteObject(ob);DeleteDC(other);}
  DrawThemeBackground(theme,dc,1,1,&r,NULL);if(spin_mode==8&&m==WM_PRINTCLIENT){spin_mode=0;CloseThemeData(theme);SelectObject(dc,old);DeleteObject(bmp);DeleteDC(dc);RaiseException(0xe0425342u,0,0,NULL);return 0;}SetViewportOrgEx(dc,0,full.bottom/2,NULL);r.bottom=full.bottom-full.bottom/2;DrawThemeBackground(theme,dc,1,1,&r,NULL);SetViewportOrgEx(dc,0,0,NULL);CloseThemeData(theme);
  if(source_clip)RestoreDC(dc,source_clip);
  if(spin_mode==11){
      HTHEME other=OpenThemeData(NULL,L"TOOLBAR");RECT upper={0,0,full.right,full.bottom/2};
      if(other){DrawThemeBackground(other,dc,1,1,&upper,NULL);CloseThemeData(other);}
      HBRUSH brush=CreateSolidBrush(RGB(253,253,253));FillRect(dc,&upper,brush);DeleteObject(brush);
  }
  if(spin_mode==12){
      RECT upper={0,0,full.right,full.bottom/2};DrawThemeBackground(NULL,dc,1,1,&upper,NULL);
      HBRUSH brush=CreateSolidBrush(RGB(253,253,253));FillRect(dc,&upper,brush);DeleteObject(brush);
  }
  HBITMAP swap=NULL;if(spin_mode==7||spin_mode==9){swap=CreateCompatibleBitmap(dest,full.right,full.bottom);SelectObject(dc,swap);if(spin_mode==9)SelectObject(dc,bmp);HBRUSH brush=CreateSolidBrush(RGB(253,253,253));FillRect(dc,&full,brush);DeleteObject(brush);}
  for(int gy=0;gy<3;gy++)for(int gx=0;gx<3;gx++)
      SetPixelV(dc,full.right/2+gx,full.bottom/4+gy,RGB(220-40*gx,40*gy,20+30*gx));
  BitBlt(dest,0,0,full.right,full.bottom,dc,0,0,SRCCOPY);
  if(swap){SelectObject(dc,bmp);DeleteObject(swap);}SelectObject(dc,old);DeleteObject(bmp);DeleteDC(dc);if(m==WM_PAINT)EndPaint(w,&ps);return 0;
 }
 return DefWindowProcW(w,m,p,l);
}
__declspec(dllexport) BOOL WINAPI FixtureRegister(int impostor)
{
    WNDCLASSEXW n={.cbSize=sizeof(n)},c={.cbSize=sizeof(c)};DWORD old;
    if(!GetClassInfoExW(NULL,L"EDIT",&n))return FALSE;
    native_edit=n.lpfnWndProc;fake=impostor;enabled=FALSE;
    thunk=VirtualAlloc(NULL,4096,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);if(!thunk)return FALSE;
#ifdef _WIN64
    unsigned char code[]={0x48,0xb8,0,0,0,0,0,0,0,0,0xff,0xe0};void *target=(void*)dispatch;memcpy(code+2,&target,sizeof(target));
#else
    unsigned char code[]={0xb8,0,0,0,0,0xff,0xe0};void *target=(void*)dispatch;memcpy(code+1,&target,sizeof(target));
#endif
    memcpy(thunk,code,sizeof(code));
    if(!VirtualProtect(thunk,4096,PAGE_EXECUTE_READ,&old)){VirtualFree(thunk,0,MEM_RELEASE);thunk=NULL;return FALSE;}
    FlushInstructionCache(GetCurrentProcess(),thunk,sizeof(code));
    c.style=CS_DBLCLKS|CS_HREDRAW|CS_VREDRAW;c.cbWndExtra=n.cbWndExtra;c.hCursor=LoadCursorW(NULL,IDC_IBEAM);
    c.hInstance=owner;c.lpszClassName=EDIT_CLASS;c.lpfnWndProc=initial;
    if(RegisterClassExW(&c)){
        WNDCLASSW child={.hInstance=owner,.lpszClassName=L"TSpinButton",.lpfnWndProc=spin_proc};
        if(RegisterClassW(&child))return TRUE;
        UnregisterClassW(EDIT_CLASS,owner);
    }
    VirtualFree(thunk,0,MEM_RELEASE);thunk=NULL;return FALSE;
}
__declspec(dllexport) HWND WINAPI FixtureCreate(HWND parent,BOOL multiline)
{
    DWORD style=WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS|WS_TABSTOP|ES_AUTOHSCROLL|ES_AUTOVSCROLL;
#ifdef T18_MEMO
    multiline=TRUE;
#endif
    (void)multiline;style|=ES_MULTILINE;
    HWND w=CreateWindowExW(WS_EX_CLIENTEDGE,EDIT_CLASS,L"probe",style,20,20,220,28,parent,NULL,owner,NULL);
    if(w){CREATESTRUCTW cs={0};cs.hwndParent=parent;cs.hInstance=owner;cs.style=GetWindowLongPtrW(w,GWL_STYLE);
        cs.dwExStyle=GetWindowLongPtrW(w,GWL_EXSTYLE);cs.lpszClass=EDIT_CLASS;cs.lpszName=L"probe";cs.cx=220;cs.cy=28;
        enabled=TRUE;CallWindowProcW(native_edit,w,WM_NCCREATE,0,(LPARAM)&cs);CallWindowProcW(native_edit,w,WM_CREATE,0,(LPARAM)&cs);}
#ifdef T18_MEMO
    if(w)SendMessageW(w,EM_SETLIMITTEXT,0,0);
#endif
    return w;
}
__declspec(dllexport) HWND WINAPI FixtureAddButton(HWND parent)
{
    HWND child=CreateWindowW(L"TSpinButton",L"",WS_CHILD|WS_VISIBLE,190,0,20,24,parent,NULL,owner,NULL);
    RECT r={2,2,180,24};if(child)SendMessageW(parent,EM_SETRECTNP,0,(LPARAM)&r);return child;
}
__declspec(dllexport) BOOL WINAPI FixtureUnregister(void)
{if(kept){SelectObject(kept,kept_old);DeleteObject(kept_bitmap);DeleteDC(kept);kept=NULL;}BOOL ok=UnregisterClassW(L"TSpinButton",owner);ok=UnregisterClassW(EDIT_CLASS,owner)&&ok;if(thunk){ok=VirtualFree(thunk,0,MEM_RELEASE)&&ok;thunk=NULL;}return ok;}
BOOL WINAPI DllMain(HINSTANCE h,DWORD reason,LPVOID unused)
{(void)unused;if(reason==DLL_PROCESS_ATTACH)owner=h;return TRUE;}
