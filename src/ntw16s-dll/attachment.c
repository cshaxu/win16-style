#include "vb_text.h"
#include "control_class.h"
#include "ntw16s.h"
#include "native_api.h"
#include "theme.h"
#include "static_text.h"
#include "injection/unwind.h"
#include <commctrl.h>
#include <stdlib.h>
#include <wchar.h>

/* This coordinator composes the one existing renderer. It observes only
 * creation inside explicit caller scopes; it never scans a desktop or infers
 * task policy from an owner, title, executable name or a process-wide switch. */
typedef struct pending_window {struct pending_window *next;HWND window;DWORD style;BOOL settled,installed;} pending_window;
struct ntw16s_creation_scope {
    struct ntw16s_creation_scope *previous;
    DWORD thread;
    BOOL enabled;
    pending_window *pending;
    ntw16s_creation_result result;
};
typedef struct attachment {
    unsigned calls;
    BOOL detached,tree,file_owner,picture,button_owner,checklist,status,spin,static_candidate,static_verified;
    BOOL edit_candidate,edit_attempted,edit_installed;
    BOOL track_candidate,track_attempted,track_installed;
    DWORD edit_original_style;
    HMODULE edit_witness_owner;
    HMODULE track_witness_owner;
    unsigned status_epoch;
    HWND themed,report_window,report_parent;
    HMODULE report_module;
    ntw16s_vb_fonts fonts;
    ntw16s_control_protocol *protocol;
} attachment;
static _Thread_local ntw16s_creation_scope *current;
typedef struct static_paint_scope {struct static_paint_scope *previous;attachment *owner;HWND window;BOOL completed;} static_paint_scope;
static _Thread_local static_paint_scope *static_paint;
static _Thread_local ntw16s_static_call *static_call;
static _Thread_local HHOOK creation_return_hook;
static _Thread_local HHOOK creation_before_hook;
typedef struct checklist_draw_scope {
    struct checklist_draw_scope *previous;
    attachment *owner;
    HDC dc;
    RECT item;
} checklist_draw_scope;
static _Thread_local checklist_draw_scope *checklist_current;

/* Status frames are drawn by the native common control before text, icons
 * and WM_DRAWITEM. A per-message snapshot makes nested unrelated drawing a
 * barrier and allows a changing or destroyed control to fall back safely. */
typedef struct status_draw_scope {
    struct status_draw_scope *previous;
    attachment *owner; HWND window; HDC dc; HMODULE native;
    unsigned epoch,count; RECT panes[64];
} status_draw_scope;
static _Thread_local status_draw_scope *status_current;
static LONG status_hooks_ready;
void ntw16s_status_hooks_enable(void){InterlockedExchange(&status_hooks_ready,1);}

/* Scope is active only while dispatching the observed control's own paint or
 * pointer/enable messages. Unrelated nested attachment messages are barriers. */
typedef struct spin_draw_scope {attachment *owner;HWND window,parent;HDC dc;} spin_draw_scope;
static _Thread_local spin_draw_scope *spin_current;

static BOOL spin_identity(HWND w)
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

/* Forward the native primitive first. Its adjusted rectangle, return value and
 * error remain caller-owned; this overlay never draws the application arrow. */
void ntw16s_spin_frame(HDC dc,const RECT *rect,UINT type,UINT flags,const void *caller)
{
    spin_draw_scope *s=spin_current;HMODULE module=NULL;RECT client,clip;POINT points[2];
    const UINT allowed=DFCS_BUTTONPUSH|DFCS_PUSHED|DFCS_ADJUSTRECT;
    if(!s || !s->owner || s->owner->detached || !dc || !rect || type!=DFC_BUTTON ||
       (flags&~allowed) || (flags&(DFCS_BUTTONPUSH|DFCS_ADJUSTRECT))!=(DFCS_BUTTONPUSH|DFCS_ADJUSTRECT) ||
       !spin_identity(s->window) || GetParent(s->window)!=s->parent || WindowFromDC(dc)!=s->window ||
       (s->dc && s->dc!=dc) || GetMapMode(dc)!=MM_TEXT || (GetLayout(dc)&LAYOUT_RTL) ||
       !GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            (LPCWSTR)caller,&module) || module!=GetModuleHandleW(NULL) || !GetClientRect(s->window,&client) ||
       client.right<=0 || client.bottom<=0 || client.right>256 || client.bottom>256 ||
       rect->left!=0 || rect->top!=0 || rect->right!=client.right || rect->bottom<=0 ||
       GetClipBox(dc,&clip)==ERROR || clip.left<rect->left || clip.top<rect->top ||
       clip.right>rect->right || clip.bottom>rect->bottom)return;
    points[0]=(POINT){rect->left,rect->top};points[1]=(POINT){rect->right,rect->bottom};
    if(!LPtoDP(dc,points,2) || points[0].x!=0 || points[1].x!=client.right ||
       points[0].y<0 || points[1].y>client.bottom || points[1].y<=points[0].y ||
       (points[0].y!=0 && points[1].y!=client.bottom) || s->owner->detached)return;
    int saved=SaveDC(dc);if(!saved)return;
    FrameRect(dc,rect,(HBRUSH)GetStockObject(BLACK_BRUSH));RestoreDC(dc,saved);
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

static BOOL status_identity(HWND w,HMODULE *native)
{
    WCHAR name[64];DWORD pid=0;WNDCLASSEXW cls={.cbSize=sizeof(cls)};HMODULE image=NULL;
    if(GetWindowThreadProcessId(w,&pid)!=GetCurrentThreadId() || pid!=GetCurrentProcessId() ||
        !(GetWindowLongPtrW(w,GWL_STYLE)&WS_CHILD) ||
        (GetWindowLongPtrW(w,GWL_EXSTYLE)&(WS_EX_LAYOUTRTL|WS_EX_RTLREADING)) ||
        !GetClassNameW(w,name,ARRAYSIZE(name)) || lstrcmpiW(name,STATUSCLASSNAMEW) ||
        !GetClassInfoExW(NULL,STATUSCLASSNAMEW,&cls) ||
        !GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            (LPCWSTR)(UINT_PTR)cls.lpfnWndProc,&image) ||
        !status_system_module(image) ||
        (HMODULE)GetClassLongPtrW(w,GCLP_HMODULE)!=image ||
        GetClassLongPtrW(w,GCLP_WNDPROC)!=(ULONG_PTR)cls.lpfnWndProc ||
        GetClassLongPtrW(w,GCL_CBWNDEXTRA)<(ULONG_PTR)cls.cbWndExtra)return FALSE;
    if(native)*native=image;
    return TRUE;
}

