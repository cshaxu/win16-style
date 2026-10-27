#define UNICODE
#include <windows.h>
#include <commctrl.h>
#include <string.h>
static HINSTANCE owner;static WNDPROC native_toolbar;static BOOL fake,enabled,skip;static void *thunk;static unsigned prepaint,commands;
static LRESULT CALLBACK group_proc(HWND w,UINT m,WPARAM p,LPARAM l){if(m==WM_NOTIFY && l && ((NMHDR*)l)->code==NM_CUSTOMDRAW && ((NMCUSTOMDRAW*)l)->dwDrawStage==CDDS_PREPAINT){++prepaint;return skip?CDRF_SKIPDEFAULT:CDRF_DODEFAULT;}if(m==WM_COMMAND && l){++commands;return 0;}return DefWindowProcW(w,m,p,l);}
static LRESULT CALLBACK dispatch(HWND w,UINT m,WPARAM p,LPARAM l){if(!enabled || (fake&&m==WM_PAINT))return DefWindowProcW(w,m,p,l);if(m==WM_PAINT){PAINTSTRUCT ps;HDC dc=BeginPaint(w,&ps);LRESULT r=CallWindowProcW(native_toolbar,w,WM_PRINTCLIENT,(WPARAM)dc,PRF_CLIENT|PRF_ERASEBKGND);EndPaint(w,&ps);return r;}return CallWindowProcW(native_toolbar,w,m,p,l);}
static LRESULT CALLBACK initial(HWND w,UINT m,WPARAM p,LPARAM l){LRESULT r=dispatch(w,m,p,l);if(m==WM_NCCREATE)SetWindowLongPtrW(w,GWLP_WNDPROC,(LONG_PTR)thunk);return r;}
__declspec(dllexport) BOOL WINAPI FixtureRegister(BOOL impostor){WNDCLASSEXW n={.cbSize=sizeof(n)},c={.cbSize=sizeof(c)};DWORD old;INITCOMMONCONTROLSEX ic={sizeof(ic),ICC_BAR_CLASSES};if(!InitCommonControlsEx(&ic)||!GetClassInfoExW(NULL,TOOLBARCLASSNAMEW,&n))return FALSE;native_toolbar=n.lpfnWndProc;fake=impostor;enabled=FALSE;thunk=VirtualAlloc(NULL,4096,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);if(!thunk)return FALSE;
#ifdef _WIN64
unsigned char code[]={0x48,0xb8,0,0,0,0,0,0,0,0,0xff,0xe0};void *target=(void*)dispatch;memcpy(code+2,&target,sizeof(target));
#else
unsigned char code[]={0xb8,0,0,0,0,0xff,0xe0};void *target=(void*)dispatch;memcpy(code+1,&target,sizeof(target));
#endif
memcpy(thunk,code,sizeof(code));if(!VirtualProtect(thunk,4096,PAGE_EXECUTE_READ,&old))return FALSE;FlushInstructionCache(GetCurrentProcess(),thunk,sizeof(code));c.hInstance=owner;c.lpszClassName=L"TControlBar";c.lpfnWndProc=group_proc;if(!RegisterClassExW(&c))return FALSE;c.style=CS_DBLCLKS|CS_HREDRAW|CS_VREDRAW;c.cbWndExtra=n.cbWndExtra;c.lpszClassName=L"TToolBar";c.lpfnWndProc=initial;return RegisterClassExW(&c)!=0;}
__declspec(dllexport) HWND WINAPI FixtureGroup(HWND p){return CreateWindowW(L"TControlBar",L"",WS_CHILD|WS_VISIBLE,0,0,300,120,p,NULL,owner,NULL);}
__declspec(dllexport) HWND WINAPI FixtureCreate(HWND parent,int mode){DWORD style=WS_CHILD|WS_VISIBLE|TBSTYLE_FLAT|TBSTYLE_LIST|(mode==4?CCS_NORESIZE:0);HWND w=CreateWindowW(L"TToolBar",L"",style,5,5,200,60,parent,NULL,owner,NULL);if(w){CREATESTRUCTW cs={0};cs.hwndParent=parent;cs.hInstance=owner;cs.style=style;cs.lpszClass=L"TToolBar";cs.lpszName=L"";cs.cx=200;cs.cy=60;enabled=TRUE;CallWindowProcW(native_toolbar,w,WM_NCCREATE,0,(LPARAM)&cs);CallWindowProcW(native_toolbar,w,WM_CREATE,0,(LPARAM)&cs);SendMessageW(w,TB_BUTTONSTRUCTSIZE,sizeof(TBBUTTON),0);TBBUTTON item={.iBitmap=mode==2?0:I_IMAGENONE,.idCommand=0,.fsState=TBSTATE_ENABLED,.fsStyle=BTNS_BUTTON,.iString=(INT_PTR)L"S57"};SendMessageW(w,TB_ADDBUTTONSW,1,(LPARAM)&item);if(mode==1)SendMessageW(w,TB_ADDBUTTONSW,1,(LPARAM)&item);
if(mode==5 || mode==6){int count=mode==5?64:65;for(int i=1;i<count;++i){item.idCommand=i;SendMessageW(w,TB_ADDBUTTONSW,1,(LPARAM)&item);}}
if(mode==4)SetWindowPos(w,NULL,0,0,200,8,SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);}
return w;}
__declspec(dllexport) void WINAPI FixtureSkip(BOOL value){skip=value;}
__declspec(dllexport) unsigned WINAPI FixturePrepaint(void){return prepaint;}
__declspec(dllexport) unsigned WINAPI FixtureCommands(void){return commands;}
__declspec(dllexport) BOOL WINAPI FixtureUnregister(void){BOOL ok=UnregisterClassW(L"TToolBar",owner)&&UnregisterClassW(L"TControlBar",owner);if(thunk){ok=VirtualFree(thunk,0,MEM_RELEASE)&&ok;thunk=NULL;}return ok;}
BOOL WINAPI DllMain(HINSTANCE h,DWORD reason,LPVOID reserved){(void)reserved;if(reason==DLL_PROCESS_ATTACH)owner=h;return TRUE;}
