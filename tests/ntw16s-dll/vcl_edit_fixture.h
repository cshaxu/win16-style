#ifndef TEST_VCL_EDIT_FIXTURE_H
#define TEST_VCL_EDIT_FIXTURE_H

typedef LRESULT (*foreign_fn)(WNDPROC,HWND,UINT,WPARAM,LPARAM);
static foreign_fn foreign_paint;
static HINSTANCE owner;
static WNDPROC native_edit;
static int fake;
static BOOL enabled;
static void *thunk;
static LRESULT CALLBACK dispatch(HWND w,UINT m,WPARAM p,LPARAM l)
{if (enabled && fake==2 && m==WM_PAINT && foreign_paint) return foreign_paint(native_edit,w,m,p,l);
 return (!enabled || (fake==1 && m==WM_PAINT))?DefWindowProcW(w,m,p,l):CallWindowProcW(native_edit,w,m,p,l);}
static LRESULT CALLBACK initial(HWND w,UINT m,WPARAM p,LPARAM l)
{LRESULT result=dispatch(w,m,p,l);if(m==WM_NCCREATE)SetWindowLongPtrW(w,GWLP_WNDPROC,(LONG_PTR)thunk);return result;}
__declspec(dllexport) void WINAPI FixtureSetForeign(foreign_fn fn){foreign_paint=fn;}

static BOOL register_native(const WNDCLASSEXW *n,int impostor)
{
    WNDCLASSEXW c={.cbSize=sizeof(c)};
    native_edit=n->lpfnWndProc;fake=impostor;enabled=FALSE;
    thunk=test_create_native_thunk((void *)dispatch);if(!thunk)return FALSE;
    c.style=CS_DBLCLKS|CS_HREDRAW|CS_VREDRAW;c.cbWndExtra=n->cbWndExtra;c.hCursor=LoadCursorW(NULL,IDC_IBEAM);
    c.hInstance=owner;c.lpszClassName=EDIT_CLASS;c.lpfnWndProc=initial;
    if(RegisterClassExW(&c))return TRUE;
    VirtualFree(thunk,0,MEM_RELEASE);thunk=NULL;return FALSE;
}

__declspec(dllexport) BOOL WINAPI FixtureUnregister(void)
{BOOL ok=UnregisterClassW(EDIT_CLASS,owner);if(thunk){ok=VirtualFree(thunk,0,MEM_RELEASE)&&ok;thunk=NULL;}return ok;}
BOOL WINAPI DllMain(HINSTANCE h,DWORD reason,LPVOID unused)
{(void)unused;if(reason==DLL_PROCESS_ATTACH)owner=h;return TRUE;}

#endif