/* Called only after the original native drawing call has completed. */
void ntw16s_status_frame(HDC dc,const RECT *rect,const RECT *clip,const void *caller)
{
    status_draw_scope *s=status_current;HMODULE module=NULL;HWND w,scoped=NULL;BOOL matches=FALSE;POINT origin;
    if(!s || !s->owner || s->owner->detached || s->epoch!=s->owner->status_epoch || !dc || !rect ||
        !status_identity(s->window,NULL) || GetMapMode(dc)!=MM_TEXT || (GetLayout(dc)&LAYOUT_RTL) ||
        !GetViewportOrgEx(dc,&origin) || origin.x || origin.y ||
        !GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            (LPCWSTR)caller,&module) || module!=s->native)return;
    if(ntw16s_native_paint_target(dc,&scoped) && scoped!=s->window)return;
    w=WindowFromDC(dc);if(s->dc?dc!=s->dc:w!=s->window)return;
    for(unsigned i=0;i<s->count;++i)if(EqualRect(rect,&s->panes[i])){matches=TRUE;break;}
    if(!matches || s->owner->detached || s->epoch!=s->owner->status_epoch)return;
    int saved=SaveDC(dc);if(!saved)return;
    if(!clip || IntersectClipRect(dc,clip->left,clip->top,clip->right,clip->bottom)!=ERROR)
        FrameRect(dc,rect,(HBRUSH)GetStockObject(BLACK_BRUSH));
    RestoreDC(dc,saved);
}

/* This precise superclass was observed in a real VCL application. It still
 * owns every item and callback; this identity never aliases generic lists. */
static BOOL checklist_identity(HWND w)
{
    WCHAR name[64],real[64];DWORD style=(DWORD)GetWindowLongPtrW(w,GWL_STYLE);
    DWORD ex=(DWORD)GetWindowLongPtrW(w,GWL_EXSTYLE),pid=0;
    return GetWindowThreadProcessId(w,&pid)==GetCurrentThreadId() && pid==GetCurrentProcessId() &&
        GetWindowThreadProcessId(GetParent(w),NULL)==GetCurrentThreadId() &&
        (HMODULE)GetClassLongPtrW(w,GCLP_HMODULE)==GetModuleHandleW(NULL) &&
        GetClassNameW(w,name,ARRAYSIZE(name)) && !lstrcmpW(name,L"THintCheckListBox") &&
        RealGetWindowClassW(w,real,ARRAYSIZE(real)) && !lstrcmpiW(real,L"LISTBOX") &&
        (style&(WS_CHILD|LBS_OWNERDRAWFIXED|LBS_OWNERDRAWVARIABLE|LBS_NODATA|LBS_HASSTRINGS))==
            (WS_CHILD|LBS_OWNERDRAWFIXED|LBS_HASSTRINGS) &&
        !(ex&(WS_EX_LAYOUTRTL|WS_EX_RTLREADING));
}
BOOL ntw16s_checklist_draw_target(HDC dc,const RECT *r,const void *address)
{
    checklist_draw_scope *s=checklist_current;HMODULE module=NULL;POINT origin;
    if(!s || !s->owner || s->owner->detached || !r || dc!=s->dc ||
        !checklist_identity(s->owner->report_window) || GetParent(s->owner->report_window)!=s->owner->report_parent ||
        WindowFromDC(dc)!=s->owner->report_window || GetMapMode(dc)!=MM_TEXT || (GetLayout(dc)&LAYOUT_RTL) ||
        !GetViewportOrgEx(dc,&origin) || origin.x || origin.y ||
        !GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            (const WCHAR *)address,&module) || module!=(HMODULE)GetClassLongPtrW(s->owner->report_window,GCLP_HMODULE))return FALSE;
    int dpi=(int)GetDpiForWindow(s->owner->report_window);
    int max_side=MulDiv(16,dpi,96);
    return dpi>0 && r->left>=s->item.left && r->top>=s->item.top &&
        r->right<=s->item.right && r->bottom<=s->item.bottom &&
        r->left-s->item.left<=max_side && r->right-r->left>0 && r->bottom-r->top>0 &&
        r->right-r->left<=max_side && r->bottom-r->top<=max_side;
}
static LRESULT CALLBACK attachment_proc(HWND,UINT,WPARAM,LPARAM,UINT_PTR,DWORD_PTR);
static void report_try_bind(HWND,attachment *);

