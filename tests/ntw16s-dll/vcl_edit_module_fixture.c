#define UNICODE
#include <windows.h>
#include "native_thunk.h"
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
    WNDCLASSEXW n={.cbSize=sizeof(n)},c={.cbSize=sizeof(c)};
    if(!GetClassInfoExW(NULL,L"EDIT",&n))return FALSE;
    native_edit=n.lpfnWndProc;fake=impostor;enabled=FALSE;
#ifdef T18_MASK
    WNDCLASSEXA ansi={.cbSize=sizeof(ansi)};if(!GetClassInfoExA(NULL,"EDIT",&ansi))return FALSE;native_edit_ansi=ansi.lpfnWndProc;
#endif
    thunk=test_create_native_thunk((void *)dispatch);if(!thunk)return FALSE;
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
#include "vcl_edit_create.h"
__declspec(dllexport) BOOL WINAPI FixtureUnregister(void)
{BOOL ok=UnregisterClassW(edit_class,owner);if(thunk){ok=VirtualFree(thunk,0,MEM_RELEASE)&&ok;thunk=NULL;}return ok;}
BOOL WINAPI DllMain(HINSTANCE h,DWORD reason,LPVOID unused)
{(void)unused;if(reason==DLL_PROCESS_ATTACH)owner=h;return TRUE;}
