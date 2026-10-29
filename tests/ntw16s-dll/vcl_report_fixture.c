#define UNICODE
#include <windows.h>
#include <commctrl.h>
#include <string.h>
static HINSTANCE owner;
static WNDPROC native_proc;
static BOOL fake,enabled,tree;
static void *thunk;
static const WCHAR *name(void){return tree?L"TTreeView":L"TListView";}
static LRESULT CALLBACK dispatch(HWND w,UINT m,WPARAM p,LPARAM l)
{return (!enabled || (fake && m==WM_PAINT))?DefWindowProcW(w,m,p,l):CallWindowProcW(native_proc,w,m,p,l);}
static LRESULT CALLBACK initial(HWND w,UINT m,WPARAM p,LPARAM l)
{LRESULT result=dispatch(w,m,p,l);if(m==WM_NCCREATE)SetWindowLongPtrW(w,GWLP_WNDPROC,(LONG_PTR)thunk);return result;}
__declspec(dllexport) BOOL WINAPI FixtureRegister(BOOL impostor,BOOL is_tree)
{
 WNDCLASSEXW n={.cbSize=sizeof(n)},c={.cbSize=sizeof(c)};DWORD old;
 tree=is_tree;fake=impostor;enabled=FALSE;
 if(!GetClassInfoExW(NULL,tree?WC_TREEVIEWW:WC_LISTVIEWW,&n))return FALSE;
 native_proc=n.lpfnWndProc;thunk=VirtualAlloc(NULL,4096,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);if(!thunk)return FALSE;
#ifdef _WIN64
 unsigned char code[]={0x48,0xb8,0,0,0,0,0,0,0,0,0xff,0xe0};void *target=(void*)dispatch;memcpy(code+2,&target,sizeof(target));
#else
 unsigned char code[]={0xb8,0,0,0,0,0xff,0xe0};void *target=(void*)dispatch;memcpy(code+1,&target,sizeof(target));
#endif
 memcpy(thunk,code,sizeof(code));if(!VirtualProtect(thunk,4096,PAGE_EXECUTE_READ,&old)){VirtualFree(thunk,0,MEM_RELEASE);thunk=NULL;return FALSE;}
 FlushInstructionCache(GetCurrentProcess(),thunk,sizeof(code));c.style=CS_DBLCLKS|CS_HREDRAW|CS_VREDRAW;c.cbWndExtra=n.cbWndExtra;c.hCursor=LoadCursorW(NULL,IDC_ARROW);c.hInstance=owner;c.lpszClassName=name();c.lpfnWndProc=initial;
 if(RegisterClassExW(&c))return TRUE;
 VirtualFree(thunk,0,MEM_RELEASE);thunk=NULL;return FALSE;
}
__declspec(dllexport) HWND WINAPI FixtureCreate(HWND parent,BOOL invalid)
{
 DWORD style=WS_CHILD|WS_VISIBLE|WS_TABSTOP|(tree?TVS_HASBUTTONS:LVS_REPORT);
 if(invalid && !tree)style|=LVS_OWNERDRAWFIXED;
 HWND w=CreateWindowExW(WS_EX_CLIENTEDGE,name(),L"",style,20,20,220,120,parent,NULL,owner,NULL);
 if(w){CREATESTRUCTW cs={0};cs.hwndParent=parent;cs.hInstance=owner;cs.style=(LONG)GetWindowLongPtrW(w,GWL_STYLE);cs.dwExStyle=(DWORD)GetWindowLongPtrW(w,GWL_EXSTYLE);cs.lpszClass=name();cs.lpszName=L"";cs.cx=220;cs.cy=120;enabled=TRUE;CallWindowProcW(native_proc,w,WM_NCCREATE,0,(LPARAM)&cs);CallWindowProcW(native_proc,w,WM_CREATE,0,(LPARAM)&cs);}
 if(w && tree){TVINSERTSTRUCTW item={0};item.hParent=TVI_ROOT;item.hInsertAfter=TVI_LAST;item.item.mask=TVIF_TEXT;item.item.pszText=L"One";SendMessageW(w,TVM_INSERTITEMW,0,(LPARAM)&item);}
 if(w && !tree){LVCOLUMNW col={.mask=LVCF_TEXT|LVCF_WIDTH,.pszText=L"Name",.cx=100};LVITEMW item={.mask=LVIF_TEXT,.iItem=0,.pszText=L"One"};SendMessageW(w,LVM_INSERTCOLUMNW,0,(LPARAM)&col);SendMessageW(w,LVM_INSERTITEMW,0,(LPARAM)&item);}
 return w;
}
__declspec(dllexport) BOOL WINAPI FixtureUnregister(void)
{BOOL ok=UnregisterClassW(name(),owner);if(thunk){ok=VirtualFree(thunk,0,MEM_RELEASE)&&ok;thunk=NULL;}return ok;}
BOOL WINAPI DllMain(HINSTANCE h,DWORD reason,LPVOID unused)
{(void)unused;if(reason==DLL_PROCESS_ATTACH)owner=h;return TRUE;}