static attachment *lookup(HWND w)
{
    DWORD_PTR value;
    return GetWindowSubclass(w,attachment_proc,(UINT_PTR)attachment_proc,&value)?(attachment *)value:NULL;
}
BOOL ntw16s_static_call_begin(ntw16s_static_call *scope,WNDPROC proc,HWND w,UINT message,const void *address)
{
    WNDCLASSEXW native={.cbSize=sizeof(native)};HMODULE caller=NULL,native_owner=NULL,owner;
    ZeroMemory(scope,sizeof(*scope));
    if(message==WM_PAINT && static_paint && static_paint->window==w &&
       static_paint->owner->track_candidate && !static_paint->owner->detached &&
       !static_paint->owner->track_attempted && lookup(w)==static_paint->owner &&
       GetClassInfoExW(NULL,TRACKBAR_CLASSW,&native) && proc==native.lpfnWndProc &&
       GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
           (LPCWSTR)address,&caller)){
        owner=(HMODULE)GetClassLongPtrW(w,GCLP_HMODULE);
        if(owner && caller==owner)static_paint->owner->track_witness_owner=owner;
    }
    native.cbSize=sizeof(native);
    if(message==WM_PAINT && static_paint && static_paint->window==w &&
       static_paint->owner->edit_candidate && !static_paint->owner->detached &&
       !static_paint->owner->edit_attempted && lookup(w)==static_paint->owner &&
       GetClassInfoExW(NULL,L"EDIT",&native) && proc==native.lpfnWndProc &&
       GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
           (LPCWSTR)address,&caller)){
        owner=(HMODULE)GetClassLongPtrW(w,GCLP_HMODULE);
        if(owner && caller==owner)static_paint->owner->edit_witness_owner=owner;
    }
    native.cbSize=sizeof(native);
    if(message!=WM_PAINT || !static_paint || static_paint->window!=w ||
       static_paint->owner->detached || lookup(w)!=static_paint->owner ||
       !GetClassInfoExW(NULL,L"STATIC",&native) || proc!=native.lpfnWndProc ||
       !GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            (LPCWSTR)address,&caller) ||
       !GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            (LPCWSTR)(UINT_PTR)proc,&native_owner))return FALSE;
    owner=(HMODULE)GetClassLongPtrW(w,GCLP_HMODULE);
    if(!owner || caller!=owner || !native_owner)return FALSE;
    scope->previous=static_call;scope->window=w;scope->native_module=native_owner;
    scope->cancelled=&static_paint->owner->detached;scope->active=TRUE;
    static_call=scope;static_paint->owner->static_verified=TRUE;return TRUE;
}
BOOL ntw16s_static_call_suspend(ntw16s_static_call *scope)
{
    if(!static_call)return FALSE;
    ZeroMemory(scope,sizeof(*scope));scope->previous=static_call;scope->active=TRUE;static_call=scope;return TRUE;
}
void ntw16s_static_call_end(void *data)
{
    ntw16s_static_call *scope=data;if(scope->active){static_call=scope->previous;scope->active=FALSE;}
}
HFONT ntw16s_static_text_font(HDC dc,LPCWSTR text,int count,UINT flags,const void *address)
{
    ntw16s_static_call *s=static_call;HMODULE caller=NULL;HWND from;HFONT source;
    if(!s || !s->active || !s->cancelled || *s->cancelled || !static_paint ||
       static_paint->window!=s->window || !dc || !text || (flags&(DT_CALCRECT|DT_RTLREADING|DT_MODIFYSTRING)) ||
       (GetLayout(dc)&LAYOUT_RTL) || !GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|
       GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,(LPCWSTR)address,&caller) || caller!=s->native_module)return NULL;
    from=WindowFromDC(dc);if(from && from!=s->window)return NULL;
    if(count< -1)return NULL;
    if(count==-1){count=0;while(count<4096 && text[count])++count;if(count==4096)return NULL;}
    if(count>4096)return NULL;
    for(int i=0;i<count;++i)if(text[i]>0x7f)return NULL;
    source=(HFONT)GetCurrentObject(dc,OBJ_FONT);return source?win16_style_system_acquire(source):NULL;
}
#define class_is ntw16s_class_is
static void release(attachment *a)
{if(!--a->calls && a->detached){ntw16s_vb_fonts_clear(&a->fonts);free(a);}}
static void static_paint_end(void *data)
{static_paint_scope *scope=data;static_paint=scope->previous;if(!scope->completed)release(scope->owner);}
static NTW16S_UNWIND_FUNCTION LRESULT static_paint_dispatch(attachment *a,HWND w,UINT m,WPARAM p,LPARAM l)
{
    static_paint_scope scope={static_paint,a,w,FALSE};ntw16s_unwind_scope unwind;LRESULT result;DWORD entry=GetLastError(),error;
    static_paint=((a->static_candidate || a->edit_candidate || a->track_candidate) && !a->detached && m==WM_PAINT)?&scope:NULL;
    NTW16S_UNWIND_BEGIN(unwind,static_paint_end,&scope);
    SetLastError(entry);result=DefSubclassProc(w,m,p,l);error=GetLastError();scope.completed=TRUE;
    NTW16S_UNWIND_END(unwind);SetLastError(error);return result;
}
static LRESULT CALLBACK attachment_proc(HWND w,UINT m,WPARAM p,LPARAM l,UINT_PTR id,DWORD_PTR data)
{
    attachment *a=(attachment *)data;LRESULT result;DWORD status_error=GetLastError();BOOL status=a->status||a->spin;(void)id;
    ++a->calls;
    if(a->status && (m==SB_SETPARTS || m==SB_SETTEXTA || m==SB_SETTEXTW || m==SB_SIMPLE ||
        m==SB_SETICON || m==SB_SETMINHEIGHT || m==WM_SETFONT || m==WM_STYLECHANGED ||
        m==WM_SIZE || m==WM_ENABLE || m==WM_SETTINGCHANGE || m==WM_DESTROY || m==WM_NCDESTROY))++a->status_epoch;
    status_draw_scope scope={0},*prior=status_current;
    spin_draw_scope spin_scope={0},*spin_prior=spin_current;
    BOOL spin_message=a->spin && !a->detached && (m==WM_PAINT || m==WM_PRINT || m==WM_PRINTCLIENT ||
        m==WM_LBUTTONDOWN || m==WM_LBUTTONUP || m==WM_MOUSEMOVE || m==WM_ENABLE || m==WM_CANCELMODE ||
        m==WM_SETFOCUS || m==WM_KILLFOCUS);
    if(spin_prior)spin_current=NULL;
    if(spin_message){spin_scope.owner=a;spin_scope.window=w;spin_scope.parent=GetParent(w);
        spin_scope.dc=(m==WM_PRINT || m==WM_PRINTCLIENT)?(HDC)p:NULL;spin_current=&spin_scope;}
    BOOL painting=a->status && !a->detached && (m==WM_PAINT || m==WM_PRINT || m==WM_PRINTCLIENT);
    if(prior && prior->window!=w)status_current=NULL;
    if(painting){
        scope.previous=status_current;scope.owner=a;scope.window=w;scope.dc=m==WM_PAINT?NULL:(HDC)p;scope.epoch=a->status_epoch;
        if(status_identity(w,&scope.native) && !SendMessageW(w,SB_ISSIMPLE,0,0) && !a->detached && scope.epoch==a->status_epoch){
            LRESULT count=SendMessageW(w,SB_GETPARTS,0,0);
            if(count>0 && count<=64 && !a->detached && scope.epoch==a->status_epoch){
                for(unsigned i=0;i<(unsigned)count;++i){RECT r={0};LRESULT type=SendMessageW(w,SB_GETTEXTLENGTHW,i,0);
                    if(a->detached || scope.epoch!=a->status_epoch)break;
                    if((HIWORD(type)&~SBT_OWNERDRAW) || !SendMessageW(w,SB_GETRECT,i,(LPARAM)&r))continue;
                    if(a->detached || scope.epoch!=a->status_epoch)break;
                    scope.panes[scope.count++]=r;
                }
            }
        }
        status_current=&scope;
    }
    if(a->picture && !a->detached && m!=WM_DESTROY && m!=WM_NCDESTROY && !ntw16s_vb_picture(w))
        (void)Ntw16sWindowRemove(w);
    if(a->button_owner && !a->detached && m!=WM_DESTROY && m!=WM_NCDESTROY && !ntw16s_vb_graphical_button(w))
        (void)Ntw16sWindowRemove(w);
    if((a->file_owner || a->button_owner || a->checklist) && !a->detached && a->report_parent && m!=WM_DESTROY && m!=WM_NCDESTROY && GetParent(w)!=a->report_parent)
        (void)Ntw16sWindowRemove(w);
    if(m==WM_NCDESTROY)(void)Ntw16sWindowTerminal(w);
    ntw16s_vb_paint paint;ntw16s_vb_paint_begin(&paint,w,m,&a->fonts);
    if(a->file_owner || a->picture || a->button_owner)paint.cancelled=&a->detached;
    if(paint.native_active)paint.native_dc=(HDC)p;
    if(paint.active && !a->detached && !a->protocol)report_try_bind(w,a);
    if(paint.active && a->report_module && !a->file_owner && !a->detached){
        DWORD style=(DWORD)GetWindowLongPtrW(w,GWL_STYLE);
        if((a->tree || ((style&LVS_TYPEMASK)==LVS_REPORT && !(style&(LVS_OWNERDRAWFIXED|LVS_OWNERDATA)) &&
            !SendMessageW(w,LVM_ISGROUPVIEWENABLED,0,0))) && !a->detached)
            ntw16s_report_paint_enable(&paint,w,a->report_module,&a->detached);
    }
    if(status)SetLastError(status_error);
    if(static_paint || ((a->static_candidate || a->edit_candidate || a->track_candidate) && !a->detached && m==WM_PAINT))
        result=static_paint_dispatch(a,w,m,p,l);
    else result=DefSubclassProc(w,m,p,l);
    if(status)status_error=GetLastError();
    ntw16s_vb_paint_end(&paint);
    status_current=prior;
    spin_current=spin_prior;
    if(m==WM_PAINT && a->edit_candidate && !a->edit_attempted && !a->detached && a->edit_witness_owner){
        DWORD saved=GetLastError();a->edit_attempted=TRUE;
        ntw16s_control_protocol *protocol=ntw16s_control_protocol_bind_verified_edit(w,a->edit_witness_owner);
        if(protocol && !a->detached){
            a->protocol=protocol;
            BOOL installed=win16_style_control_install_created(w,TRUE,(a->edit_original_style&WS_BORDER)!=0,FALSE,NULL,a,&a->detached);
            if(installed && !a->detached && (a->edit_original_style&ES_MULTILINE)){
                a->themed=w;
                installed=ntw16s_theme_enable(w,a) && win16_style_edit_scrollbars_enable(w);
            }
            if(installed && !a->detached){a->edit_installed=TRUE;InvalidateRect(w,NULL,TRUE);}
            else if(!a->detached){win16_style_control_remove_owned(w,a);if(a->themed){ntw16s_theme_remove(w,a);a->themed=NULL;}
                ntw16s_control_protocol_release(a->protocol);a->protocol=NULL;}
        }else if(protocol)ntw16s_control_protocol_release(protocol);
        SetLastError(saved);
    }
    if(m==WM_PAINT && a->track_candidate && !a->track_attempted && !a->detached && a->track_witness_owner){
        DWORD saved=GetLastError();a->track_attempted=TRUE;
        ntw16s_control_protocol *protocol=ntw16s_control_protocol_bind_verified_trackbar(w,a->track_witness_owner);
        if(protocol && !a->detached){
            a->protocol=protocol;
            BOOL installed=win16_style_trackbar_install_created(w,NULL,a,&a->detached);
            if(installed && !a->detached){a->track_installed=TRUE;InvalidateRect(w,NULL,TRUE);}
            else if(!a->detached){win16_style_control_remove_owned(w,a);ntw16s_control_protocol_release(a->protocol);a->protocol=NULL;}
        }else if(protocol)ntw16s_control_protocol_release(protocol);
        SetLastError(saved);
    }
    release(a);if(status)SetLastError(status_error);return result;
}
/* Observe this verified child's native custom/owner-draw DC. Never request extra stages,
 * rewrite the application's result, select its font, or inspect item lParam. */
