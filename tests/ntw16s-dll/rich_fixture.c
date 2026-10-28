#define _WIN32_WINNT 0x0A00
#include <windows.h>
#include <richedit.h>
static HINSTANCE instance;static HMODULE rich;static WNDPROC native_proc;static BOOL fake,raise_paint;static HWND switch_parent;static unsigned link_notices;typedef LRESULT (WINAPI *forwardfn)(WNDPROC,HWND,UINT,WPARAM,LPARAM);static forwardfn foreign_forward;
static LRESULT CALLBACK parent_proc(HWND w,UINT m,WPARAM p,LPARAM l){if(m==WM_NOTIFY && l && ((NMHDR*)l)->code==EN_LINK)++link_notices;return DefWindowProcA(w,m,p,l);}
__declspec(dllexport) unsigned WINAPI FixtureLinks(void){return link_notices;}
static LRESULT CALLBACK rich_proc(HWND w,UINT m,WPARAM p,LPARAM l){if(fake&&m==WM_PAINT)return DefWindowProcA(w,m,p,l);if(foreign_forward&&m==WM_PAINT)return foreign_forward(native_proc,w,m,p,l);LRESULT result=CallWindowProcA(native_proc,w,m,p,l);if(switch_parent&&m==WM_PAINT){HWND parent=switch_parent;switch_parent=NULL;SetParent(w,parent);}if(raise_paint&&m==WM_PAINT){raise_paint=FALSE;RaiseException(0xe0425342u,0,0,NULL);}return result;}
__declspec(dllexport) void WINAPI FixtureChangeParent(HWND p){switch_parent=p;}
__declspec(dllexport) void WINAPI FixtureFake(BOOL value){fake=value;}
__declspec(dllexport) void WINAPI FixtureForward(forwardfn f){foreign_forward=f;}
__declspec(dllexport) void WINAPI FixtureRaise(void){raise_paint=TRUE;}
__declspec(dllexport) BOOL WINAPI FixtureRegister(void){WNDCLASSEXA r={.cbSize=sizeof(r)},p={.cbSize=sizeof(p)};rich=LoadLibraryW(L"riched20.dll");if(!rich||!GetClassInfoExA(rich,"RichEdit20A",&r))return FALSE;native_proc=r.lpfnWndProc;r.hInstance=instance;r.lpszClassName="TRichEditURL";r.lpfnWndProc=rich_proc;p.hInstance=instance;p.lpszClassName="RichFixtureParent";p.lpfnWndProc=parent_proc;p.hCursor=LoadCursorA(NULL,IDC_ARROW);return RegisterClassExA(&p)&&RegisterClassExA(&r);}
__declspec(dllexport) HWND WINAPI FixtureParent(HWND parent){return CreateWindowExA(0,"RichFixtureParent","",WS_CHILD|WS_VISIBLE,0,0,400,200,parent,NULL,instance,NULL);}
__declspec(dllexport) HWND WINAPI FixtureCreate(HWND parent,BOOL impostor){fake=impostor;return CreateWindowExA(WS_EX_CLIENTEDGE,"TRichEditURL","first\r\nsecond\r\nthird\r\nfourth\r\nfifth\r\nsixth\r\nseventh\r\neighth\r\nninth\r\ntenth\r\neleventh\r\ntwelfth\r\nthirteenth\r\nfourteenth\r\nfifteenth\r\nsixteenth\r\nseventeenth\r\neighteenth\r\nnineteenth\r\ntwentieth",WS_CHILD|WS_VISIBLE|WS_VSCROLL|ES_MULTILINE|ES_AUTOVSCROLL,0,0,270,100,parent,(HMENU)7,instance,NULL);}
__declspec(dllexport) BOOL WINAPI FixtureUnregister(void){BOOL a=UnregisterClassA("TRichEditURL",instance),b=UnregisterClassA("RichFixtureParent",instance);if(rich)FreeLibrary(rich);return a&&b;}
BOOL WINAPI DllMain(HINSTANCE h,DWORD reason,LPVOID reserved){(void)reserved;if(reason==DLL_PROCESS_ATTACH)instance=h;return TRUE;}
