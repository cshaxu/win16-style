#include "spin_identity.h"
#include "../common/control_protocol.h"

BOOL win16_control_spin_identity(HWND w)
{
    HWND parent=GetParent(w);DWORD pid=0;WCHAR name[64],real[64],parent_name[64],parent_real[64];
    WNDCLASSEXW wide={.cbSize=sizeof(wide)};WNDCLASSEXA ansi={.cbSize=sizeof(ansi)};
    DWORD style=(DWORD)GetWindowLongPtrW(w,GWL_STYLE),ex=(DWORD)GetWindowLongPtrW(w,GWL_EXSTYLE);
    if(!GetClassInfoExW(GetModuleHandleW(NULL),L"TSpinButton",&wide) ||
       !GetClassInfoExA(GetModuleHandleW(NULL),"TSpinButton",&ansi))return FALSE;
    return parent && GetWindowThreadProcessId(w,&pid)==GetCurrentThreadId() && pid==GetCurrentProcessId() &&
        GetWindowThreadProcessId(parent,NULL)==GetCurrentThreadId() && (style&WS_CHILD) &&
        !(ex&(WS_EX_LAYOUTRTL|WS_EX_RTLREADING)) && GetClassLongPtrW(w,GCL_CBWNDEXTRA)==0 &&
        (HMODULE)GetClassLongPtrW(w,GCLP_HMODULE)==GetModuleHandleW(NULL) &&
        (HMODULE)GetClassLongPtrW(parent,GCLP_HMODULE)==GetModuleHandleW(NULL) &&
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
BOOL win16_control_modern_spin_shape(HWND w)
{
    HWND p=GetParent(w);WCHAR c[64],r[64],pc[64],pr[64];DWORD pid=0;
    HMODULE owner=(HMODULE)GetClassLongPtrW(w,GCLP_HMODULE);
    return p && owner && owner==(HMODULE)GetClassLongPtrW(p,GCLP_HMODULE) &&
        GetWindowThreadProcessId(w,&pid)==GetCurrentThreadId() && pid==GetCurrentProcessId() &&
        GetWindowThreadProcessId(p,NULL)==GetCurrentThreadId() && (GetWindowLongPtrW(w,GWL_STYLE)&WS_CHILD) &&
        GetClassLongPtrW(w,GCL_CBWNDEXTRA)==0 && !(GetWindowLongPtrW(w,GWL_EXSTYLE)&(WS_EX_LAYOUTRTL|WS_EX_RTLREADING)) &&
        GetClassNameW(w,c,64) && !lstrcmpW(c,L"TSpinButton") && RealGetWindowClassW(w,r,64) && !lstrcmpW(r,c) &&
        GetClassNameW(p,pc,64) && !lstrcmpW(pc,L"TSpinEdit") && RealGetWindowClassW(p,pr,64) && !lstrcmpW(pr,pc);
}

BOOL win16_control_spin_full_clip(HDC dc,const POINT pt[2])
{
    int saved=SaveDC(dc);if(!saved)return FALSE;
    SetViewportOrgEx(dc,0,0,NULL);SetWindowOrgEx(dc,0,0,NULL);
    RECT bounds;int kind=GetClipBox(dc,&bounds);BOOL covered=FALSE;
    if(kind!=ERROR && kind!=NULLREGION && bounds.left<=pt[0].x && bounds.top<=pt[0].y &&
       bounds.right>=pt[1].x && bounds.bottom>=pt[1].y){
        HRGN clip=CreateRectRgn(0,0,0,0);
        if(clip){
            int have=GetClipRgn(dc,clip);
            if(have==0)covered=TRUE;
            else if(have==1){
                HRGN missing=CreateRectRgn(pt[0].x,pt[0].y,pt[1].x,pt[1].y);
                if(missing){covered=CombineRgn(missing,missing,clip,RGN_DIFF)==NULLREGION;DeleteObject(missing);}
            }
            DeleteObject(clip);
        }
    }
    RestoreDC(dc,saved);return covered;
}

/* A VCL spin edit reserves part of its format rectangle for its own child.
 * Native WM_SETFONT can reset that reservation; never cover the button. */
HWND win16_control_spinedit_button(HWND w)
{
    WCHAR name[64];HWND child;
    if(!style_control_protocol_is(w,L"EDIT") || !GetClassNameW(w,name,ARRAYSIZE(name)) || lstrcmpW(name,L"TSpinEdit"))return NULL;
    child=FindWindowExW(w,NULL,L"TSpinButton",NULL);
    return child && GetParent(child)==w && GetWindowThreadProcessId(child,NULL)==GetCurrentThreadId() &&
        GetClassLongPtrW(child,GCLP_HMODULE)==GetClassLongPtrW(w,GCLP_HMODULE)?child:NULL;
}