static LRESULT CALLBACK report_notify(HWND w,UINT m,WPARAM p,LPARAM l,UINT_PTR id,DWORD_PTR data)
{
    DWORD entry=GetLastError();attachment *a=(attachment *)data;BOOL draw=FALSE;HWND list=a->report_window;HDC dc=NULL;DWORD stage=0;
    ntw16s_vb_paint file_paint;BOOL file_active=FALSE;
    checklist_draw_scope checklist_scope;BOOL checklist_active=FALSE;
    ++a->calls;
    if(a->checklist && !a->detached && m==WM_DRAWITEM && l && GetParent(list)==w && checklist_identity(list)){
        const DRAWITEMSTRUCT *item=(const DRAWITEMSTRUCT *)l;
        if(item->CtlType==ODT_LISTBOX && item->hwndItem==list && item->hDC && item->itemID!=(UINT)-1 &&
            WindowFromDC(item->hDC)==list){
            checklist_scope=(checklist_draw_scope){checklist_current,a,item->hDC,item->rcItem};
            checklist_current=&checklist_scope;checklist_active=TRUE;
        }
    }
    if((a->file_owner || a->button_owner) && !a->detached && m==WM_DRAWITEM && l && GetParent(list)==w &&
            (a->button_owner?ntw16s_vb_graphical_button(list):ntw16s_vb_owner_file(list))){
        DRAWITEMSTRUCT *item=(DRAWITEMSTRUCT *)l;
        if(item->hwndItem==list && item->hDC && item->CtlType==(a->button_owner?ODT_BUTTON:ntw16s_vb_drive(list)?ODT_COMBOBOX:ODT_LISTBOX)){
            HWND target=WindowFromDC(item->hDC);BOOL valid=!target || target==list;
            if(!valid && ntw16s_vb_drive(list)){
                COMBOBOXINFO combo={.cbSize=sizeof(combo)};
                valid=GetComboBoxInfo(list,&combo) && combo.hwndCombo==list && target==combo.hwndList &&
                    GetWindowThreadProcessId(target,NULL)==GetCurrentThreadId();
            }
            if(valid && !a->detached){
                ntw16s_vb_paint_begin(&file_paint,list,WM_PRINTCLIENT,&a->fonts);file_active=file_paint.active;
                if(file_active){HWND painted=target?target:list;
                    ntw16s_report_paint_enable(&file_paint,painted,a->report_module,&a->detached);
                    ntw16s_report_paint_bind(painted,item->hDC);
                }
            }
        }
    }
    if(!a->file_owner && !a->button_owner && !a->checklist && m==WM_NOTIFY && l){
        NMHDR *header=(NMHDR *)l;
        if(header->hwndFrom==list && header->code==NM_CUSTOMDRAW){
            NMCUSTOMDRAW *cd=(NMCUSTOMDRAW *)l;dc=cd->hdc;stage=cd->dwDrawStage;draw=TRUE;
            ntw16s_report_paint_bind(list,NULL);
        }
    }
    if(m==WM_NCDESTROY){RemoveWindowSubclass(w,report_notify,id);a->report_parent=NULL;}
    SetLastError(entry);LRESULT result=DefSubclassProc(w,m,p,l);DWORD error=GetLastError();
    if(checklist_active)checklist_current=checklist_scope.previous;
    if(file_active)ntw16s_vb_paint_end(&file_paint);
    if(draw && !a->detached && !(result&CDRF_SKIPDEFAULT) &&
        (stage==CDDS_PREPAINT || stage==CDDS_ITEMPREPAINT || stage==(CDDS_ITEMPREPAINT|CDDS_SUBITEM)))
        ntw16s_report_paint_bind(list,dc);
    release(a);SetLastError(error);return result;
}
/* Some frameworks set the report view after native WM_CREATE. This is still
 * our exact newly created attachment, on its GUI thread, after initialization.
 * A failed public protocol remains native; there is no existing-window scan. */
