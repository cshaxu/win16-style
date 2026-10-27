#define UNICODE
#include <windows.h>
#include <string.h>
#ifdef T18_RADIO
#define BUTTON_CLASS L"TRadioButton"
#else
#define BUTTON_CLASS L"TGroupButton"
#endif
static HINSTANCE owner;static WNDPROC native_button;static BOOL fake,enabled;static void *thunk;
static LRESULT CALLBACK group_proc(HWND w,UINT m,WPARAM p,LPARAM l){if(m==WM_COMMAND && HIWORD(p)==BN_CLICKED && l){HWND child=(HWND)l;for(HWND it=GetWindow(w,GW_CHILD);it;it=GetWindow(it,GW_HWNDNEXT))SendMessageW(it,BM_SETCHECK,it==child?BST_CHECKED:BST_UNCHECKED,0);return 0;}return DefWindowProcW(w,m,p,l);}
static LRESULT CALLBACK dispatch(HWND w,UINT m,WPARAM p,LPARAM l){return (!enabled || (fake&&m==WM_PAINT))?DefWindowProcW(w,m,p,l):CallWindowProcW(native_button,w,m,p,l);}
static LRESULT CALLBACK initial(HWND w,UINT m,WPARAM p,LPARAM l){LRESULT r=dispatch(w,m,p,l);if(m==WM_NCCREATE)SetWindowLongPtrW(w,GWLP_WNDPROC,(LONG_PTR)thunk);return r;}
__declspec(dllexport) BOOL WINAPI FixtureRegister(BOOL impostor){WNDCLASSEXW n={.cbSize=sizeof(n)},c={.cbSize=sizeof(c)};DWORD old;if(!GetClassInfoExW(NULL,L"BUTTON",&n))return FALSE;native_button=n.lpfnWndProc;fake=impostor;enabled=FALSE;thunk=VirtualAlloc(NULL,4096,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);if(!thunk)return FALSE;
#ifdef _WIN64
unsigned char code[]={0x48,0xb8,0,0,0,0,0,0,0,0,0xff,0xe0};void *target=(void*)dispatch;memcpy(code+2,&target,sizeof(target));
#else
unsigned char code[]={0xb8,0,0,0,0,0xff,0xe0};void *target=(void*)dispatch;memcpy(code+1,&target,sizeof(target));
#endif
memcpy(thunk,code,sizeof(code));if(!VirtualProtect(thunk,4096,PAGE_EXECUTE_READ,&old))return FALSE;FlushInstructionCache(GetCurrentProcess(),thunk,sizeof(code));c.hInstance=owner;c.lpszClassName=L"TRadioGroup";c.lpfnWndProc=group_proc;if(!RegisterClassExW(&c))return FALSE;c.style=CS_DBLCLKS|CS_HREDRAW|CS_VREDRAW;c.cbWndExtra=n.cbWndExtra;c.lpszClassName=BUTTON_CLASS;c.lpfnWndProc=initial;return RegisterClassExW(&c)!=0;}
__declspec(dllexport) HWND WINAPI FixtureGroup(HWND parent){return CreateWindowW(L"TRadioGroup",L"Group",WS_CHILD|WS_VISIBLE,0,0,300,120,parent,NULL,owner,NULL);}
__declspec(dllexport) HWND WINAPI FixtureCreate(HWND parent,BOOL first){DWORD style=WS_CHILD|WS_VISIBLE|BS_RADIOBUTTON|(first?WS_TABSTOP|WS_GROUP:0);HWND w=CreateWindowW(BUTTON_CLASS,first?L"First":L"Second",style,5,first?5:40,180,30,parent,NULL,owner,NULL);if(w){CREATESTRUCTW cs={0};cs.hwndParent=parent;cs.hInstance=owner;cs.style=style;cs.lpszClass=BUTTON_CLASS;cs.lpszName=first?L"First":L"Second";cs.cx=180;cs.cy=30;enabled=TRUE;CallWindowProcW(native_button,w,WM_NCCREATE,0,(LPARAM)&cs);CallWindowProcW(native_button,w,WM_CREATE,0,(LPARAM)&cs);}return w;}
__declspec(dllexport) BOOL WINAPI FixtureUnregister(void){BOOL ok=UnregisterClassW(BUTTON_CLASS,owner) && UnregisterClassW(L"TRadioGroup",owner);if(thunk){ok=VirtualFree(thunk,0,MEM_RELEASE)&&ok;thunk=NULL;}return ok;}
BOOL WINAPI DllMain(HINSTANCE h,DWORD reason,LPVOID reserved){(void)reserved;if(reason==DLL_PROCESS_ATTACH)owner=h;return TRUE;}
