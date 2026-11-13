#include "lib-style/common/control_vcl_spin.h"
#include "lib-win32/style/window_identity.h"
#include "control_protocol.h"

BOOL style_control_vcl_spin_identity(HWND w)
{
    HWND parent=GetParent(w);WCHAR name[64],real[64],parent_name[64],parent_real[64];
    WNDCLASSEXW wide={.cbSize=sizeof(wide)};WNDCLASSEXA ansi={.cbSize=sizeof(ansi)};
    DWORD ex=(DWORD)GetWindowLongPtrW(w,GWL_EXSTYLE);
    if(!GetClassInfoExW(GetModuleHandleW(NULL),L"TSpinButton",&wide) ||
       !GetClassInfoExA(GetModuleHandleW(NULL),"TSpinButton",&ansi))return FALSE;
    return win32_style_window_owned_child(w,GetModuleHandleW(NULL),parent) &&
        !(ex&(WS_EX_LAYOUTRTL|WS_EX_RTLREADING)) && GetClassLongPtrW(w,GCL_CBWNDEXTRA)==0 &&
        GetClassNameW(w,name,ARRAYSIZE(name)) && !lstrcmpW(name,L"TSpinButton") &&
        wide.lpfnWndProc && ansi.lpfnWndProc &&
        wide.lpfnWndProc!=DefWindowProcW && wide.lpfnWndProc!=DefWindowProcA &&
        ansi.lpfnWndProc!=DefWindowProcW && ansi.lpfnWndProc!=DefWindowProcA &&
        RealGetWindowClassW(w,real,ARRAYSIZE(real)) && !lstrcmpW(real,L"TSpinButton") &&
        GetClassNameW(parent,parent_name,ARRAYSIZE(parent_name)) && !lstrcmpW(parent_name,L"TSpinEdit") &&
        RealGetWindowClassW(parent,parent_real,ARRAYSIZE(parent_real)) && !lstrcmpiW(parent_real,L"EDIT");
}

/* Newer VCL paints a buffered themed TSpinButton under a witnessed EDIT.
 * Only a buffer derived in this exact paint and copied to this exact window
 * can be composed; unrelated or uncertain images keep their native pixels. */
BOOL style_control_vcl_modern_spin_shape(HWND w)
{
    HWND p=GetParent(w);WCHAR c[64],r[64],pc[64],pr[64];
    HMODULE owner=(HMODULE)GetClassLongPtrW(w,GCLP_HMODULE);
    return win32_style_window_owned_child(w,owner,p) &&
        GetClassLongPtrW(w,GCL_CBWNDEXTRA)==0 && !(GetWindowLongPtrW(w,GWL_EXSTYLE)&(WS_EX_LAYOUTRTL|WS_EX_RTLREADING)) &&
        GetClassNameW(w,c,64) && !lstrcmpW(c,L"TSpinButton") && RealGetWindowClassW(w,r,64) && !lstrcmpW(r,c) &&
        GetClassNameW(p,pc,64) && !lstrcmpW(pc,L"TSpinEdit") && RealGetWindowClassW(p,pr,64) && !lstrcmpW(pr,pc);
}

/* A VCL spin edit reserves part of its format rectangle for its own child.
 * Native WM_SETFONT can reset that reservation; never cover the button. */
HWND style_control_vcl_spinedit_button(HWND w)
{
    WCHAR name[64];HWND child;
    if(!style_control_protocol_is(w,L"EDIT") || !GetClassNameW(w,name,ARRAYSIZE(name)) || lstrcmpW(name,L"TSpinEdit"))return NULL;
    child=FindWindowExW(w,NULL,L"TSpinButton",NULL);
    return child && GetParent(child)==w && GetWindowThreadProcessId(child,NULL)==GetCurrentThreadId() &&
        GetClassLongPtrW(child,GCLP_HMODULE)==GetClassLongPtrW(w,GCLP_HMODULE)?child:NULL;
}

HWND style_control_vcl_spin_before(HWND w,UINT message,RECT *before)
{
    HWND button=NULL;DWORD error=GetLastError();
    if(message==WM_SETFONT && (button=style_control_vcl_spinedit_button(w))!=NULL)
        SendMessageW(w,EM_GETRECT,0,(LPARAM)before);
    SetLastError(error);return button;
}
void style_control_vcl_spin_after(HWND w,HWND spin_button,const RECT *before,const BOOL *detached)
{
    if(spin_button && !*detached && IsWindow(w) && style_control_vcl_spinedit_button(w)==spin_button){
        DWORD error=GetLastError();RECT button,after;
        if(GetWindowRect(spin_button,&button)){
            MapWindowPoints(NULL,w,(POINT*)&button,2);
            SendMessageW(w,EM_GETRECT,0,(LPARAM)&after);
            if(!*detached && before->right>before->left && before->right<=button.left &&
               after.right>button.left && before->right>after.left){
                after.right=before->right;SendMessageW(w,EM_SETRECTNP,0,(LPARAM)&after);
            }
        }
        SetLastError(error);
    }
}