static void report_try_bind(HWND w,attachment *a)
{
    WCHAR name[64];
    if(!GetClassNameW(w,name,ARRAYSIZE(name)) ||
        (lstrcmpW(name,WC_LISTVIEWW) && lstrcmpW(name,L"TListView") &&
         lstrcmpW(name,WC_TREEVIEWW) && lstrcmpW(name,L"TTreeView") && lstrcmpW(name,L"TShellTreeView") &&
         lstrcmpW(name,L"TreeView20WndClass")))return;
    ntw16s_control_protocol *protocol=ntw16s_control_protocol_bind(w);
    if(!protocol)return;
    HMODULE module=ntw16s_control_protocol_list_module(w);
    BOOL tree=ntw16s_control_protocol_tree_module(w)!=NULL;if(tree)module=ntw16s_control_protocol_tree_module(w);
    HWND parent=GetParent(w);
    if(!a->detached && module && parent &&
        SetWindowSubclass(parent,report_notify,(UINT_PTR)w,(DWORD_PTR)a)){
        a->protocol=protocol;a->report_window=w;a->report_parent=parent;a->report_module=module;a->tree=tree;
    }else ntw16s_control_protocol_release(protocol);
}
static void report_unbind(attachment *a)
{
    DWORD_PTR value;
    if(a->report_parent && GetWindowSubclass(a->report_parent,report_notify,(UINT_PTR)a->report_window,&value) &&
        value==(DWORD_PTR)a)RemoveWindowSubclass(a->report_parent,report_notify,(UINT_PTR)a->report_window);
    a->report_parent=NULL;
}
BOOL __cdecl Ntw16sWindowAttached(HWND w)
{
    if(GetWindowThreadProcessId(w,NULL)!=GetCurrentThreadId()){
        SetLastError(ERROR_INVALID_THREAD_ID);return FALSE;
    }
    attachment *a=lookup(w);return a && (!a->static_candidate || a->static_verified) && (!a->edit_candidate || a->edit_installed) && (!a->track_candidate || a->track_installed);
}
void __cdecl Ntw16sScrollChanged(HWND w)
{
    if(GetWindowThreadProcessId(w,NULL)==GetCurrentThreadId() && lookup(w)){
        if(class_is(w,L"SCROLLBAR"))RedrawWindow(w,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW);
        else win16_style_scrollbars_paint(w,NULL,NULL);
    }
}
BOOL __cdecl Ntw16sWindowRemove(HWND w)
{
    attachment *a;
    if(GetWindowThreadProcessId(w,NULL)!=GetCurrentThreadId()){
        SetLastError(ERROR_INVALID_THREAD_ID);return FALSE;
    }
    a=lookup(w);if(!a){SetLastError(ERROR_NOT_FOUND);return FALSE;}
    ++a->calls;
    RemoveWindowSubclass(w,attachment_proc,(UINT_PTR)attachment_proc);
    a->detached=TRUE;report_unbind(a);
    /* Existing owners handle reentrant destruction/removal independently. */
    win16_style_frame_remove_owned(w,a);
    win16_style_control_remove_owned(w,a);
    if(a->themed)ntw16s_theme_remove(a->themed,a);
    ntw16s_theme_remove(w,NULL);
    ntw16s_control_protocol_release(a->protocol);a->protocol=NULL;
    release(a);return TRUE;
}
BOOL __cdecl Ntw16sWindowTerminal(HWND w)
{
    attachment *a;
    DWORD saved=GetLastError(),error=ERROR_SUCCESS;
    if(GetWindowThreadProcessId(w,NULL)!=GetCurrentThreadId()){
        SetLastError(ERROR_INVALID_THREAD_ID);return FALSE;
    }
    a=lookup(w);
    if(a){
        ++a->calls;
        if(RemoveWindowSubclass(w,attachment_proc,(UINT_PTR)attachment_proc))a->detached=TRUE;
        else error=ERROR_INVALID_STATE;
    }
    /* These leaves only retire our exact nodes. They do not restore native
     * appearance, send application messages, or call a saved procedure. */
    if(!win16_style_frame_terminal(w) && !error)error=GetLastError();
    if(!win16_style_control_terminal(w) && !error)error=GetLastError();
    if(a && a->themed && a->themed!=w &&
            !ntw16s_theme_terminal(a->themed,a) && !error)error=GetLastError();
    if(!ntw16s_theme_terminal(w,NULL) && !error)error=GetLastError();
    if(a){report_unbind(a);ntw16s_control_protocol_release(a->protocol);a->protocol=NULL;release(a);}
    SetLastError(error?error:saved);return !error;
}

