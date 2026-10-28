#define UNICODE
#include <windows.h>
#include <string.h>
typedef LRESULT (*foreign_fn)(WNDPROC,HWND,UINT,WPARAM,LPARAM);
static unsigned disabled_mouse_calls;static foreign_fn foreign_paint;static HINSTANCE owner;static WNDPROC native_scroll;static int fake;static BOOL enabled;static void *thunk;
static LRESULT CALLBACK dispatch(HWND w,UINT m,WPARAM p,LPARAM l){if(!IsWindowEnabled(w)&&(m==WM_LBUTTONDOWN||m==WM_LBUTTONDBLCLK))++disabled_mouse_calls;if(fake==2&&enabled&&m==WM_PAINT&&foreign_paint)return foreign_paint(native_scroll,w,m,p,l);return (!enabled||(fake==1&&m==WM_PAINT))?DefWindowProcW(w,m,p,l):CallWindowProcW(native_scroll,w,m,p,l);}
static LRESULT CALLBACK initial(HWND w,UINT m,WPARAM p,LPARAM l){LRESULT r=dispatch(w,m,p,l);if(m==WM_NCCREATE)SetWindowLongPtrW(w,GWLP_WNDPROC,(LONG_PTR)thunk);return r;}
__declspec(dllexport) BOOL WINAPI FixtureRegister(BOOL impostor){
 WNDCLASSEXW n={.cbSize=sizeof(n)},c={.cbSize=sizeof(c)};DWORD old;if(!GetClassInfoExW(NULL,L"SCROLLBAR",&n))return FALSE;
 native_scroll=n.lpfnWndProc;fake=impostor;enabled=FALSE;thunk=VirtualAlloc(NULL,4096,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);if(!thunk)return FALSE;
#ifdef _WIN64
 unsigned char code[]={0x48,0xb8,0,0,0,0,0,0,0,0,0xff,0xe0};void *target=(void*)dispatch;memcpy(code+2,&target,sizeof(target));
#else
 unsigned char code[]={0xb8,0,0,0,0,0xff,0xe0};void *target=(void*)dispatch;memcpy(code+1,&target,sizeof(target));
#endif
 memcpy(thunk,code,sizeof(code));if(!VirtualProtect(thunk,4096,PAGE_EXECUTE_READ,&old)){VirtualFree(thunk,0,MEM_RELEASE);thunk=NULL;return FALSE;}FlushInstructionCache(GetCurrentProcess(),thunk,sizeof(code));
 c.style=CS_DBLCLKS|CS_HREDRAW|CS_VREDRAW;c.cbWndExtra=n.cbWndExtra;c.hCursor=LoadCursorW(NULL,IDC_ARROW);c.hInstance=owner;c.lpszClassName=L"TScrollBar";c.lpfnWndProc=initial;
 return RegisterClassExW(&c)!=0;
}
__declspec(dllexport) void WINAPI FixtureResetMouse(void){disabled_mouse_calls=0;}
__declspec(dllexport) unsigned WINAPI FixtureMouseCalls(void){return disabled_mouse_calls;}
__declspec(dllexport) void WINAPI FixtureSetForeign(foreign_fn fn){foreign_paint=fn;}
__declspec(dllexport) HWND WINAPI FixtureCreate(HWND parent,BOOL vertical){
 DWORD style=WS_CHILD|WS_VISIBLE|WS_TABSTOP|(vertical?SBS_VERT:SBS_HORZ);int width=vertical?25:250,height=vertical?250:25;
 HWND w=CreateWindowExW(0,L"TScrollBar",L"",style,10,10,width,height,parent,NULL,owner,NULL);
 if(w){CREATESTRUCTW cs={0};cs.hwndParent=parent;cs.hInstance=owner;cs.style=style;cs.lpszClass=L"TScrollBar";cs.lpszName=L"";cs.cx=width;cs.cy=height;
  enabled=TRUE;CallWindowProcW(native_scroll,w,WM_NCCREATE,0,(LPARAM)&cs);CallWindowProcW(native_scroll,w,WM_CREATE,0,(LPARAM)&cs);
  SCROLLINFO info={.cbSize=sizeof(info),.fMask=SIF_RANGE|SIF_PAGE|SIF_POS,.nMin=0,.nMax=100,.nPage=10,.nPos=25};SetScrollInfo(w,SB_CTL,&info,TRUE);}
 return w;
}
__declspec(dllexport) BOOL WINAPI FixtureUnregister(void){BOOL ok=UnregisterClassW(L"TScrollBar",owner);if(thunk){ok=VirtualFree(thunk,0,MEM_RELEASE)&&ok;thunk=NULL;}return ok;}
BOOL WINAPI DllMain(HINSTANCE h,DWORD reason,LPVOID unused){(void)unused;if(reason==DLL_PROCESS_ATTACH)owner=h;return TRUE;}
