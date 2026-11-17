#include "control_status.h"
#include <commctrl.h>
#include <wchar.h>

void style_control_status_invalidate(UINT m,BOOL *font_invalid,style_control_status_state *state)
{
    if(m==WM_SETFONT || m==WM_FONTCHANGE || m==WM_SETTINGCHANGE || m==WM_THEMECHANGED || m==WM_DPICHANGED)*font_invalid=TRUE;
    if(m==WM_SETTEXT || m==SB_SETPARTS || m==SB_SETTEXTA || m==SB_SETTEXTW || m==SB_SIMPLE ||
        m==SB_SETICON || m==SB_SETMINHEIGHT || m==WM_SETFONT || m==WM_STYLECHANGED ||
        m==WM_SIZE || m==WM_ENABLE || m==WM_SETTINGCHANGE || m==WM_DESTROY || m==WM_NCDESTROY)++state->epoch;
}

static BOOL status_system_module(HMODULE image)
{
    WCHAR path[MAX_PATH],system[MAX_PATH],winsxs[MAX_PATH];DWORD n;
    n=GetModuleFileNameW(image,path,ARRAYSIZE(path));if(!n || n>=ARRAYSIZE(path))return FALSE;
    WCHAR *base=wcsrchr(path,L'\\');if(!base || lstrcmpiW(base+1,L"comctl32.dll"))return FALSE;
    *base=L'\0';n=GetSystemDirectoryW(system,ARRAYSIZE(system));
    if(n && n<ARRAYSIZE(system) && !lstrcmpiW(path,system))return TRUE;
    n=GetWindowsDirectoryW(winsxs,ARRAYSIZE(winsxs));if(!n || n+8>=ARRAYSIZE(winsxs))return FALSE;
    lstrcatW(winsxs,L"\\WinSxS\\");size_t length=wcslen(winsxs);
    return wcslen(path)>length && CompareStringOrdinal(path,(int)length,winsxs,(int)length,TRUE)==CSTR_EQUAL;
}

BOOL style_control_status_identity(HWND w,const style_instance *instance,const style_control_status_state *a,LPCWSTR class_name,HMODULE *native)
{
    WCHAR name[64];DWORD pid=0;WNDCLASSEXW cls={.cbSize=sizeof(cls)};HMODULE image=NULL;
    if(GetWindowThreadProcessId(w,&pid)!=GetCurrentThreadId() || pid!=GetCurrentProcessId() ||
        !(GetWindowLongPtrW(w,GWL_STYLE)&WS_CHILD) ||
        (GetWindowLongPtrW(w,GWL_EXSTYLE)&(WS_EX_LAYOUTRTL|WS_EX_RTLREADING)) ||
        !GetClassNameW(w,name,ARRAYSIZE(name)) || !GetClassInfoExW(NULL,STATUSCLASSNAMEW,&cls) ||
        !GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            (LPCWSTR)(UINT_PTR)cls.lpfnWndProc,&image) || !status_system_module(image) ||
        GetClassLongPtrW(w,GCL_CBWNDEXTRA)<(ULONG_PTR)cls.cbWndExtra)return FALSE;
    if(lstrcmpiW(name,STATUSCLASSNAMEW)){
        if(!a || !a->candidate || !a->witness_owner || instance->detached ||
            !class_name || lstrcmpW(name,class_name) || (HMODULE)GetClassLongPtrW(w,GCLP_HMODULE)!=a->witness_owner ||
            GetWindowThreadProcessId(GetParent(w),NULL)!=GetCurrentThreadId() ||
            (a->parent && GetParent(w)!=a->parent))return FALSE;
    }else if((HMODULE)GetClassLongPtrW(w,GCLP_HMODULE)!=image ||
        GetClassLongPtrW(w,GCLP_WNDPROC)!=(ULONG_PTR)cls.lpfnWndProc)return FALSE;
    if(native)*native=image;
    return TRUE;
}

BOOL style_control_status_settle(HWND w,UINT m,const style_instance *instance,style_control_status_state *state,LPCWSTR class_name)
{
    if(m!=WM_PAINT || !state->candidate || state->attempted || instance->detached || !state->witness_owner)return FALSE;
    DWORD error=GetLastError();BOOL installed=FALSE;state->attempted=TRUE;
    if(style_control_status_identity(w,instance,state,class_name,NULL)){
        state->parent=GetParent(w);state->installed=TRUE;installed=TRUE;InvalidateRect(w,NULL,TRUE);
    }
    if(installed)state->present=TRUE;
    SetLastError(error);return installed;
}
void style_control_status_read(HWND w,const style_instance *instance,const style_control_status_state *state,LPCWSTR class_name,style_control_status_snapshot *s)
{
    *s=(style_control_status_snapshot){.epoch=state->epoch};
    s->simple=style_control_status_identity(w,instance,state,class_name,&s->native) && SendMessageW(w,SB_ISSIMPLE,0,0) && !instance->detached && s->epoch==state->epoch;
    if(s->simple){
        LRESULT length=SendMessageW(w,SB_GETTEXTLENGTHW,0,0);
        if(instance->detached || s->epoch!=state->epoch || HIWORD(length)!=0 || LOWORD(length)==0 || LOWORD(length)>512)s->simple=FALSE;
        else {
            int copied=GetWindowTextW(w,s->simple_text,(int)LOWORD(length)+1);
            if(instance->detached || s->epoch!=state->epoch || copied!=(int)LOWORD(length) || !style_control_status_identity(w,instance,state,class_name,NULL))s->simple=FALSE;
            else s->simple_length=copied;
        }
    }
    if(!s->simple && style_control_status_identity(w,instance,state,class_name,&s->native) && !SendMessageW(w,SB_ISSIMPLE,0,0) && !instance->detached && s->epoch==state->epoch){
        LRESULT count=SendMessageW(w,SB_GETPARTS,0,0);
        if(count>0 && count<=64 && !instance->detached && s->epoch==state->epoch){
            for(unsigned i=0;i<(unsigned)count;++i){RECT r={0};LRESULT type=SendMessageW(w,SB_GETTEXTLENGTHW,i,0);
                if(instance->detached || s->epoch!=state->epoch)break;
                if((HIWORD(type)&~(SBT_OWNERDRAW|SBT_NOBORDERS)) || !SendMessageW(w,SB_GETRECT,i,(LPARAM)&r))continue;
                if(instance->detached || s->epoch!=state->epoch)break;
                if(!(HIWORD(type)&SBT_OWNERDRAW))s->text_panes[(s->text_count)++]=r;
                if(!(HIWORD(type)&SBT_NOBORDERS))s->panes[(s->count)++]=r;
            }
        }
    }
}