static BOOL install_created(HWND w,DWORD original_style)
{
    BOOL ready=InterlockedCompareExchange(&status_hooks_ready,0,0)!=0;
    BOOL control,edit,installed,report=FALSE,tree=FALSE,legacy_progress=FALSE,picture=ntw16s_vb_picture(w),checklist=checklist_identity(w),status=ready && status_identity(w,NULL),spin=ready && spin_identity(w),static_candidate=FALSE,edit_candidate=FALSE,track_candidate=FALSE;HWND parent;HMODULE owner;attachment *a;DWORD error;
    ntw16s_control_protocol *protocol=NULL;
    if(lookup(w))return TRUE;
    /* COMBOBOX installs its own internal list when construction finishes.
     * The internal EDIT also belongs to that native composite, not a frame. */
    for(parent=GetParent(w);parent;parent=GetParent(parent))
        if(class_is(parent,L"COMBOBOX"))return TRUE;
    if(class_is(w,L"ComboLBox") || class_is(w,L"#32768"))return TRUE;
    edit=(original_style&WS_CHILD) && class_is(w,L"EDIT");
    control=(original_style&WS_CHILD) && (edit || class_is(w,L"BUTTON") || class_is(w,L"STATIC") ||
        class_is(w,L"LISTBOX") || class_is(w,L"COMBOBOX") || class_is(w,L"SCROLLBAR") || class_is(w,L"SysLink"));
    if(checklist)control=FALSE;
    if(!control && !checklist && !status && !spin && (original_style&WS_CHILD)){
        SetLastError(ERROR_SUCCESS);
        protocol=ntw16s_control_protocol_bind(w);
        if(!protocol && GetLastError()==ERROR_NOT_ENOUGH_MEMORY)return FALSE;
        report=protocol && ntw16s_control_protocol_list_module(w);
        tree=protocol && ntw16s_control_protocol_tree_module(w);
        legacy_progress=protocol && ntw16s_control_protocol_legacy_progress(w);
        control=protocol && !report && !legacy_progress;
        edit=protocol && ntw16s_control_protocol_is(w,L"EDIT");
    }
    owner=(HMODULE)GetClassLongPtrW(w,GCLP_HMODULE);
    if(!control && !report && !tree && !picture && !checklist && !legacy_progress && !status && !spin &&
       (original_style&WS_CHILD) && owner){
        WCHAR name[64],real[64];DWORD type=original_style&SS_TYPEMASK;
        static_candidate=GetClassNameW(w,name,ARRAYSIZE(name)) && !lstrcmpW(name,L"TStaticText") &&
            RealGetWindowClassW(w,real,ARRAYSIZE(real)) && !lstrcmpW(real,L"TStaticText") &&
            (type==SS_LEFT || type==SS_CENTER || type==SS_RIGHT || type==SS_SIMPLE || type==SS_LEFTNOWORDWRAP) &&
            !(GetWindowLongPtrW(w,GWL_EXSTYLE)&(WS_EX_LAYOUTRTL|WS_EX_RTLREADING));
        edit_candidate=ready && GetClassNameW(w,name,ARRAYSIZE(name)) && !lstrcmpW(name,L"TEdit") &&
            RealGetWindowClassW(w,real,ARRAYSIZE(real)) && !lstrcmpW(real,L"TEdit");
        track_candidate=ready && GetClassNameW(w,name,ARRAYSIZE(name)) && !lstrcmpW(name,L"TTrackBar") &&
            RealGetWindowClassW(w,real,ARRAYSIZE(real)) && !lstrcmpW(real,L"TTrackBar");
    }
    /* An external OCX can create itself with frame bits during construction
     * and then become a custom client surface.  Without an explicitly
     * verified public protocol it must stay native; only an application-owned
     * generic child frame may use the ordinary frame path. */
    if(!control && !report && !tree && !picture && !checklist && !legacy_progress && !status && !spin && !static_candidate && !edit_candidate && !track_candidate && (original_style&WS_CHILD) &&
            owner && owner!=GetModuleHandleW(NULL)){
        ntw16s_control_protocol_release(protocol);SetLastError(ERROR_NOT_SUPPORTED);return FALSE;
    }
    /* A custom client-only child and MDICLIENT keep application painting.
     * MDI child frames have WS_CAPTION and take the ordinary frame route. */
    if(!control && !report && !picture && !checklist && !legacy_progress && !status && !spin && !static_candidate && !edit_candidate && !track_candidate && (original_style&WS_CHILD) && !(original_style&(WS_CAPTION|WS_THICKFRAME))){
        SetLastError(ERROR_NOT_SUPPORTED);return FALSE;
    }
    a=calloc(1,sizeof(*a));
    if(!a){ntw16s_control_protocol_release(protocol);SetLastError(ERROR_NOT_ENOUGH_MEMORY);return FALSE;}
    a->calls=1;a->protocol=protocol;a->file_owner=ntw16s_vb_owner_file(w);a->picture=picture;a->button_owner=ntw16s_vb_graphical_button(w);a->checklist=checklist;a->status=status;a->spin=spin;a->static_candidate=static_candidate;
    a->edit_candidate=edit_candidate;a->edit_original_style=original_style;a->track_candidate=track_candidate;
    if(report || tree || a->file_owner || a->button_owner || a->checklist){a->report_window=w;a->tree=tree;
        a->report_module=(a->file_owner || a->button_owner)?(HMODULE)GetClassLongPtrW(w,GCLP_HMODULE):tree?ntw16s_control_protocol_tree_module(w):ntw16s_control_protocol_list_module(w);a->report_parent=GetParent(w);}
    if(!SetWindowSubclass(w,attachment_proc,(UINT_PTR)attachment_proc,(DWORD_PTR)a)){
        ntw16s_control_protocol_release(protocol);free(a);return FALSE;
    }
    if((report || tree || a->file_owner || a->button_owner || a->checklist) && (!a->report_parent || !SetWindowSubclass(a->report_parent,report_notify,(UINT_PTR)w,(DWORD_PTR)a))){
        DWORD error=GetLastError();Ntw16sWindowRemove(w);release(a);SetLastError(error);return FALSE;
    }
    SetLastError(ERROR_SUCCESS);
    if(protocol && class_is(w,L"COMBOBOX")){
        COMBOBOXINFO info={.cbSize=sizeof(info)};
        if(ntw16s_native_get_combo_info(w,&info) && info.hwndItem!=w && lookup(info.hwndItem))
            Ntw16sWindowRemove(info.hwndItem);
    }
    if(a->detached){release(a);SetLastError(ERROR_CANCELLED);return FALSE;}
    if(a->checklist || a->status || a->spin || a->static_candidate || a->edit_candidate || a->track_candidate)installed=TRUE;
    else if(legacy_progress)installed=win16_style_legacy_progress_border_install_created(w,NULL,a,&a->detached);
    else if(protocol && ntw16s_control_protocol_toolbar(w))
        installed=win16_style_toolbar_install_created(w,NULL,a,&a->detached);
    else if(protocol && ntw16s_control_protocol_is(w,UPDOWN_CLASSW))installed=win16_style_updown_install_created(w,NULL,a,&a->detached);
    else if(protocol && ntw16s_control_protocol_is(w,TRACKBAR_CLASSW))installed=win16_style_trackbar_install_created(w,NULL,a,&a->detached);
    else if(protocol && ntw16s_control_protocol_is(w,PROGRESS_CLASSW))installed=win16_style_progress_install_created(w,NULL,a,&a->detached);
    else if(tree)installed=win16_style_tree_install_created(w,NULL,a,&a->detached);
    else if(picture)installed=win16_style_picture_install_created(w,NULL,a,&a->detached);
    else if(a->button_owner)installed=win16_style_frame_scrollbars_install_owned(w,NULL,a,&a->detached);
    else if(control)installed=win16_style_control_install_created(w,edit,(original_style&WS_BORDER)!=0,
        class_is(w,L"LISTBOX") && (original_style&LBS_USETABSTOPS),NULL,a,&a->detached);
    else if(report && !(original_style&(WS_BORDER|WS_THICKFRAME|WS_CAPTION)))installed=TRUE;
    else installed=win16_style_install_created(w,NULL,a,&a->detached);
    error=GetLastError();
    if(installed && !a->detached){
        if(control){
            HWND themed=NULL;BOOL list=FALSE;
            if(!a->file_owner && class_is(w,L"COMBOBOX")){
                COMBOBOXINFO info={.cbSize=sizeof(info)};
                if(ntw16s_native_get_combo_info(w,&info)){themed=info.hwndList;list=TRUE;}
            }else if(class_is(w,L"LISTBOX")){themed=w;list=TRUE;}
            else if(edit && (original_style&ES_MULTILINE))themed=w;
            if(themed){
                a->themed=themed;
                if(!ntw16s_theme_enable(themed,a))installed=FALSE;
                else {
                    installed=list?win16_style_list_scrollbar_enable(themed):win16_style_edit_scrollbars_enable(themed);}
                error=GetLastError();
            }
        }
    }
    if(installed && !a->detached && !a->edit_candidate && !a->track_candidate && a->themed!=w)ntw16s_theme_remove(w,NULL);
    if(a->detached){installed=FALSE;error=ERROR_CANCELLED;}
    if(!installed && !a->detached)Ntw16sWindowRemove(w);
    release(a);
    if(!installed)SetLastError(error?error:ERROR_GEN_FAILURE);
    return installed;
}
static LRESULT CALLBACK creation_before(int code,WPARAM p,LPARAM l)
{
    ntw16s_creation_scope *scope=current;DWORD saved=GetLastError();
    if(code==HCBT_CREATEWND && scope && scope->enabled){
        const CBT_CREATEWNDW *create=(const CBT_CREATEWNDW *)l;
        pending_window *pending=calloc(1,sizeof(*pending));
        if(!pending){scope->result.failure=ERROR_NOT_ENOUGH_MEMORY;return 1;}
        pending->window=(HWND)p;pending->style=(DWORD)create->lpcs->style;
        pending->next=scope->pending;scope->pending=pending;
        if(class_is((HWND)p,L"LISTBOX") || class_is((HWND)p,L"ComboLBox") ||
           (class_is((HWND)p,L"EDIT") && (pending->style&ES_MULTILINE))){
            if(!ntw16s_theme_capture((HWND)p)){
                scope->result.failure=GetLastError();return 1;
            }
        }else if((pending->style&WS_CHILD) && !ntw16s_theme_stage((HWND)p)){
            scope->result.failure=GetLastError();return 1;
        }
    }
    SetLastError(saved);return CallNextHookEx(NULL,code,p,l);
}
static pending_window *take_pending(ntw16s_creation_scope *scope,HWND w)
{
    pending_window **link=&scope->pending,*found;
    while(*link && (*link)->window!=w)link=&(*link)->next;
    found=*link;if(found)*link=found->next;return found;
}
static LRESULT CALLBACK creation_return(int code,WPARAM p,LPARAM l)
{
    LRESULT result=CallNextHookEx(NULL,code,p,l);DWORD saved=GetLastError();
    ntw16s_creation_scope *scope=current;
    if(code>=0 && scope && scope->enabled){
        const CWPRETSTRUCT *m=(const CWPRETSTRUCT *)l;
        BOOL created=m->message==WM_CREATE && m->lResult!=-1;
        /* Dialog managers finish layout and format their caption during
         * WM_INITDIALOG. Attach after that native initialization (ShellAbout
         * also formats its caption there); never rewrite application text.
         * Keep its pending style until the real dialog initialization exits. */
        if(created && class_is(m->hwnd,L"#32770")){
            SetLastError(saved);return result;
        }
        if(m->message==WM_NCDESTROY){pending_window *pending=take_pending(scope,m->hwnd);ntw16s_theme_cancel_stage(m->hwnd);free(pending);}
        /* Some dialogs show themselves and run a nested loop inside
         * WM_INITDIALOG. Its return may be delayed until the application exits.
         * A completed first-show notification is also a safe boundary, but
         * only for a captioned top-level dialog owned by this creation chain.
         * Mark its pending node before installation can reenter USER. */
        BOOL first_show=(m->message==WM_SHOWWINDOW && m->wParam) ||
            (m->message==WM_WINDOWPOSCHANGED && m->lParam &&
             (((const WINDOWPOS *)m->lParam)->flags&SWP_SHOWWINDOW));
        ntw16s_creation_scope *owner=scope;
        if(first_show){
            DWORD style=(DWORD)GetWindowLongPtrW(m->hwnd,GWL_STYLE);
            first_show=(style&WS_VISIBLE) && !(style&WS_CHILD) &&
                (style&WS_CAPTION)==WS_CAPTION && class_is(m->hwnd,L"#32770");
            if(first_show){
                for(;owner;owner=owner->previous){
                    pending_window *pending=owner->pending;
                    while(pending && pending->window!=m->hwnd)pending=pending->next;
                    if(owner->enabled && pending && !pending->settled)break;
                }
                first_show=owner!=NULL;
            }
        }
        if((created || m->message==WM_INITDIALOG || first_show) && IsWindow(m->hwnd)){
            if(!first_show)owner=scope;
            pending_window *pending;
            if(first_show){
                pending=owner->pending;
                while(pending && pending->window!=m->hwnd)pending=pending->next;
                if(pending)pending->settled=TRUE;
            }else pending=take_pending(owner,m->hwnd);
            /* A first-show attachment has already completed this creation.
             * In particular, do not undo an explicit removal by attaching
             * again when the delayed initializer finally returns. */
            if(!first_show && pending && pending->settled && pending->installed){free(pending);SetLastError(saved);return result;}
            DWORD style=created?(DWORD)((const CREATESTRUCTW *)m->lParam)->style:
                (DWORD)GetWindowLongPtrW(m->hwnd,GWL_STYLE);
            if(pending){style=pending->style;if(!first_show)free(pending);}
            BOOL already=lookup(m->hwnd)!=NULL;
            BOOL installed=ntw16s_theme_commit(m->hwnd) && install_created(m->hwnd,style);
            if(installed){
                if(first_show){
                    for(pending_window *record=owner->pending;record;record=record->next)
                        if(record==pending && record->window==m->hwnd){record->installed=TRUE;break;}
                }
                if(!already && lookup(m->hwnd))++owner->result.installed;
            }else{
                DWORD error=GetLastError();
                if(error==ERROR_NOT_SUPPORTED){
                    ++owner->result.unsupported;ntw16s_theme_remove(m->hwnd,NULL);
                    if(class_is(m->hwnd,L"COMBOBOX")){
                        COMBOBOXINFO info={.cbSize=sizeof(info)};
                        if(ntw16s_native_get_combo_info(m->hwnd,&info))ntw16s_theme_remove(info.hwndList,NULL);
                    }
                }
                else if(!owner->result.failure)owner->result.failure=error?error:ERROR_GEN_FAILURE;
            }
            if(owner->result.failure && m->message==WM_INITDIALOG)
                EndDialog(m->hwnd,-1);
        }
    }
    SetLastError(saved);return result;
}
ntw16s_creation_scope *__cdecl Ntw16sCreationBegin(BOOL enabled)
{
    ntw16s_creation_scope *scope=calloc(1,sizeof(*scope));
    if(!scope){SetLastError(ERROR_NOT_ENOUGH_MEMORY);return NULL;}
    if(!current){
        creation_before_hook=SetWindowsHookExW(WH_CBT,creation_before,NULL,GetCurrentThreadId());
        if(!creation_before_hook){free(scope);return NULL;}
        creation_return_hook=SetWindowsHookExW(WH_CALLWNDPROCRET,creation_return,NULL,GetCurrentThreadId());
        if(!creation_return_hook){DWORD error=GetLastError();UnhookWindowsHookEx(creation_before_hook);creation_before_hook=NULL;free(scope);SetLastError(error);return NULL;}
    }
    scope->thread=GetCurrentThreadId();scope->enabled=enabled;
    scope->previous=current;current=scope;return scope;
}
BOOL __cdecl Ntw16sCreationEnd(ntw16s_creation_scope *scope,ntw16s_creation_result *result)
{
    if(!scope || scope!=current){SetLastError(ERROR_INVALID_STATE);return FALSE;}
    if(scope->thread!=GetCurrentThreadId()){SetLastError(ERROR_INVALID_THREAD_ID);return FALSE;}
    current=scope->previous;if(result)*result=scope->result;
    while(scope->pending){pending_window *pending=scope->pending;scope->pending=pending->next;ntw16s_theme_cancel_stage(pending->window);free(pending);}
    if(!current){UnhookWindowsHookEx(creation_return_hook);creation_return_hook=NULL;
        UnhookWindowsHookEx(creation_before_hook);creation_before_hook=NULL;}
    free(scope);return TRUE;
}
