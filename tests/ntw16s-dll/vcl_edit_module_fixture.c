#define UNICODE
#include <windows.h>
#include <string.h>
/* Test-only DLL: its class is owned outside the EXE. The synthetic delayed
 * native initialization exercises creation-scope admission; it does not
 * claim to reproduce a Delphi runtime's actual creation order. */
#ifdef T18_MASK
#define EDIT_CLASS L"TMaskEdit"
#elif defined(T18_MEMO)
#define EDIT_CLASS L"TMemo"
#else
#define EDIT_CLASS L"TEdit"
#endif
static HINSTANCE owner;
static WNDPROC native_edit;
#ifdef T18_MASK
static WNDPROC native_edit_ansi;
static BOOL dbedit;
static WNDPROC foreign_edit;
static const WCHAR *edit_class=EDIT_CLASS;
#else
#define edit_class EDIT_CLASS
#endif
static BOOL fake,enabled;
static void *thunk;
static LRESULT CALLBACK dispatch(HWND w,UINT m,WPARAM p,LPARAM l)
{if(!enabled || (fake==1 && m==WM_PAINT))return DefWindowProcW(w,m,p,l);
#ifdef T18_MASK
 if(dbedit && fake==2 && m==WM_PAINT && foreign_edit)return foreign_edit(w,m,p,l);
 if(dbedit && m==WM_PAINT)return CallWindowProcA(native_edit_ansi,w,m,p,l);
#endif
 return CallWindowProcW(native_edit,w,m,p,l);}
static LRESULT CALLBACK initial(HWND w,UINT m,WPARAM p,LPARAM l)
{LRESULT result=dispatch(w,m,p,l);if(m==WM_NCCREATE)SetWindowLongPtrW(w,GWLP_WNDPROC,(LONG_PTR)thunk);return result;}
static BOOL register_fixture(BOOL impostor)
{
    WNDCLASSEXW n={.cbSize=sizeof(n)},c={.cbSize=sizeof(c)};DWORD old;
    if(!GetClassInfoExW(NULL,L"EDIT",&n))return FALSE;
    native_edit=n.lpfnWndProc;fake=impostor;enabled=FALSE;
#ifdef T18_MASK
    WNDCLASSEXA ansi={.cbSize=sizeof(ansi)};if(!GetClassInfoExA(NULL,"EDIT",&ansi))return FALSE;native_edit_ansi=ansi.lpfnWndProc;
#endif
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
    c.hInstance=owner;c.lpszClassName=edit_class;c.lpfnWndProc=initial;
    if(RegisterClassExW(&c))return TRUE;
    VirtualFree(thunk,0,MEM_RELEASE);thunk=NULL;return FALSE;
}
__declspec(dllexport) BOOL WINAPI FixtureRegister(BOOL impostor)
{
#ifdef T18_MASK
    dbedit=FALSE;edit_class=EDIT_CLASS;
#endif
    return register_fixture(impostor);
}
#ifdef T18_MASK
__declspec(dllexport) void WINAPI FixtureSetForeign(WNDPROC proc)
{foreign_edit=proc;}
__declspec(dllexport) BOOL WINAPI FixtureRegisterDbEdit(BOOL impostor)
{dbedit=TRUE;edit_class=L"TDBEdit";return register_fixture(impostor);}
#endif
__declspec(dllexport) HWND WINAPI FixtureCreate(HWND parent,BOOL multiline)
{
    DWORD style=WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS|WS_TABSTOP|ES_AUTOHSCROLL|ES_AUTOVSCROLL;
#ifdef T18_MEMO
    multiline=TRUE;
#endif
    if(multiline)style|=ES_MULTILINE|WS_VSCROLL|WS_HSCROLL;
    HWND w=CreateWindowExW(WS_EX_CLIENTEDGE,edit_class,L"probe",style,20,20,220,multiline?70:28,parent,NULL,owner,NULL);
    if(w){CREATESTRUCTW cs={0};cs.hwndParent=parent;cs.hInstance=owner;cs.style=GetWindowLongPtrW(w,GWL_STYLE);
        cs.dwExStyle=GetWindowLongPtrW(w,GWL_EXSTYLE);cs.lpszClass=edit_class;cs.lpszName=L"probe";cs.cx=220;cs.cy=28;
        enabled=TRUE;CallWindowProcW(native_edit,w,WM_NCCREATE,0,(LPARAM)&cs);CallWindowProcW(native_edit,w,WM_CREATE,0,(LPARAM)&cs);}
#ifdef T18_MEMO
    if(w)SendMessageW(w,EM_SETLIMITTEXT,0,0);
#endif
    return w;
}
__declspec(dllexport) BOOL WINAPI FixtureUnregister(void)
{BOOL ok=UnregisterClassW(edit_class,owner);if(thunk){ok=VirtualFree(thunk,0,MEM_RELEASE)&&ok;thunk=NULL;}return ok;}
BOOL WINAPI DllMain(HINSTANCE h,DWORD reason,LPVOID unused)
{(void)unused;if(reason==DLL_PROCESS_ATTACH)owner=h;return TRUE;}
