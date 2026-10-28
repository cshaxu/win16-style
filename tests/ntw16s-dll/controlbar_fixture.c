#define _WIN32_WINNT 0x0A00
#include <windows.h>
static HINSTANCE owner;static int mode;static UINT selected_edge=EDGE_ETCHED;static HWND move_to;static unsigned clicks;static BOOL adjusted;static void (WINAPI *foreign_edge)(HDC,RECT*);
static LRESULT CALLBACK parent_proc(HWND w,UINT m,WPARAM p,LPARAM l){if(m==WM_COMMAND && HIWORD(p)==BN_CLICKED)++clicks;return DefWindowProcW(w,m,p,l);}
static LRESULT CALLBACK bar_proc(HWND w,UINT m,WPARAM p,LPARAM l){
 if(m==WM_COMMAND && HIWORD(p)==BN_CLICKED)++clicks;
 if(m==WM_NCCALCSIZE){LRESULT r=DefWindowProcW(w,m,p,l);RECT *rect=p?&((NCCALCSIZE_PARAMS*)l)->rgrc[0]:(RECT*)l;int inset=((selected_edge&BDR_OUTER)!=0)+((selected_edge&BDR_INNER)!=0);if(rect)InflateRect(rect,-inset,-inset);return r;}
 if(m==WM_NCPAINT){LRESULT r=DefWindowProcW(w,m,p,l);HDC dc=GetWindowDC(w);RECT rect;GetWindowRect(w,&rect);OffsetRect(&rect,-rect.left,-rect.top);RECT initial=rect;
  int saved=SaveDC(dc);if(mode==6)SetViewportOrgEx(dc,1,0,NULL);if(mode==7)IntersectClipRect(dc,0,0,(initial.right-initial.left)/2,initial.bottom);
  if(mode==1)DrawEdge(dc,&rect,BDR_RAISEDINNER|BDR_SUNKENINNER,BF_RECT|BF_ADJUST);
  else if(mode==2){rect.right--;DrawEdge(dc,&rect,EDGE_ETCHED,BF_RECT|BF_ADJUST);}
  else if(mode==3 && foreign_edge)foreign_edge(dc,&rect);
  else if(mode==5){HDC mem=CreateCompatibleDC(dc);HBITMAP bmp=CreateCompatibleBitmap(dc,initial.right,initial.bottom);HGDIOBJ old=SelectObject(mem,bmp);DrawEdge(mem,&rect,EDGE_ETCHED,BF_RECT|BF_ADJUST);SelectObject(mem,old);DeleteObject(bmp);DeleteDC(mem);}
  else if(mode!=4)DrawEdge(dc,&rect,selected_edge,BF_RECT|BF_ADJUST);
  int inset=((selected_edge&BDR_OUTER)!=0)+((selected_edge&BDR_INNER)!=0);
  adjusted=mode!=4 && rect.left==initial.left+inset && rect.top==initial.top+inset && rect.right==initial.right-inset && rect.bottom==initial.bottom-inset;
  if(saved){RestoreDC(dc,saved);}ReleaseDC(w,dc);if(move_to){HWND next=move_to;move_to=NULL;SetParent(w,next);}return r;}
 return DefWindowProcW(w,m,p,l);
}
__declspec(dllexport) BOOL WINAPI FixtureRegister(void){WNDCLASSEXW a={.cbSize=sizeof(a),.hInstance=owner,.lpfnWndProc=parent_proc,.hbrBackground=(HBRUSH)(COLOR_BTNFACE+1),.lpszClassName=L"ControlBarTestParent"};if(!RegisterClassExW(&a))return FALSE;a.lpfnWndProc=bar_proc;a.lpszClassName=L"TControlBar";return RegisterClassExW(&a)!=0;}
__declspec(dllexport) HWND WINAPI FixtureParent(HWND root){return CreateWindowW(L"ControlBarTestParent",L"",WS_CHILD|WS_VISIBLE,0,0,260,170,root,NULL,owner,NULL);}
__declspec(dllexport) HWND WINAPI FixtureBar(HWND parent){return CreateWindowW(L"TControlBar",L"",WS_CHILD|WS_VISIBLE,10,10,220,120,parent,NULL,owner,NULL);}
__declspec(dllexport) HWND WINAPI FixtureButton(HWND bar){return CreateWindowW(L"BUTTON",L"OK",WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON,20,20,80,24,bar,(HMENU)1,owner,NULL);}
__declspec(dllexport) void WINAPI FixtureMode(int m){mode=m;}
__declspec(dllexport) void WINAPI FixtureEdge(UINT edge){selected_edge=edge;}
__declspec(dllexport) void WINAPI FixtureForeign(void (WINAPI *f)(HDC,RECT*)){foreign_edge=f;}
__declspec(dllexport) void WINAPI FixtureMove(HWND w){move_to=w;}
__declspec(dllexport) BOOL WINAPI FixtureAdjusted(void){return adjusted;}
__declspec(dllexport) unsigned WINAPI FixtureClicks(void){return clicks;}
__declspec(dllexport) BOOL WINAPI FixtureUnregister(void){return UnregisterClassW(L"TControlBar",owner)&&UnregisterClassW(L"ControlBarTestParent",owner);}
BOOL WINAPI DllMain(HINSTANCE h,DWORD reason,void *unused){(void)unused;if(reason==DLL_PROCESS_ATTACH)owner=h;return TRUE;}
