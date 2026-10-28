#include <windows.h>
#include <string.h>
static HINSTANCE owner;static WNDPROC native,native_unicode;static void *thunk;static BOOL enabled;static int mode;
typedef LRESULT(WINAPI *foreign_fn)(WNDPROC,HWND,UINT,WPARAM,LPARAM);static foreign_fn foreign;static HWND move_on_count,move_to,move_next;
__declspec(dllexport) void WINAPI ArmNext(HWND p){move_next=p;}
__declspec(dllexport) void WINAPI ArmMove(HWND w,HWND parent){move_on_count=w;move_to=parent;}
__declspec(dllexport) void WINAPI SetForeign(foreign_fn f){foreign=f;}
static LRESULT CALLBACK parent_proc(HWND w,UINT m,WPARAM p,LPARAM l){return DefWindowProcA(w,m,p,l);}
static LRESULT CALLBACK dispatch(HWND w,UINT m,WPARAM p,LPARAM l){if(w==move_on_count && m==CB_GETCOUNT){move_on_count=NULL;SetParent(w,move_to);}if(enabled&&mode==2&&m==WM_PAINT&&foreign)return foreign(native,w,m,p,l);if(enabled&&mode==3&&m==WM_PAINT)return CallWindowProcW(native_unicode,w,m,p,l);return (!enabled||(mode==1&&m==WM_PAINT))?DefWindowProcA(w,m,p,l):CallWindowProcA(native,w,m,p,l);}
static LRESULT CALLBACK initial(HWND w,UINT m,WPARAM p,LPARAM l){LRESULT r=dispatch(w,m,p,l);if(m==WM_NCCREATE)SetWindowLongPtrA(w,GWLP_WNDPROC,(LONG_PTR)thunk);return r;}
__declspec(dllexport) BOOL WINAPI Register(int x){WNDCLASSEXA n={.cbSize=sizeof(n)},c={.cbSize=sizeof(c)};DWORD old;if(!GetClassInfoExA(NULL,"COMBOBOX",&n))return FALSE;WNDCLASSEXW unicode={.cbSize=sizeof(unicode)};if(!GetClassInfoExW(NULL,L"COMBOBOX",&unicode))return FALSE;native_unicode=unicode.lpfnWndProc;native=n.lpfnWndProc;mode=x;enabled=FALSE;thunk=VirtualAlloc(NULL,4096,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);if(!thunk)return FALSE;
#ifdef _WIN64
unsigned char code[]={0x48,0xb8,0,0,0,0,0,0,0,0,0xff,0xe0};void *target=(void*)dispatch;memcpy(code+2,&target,sizeof(target));
#else
unsigned char code[]={0xb8,0,0,0,0,0xff,0xe0};void *target=(void*)dispatch;memcpy(code+1,&target,sizeof(target));
#endif
memcpy(thunk,code,sizeof(code));if(!VirtualProtect(thunk,4096,PAGE_EXECUTE_READ,&old))return FALSE;FlushInstructionCache(GetCurrentProcess(),thunk,sizeof(code));c.hInstance=owner;c.lpszClassName="ComboParent";c.lpfnWndProc=parent_proc;if(!RegisterClassExA(&c))return FALSE;c.style=CS_DBLCLKS|CS_HREDRAW|CS_VREDRAW;c.cbWndExtra=n.cbWndExtra;c.lpszClassName="TDBComboBox";c.lpfnWndProc=initial;return RegisterClassExA(&c)!=0;}
__declspec(dllexport) HWND WINAPI Parent(HWND root){return CreateWindowA("ComboParent","",WS_CHILD|WS_VISIBLE,0,0,300,120,root,NULL,owner,NULL);}
__declspec(dllexport) HWND WINAPI Create(HWND parent,DWORD extra){DWORD style=WS_CHILD|WS_VISIBLE|WS_TABSTOP|WS_VSCROLL|(extra?extra:CBS_DROPDOWN);enabled=FALSE;HWND w=CreateWindowA("TDBComboBox","",style,5,5,180,150,parent,NULL,owner,NULL);if(w){if(move_next){move_on_count=w;move_to=move_next;move_next=NULL;}CREATESTRUCTA cs={0};cs.hwndParent=parent;cs.hInstance=owner;cs.style=style;cs.lpszClass="TDBComboBox";cs.lpszName="";cs.cx=180;cs.cy=150;enabled=TRUE;CallWindowProcA(native,w,WM_NCCREATE,0,(LPARAM)&cs);CallWindowProcA(native,w,WM_CREATE,0,(LPARAM)&cs);}return w;}
__declspec(dllexport) BOOL WINAPI Unregister(void){BOOL ok=UnregisterClassA("TDBComboBox",owner)&&UnregisterClassA("ComboParent",owner);if(thunk)ok=VirtualFree(thunk,0,MEM_RELEASE)&&ok;thunk=NULL;return ok;}
BOOL WINAPI DllMain(HINSTANCE h,DWORD r,LPVOID p){(void)p;if(r==DLL_PROCESS_ATTACH)owner=h;return TRUE;}
