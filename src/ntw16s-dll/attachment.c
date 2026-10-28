#include "vb_text.h"
#include "control_class.h"
#include "ntw16s.h"
#include "native_api.h"
#include "theme.h"
#include "static_text.h"
#include "combo_glyph.h"
#include "injection/unwind.h"
#include "status_theme.h"
#include "month_text.h"
#include <commctrl.h>
#include <uxtheme.h>
#include <vssym32.h>
#include <oleacc.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <limits.h>

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
    BOOL edit_candidate,edit_attempted,edit_installed,link_candidate,spinedit_candidate,combo_candidate,modern_spin,modern_spin_verified;
    BOOL scroll_candidate,scroll_attempted,scroll_installed; HWND scroll_parent; HMODULE scroll_witness;
    BOOL group_button_candidate,group_button_attempted,group_button_installed;
    BOOL grid_candidate,grid_attempted,grid_installed;
    BOOL vb_user_candidate,vb_user_attempted,vb_user_edge; HWND vb_user_parent;
    BOOL vb_data_candidate,vb_data_attempted,vb_data_edge; HWND vb_data_parent;
    BOOL vcl_toolbar_candidate,vcl_toolbar_attempted,vcl_toolbar_installed;
    BOOL track_candidate,track_attempted,track_installed;
    BOOL tab_candidate,tab_attempted,tab_installed;
    BOOL color_candidate,color_attempted,color_installed;
    BOOL date_candidate,date_attempted,date_installed;
    BOOL status_candidate,status_attempted,status_installed;
    BOOL box_candidate,box_installed,controlbar_candidate,controlbar_verified,controlbar_retired; HWND box_parent,controlbar_parent; HMODULE box_owner,controlbar_owner;
    BOOL rich_candidate,rich_installed,rich_witness,rich_unicode,rich_hotkey,rich_month,rich_popup; HWND rich_parent; HMODULE rich_owner;
    BOOL bit_candidate,bit_verified; BOOL inplace_candidate,inplace_verified; HWND inplace_parent; HMODULE inplace_owner;
    BOOL group_frame_candidate,group_frame_verified,group_frame_retired; HWND group_frame_parent;
    BOOL panel_frame_candidate,panel_frame_verified,panel_frame_retired; HWND panel_frame_parent;
    HMODULE status_witness_owner; HWND status_parent;
    HFONT month_cached_font;HFONT status_cached_font,status_cached_source;LOGFONTW status_font_log;TEXTMETRICW status_font_metrics;
    int status_font_widths[96];unsigned status_font_borrows;BOOL status_font_invalid;
    DWORD edit_original_style,combo_original_type;
    HMODULE edit_witness_owner,group_button_witness_owner;
    HWND combo_parent,group_button_parent,grid_parent,vcl_toolbar_parent; HMODULE grid_owner,vcl_toolbar_owner,vcl_toolbar_witness;
    HMODULE track_witness_owner,tab_witness_owner; HWND tab_parent;
    HMODULE color_witness_owner;
    HWND color_parent;
    HMODULE date_witness_owner;
    HWND date_parent;
    unsigned status_epoch;
    HWND themed,report_window,report_parent;
    HMODULE report_module;
    ntw16s_vb_fonts fonts;
    ntw16s_control_protocol *protocol;
} attachment;
static _Thread_local ntw16s_creation_scope *current;
typedef struct static_paint_scope {struct static_paint_scope *previous;attachment *owner;HWND window;ntw16s_vb_paint *owned_paint;void *status_prior,*spin_prior;BOOL completed;BOOL month_header_valid,month_header_known,month_nav_valid;RECT month_header,month_prev,month_next;BOOL month_selected;HTHEME month_theme;HDC month_dc;RECT month_rect,month_clip;BOOL month_has_clip;BOOL group_background;int group_state;RECT group_rect;BOOL panel_first,panel_enabled;HWND panel_parent;HDC panel_dc;POINT panel_points[3];COLORREF panel_color;} static_paint_scope;
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
typedef struct bit_draw_scope {struct bit_draw_scope *previous;attachment *owner;HWND window;HDC dc;RECT item;UINT state;BOOL completed;} bit_draw_scope;
static _Thread_local bit_draw_scope *bit_current;


/* Status frames are drawn by the native common control before text, icons
 * and WM_DRAWITEM. A per-message snapshot makes nested unrelated drawing a
 * barrier and allows a changing or destroyed control to fall back safely. */
typedef struct status_draw_scope {
    struct status_draw_scope *previous;
    attachment *owner; HWND window; HDC dc; HMODULE native;
    unsigned epoch,count,text_count; BOOL simple; int simple_length; WCHAR simple_text[513]; RECT panes[64],text_panes[64];
} status_draw_scope;
static _Thread_local status_draw_scope *status_current;
static _Thread_local ntw16s_status_theme_scope *status_theme_current;
static LONG status_hooks_ready;
void ntw16s_status_hooks_enable(void){InterlockedExchange(&status_hooks_ready,1);}

/* Scope is active only while dispatching the observed control's own paint or
 * pointer/enable messages. Unrelated nested attachment messages are barriers. */
typedef struct spin_buffers { DWORD background[4096],original[4096];BYTE valid[4096]; } spin_buffers;
typedef struct spin_draw_scope {
    attachment *owner; HWND window,parent; HDC dc,clones[16],snapshot_dc; HGDIOBJ snapshot_bitmap;
    spin_buffers *buffers; RECT halves[2]; int states[2]; struct spin_draw_scope *previous_stack;
} spin_draw_scope;
static _Thread_local spin_draw_scope *spin_stack;
static void spin_buffers_clear(spin_draw_scope *s)
{ if(s->buffers){HeapFree(GetProcessHeap(),0,s->buffers);s->buffers=NULL;} }
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

/* Newer VCL paints a buffered themed TSpinButton under a witnessed EDIT.
 * Only a buffer derived in this exact paint and copied to this exact window
 * can be composed; unrelated or uncertain images keep their native pixels. */
static BOOL modern_spin_shape(HWND w)
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
void ntw16s_modern_spin_dc_created(HDC source,HDC result)
{
    spin_draw_scope *s=spin_current;
    if(!s || !s->owner->modern_spin || !result || (WindowFromDC(source)!=s->window && (!s->dc || source!=s->dc)))return;
    for(unsigned i=0;i<16;i++)if(!s->clones[i]){s->clones[i]=result;return;}
}
static void spin_snapshot_invalidate(spin_draw_scope *s)
{
    s->snapshot_dc=NULL;s->snapshot_bitmap=NULL;s->states[0]=s->states[1]=0;
    if(s->buffers)memset(s->buffers->valid,0,sizeof(s->buffers->valid));
}
/* Invalidate before forwarding every new theme draw on the witnessed DC.
 * An unknown theme or a failed draw must never leave its old pixels eligible. */
void ntw16s_modern_spin_theme_begin(HDC dc,const RECT *rect)
{
    spin_draw_scope *s=spin_current;RECT client,safe;SIZE_T got=0;POINT pt[2];
    if(!s || !s->buffers || s->snapshot_dc!=dc)return;
    if(!rect || !ReadProcessMemory(GetCurrentProcess(),rect,&safe,sizeof(safe),&got) || got!=sizeof(safe) ||
       !GetClientRect(s->window,&client) || client.right<=0 || client.bottom<=0 || client.right>64 || client.bottom>64){
        spin_snapshot_invalidate(s);return;
    }
    pt[0]=(POINT){safe.left,safe.top};pt[1]=(POINT){safe.right,safe.bottom};
    if(!LPtoDP(dc,pt,2) || pt[0].x!=0 || pt[1].x!=client.right || pt[0].y<0 || pt[1].y>client.bottom ||
       pt[1].y<=pt[0].y || !((pt[0].y==0 && pt[1].y<client.bottom) || (pt[0].y>0 && pt[1].y==client.bottom))){
        spin_snapshot_invalidate(s);return;
    }
    s->states[pt[0].y==0?0:1]=0;
    for(int y=pt[0].y;y<pt[1].y;y++)for(int x=0;x<client.right;x++)s->buffers->valid[y*64+x]=0;
}
void ntw16s_modern_spin_dc_deleted(HDC dc)
{
    for(spin_draw_scope *s=spin_stack;s;s=s->previous_stack){
        for(unsigned i=0;i<16;i++)if(s->clones[i]==dc)s->clones[i]=NULL;
        if(s->snapshot_dc==dc)spin_snapshot_invalidate(s);
    }
}
void ntw16s_modern_spin_selected(HDC dc)
{
    for(spin_draw_scope *s=spin_stack;s;s=s->previous_stack)
        if(s->snapshot_dc==dc && GetCurrentObject(dc,OBJ_BITMAP)!=s->snapshot_bitmap)spin_snapshot_invalidate(s);
}
void ntw16s_modern_spin_object_deleted(HGDIOBJ object)
{
    for(spin_draw_scope *s=spin_stack;s;s=s->previous_stack)
        if(s->snapshot_bitmap==object)spin_snapshot_invalidate(s);
}
typedef struct spin_dib {HDC dc;HBITMAP bitmap;HGDIOBJ old;DWORD *pixels;} spin_dib;
static void spin_dib_close(spin_dib *d)
{ SelectObject(d->dc,d->old);DeleteObject(d->bitmap);DeleteDC(d->dc); }
static BOOL spin_dib_open(HDC source,int width,int height,spin_dib *d)
{
    BITMAPINFO info={0};info.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);info.bmiHeader.biWidth=width;
    info.bmiHeader.biHeight=-height;info.bmiHeader.biPlanes=1;info.bmiHeader.biBitCount=32;info.bmiHeader.biCompression=BI_RGB;
    d->dc=CreateCompatibleDC(source);if(!d->dc)return FALSE;
    d->bitmap=CreateDIBSection(source,&info,DIB_RGB_COLORS,(void**)&d->pixels,NULL,0);
    if(!d->bitmap){DeleteDC(d->dc);return FALSE;}
    d->old=SelectObject(d->dc,d->bitmap);
    if(!d->old || d->old==HGDI_ERROR){DeleteObject(d->bitmap);DeleteDC(d->dc);return FALSE;}
    int saved=SaveDC(source);BOOL ok=FALSE;
    if(saved){SetViewportOrgEx(source,0,0,NULL);SetWindowOrgEx(source,0,0,NULL);SelectClipRgn(source,NULL);
        ok=BitBlt(d->dc,0,0,width,height,source,0,0,SRCCOPY);GdiFlush();RestoreDC(source,saved);}
    if(!ok){spin_dib_close(d);return FALSE;}return TRUE;
}
static BOOL spin_full_clip(HDC dc,const POINT pt[2])
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
void ntw16s_modern_spin_theme(HDC dc,int part,int state,const RECT *rect,const RECT *clip,const void *caller)
{
    spin_draw_scope *s=spin_current;HMODULE module=NULL;BOOL owned=FALSE;RECT client;POINT pt[2];
    if(!s || !s->owner->modern_spin || s->owner->detached || !rect || clip || part!=1 || state<1 || state>4 ||
       !modern_spin_shape(s->window) || GetParent(s->window)!=s->parent || !ntw16s_control_protocol_is(s->parent,L"EDIT") ||
       GetMapMode(dc)!=MM_TEXT || (GetLayout(dc)&LAYOUT_RTL) ||
       !GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,(LPCWSTR)caller,&module) ||
       module!=(HMODULE)GetClassLongPtrW(s->window,GCLP_HMODULE) || !GetClientRect(s->window,&client))return;
    if(WindowFromDC(dc)==s->window)owned=TRUE;
    for(unsigned i=0;i<16;i++)if(s->clones[i]==dc)owned=TRUE;
    if(!owned || rect->left!=0 || rect->top!=0 || rect->right!=client.right || rect->bottom<=0 ||
       client.right<=0 || client.bottom<=0 || client.right>64 || client.bottom>64)return;
    pt[0]=(POINT){rect->left,rect->top};pt[1]=(POINT){rect->right,rect->bottom};
    if(!LPtoDP(dc,pt,2) || pt[0].x!=0 || pt[1].x!=client.right || pt[0].y<0 || pt[1].y>client.bottom ||
       pt[1].y<=pt[0].y || !((pt[0].y==0 && pt[1].y<client.bottom) || (pt[0].y>0 && pt[1].y==client.bottom)))return;
    if(!spin_full_clip(dc,pt))return;
    if(!s->buffers){s->buffers=HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,sizeof(*s->buffers));if(!s->buffers)return;}
    if(s->snapshot_dc!=dc || s->snapshot_bitmap!=GetCurrentObject(dc,OBJ_BITMAP)){
        s->snapshot_dc=dc;s->snapshot_bitmap=GetCurrentObject(dc,OBJ_BITMAP);
        memset(s->buffers->valid,0,sizeof(s->buffers->valid));s->states[0]=s->states[1]=0;
    }
    int half=pt[0].y==0?0:1;s->states[half]=0;
    for(int y=pt[0].y;y<pt[1].y;y++)for(int x=0;x<client.right;x++)s->buffers->valid[y*64+x]=0;
    spin_dib d={0};if(!spin_dib_open(dc,client.right,client.bottom,&d))return;
    for(int y=pt[0].y;y<pt[1].y;y++)for(int x=0;x<client.right;x++){
        int at=y*64+x;s->buffers->background[at]=d.pixels[y*client.right+x]&0x00ffffff;s->buffers->valid[at]=1;
    }
    spin_dib_close(&d);s->halves[half]=(RECT){pt[0].x,pt[0].y,pt[1].x,pt[1].y};s->states[half]=state;
}
void ntw16s_modern_spin_blit(HDC dest,int x,int y,int width,int height,HDC source,int sx,int sy,DWORD rop)
{
    spin_draw_scope *s=spin_current;RECT r;
    if(!s || !s->buffers || !s->owner->modern_spin || s->owner->detached || !modern_spin_shape(s->window) ||
       GetParent(s->window)!=s->parent || !ntw16s_control_protocol_is(s->parent,L"EDIT") || !GetClientRect(s->window,&r) ||
       (WindowFromDC(dest)!=s->window && (!s->dc || dest!=s->dc)) || source!=s->snapshot_dc ||
       GetCurrentObject(source,OBJ_BITMAP)!=s->snapshot_bitmap || x<0 || y<0 || sx!=x || sy!=y ||
       width<=0 || height<=0 || x>r.right-width || y>r.bottom-height || r.right>64 || r.bottom>64 ||
       rop!=SRCCOPY || !s->states[0] || !s->states[1])return;
    for(int j=y;j<y+height;j++)for(int i=x;i<x+width;i++)if(!s->buffers->valid[j*64+i])return;
    spin_dib d={0};if(!spin_dib_open(source,r.right,r.bottom,&d))return;
    for(int j=y;j<y+height;j++)for(int i=x;i<x+width;i++)s->buffers->original[j*64+i]=d.pixels[j*r.right+i];
    for(int half=0;half<2;half++)if(!win16_style_bitbtn_face(s->window,d.dc,&s->halves[half],NULL,s->states[half]==3?ODS_SELECTED:0)){spin_dib_close(&d);return;}
    GdiFlush();for(int j=y;j<y+height;j++)for(int i=x;i<x+width;i++){
        int at=j*64+i;if((s->buffers->original[at]&0x00ffffff)!=s->buffers->background[at])d.pixels[j*r.right+i]=s->buffers->original[at];
    }
    BOOL copied=BitBlt(dest,x,y,width,height,d.dc,x,y,SRCCOPY);spin_dib_close(&d);
    if(copied && !s->owner->detached)s->owner->modern_spin_verified=TRUE;
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

/* Editable VCL combos are bound only to the original same-runtime parent. */
static BOOL combo_identity(HWND w,HWND parent)
{
    WCHAR name[64],real[64];DWORD pid=0,style=(DWORD)GetWindowLongPtrW(w,GWL_STYLE),ex=(DWORD)GetWindowLongPtrW(w,GWL_EXSTYLE);HMODULE owner=(HMODULE)GetClassLongPtrW(w,GCLP_HMODULE);
    return owner && parent && GetParent(w)==parent && GetWindowThreadProcessId(w,&pid)==GetCurrentThreadId() && pid==GetCurrentProcessId() && GetWindowThreadProcessId(parent,NULL)==GetCurrentThreadId() && (HMODULE)GetClassLongPtrW(parent,GCLP_HMODULE)==owner && (style&WS_CHILD) && ((style&3)==CBS_SIMPLE || (style&3)==CBS_DROPDOWN || (style&3)==CBS_DROPDOWNLIST) && !(style&(CBS_OWNERDRAWFIXED|CBS_OWNERDRAWVARIABLE)) && !(ex&(WS_EX_LAYOUTRTL|WS_EX_RTLREADING)) && GetClassNameW(w,name,ARRAYSIZE(name)) && (!lstrcmpW(name,L"TComboBox") || (!lstrcmpW(name,L"TDBComboBox") && (style&3)==CBS_DROPDOWN)) && RealGetWindowClassW(w,real,ARRAYSIZE(real)) && (!lstrcmpW(name,real) || !lstrcmpiW(real,L"COMBOBOX"));
}
/* Same-runtime VCL buttons witness the registered BUTTON procedure in paint. */
static BOOL group_button_identity(HWND w)
{
    WCHAR name[64],real[64],parent_name[64];HWND parent=GetParent(w);
    HMODULE owner=(HMODULE)GetClassLongPtrW(w,GCLP_HMODULE);
    DWORD style=(DWORD)GetWindowLongPtrW(w,GWL_STYLE),pid=0;
    if(!(owner && parent && GetWindowThreadProcessId(w,&pid)==GetCurrentThreadId() && pid==GetCurrentProcessId() &&
        GetWindowThreadProcessId(parent,NULL)==GetCurrentThreadId() &&
        (HMODULE)GetClassLongPtrW(parent,GCLP_HMODULE)==owner && (style&WS_CHILD) &&
        !(style&(BS_BITMAP|BS_ICON|BS_PUSHLIKE)) && !(GetWindowLongPtrW(w,GWL_EXSTYLE)&(WS_EX_LAYOUTRTL|WS_EX_RTLREADING)) &&
        GetClassNameW(w,name,ARRAYSIZE(name)) && RealGetWindowClassW(w,real,ARRAYSIZE(real)) && !lstrcmpW(name,real)))return FALSE;
    if(!lstrcmpW(name,L"TCheckBox"))return (style&BS_TYPEMASK)==BS_3STATE;
    if(!lstrcmpW(name,L"TButton"))
        return (style&BS_TYPEMASK)==BS_PUSHBUTTON || (style&BS_TYPEMASK)==BS_DEFPUSHBUTTON;
    return (style&BS_TYPEMASK)==BS_RADIOBUTTON &&
        (!lstrcmpW(name,L"TRadioButton") || (!lstrcmpW(name,L"TGroupButton") &&
         GetClassNameW(parent,parent_name,ARRAYSIZE(parent_name)) && !lstrcmpW(parent_name,L"TRadioGroup")));
}
static BOOL grid_identity(HWND w,HMODULE owner,HWND parent)
{
    WCHAR name[64],real[64];DWORD pid=0,style=(DWORD)GetWindowLongPtrW(w,GWL_STYLE),ex=(DWORD)GetWindowLongPtrW(w,GWL_EXSTYLE);
    return owner && parent && GetParent(w)==parent && GetWindowThreadProcessId(w,&pid)==GetCurrentThreadId() && pid==GetCurrentProcessId() &&
        GetWindowThreadProcessId(parent,NULL)==GetCurrentThreadId() && (HMODULE)GetClassLongPtrW(w,GCLP_HMODULE)==owner &&
        (style&WS_CHILD) && (ex&WS_EX_CLIENTEDGE) && !(ex&(WS_EX_LAYOUTRTL|WS_EX_RTLREADING)) &&
        GetClassNameW(w,name,ARRAYSIZE(name)) &&
        ((!lstrcmpW(name,L"TStringGrid") || !lstrcmpW(name,L"TDrawGrid")) ||
         (owner==GetModuleHandleW(NULL) && IsWindowUnicode(w) && (!lstrcmpW(name,L"TValueListEditor") || !lstrcmpW(name,L"TDBGrid")))) &&
        RealGetWindowClassW(w,real,ARRAYSIZE(real)) && !lstrcmpW(name,real);
}
static BOOL vcl_toolbar_identity(HWND w,HMODULE owner,HWND parent)
{
    WCHAR name[64],real[64],pn[64];DWORD pid=0,style=(DWORD)GetWindowLongPtrW(w,GWL_STYLE),ex=(DWORD)GetWindowLongPtrW(w,GWL_EXSTYLE);
    return owner && parent && GetParent(w)==parent && GetWindowThreadProcessId(w,&pid)==GetCurrentThreadId() && pid==GetCurrentProcessId() &&
        GetWindowThreadProcessId(parent,NULL)==GetCurrentThreadId() && (HMODULE)GetClassLongPtrW(w,GCLP_HMODULE)==owner &&
        (HMODULE)GetClassLongPtrW(parent,GCLP_HMODULE)==owner && (style&WS_CHILD) &&
        !(ex&(WS_EX_CLIENTEDGE|WS_EX_STATICEDGE|WS_EX_LAYOUTRTL|WS_EX_RTLREADING)) &&
        GetClassNameW(w,name,ARRAYSIZE(name)) && !lstrcmpW(name,L"TToolBar") &&
        RealGetWindowClassW(w,real,ARRAYSIZE(real)) && !lstrcmpW(real,L"TToolBar") &&
        GetClassNameW(parent,pn,ARRAYSIZE(pn)) && !lstrcmpW(pn,L"TControlBar");
}
static BOOL inplace_identity(HWND w,HMODULE owner,HWND parent)
{WCHAR name[64],real[64],pname[64];DWORD pid=0;return w && parent && owner && IsWindow(w) && GetParent(w)==parent && GetWindowThreadProcessId(w,&pid)==GetCurrentThreadId() && pid==GetCurrentProcessId() && GetWindowThreadProcessId(parent,NULL)==GetCurrentThreadId() && (HMODULE)GetClassLongPtrW(w,GCLP_HMODULE)==owner && (HMODULE)GetClassLongPtrW(parent,GCLP_HMODULE)==owner && (GetWindowLongPtrW(w,GWL_STYLE)&WS_CHILD) && !(GetWindowLongPtrW(w,GWL_EXSTYLE)&(WS_EX_LAYOUTRTL|WS_EX_RTLREADING|WS_EX_LAYERED)) && IsWindowUnicode(w) && GetClassNameW(w,name,ARRAYSIZE(name)) && !lstrcmpW(name,L"TInplaceEditList") && RealGetWindowClassW(w,real,ARRAYSIZE(real)) && !lstrcmpW(real,name) && GetClassNameW(parent,pname,ARRAYSIZE(pname)) && !lstrcmpW(pname,L"TValueListEditor");}
static BOOL lookup_identity(HWND w,HMODULE owner,HWND parent)
{WCHAR name[64],real[64];DWORD pid=0;return w && parent && owner && IsWindow(w) && GetParent(w)==parent && GetWindowThreadProcessId(w,&pid)==GetCurrentThreadId() && pid==GetCurrentProcessId() && GetWindowThreadProcessId(parent,NULL)==GetCurrentThreadId() && (HMODULE)GetClassLongPtrW(w,GCLP_HMODULE)==owner && (HMODULE)GetClassLongPtrW(parent,GCLP_HMODULE)==owner && (GetWindowLongPtrW(w,GWL_STYLE)&WS_CHILD) && (GetWindowLongPtrW(w,GWL_EXSTYLE)&WS_EX_CLIENTEDGE) && !(GetWindowLongPtrW(w,GWL_EXSTYLE)&(WS_EX_LAYOUTRTL|WS_EX_RTLREADING|WS_EX_LAYERED)) && GetClassNameW(w,name,ARRAYSIZE(name)) && !lstrcmpW(name,L"TDBLookupComboBox") && RealGetWindowClassW(w,real,ARRAYSIZE(real)) && !lstrcmpW(real,name);}
static BOOL inplace_or_lookup(HWND w,HMODULE owner,HWND parent)
{return inplace_identity(w,owner,parent) || lookup_identity(w,owner,parent);}
static BOOL bit_identity(HWND w,HMODULE owner)
{
    WCHAR name[64],real[64];DWORD pid=0,style=(DWORD)GetWindowLongPtrW(w,GWL_STYLE);
    return owner && GetWindowThreadProcessId(w,&pid)==GetCurrentThreadId() && pid==GetCurrentProcessId() &&
        (style&WS_CHILD) && (style&BS_TYPEMASK)==BS_OWNERDRAW &&
        !(GetWindowLongPtrW(w,GWL_EXSTYLE)&(WS_EX_LAYOUTRTL|WS_EX_RTLREADING)) &&
        GetWindowThreadProcessId(GetParent(w),NULL)==GetCurrentThreadId() &&
        (HMODULE)GetClassLongPtrW(w,GCLP_HMODULE)==owner &&
        GetClassNameW(w,name,ARRAYSIZE(name)) && !lstrcmpW(name,L"TBitBtn") &&
        RealGetWindowClassW(w,real,ARRAYSIZE(real)) && !lstrcmpW(real,L"TBitBtn");
}
void ntw16s_bit_theme_frame(HDC dc,int part,const RECT *rect,const RECT *clip,const void *address)
{
    bit_draw_scope *s=bit_current;HMODULE caller=NULL;RECT client;POINT origin;
    if(!s || !s->owner || s->owner->detached || !dc || dc!=s->dc || part!=1 || !rect ||
        !bit_identity(s->window,s->owner->report_module) || GetParent(s->window)!=s->owner->report_parent ||
        !GetClientRect(s->window,&client) || !EqualRect(rect,&client) || !EqualRect(&s->item,&client) ||
        GetMapMode(dc)!=MM_TEXT || (GetLayout(dc)&LAYOUT_RTL) || !GetViewportOrgEx(dc,&origin) || origin.x || origin.y ||
        !GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            (LPCWSTR)address,&caller) || caller!=s->owner->report_module)return;
    if(win16_style_bitbtn_face(s->window,dc,rect,clip,s->state) && !s->owner->detached)s->owner->bit_verified=TRUE;
}
static void inplace_draw(HDC dc,int part,int state,const RECT *r,const RECT *clip,const void *address,BOOL drop)
{static_paint_scope *s=static_paint;HMODULE caller=NULL;RECT client;POINT origin;if(!s || !s->owner->inplace_candidate || s->owner->detached || !dc || !r || part!=1 || (state!=1 && state!=3) || !IsWindowEnabled(s->window) || !inplace_or_lookup(s->window,s->owner->inplace_owner,s->owner->inplace_parent) || (!drop && lookup_identity(s->window,s->owner->inplace_owner,s->owner->inplace_parent)) || WindowFromDC(dc)!=s->window || !GetClientRect(s->window,&client) || r->right!=client.right || r->top!=client.top || r->bottom!=client.bottom || r->left<client.left || r->right<=r->left || r->right-r->left>client.bottom-client.top || GetMapMode(dc)!=MM_TEXT || (GetLayout(dc)&LAYOUT_RTL) || !GetViewportOrgEx(dc,&origin) || origin.x || origin.y || !GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,(LPCWSTR)address,&caller) || caller!=s->owner->inplace_owner)return;
 if(drop && state!=1)return;
 int saved=SaveDC(dc);if(!saved)return;int u=max(1,MulDiv(1,(int)GetDpiForWindow(s->window),96));int left=(r->left+r->right-7*u)/2,top=(r->top+r->bottom-9*u)/2;
 if(drop && (left<r->left+u || top<r->top+u || left+7*u>r->right-u || top+9*u>r->bottom-u)){RestoreDC(dc,saved);return;}
 if(clip && IntersectClipRect(dc,clip->left,clip->top,clip->right,clip->bottom)==ERROR){RestoreDC(dc,saved);return;}
 if(win16_style_bitbtn_face(s->window,dc,r,clip,state==3?ODS_SELECTED:0) && !s->owner->detached){if(drop){win16_style_palette palette;win16_style_default_palette(&palette);SetDCBrushColor(dc,palette.edge);for(int y=0;y<9;y++)for(int x=0;x<7;x++)if(combo_arrow[y][x]=='0'){RECT pixel={left+x*u,top+y*u,left+(x+1)*u,top+(y+1)*u};FillRect(dc,&pixel,(HBRUSH)GetStockObject(DC_BRUSH));}}s->owner->inplace_verified=TRUE;}RestoreDC(dc,saved);}
BOOL ntw16s_inplace_paint(HDC dc){static_paint_scope *s=static_paint;return s && s->owner->inplace_candidate && !s->owner->detached && dc && WindowFromDC(dc)==s->window;}
void ntw16s_inplace_button(HDC dc,int part,int state,const RECT *r,const RECT *clip,const void *address){inplace_draw(dc,part,state,r,clip,address,FALSE);}
void ntw16s_inplace_dropdown(HDC dc,int part,int state,const RECT *r,const RECT *clip,const void *address){inplace_draw(dc,part,state,r,clip,address,TRUE);}
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

static BOOL status_identity(HWND w,attachment *a,HMODULE *native)
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
        if(!a || !a->status_candidate || !a->status_witness_owner || a->detached ||
            lstrcmpW(name,L"TStatusBar") || (HMODULE)GetClassLongPtrW(w,GCLP_HMODULE)!=a->status_witness_owner ||
            GetWindowThreadProcessId(GetParent(w),NULL)!=GetCurrentThreadId() ||
            (a->status_parent && GetParent(w)!=a->status_parent))return FALSE;
    }else if((HMODULE)GetClassLongPtrW(w,GCLP_HMODULE)!=image ||
        GetClassLongPtrW(w,GCLP_WNDPROC)!=(ULONG_PTR)cls.lpfnWndProc)return FALSE;
    if(native)*native=image;
    return TRUE;
}

BOOL ntw16s_status_theme_begin(ntw16s_status_theme_scope *t,HDC dc,int part,int state,LPCWSTR text,int count,DWORD flags,const RECT *rect,const void *address)
{
    status_draw_scope *s=status_current;HMODULE caller=NULL;POINT origin;HWND from;BOOL pane=FALSE;
    if(!t || (!s && !status_theme_current))return FALSE;
    memset(t,0,sizeof(*t));t->previous=status_theme_current;status_theme_current=t;
    if(!s || !s->owner || !s->owner->status_candidate || s->simple || s->owner->detached || s->epoch!=s->owner->status_epoch ||
       !dc || !text || !rect || part || state || (flags&(DT_RTLREADING|DT_MODIFYSTRING|DT_CALCRECT)) ||
       !IsWindowEnabled(s->window) || !status_identity(s->window,s->owner,NULL) || GetMapMode(dc)!=MM_TEXT ||
       (GetLayout(dc)&LAYOUT_RTL) || !GetViewportOrgEx(dc,&origin) || origin.x || origin.y ||
       !GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,(LPCWSTR)address,&caller) || caller!=s->native)return TRUE;
    from=WindowFromDC(dc);if(s->dc?dc!=s->dc:from!=s->window)return TRUE;
    for(unsigned i=0;i<s->text_count;++i){RECT p=s->text_panes[i];if(rect->left>=p.left && rect->left<p.right && rect->right<=p.right && rect->right>rect->left && rect->top<p.bottom && rect->bottom>p.top){pane=TRUE;break;}}
    if(!pane || count< -1)return TRUE;
    if(count==-1){count=0;while(count<513 && text[count])++count;}
    if(count<=0 || count>512)return TRUE;
    for(int i=0;i<count;++i)if(text[i]>0x7f)return TRUE;
    memcpy(t->text,text,(size_t)count*sizeof(WCHAR));t->length=count;t->dc=dc;t->paint=s;t->valid=TRUE;return TRUE;
}
void ntw16s_status_theme_end(void *data)
{ntw16s_status_theme_scope *t=data;if(status_theme_current==t)status_theme_current=t->previous;}

/* Borrow a private font only for verified native simple-status text. */
HFONT ntw16s_status_simple_font(HDC dc,LPCWSTR text,int count,UINT flags,const void *address,void **token)
{
    status_draw_scope *s=status_current;HMODULE caller=NULL;POINT origin;HWND from;HFONT source;LOGFONTW log={0};TEXTMETRICW tm={0};int widths[96]={0};
    if(token)*token=NULL;
    if(!token || !s || !s->owner || !s->owner->status_candidate || s->owner->detached || s->epoch!=s->owner->status_epoch ||
       !dc || !text || (flags&(DT_RTLREADING|DT_MODIFYSTRING)) || GetMapMode(dc)!=MM_TEXT || (GetLayout(dc)&LAYOUT_RTL) ||
       !GetViewportOrgEx(dc,&origin) || origin.x || origin.y || !status_identity(s->window,s->owner,NULL) || !IsWindowEnabled(s->window) ||
       !GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,(LPCWSTR)address,&caller) || caller!=GetModuleHandleW(L"uxtheme.dll"))return NULL;
    from=WindowFromDC(dc);if(s->dc?dc!=s->dc:from!=s->window)return NULL;
    if(count< -1)return NULL;
    if(count==-1){count=0;while(count<4096 && text[count])++count;if(count==4096)return NULL;}
    if(count<=0 || count>4096)return NULL;
    for(int i=0;i<count;++i)if(text[i]>0x7f)return NULL;
    if(s->simple){if(s->simple_length!=count || memcmp(s->simple_text,text,(size_t)count*sizeof(WCHAR)))return NULL;}
    else {ntw16s_status_theme_scope *t=status_theme_current;if(!t || !t->valid || t->paint!=s || t->dc!=dc || t->length!=count || memcmp(t->text,text,(size_t)count*sizeof(WCHAR)))return NULL;}
    if(!status_identity(s->window,s->owner,NULL) || s->owner->detached || s->epoch!=s->owner->status_epoch)return NULL;
    source=(HFONT)GetCurrentObject(dc,OBJ_FONT);
    if(!source || GetObjectW(source,sizeof(log),&log)!=sizeof(log) || !GetTextMetricsW(dc,&tm))return NULL;
    for(unsigned i=0;i<96;++i){WCHAR character=(WCHAR)(32+i);SIZE extent;
        if(!GetTextExtentPoint32W(dc,&character,1,&extent))return NULL;
        widths[i]=extent.cx;
    }
    attachment *a=s->owner;
    BOOL changed=a->status_font_invalid || a->status_cached_source!=source || memcmp(&a->status_font_log,&log,sizeof(log)) || memcmp(&a->status_font_metrics,&tm,sizeof(tm)) || memcmp(a->status_font_widths,widths,sizeof(widths));
    if(changed){
        if(a->status_font_borrows)return NULL;
        win16_style_system_release(a->status_cached_font);a->status_cached_font=NULL;a->status_cached_source=NULL;
        a->status_font_invalid=FALSE;HFONT replacement=win16_style_system_acquire_isolated(source);if(!replacement)return NULL;
        a->status_cached_source=source;a->status_cached_font=replacement;a->status_font_log=log;a->status_font_metrics=tm;memcpy(a->status_font_widths,widths,sizeof(widths));
    }
    if(!a->status_cached_font || a->status_font_borrows==UINT_MAX)return NULL;
    if(a->status_cached_font!=source && !win16_style_system_try_retain(a->status_cached_font))return NULL;
    ++a->status_font_borrows;*token=a;return a->status_cached_font;
}
void ntw16s_status_simple_return(void *token,HFONT font)
{
    attachment *a=token;if(!a || !a->status_font_borrows)return;
    win16_style_system_release(font);--a->status_font_borrows;
    if(!a->status_font_borrows && a->status_font_invalid){win16_style_system_release(a->status_cached_font);a->status_cached_font=NULL;a->status_cached_source=NULL;}
}

/* The themed native status control omits the final pane background when it
 * owns a sizing grip. Only a native text call for an icon-free,
 * ordinary final pane supplies a verified pre-content drawing boundary. */
void ntw16s_status_text_frame(HDC dc,const RECT *text,UINT flags,const void *address)
{
    status_draw_scope *s=status_current;HMODULE module=NULL;POINT origin;RECT pane;HWND window;
    LRESULT parts,icon,type;
    if(!s || !s->owner || s->owner->detached || s->epoch!=s->owner->status_epoch ||
        s->count<2 || !text || (flags&(DT_CALCRECT|DT_RTLREADING)) ||
        !(GetWindowLongPtrW(s->window,GWL_STYLE)&SBARS_SIZEGRIP) ||
        !status_identity(s->window,s->owner,NULL) ||
        !GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            (LPCWSTR)address,&module) || module!=s->native || !dc ||
        GetMapMode(dc)!=MM_TEXT || (GetLayout(dc)&LAYOUT_RTL) ||
        !GetViewportOrgEx(dc,&origin) || origin.x || origin.y)return;
    window=WindowFromDC(dc);if(s->dc?dc!=s->dc:window!=s->window)return;
    pane=s->panes[s->count-1];
    if(text->left<=pane.left || text->left>=pane.right || text->right>pane.right ||
        text->right<=text->left || text->top>=pane.bottom || text->bottom<=pane.top)return;
    parts=SendMessageW(s->window,SB_GETPARTS,0,0);
    if(s->owner->detached || s->epoch!=s->owner->status_epoch || parts!=(LRESULT)s->count)return;
    type=SendMessageW(s->window,SB_GETTEXTLENGTHW,s->count-1,0);
    if(s->owner->detached || s->epoch!=s->owner->status_epoch || HIWORD(type)!=0)return;
    icon=SendMessageW(s->window,SB_GETICON,s->count-1,0);
    if(s->owner->detached || s->epoch!=s->owner->status_epoch || icon)return;
    int saved=SaveDC(dc);if(saved){FrameRect(dc,&pane,(HBRUSH)GetStockObject(BLACK_BRUSH));RestoreDC(dc,saved);}
}

/* For a size-grip pane, native comctl draws its icon before its text.
 * Frame only the verified native pane before the same icon is painted. */
void ntw16s_status_icon_frame(HDC dc,int x,int y,int cx,int cy,HICON icon,const void *address)
{
    status_draw_scope *s=status_current;HMODULE module=NULL;POINT origin;RECT pane;HWND window;
    LRESULT parts,type,current;
    if(!s || !s->owner || s->owner->detached || s->epoch!=s->owner->status_epoch || s->count<2 ||
        !(GetWindowLongPtrW(s->window,GWL_STYLE)&SBARS_SIZEGRIP) || !status_identity(s->window,s->owner,NULL) ||
        !dc || !icon || cx<=0 || cy<=0 ||
        !GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            (LPCWSTR)address,&module) || module!=s->native || GetMapMode(dc)!=MM_TEXT || (GetLayout(dc)&LAYOUT_RTL) ||
        !GetViewportOrgEx(dc,&origin) || origin.x || origin.y)return;
    window=WindowFromDC(dc);if(s->dc?dc!=s->dc:window!=s->window)return;
    pane=s->panes[s->count-1];
    if(x<pane.left || x>=pane.right || (LONGLONG)y+cy<=pane.top || y>=pane.bottom)return;
    parts=SendMessageW(s->window,SB_GETPARTS,0,0);
    if(s->owner->detached || s->epoch!=s->owner->status_epoch || parts!=(LRESULT)s->count)return;
    type=SendMessageW(s->window,SB_GETTEXTLENGTHW,s->count-1,0);
    if(s->owner->detached || s->epoch!=s->owner->status_epoch || HIWORD(type)!=0)return;
    current=SendMessageW(s->window,SB_GETICON,s->count-1,0);
    if(s->owner->detached || s->epoch!=s->owner->status_epoch || (HICON)current!=icon)return;
    int saved=SaveDC(dc);if(saved){FrameRect(dc,&pane,(HBRUSH)GetStockObject(BLACK_BRUSH));RestoreDC(dc,saved);}
}

/* Called only after the original native drawing call has completed. */
void ntw16s_status_frame(HDC dc,const RECT *rect,const RECT *clip,const void *caller)
{
    status_draw_scope *s=status_current;HMODULE module=NULL;HWND w,scoped=NULL;BOOL matches=FALSE;POINT origin;
    if(!s || !s->owner || s->owner->detached || s->epoch!=s->owner->status_epoch || !dc || !rect ||
        !status_identity(s->window,s->owner,NULL) || GetMapMode(dc)!=MM_TEXT || (GetLayout(dc)&LAYOUT_RTL) ||
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
static BOOL combo_same_role(HWND w,attachment *a)
{return combo_identity(w,a->combo_parent) && ((DWORD)GetWindowLongPtrW(w,GWL_STYLE)&3)==a->combo_original_type;}
BOOL ntw16s_vcl_combo_candidate(HWND w)
{attachment *a=lookup(w);return a && !a->detached && a->combo_candidate;}
BOOL ntw16s_vcl_combo_valid(HWND w)
{
    attachment *a=lookup(w);return !a || a->detached || !a->combo_candidate || !a->edit_installed || combo_same_role(w,a);
}
BOOL ntw16s_group_button_valid(HWND w)
{
    attachment *a=lookup(w);
    return !a || a->detached || !a->group_button_candidate || !a->group_button_installed ||
        (a->group_button_parent==GetParent(w) && group_button_identity(w));
}
BOOL ntw16s_group_push_button(HWND w)
{
    attachment *a=lookup(w);WCHAR name[64];
    return a && !a->detached && a->group_button_candidate && a->group_button_installed &&
        a->group_button_parent==GetParent(w) && group_button_identity(w) &&
        GetClassNameW(w,name,ARRAYSIZE(name)) && !lstrcmpW(name,L"TButton");
}
BOOL ntw16s_group_choice_button(HWND w)
{
    attachment *a=lookup(w);WCHAR name[64];
    return a && !a->detached && a->group_button_candidate && a->group_button_installed &&
        a->group_button_parent==GetParent(w) && group_button_identity(w) &&
        GetClassNameW(w,name,ARRAYSIZE(name)) && (!lstrcmpW(name,L"TCheckBox") || !lstrcmpW(name,L"TRadioButton") || !lstrcmpW(name,L"TGroupButton"));
}
static BOOL vb_data_role(HWND w,HWND parent)
{
    DWORD style=(DWORD)GetWindowLongPtrW(w,GWL_STYLE),ex=(DWORD)GetWindowLongPtrW(w,GWL_EXSTYLE);
    return parent && GetParent(w)==parent && (style&(WS_CHILD|WS_BORDER))==(WS_CHILD|WS_BORDER) &&
        (style&WS_CAPTION)!=WS_CAPTION && !(style&(WS_THICKFRAME|WS_HSCROLL|WS_VSCROLL)) &&
        !(ex&(WS_EX_LAYOUTRTL|WS_EX_RTLREADING|WS_EX_TOOLWINDOW|WS_EX_LAYERED)) &&
        (GetDpiForWindow(w)==96 || GetDpiForWindow(w)==144) && ntw16s_vb_class(w,L"ThunderRT6Data");
}
BOOL ntw16s_vb_data_valid(HWND w)
{
    attachment *a=lookup(w);
    return !a || a->detached || !a->vb_data_edge || vb_data_role(w,a->vb_data_parent);
}
BOOL ntw16s_vb_data_attached(HWND w)
{
    attachment *a=lookup(w);
    return a && !a->detached && a->vb_data_edge && vb_data_role(w,a->vb_data_parent);
}
static BOOL vb_user_edge_role(HWND w,HWND parent)
{
    DWORD style=(DWORD)GetWindowLongPtrW(w,GWL_STYLE),ex=(DWORD)GetWindowLongPtrW(w,GWL_EXSTYLE);
    return parent && GetParent(w)==parent && (style&WS_CHILD) &&
        (style&WS_CAPTION)!=WS_CAPTION && !(style&(WS_THICKFRAME|WS_HSCROLL|WS_VSCROLL)) &&
        !(ex&(WS_EX_LAYOUTRTL|WS_EX_RTLREADING|WS_EX_TOOLWINDOW|WS_EX_LAYERED)) &&
        (ntw16s_vb_class(w,L"ThunderRT6UserControlDC") || ntw16s_vb5_class(w,L"ThunderRT5UserControl"));
}
static BOOL vb_user_edge_identity(HWND w,HWND parent)
{return vb_user_edge_role(w,parent) && (GetWindowLongPtrW(w,GWL_EXSTYLE)&WS_EX_CLIENTEDGE);}
BOOL ntw16s_vb_user_label_target(HWND w)
{
    attachment *a=lookup(w);
    if(!a || a->detached || !a->vb_user_candidate)return FALSE;
    HWND parent=GetParent(w);
    return vb_user_edge_role(w,parent) && (!a->vb_user_parent || a->vb_user_parent==parent);
}
BOOL ntw16s_vb_user_edge_dormant(HWND w)
{
    attachment *a=lookup(w);
    return a && !a->detached && a->vb_user_edge && vb_user_edge_role(w,a->vb_user_parent) &&
        !(GetWindowLongPtrW(w,GWL_EXSTYLE)&WS_EX_CLIENTEDGE);
}
BOOL ntw16s_vb_user_edge_valid(HWND w)
{
    attachment *a=lookup(w);
    return !a || a->detached || !a->vb_user_edge || vb_user_edge_identity(w,a->vb_user_parent);
}
BOOL ntw16s_grid_valid(HWND w)
{
    attachment *a=lookup(w);
    return !a || a->detached || !a->grid_candidate || !a->grid_installed || grid_identity(w,a->grid_owner,a->grid_parent);
}
BOOL ntw16s_vcl_toolbar_valid(HWND w)
{
    attachment *a=lookup(w);
    return !a || a->detached || !a->vcl_toolbar_candidate || !a->vcl_toolbar_installed ||
        vcl_toolbar_identity(w,a->vcl_toolbar_owner,a->vcl_toolbar_parent);
}
BOOL ntw16s_vcl_tab_valid(HWND w)
{
    attachment *a=lookup(w);WCHAR name[64];
    return !a || a->detached || !a->tab_candidate || !a->tab_installed ||
        (GetParent(w)==a->tab_parent && GetClassNameW(w,name,ARRAYSIZE(name)) && !lstrcmpW(name,L"TTabControl") &&
         !(GetWindowLongPtrW(w,GWL_EXSTYLE)&(WS_EX_LAYOUTRTL|WS_EX_RTLREADING)));
}
/* Observe only the exact data edit forwarding its own paint to ANSI EDIT. */
void ntw16s_dbedit_ansi_witness(WNDPROC proc,HWND w,UINT message,const void *address)
{
    WNDCLASSEXA native={.cbSize=sizeof(native)};HMODULE caller=NULL,owner;WCHAR name[64];
    if(message!=WM_PAINT || !static_paint || static_paint->window!=w || !static_paint->owner->edit_candidate ||
        static_paint->owner->detached || static_paint->owner->edit_attempted || lookup(w)!=static_paint->owner ||
        !GetClassNameW(w,name,ARRAYSIZE(name)) || lstrcmpW(name,L"TDBEdit") ||
        !GetClassInfoExA(NULL,"EDIT",&native) || proc!=native.lpfnWndProc ||
        !GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,(LPCWSTR)address,&caller))return;
    owner=(HMODULE)GetClassLongPtrW(w,GCLP_HMODULE);
    if(owner && caller==owner)static_paint->owner->edit_witness_owner=owner;
}
/* Data-aware ANSI combos must forward their own paint to the native class. */
void ntw16s_dbcombo_ansi_witness(WNDPROC proc,HWND w,UINT message,const void *address)
{
    WNDCLASSEXA native={.cbSize=sizeof(native)};HMODULE caller=NULL,owner;WCHAR name[64];
    if(message!=WM_PAINT || !static_paint || static_paint->window!=w || !static_paint->owner->combo_candidate ||
        static_paint->owner->detached || static_paint->owner->edit_attempted || lookup(w)!=static_paint->owner ||
        !GetClassNameW(w,name,ARRAYSIZE(name)) || lstrcmpW(name,L"TDBComboBox") ||
        !GetClassInfoExA(NULL,"COMBOBOX",&native) || proc!=native.lpfnWndProc ||
        !GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,(LPCWSTR)address,&caller))return;
    owner=(HMODULE)GetClassLongPtrW(w,GCLP_HMODULE);
    if(owner && caller==owner)static_paint->owner->edit_witness_owner=owner;
}
BOOL ntw16s_static_call_begin(ntw16s_static_call *scope,WNDPROC proc,HWND w,UINT message,const void *address)
{
    WNDCLASSEXW native={.cbSize=sizeof(native)};HMODULE caller=NULL,native_owner=NULL,owner;
    ZeroMemory(scope,sizeof(*scope));
    if(message==WM_PAINT && static_paint && static_paint->window==w &&
       static_paint->owner->status_candidate && !static_paint->owner->detached &&
       !static_paint->owner->status_attempted && lookup(w)==static_paint->owner &&
       GetClassInfoExW(NULL,STATUSCLASSNAMEW,&native) && proc==native.lpfnWndProc &&
       GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
           (LPCWSTR)address,&caller)){
        owner=(HMODULE)GetClassLongPtrW(w,GCLP_HMODULE);
        if(owner && caller==owner)static_paint->owner->status_witness_owner=owner;
    }
    native.cbSize=sizeof(native);
    if(message==WM_PAINT && static_paint && static_paint->window==w &&
       static_paint->owner->date_candidate && !static_paint->owner->detached &&
       !static_paint->owner->date_attempted && lookup(w)==static_paint->owner &&
       GetClassInfoExW(NULL,DATETIMEPICK_CLASSW,&native) && proc==native.lpfnWndProc &&
       GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
           (LPCWSTR)address,&caller)){
        owner=(HMODULE)GetClassLongPtrW(w,GCLP_HMODULE);
        if(owner && caller==owner)static_paint->owner->date_witness_owner=owner;
    }
    native.cbSize=sizeof(native);
    if(message==WM_PAINT && static_paint && static_paint->window==w &&
       static_paint->owner->color_candidate && !static_paint->owner->detached &&
       !static_paint->owner->color_attempted && lookup(w)==static_paint->owner &&
       GetClassInfoExW(NULL,L"COMBOBOX",&native) && proc==native.lpfnWndProc &&
       GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
           (LPCWSTR)address,&caller)){
        owner=(HMODULE)GetClassLongPtrW(w,GCLP_HMODULE);
        if(owner && caller==owner)static_paint->owner->color_witness_owner=owner;
    }
    native.cbSize=sizeof(native);
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
       static_paint->owner->scroll_candidate && !static_paint->owner->detached &&
       !static_paint->owner->scroll_attempted && lookup(w)==static_paint->owner &&
       GetParent(w)==static_paint->owner->scroll_parent &&
       GetClassInfoExW(NULL,L"SCROLLBAR",&native) && proc==native.lpfnWndProc &&
       GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
           (LPCWSTR)address,&caller)){
        owner=(HMODULE)GetClassLongPtrW(w,GCLP_HMODULE);
        if(owner && caller==owner)static_paint->owner->scroll_witness=owner;
    }
    native.cbSize=sizeof(native);
    if(message==WM_PAINT && static_paint && static_paint->window==w &&
       static_paint->owner->edit_candidate && !static_paint->owner->detached &&
       !static_paint->owner->edit_attempted && lookup(w)==static_paint->owner &&
       GetClassInfoExW(NULL,static_paint->owner->link_candidate?WC_LINK:static_paint->owner->combo_candidate?L"COMBOBOX":L"EDIT",&native) && proc==native.lpfnWndProc &&
       GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
           (LPCWSTR)address,&caller)){
        owner=(HMODULE)GetClassLongPtrW(w,GCLP_HMODULE);
        if(owner && caller==owner){WCHAR name[64];
            if(!static_paint->owner->combo_candidate || (GetClassNameW(w,name,ARRAYSIZE(name)) && lstrcmpW(name,L"TDBComboBox")))
                static_paint->owner->edit_witness_owner=owner;
        }
    }
    native.cbSize=sizeof(native);
    if(message==WM_PAINT && static_paint && static_paint->window==w &&
       static_paint->owner->group_button_candidate && !static_paint->owner->detached &&
       !static_paint->owner->group_button_attempted && lookup(w)==static_paint->owner &&
       group_button_identity(w) && GetClassInfoExW(NULL,L"BUTTON",&native) && proc==native.lpfnWndProc &&
       GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
           (LPCWSTR)address,&caller)){
        owner=(HMODULE)GetClassLongPtrW(w,GCLP_HMODULE);
        if(owner && caller==owner)static_paint->owner->group_button_witness_owner=owner;
    }
    native.cbSize=sizeof(native);
    if(message==WM_PRINTCLIENT && static_paint && static_paint->window==w &&
       static_paint->owner->vcl_toolbar_candidate && !static_paint->owner->detached &&
       !static_paint->owner->vcl_toolbar_attempted && lookup(w)==static_paint->owner &&
       vcl_toolbar_identity(w,static_paint->owner->vcl_toolbar_owner,static_paint->owner->vcl_toolbar_parent) &&
       GetClassInfoExW(NULL,TOOLBARCLASSNAMEW,&native) && proc==native.lpfnWndProc &&
       GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,(LPCWSTR)address,&caller) &&
       caller==static_paint->owner->vcl_toolbar_owner)static_paint->owner->vcl_toolbar_witness=caller;
    native.cbSize=sizeof(native);
    if(message==WM_PAINT && static_paint && static_paint->window==w &&
       static_paint->owner->tab_candidate && !static_paint->owner->detached &&
       !static_paint->owner->tab_attempted && lookup(w)==static_paint->owner &&
       GetParent(w)==static_paint->owner->tab_parent && GetClassInfoExW(NULL,WC_TABCONTROLW,&native) && proc==native.lpfnWndProc &&
       GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,(LPCWSTR)address,&caller)){
        owner=(HMODULE)GetClassLongPtrW(w,GCLP_HMODULE);
        if(owner && caller==owner)static_paint->owner->tab_witness_owner=owner;
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
/* Only the newly created group's own verified paint may draw this frame. */
void ntw16s_group_frame(HDC dc,int part,int state,const RECT *rect,const RECT *clip,LPCWSTR text,int count,DWORD flags,const void *caller,BOOL is_text)
{
 static_paint_scope *scope=static_paint;attachment *a=scope?scope->owner:NULL; HWND w=scope?scope->window:NULL,parent=w?GetParent(w):NULL;
 WCHAR cls[64],real[64],caption[512];HMODULE module=NULL;RECT client={0},frame={0};POINT org={0},view={0};DWORD pid=0;
 if(!a||a->detached||!a->group_frame_candidate||a->group_frame_retired||lookup(w)!=a||!parent||part!=4||!rect||!((state==1&&IsWindowEnabled(w))||(state==2&&!IsWindowEnabled(w)))||
    GetWindowThreadProcessId(w,&pid)!=GetCurrentThreadId()||pid!=GetCurrentProcessId()||GetWindowThreadProcessId(parent,NULL)!=GetCurrentThreadId()||
    (a->group_frame_parent&&a->group_frame_parent!=parent)||
    !GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,(LPCWSTR)caller,&module)||
    module!=(HMODULE)GetClassLongPtrW(w,GCLP_HMODULE)||module!=(HMODULE)GetClassLongPtrW(parent,GCLP_HMODULE)||WindowFromDC(dc)!=w||
    !GetClassNameW(w,cls,64)||(lstrcmpW(cls,L"TGroupBox")&&lstrcmpW(cls,L"TRadioGroup"))||!RealGetWindowClassW(w,real,64)||lstrcmpW(real,cls)||!(GetWindowLongPtrW(w,GWL_STYLE)&WS_CHILD)||
    (GetWindowLongPtrW(w,GWL_EXSTYLE)&(WS_EX_LAYOUTRTL|WS_EX_RTLREADING))||GetMapMode(dc)!=MM_TEXT||GetLayout(dc)!=0||
    !GetWindowOrgEx(dc,&org)||!GetViewportOrgEx(dc,&view)||org.x||org.y||view.x||view.y||!GetClientRect(w,&client)||client.right<4||client.bottom<4)return;
 if(is_text){int n;if(!text||count<=0||count>=512||flags!=0||(n=GetWindowTextW(w,caption,512))!=count||wcsncmp(caption,text,(size_t)count)||
      rect->left<0||rect->right>client.right||rect->top!=0||rect->bottom<=0||rect->bottom>=client.bottom)return;
    frame=(RECT){0,rect->top+(rect->bottom-rect->top)/2,client.right,client.bottom};
    if(!scope->group_background||scope->group_state!=state||!EqualRect(&frame,&scope->group_rect))return;
 }else {if(rect->left!=0||rect->top<0||rect->right!=client.right||rect->bottom!=client.bottom)return;scope->group_background=TRUE;scope->group_state=state;scope->group_rect=*rect;
    if(GetWindowTextLengthW(w)!=0||rect->top!=0)return;
    frame=*rect;}
 if(a->detached||a->group_frame_retired||static_paint!=scope||lookup(w)!=a||GetParent(w)!=parent||IsWindowEnabled(w)!=(state==1))return;
 int saved=SaveDC(dc);if(!saved)return;if(clip)IntersectClipRect(dc,clip->left,clip->top,clip->right,clip->bottom);
 if(is_text)ExcludeClipRect(dc,rect->left,rect->top,rect->right,rect->bottom);
 if(FrameRect(dc,&frame,(HBRUSH)GetStockObject(BLACK_BRUSH))&&!a->detached){a->group_frame_parent=parent;a->group_frame_verified=TRUE;}RestoreDC(dc,saved);
}
/* Accept only paired outer bevel strokes from the panel owner in its paint. */
void ntw16s_panel_polyline(HDC dc,const POINT *points,int count,const void *caller,BOOL (WINAPI *draw)(HDC,const POINT*,int))
{
 static_paint_scope *scope=static_paint;attachment *a=scope?scope->owner:NULL;HWND w=scope?scope->window:NULL,parent=w?GetParent(w):NULL;
 HMODULE module=NULL;RECT client={0},clip={0};POINT org={0},view={0};LOGPEN pen={0};DWORD pid=0;
 if(!a||a->detached||!a->panel_frame_candidate||a->panel_frame_retired||lookup(w)!=a||!parent||
    (a->panel_frame_parent&&a->panel_frame_parent!=parent)||GetWindowThreadProcessId(w,&pid)!=GetCurrentThreadId()||pid!=GetCurrentProcessId()||
    GetWindowThreadProcessId(parent,NULL)!=GetCurrentThreadId()||
    !GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,(LPCWSTR)caller,&module)||
    module!=(HMODULE)GetClassLongPtrW(w,GCLP_HMODULE)||module!=(HMODULE)GetClassLongPtrW(parent,GCLP_HMODULE)||
    WindowFromDC(dc)!=w||count!=3||!points||GetMapMode(dc)!=MM_TEXT||GetLayout(dc)!=0||
    !GetWindowOrgEx(dc,&org)||!GetViewportOrgEx(dc,&view)||org.x||org.y||view.x||view.y||
    !GetClientRect(w,&client)||client.right<4||client.bottom<4||GetClipBox(dc,&clip)<=NULLREGION||
    (clip.left>0&&clip.top>0&&clip.right<client.right&&clip.bottom<client.bottom)||
    GetObjectW(GetCurrentObject(dc,OBJ_PEN),sizeof(pen),&pen)!=sizeof(pen)||pen.lopnStyle!=PS_SOLID||pen.lopnWidth.x!=1){if(scope)scope->panel_first=FALSE;return;}
 COLORREF hi=GetSysColor(COLOR_BTNHIGHLIGHT),lo=GetSysColor(COLOR_BTNSHADOW);if(hi==lo){scope->panel_first=FALSE;return;}
 if(points[0].x==0&&points[0].y==client.bottom-1&&points[1].x==0&&points[1].y==0&&points[2].x==client.right-1&&points[2].y==0&&(pen.lopnColor==hi||pen.lopnColor==lo)){
    scope->panel_first=TRUE;scope->panel_enabled=IsWindowEnabled(w);scope->panel_parent=parent;scope->panel_dc=dc;scope->panel_color=pen.lopnColor;memcpy(scope->panel_points,points,sizeof(scope->panel_points));return;}
 if(!scope->panel_first||scope->panel_enabled!=IsWindowEnabled(w)||scope->panel_parent!=parent||scope->panel_dc!=dc||points[0].x!=client.right-1||points[0].y!=0||points[1].x!=client.right-1||points[1].y!=client.bottom-1||points[2].x!=-1||points[2].y!=client.bottom-1||
    !((scope->panel_color==hi&&pen.lopnColor==lo)||(scope->panel_color==lo&&pen.lopnColor==hi))){scope->panel_first=FALSE;return;}
 scope->panel_first=FALSE;if(scope->panel_enabled!=IsWindowEnabled(w)||GetParent(w)!=parent)return;int saved=SaveDC(dc);if(!saved)return;
 if(SelectObject(dc,GetStockObject(BLACK_PEN))){const POINT *shadow=scope->panel_color==lo?scope->panel_points:points;
    if(draw(dc,shadow,3)&&!a->detached){a->panel_frame_parent=parent;a->panel_frame_verified=TRUE;}}
 RestoreDC(dc,saved);
}
static BOOL box_identity(HWND w,HMODULE owner,HWND parent)
{
    WCHAR name[64],real[64];WNDCLASSEXW c={.cbSize=sizeof(c)};HMODULE proc_owner=NULL;
    DWORD style=(DWORD)GetWindowLongPtrW(w,GWL_STYLE),ex=(DWORD)GetWindowLongPtrW(w,GWL_EXSTYLE);
    return owner && parent && IsWindow(w) && GetParent(w)==parent &&
        GetWindowThreadProcessId(w,NULL)==GetCurrentThreadId() && GetWindowThreadProcessId(parent,NULL)==GetCurrentThreadId() &&
        (HMODULE)GetClassLongPtrW(w,GCLP_HMODULE)==owner && (HMODULE)GetClassLongPtrW(parent,GCLP_HMODULE)==owner &&
        GetClassNameW(w,name,ARRAYSIZE(name)) && !lstrcmpW(name,L"TScrollBox") && RealGetWindowClassW(w,real,ARRAYSIZE(real)) && !lstrcmpW(name,real) &&
        IsWindowUnicode(w) && (style&WS_CHILD) && (ex&WS_EX_CLIENTEDGE) && !(ex&(WS_EX_LAYOUTRTL|WS_EX_RTLREADING|WS_EX_LAYERED)) &&
        GetClassInfoExW(owner,name,&c) && c.lpfnWndProc &&
        GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,(LPCWSTR)c.lpfnWndProc,&proc_owner) && proc_owner==owner;
}
typedef struct box_witness_scope {struct box_witness_scope *previous;attachment *owner;HWND window;BOOL witnessed,completed,nested;ntw16s_vb_paint *paint;void *status_prior,*spin_prior;} box_witness_scope;
static _Thread_local box_witness_scope *box_current;
void ntw16s_box_witness(HWND w,UINT msg,const void *address)
{
    box_witness_scope *s=box_current;HMODULE caller=NULL;
    if(!s || s->window!=w || msg!=WM_NCPAINT || s->owner->detached ||
       !box_identity(w,s->owner->box_owner,s->owner->box_parent) ||
       !GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,(LPCWSTR)address,&caller) ||
       (caller!=s->owner->box_owner && caller!=GetModuleHandleW(L"user32.dll")))return;
    s->witnessed=TRUE;
}
static BOOL controlbar_identity(HWND w,attachment *a)
{
 WCHAR name[64],real[64];DWORD pid=0;WNDCLASSEXW c={.cbSize=sizeof(c)};HMODULE proc_owner=NULL;HWND parent=GetParent(w);DWORD ex=(DWORD)GetWindowLongPtrW(w,GWL_EXSTYLE);
 return a && !a->detached && !a->controlbar_retired && IsWindow(w) && parent && parent==a->controlbar_parent &&
  GetWindowThreadProcessId(w,&pid)==GetCurrentThreadId() && pid==GetCurrentProcessId() && GetWindowThreadProcessId(parent,NULL)==GetCurrentThreadId() &&
  (HMODULE)GetClassLongPtrW(w,GCLP_HMODULE)==a->controlbar_owner && (HMODULE)GetClassLongPtrW(parent,GCLP_HMODULE)==a->controlbar_owner &&
  GetClassNameW(w,name,ARRAYSIZE(name)) && !lstrcmpW(name,L"TControlBar") && RealGetWindowClassW(w,real,ARRAYSIZE(real)) && !lstrcmpW(name,real) &&
  IsWindowUnicode(w) && (GetWindowLongPtrW(w,GWL_STYLE)&WS_CHILD) && !(ex&(WS_EX_LAYOUTRTL|WS_EX_RTLREADING|WS_EX_LAYERED)) &&
  GetClassInfoExW(a->controlbar_owner,name,&c) && c.lpfnWndProc && GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,(LPCWSTR)c.lpfnWndProc,&proc_owner) && proc_owner==a->controlbar_owner;
}
BOOL ntw16s_controlbar_edge_allowed(UINT edge)
{return edge==BDR_RAISEDOUTER || edge==BDR_SUNKENOUTER || edge==BDR_RAISEDINNER || edge==BDR_SUNKENINNER || edge==EDGE_RAISED || edge==EDGE_SUNKEN || edge==EDGE_ETCHED || edge==EDGE_BUMP;}
BOOL ntw16s_controlbar_edge_active(void)
{return box_current && box_current->owner->controlbar_candidate && !box_current->owner->detached;}
void ntw16s_controlbar_edge(HDC dc,const RECT *rect,UINT edge,UINT flags,const void *address)
{
 box_witness_scope *s=box_current;HMODULE caller=NULL;RECT bounds;POINT origin={0};
 if(!s || !s->owner->controlbar_candidate || !controlbar_identity(s->window,s->owner) || !dc || !rect || !ntw16s_controlbar_edge_allowed(edge) || flags!=(BF_RECT|BF_ADJUST) || WindowFromDC(dc)!=s->window || GetMapMode(dc)!=MM_TEXT || (GetLayout(dc)&LAYOUT_RTL) || !GetViewportOrgEx(dc,&origin) || origin.x || origin.y || !GetWindowOrgEx(dc,&origin) || origin.x || origin.y || !GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,(LPCWSTR)address,&caller) || caller!=s->owner->controlbar_owner || !GetWindowRect(s->window,&bounds))return;
 OffsetRect(&bounds,-bounds.left,-bounds.top);if(!EqualRect(rect,&bounds))return;
 int saved=SaveDC(dc);if(!saved)return;BOOL drew=FrameRect(dc,rect,(HBRUSH)GetStockObject(BLACK_BRUSH));RestoreDC(dc,saved);if(drew && controlbar_identity(s->window,s->owner))s->owner->controlbar_verified=TRUE;
}
static BOOL rich_identity(HWND w,attachment *a)
{
    WCHAR name[64];HWND parent=GetParent(w);DWORD style=(DWORD)GetWindowLongPtrW(w,GWL_STYLE),ex=(DWORD)GetWindowLongPtrW(w,GWL_EXSTYLE);
    return a && !a->detached && IsWindow(w) && parent && parent==a->rich_parent &&
        GetWindowThreadProcessId(w,NULL)==GetCurrentThreadId() &&
        (a->rich_popup?(parent==GetDesktopWindow() && a->rich_owner==GetModuleHandleW(NULL) && (style&WS_BORDER) && !(style&(LBS_OWNERDRAWFIXED|LBS_OWNERDRAWVARIABLE|LBS_NODATA))):(GetWindowThreadProcessId(parent,NULL)==GetCurrentThreadId() && (HMODULE)GetClassLongPtrW(parent,GCLP_HMODULE)==a->rich_owner)) &&
        (HMODULE)GetClassLongPtrW(w,GCLP_HMODULE)==a->rich_owner &&
        GetClassNameW(w,name,ARRAYSIZE(name)) && !lstrcmpW(name,a->rich_popup?L"TPopupListbox":a->rich_month?L"TMonthCalendar":a->rich_hotkey?L"THotKey":a->rich_unicode?L"TRichEdit":L"TRichEditURL") && (!a->rich_unicode || IsWindowUnicode(w)) && (!a->rich_month || !(style&MCS_MULTISELECT)) && (style&WS_CHILD) &&
        (a->rich_popup || a->rich_month || (ex&WS_EX_CLIENTEDGE)) && !(ex&(WS_EX_LAYOUTRTL|WS_EX_RTLREADING|WS_EX_LAYERED));
}
BOOL ntw16s_rich_valid(HWND w)
{attachment *a=lookup(w);return !a || a->detached || ((!a->rich_candidate || !a->rich_installed || rich_identity(w,a)) && (!a->box_candidate || !a->box_installed || box_identity(w,a->box_owner,a->box_parent)));}
static _Thread_local BOOL month_query_active;
static void month_query_end(void *unused){(void)unused;month_query_active=FALSE;}
static NTW16S_UNWIND_FUNCTION void month_header_query(HWND w,static_paint_scope *scope)
{if(month_query_active)return;ntw16s_unwind_scope unwind;MCGRIDINFO grid={.cbSize=sizeof(grid),.dwPart=MCGIP_CALENDARHEADER,.dwFlags=MCGIF_RECT};
 scope->month_header_valid=FALSE;month_query_active=TRUE;NTW16S_UNWIND_BEGIN(unwind,month_query_end,NULL);
 LRESULT count=SendMessageW(w,MCM_GETCALENDARCOUNT,0,0);
 if(count==1 && static_paint==scope && !scope->owner->detached && rich_identity(w,scope->owner) && SendMessageW(w,MCM_GETCALENDARGRIDINFO,0,(LPARAM)&grid) && static_paint==scope && !scope->owner->detached && rich_identity(w,scope->owner) && grid.rc.right>grid.rc.left && grid.rc.bottom>grid.rc.top){scope->month_header=grid.rc;scope->month_header_valid=TRUE;scope->month_header_known=TRUE;
 MCGRIDINFO prev={.cbSize=sizeof(prev),.dwPart=MCGIP_PREV,.dwFlags=MCGIF_RECT},next={.cbSize=sizeof(next),.dwPart=MCGIP_NEXT,.dwFlags=MCGIF_RECT};
 if(SendMessageW(w,MCM_GETCALENDARGRIDINFO,0,(LPARAM)&prev) && static_paint==scope && !scope->owner->detached && rich_identity(w,scope->owner) && SendMessageW(w,MCM_GETCALENDARGRIDINFO,0,(LPARAM)&next) && static_paint==scope && !scope->owner->detached && rich_identity(w,scope->owner) && prev.rc.right>prev.rc.left && next.rc.right>next.rc.left){scope->month_prev=prev.rc;scope->month_next=next.rc;scope->month_nav_valid=TRUE;}}
 NTW16S_UNWIND_END(unwind);
}
void ntw16s_rich_witness(WNDPROC proc,HWND w,UINT msg,const void *address,BOOL wide)
{
    WNDCLASSEXA c={.cbSize=sizeof(c)};WNDCLASSEXW cw={.cbSize=sizeof(cw)};HMODULE caller=NULL,rich;
    if(msg!=WM_PAINT || !static_paint || static_paint->window!=w || !static_paint->owner->rich_candidate)return;
    rich=GetModuleHandleW(static_paint->owner->rich_popup?L"user32.dll":(static_paint->owner->rich_hotkey || static_paint->owner->rich_month)?L"comctl32.dll":L"riched20.dll");
    if(!rich_identity(w,static_paint->owner) || wide!=static_paint->owner->rich_unicode || !rich ||
       (wide?(!GetClassInfoExW(static_paint->owner->rich_popup?NULL:rich,static_paint->owner->rich_popup?L"LISTBOX":static_paint->owner->rich_month?L"SysMonthCal32":static_paint->owner->rich_hotkey?L"msctls_hotkey32":L"RichEdit20W",&cw) || proc!=cw.lpfnWndProc):(!GetClassInfoExA(rich,"RichEdit20A",&c) || proc!=c.lpfnWndProc)) ||
       !GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,(LPCWSTR)address,&caller) ||
       caller!=static_paint->owner->rich_owner)return;
    static_paint->owner->rich_witness=TRUE;
    if(static_paint->owner->rich_month)month_header_query(w,static_paint);
}
void win16_style_month_arrow(HDC,const RECT *,const RECT *,int,int);
void ntw16s_month_arrow(HTHEME theme,HDC dc,int part,int state,const RECT *r,const RECT *clip,const void *address)
{static_paint_scope *s=static_paint;HMODULE caller=NULL,native=GetModuleHandleW(L"comctl32.dll");
 if(!s || !s->owner->rich_month || !s->owner->rich_witness || s->owner->detached || !r || (part!=MC_NAVNEXT && part!=MC_NAVPREV) || state<MCNN_NORMAL || state>MCNN_DISABLED ||
    !rich_identity(s->window,s->owner) || !native || GetWindowTheme(s->window)!=theme ||
    !GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,(LPCWSTR)address,&caller) || caller!=native)return;
 win16_style_month_arrow(dc,r,clip,part,state);
}
static BOOL month_draw_identity(static_paint_scope *s,HTHEME theme,HDC dc,const void *address)
{HMODULE caller=NULL,native=GetModuleHandleW(L"comctl32.dll");return s && s->owner->rich_month && s->owner->rich_witness && !s->owner->detached && dc && native &&
 rich_identity(s->window,s->owner) && GetWindowTheme(s->window)==theme &&
 GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,(LPCWSTR)address,&caller) && caller==native;}
static BOOL month_text_allowed(HTHEME theme,HDC dc,int part,int state,DWORD flags,DWORD flags2,const RECT *r,const void *address)
{static_paint_scope *s=static_paint;return s && r && (part==MC_GRIDCELL || part==MC_GRIDCELLUPPER) && state>=0 && state<=7 && flags2==0 && (flags==0x925 || flags==0x926 || flags==0x924) && IsWindowEnabled(s->window) && month_draw_identity(s,theme,dc,address);}
HFONT ntw16s_month_text_borrow(HTHEME theme,HDC dc,int part,int state,DWORD flags,DWORD flags2,const RECT *r,const void *address)
{if(!month_text_allowed(theme,dc,part,state,flags,flags2,r,address))return NULL;static_paint_scope *s=static_paint;attachment *a=s->owner;if(GetDpiForWindow(s->window)!=96)return NULL;if(!a->month_cached_font)a->month_cached_font=win16_style_calendar_font_acquire();if(static_paint!=s || !month_text_allowed(theme,dc,part,state,flags,flags2,r,address) || !a->month_cached_font || !win16_style_system_try_retain(a->month_cached_font))return NULL;return a->month_cached_font;}
BOOL ntw16s_month_text_room(HTHEME theme,HDC dc,int part,int state,DWORD flags,DWORD flags2,const RECT *r,int width,const void *address)
{static_paint_scope *s=static_paint;if(!month_text_allowed(theme,dc,part,state,flags,flags2,r,address) || !s->month_header_known)return FALSE;
 if(EqualRect(r,&s->month_header))return s->month_nav_valid && ntw16s_month_title_fits(r,&s->month_prev,&s->month_next,width);
 return TRUE;}
void ntw16s_month_header_text(HTHEME theme,HDC dc,int part,int state,const RECT *r,DWORD flags,const void *address)
{static_paint_scope *s=static_paint;COLORREF text;if(!s || !s->month_header_valid || !IsWindowEnabled(s->window) || part!=MC_GRIDCELL || state!=0 || !r || (flags&DT_CALCRECT) || !EqualRect(r,&s->month_header) ||
 !month_draw_identity(s,theme,dc,address) || FAILED(GetThemeColor(theme,part,state,TMT_TEXTCOLOR,&text)) || text!=RGB(0,0,0))return;
 s->month_header_valid=FALSE;int saved=SaveDC(dc);if(!saved)return;win16_style_palette palette;win16_style_default_palette(&palette);HBRUSH brush=(HBRUSH)GetStockObject(DC_BRUSH);SetDCBrushColor(dc,palette.active);FillRect(dc,r,brush);RestoreDC(dc,saved);}
void ntw16s_month_selection_background(HTHEME theme,HDC dc,int part,int state,const RECT *r,const RECT *clip,const void *address)
{static_paint_scope *s=static_paint;if(!s)return;s->month_selected=FALSE;if(part!=MC_GRIDCELLBACKGROUND || (state!=MCGCB_SELECTED && state!=MCGCB_SELECTEDNOTFOCUSED) || !r || r->right<=r->left || r->bottom<=r->top || !month_draw_identity(s,theme,dc,address))return;
 s->month_selected=TRUE;s->month_theme=theme;s->month_dc=dc;s->month_rect=*r;s->month_has_clip=clip!=NULL;if(clip)s->month_clip=*clip;}
void ntw16s_month_selection_text(HTHEME theme,HDC dc,int part,int state,const RECT *r,DWORD flags,const void *address)
{static_paint_scope *s=static_paint;if(!s||!s->month_selected)return;s->month_selected=FALSE;COLORREF foreground;RECT *cell=&s->month_rect;
 if(part!=MC_GRIDCELL || state!=6 || !r || !IsWindowEnabled(s->window) || (flags&DT_CALCRECT) || theme!=s->month_theme || dc!=s->month_dc ||
    r->left!=cell->left || r->top!=cell->top || r->right>cell->right || r->bottom!=cell->bottom ||
    !month_draw_identity(s,theme,dc,address) || FAILED(GetThemeColor(theme,part,state,TMT_TEXTCOLOR,&foreground)) || foreground!=RGB(0,0,0))return;
 int saved=SaveDC(dc);if(!saved)return;if(s->month_has_clip && IntersectClipRect(dc,s->month_clip.left,s->month_clip.top,s->month_clip.right,s->month_clip.bottom)==ERROR){RestoreDC(dc,saved);return;}
 win16_style_palette palette;win16_style_default_palette(&palette);HBRUSH brush=(HBRUSH)GetStockObject(DC_BRUSH);SetDCBrushColor(dc,palette.active);FillRect(dc,cell,brush);RestoreDC(dc,saved);
}
static void release(attachment *a)
{if(!--a->calls && a->detached){win16_style_system_release(a->month_cached_font);win16_style_system_release(a->status_cached_font);ntw16s_vb_fonts_clear(&a->fonts);free(a);}}
static void static_paint_end(void *data)
{static_paint_scope *scope=data;static_paint=scope->previous;if(!scope->completed){if(scope->owned_paint){ntw16s_vb_paint_end(scope->owned_paint);status_current=scope->status_prior;spin_current=scope->spin_prior;}release(scope->owner);}}
static NTW16S_UNWIND_FUNCTION LRESULT static_paint_dispatch(attachment *a,HWND w,UINT m,WPARAM p,LPARAM l,ntw16s_vb_paint *owned_paint,void *status_prior,void *spin_prior)
{
    static_paint_scope scope={.previous=static_paint,.owner=a,.window=w,.owned_paint=owned_paint,.status_prior=status_prior,.spin_prior=spin_prior};ntw16s_unwind_scope unwind;LRESULT result;DWORD entry=GetLastError(),error;
    static_paint=((a->static_candidate || a->scroll_candidate || a->edit_candidate || a->group_button_candidate || a->grid_candidate || a->inplace_candidate || a->vcl_toolbar_candidate || a->track_candidate || a->tab_candidate || a->color_candidate || a->date_candidate || a->status_candidate || a->rich_candidate || a->group_frame_candidate || a->panel_frame_candidate) && !a->detached && m==WM_PAINT)?&scope:NULL;
    NTW16S_UNWIND_BEGIN(unwind,static_paint_end,&scope);
    SetLastError(entry);result=DefSubclassProc(w,m,p,l);error=GetLastError();scope.completed=TRUE;
    NTW16S_UNWIND_END(unwind);SetLastError(error);return result;
}
static void box_witness_end(void *data)
{box_witness_scope *s=data;box_current=s->previous;if(!s->completed){ntw16s_vb_paint_end(s->paint);status_current=s->status_prior;spin_current=s->spin_prior;if(!s->nested)release(s->owner);}}
static NTW16S_UNWIND_FUNCTION LRESULT box_dispatch(attachment *a,HWND w,UINT m,WPARAM p,LPARAM l,ntw16s_vb_paint *paint,void *status_prior,void *spin_prior,BOOL *witnessed)
{box_witness_scope scope={.previous=box_current,.owner=a,.window=w,.nested=static_paint!=NULL,.paint=paint,.status_prior=status_prior,.spin_prior=spin_prior};ntw16s_unwind_scope unwind;DWORD entry=GetLastError(),error;LRESULT result;box_current=&scope;NTW16S_UNWIND_BEGIN(unwind,box_witness_end,&scope);SetLastError(entry);result=scope.nested?static_paint_dispatch(a,w,m,p,l,NULL,NULL,NULL):DefSubclassProc(w,m,p,l);error=GetLastError();*witnessed=scope.witnessed;scope.completed=TRUE;NTW16S_UNWIND_END(unwind);SetLastError(error);return result;}
typedef struct modern_spin_unwind {
    spin_draw_scope *scope,*prior;status_draw_scope *status_prior;ntw16s_vb_paint *paint;attachment *owner;BOOL completed,nested;
} modern_spin_unwind;
static void modern_spin_end(void *data)
{
    modern_spin_unwind *s=data;if(!s->completed){spin_current=s->prior;spin_stack=s->scope->previous_stack;
        spin_buffers_clear(s->scope);status_current=s->status_prior;ntw16s_vb_paint_end(s->paint);if(!s->nested)release(s->owner);}
}
static NTW16S_UNWIND_FUNCTION LRESULT modern_spin_dispatch(attachment *a,HWND w,UINT m,WPARAM p,LPARAM l,spin_draw_scope *scope,spin_draw_scope *prior,status_draw_scope *status_prior,ntw16s_vb_paint *paint)
{
    modern_spin_unwind state={scope,prior,status_prior,paint,a,FALSE,static_paint!=NULL};ntw16s_unwind_scope unwind;
    DWORD entry=GetLastError(),error;LRESULT result;NTW16S_UNWIND_BEGIN(unwind,modern_spin_end,&state);
    SetLastError(entry);result=state.nested?static_paint_dispatch(a,w,m,p,l,NULL,NULL,NULL):DefSubclassProc(w,m,p,l);
    error=GetLastError();state.completed=TRUE;NTW16S_UNWIND_END(unwind);SetLastError(error);return result;
}
typedef struct color_print_scope {attachment *owner;HRGN clip;BOOL completed,nested;ntw16s_vb_paint *paint;status_draw_scope *status_prior;spin_draw_scope *spin_prior;} color_print_scope;
static void color_print_end(void *data)
{
    color_print_scope *scope=data;
    if(!scope->completed){ntw16s_vb_paint_end(scope->paint);status_current=scope->status_prior;spin_current=scope->spin_prior;DeleteObject(scope->clip);if(!scope->nested)release(scope->owner);}
}
static NTW16S_UNWIND_FUNCTION LRESULT color_print_dispatch(attachment *a,HWND w,UINT m,WPARAM p,LPARAM l,HRGN clip,ntw16s_vb_paint *paint,status_draw_scope *status_prior,spin_draw_scope *spin_prior)
{
    color_print_scope scope={a,clip,FALSE,static_paint!=NULL,paint,status_prior,spin_prior};ntw16s_unwind_scope unwind;
    DWORD entry=GetLastError(),error;LRESULT result;
    NTW16S_UNWIND_BEGIN(unwind,color_print_end,&scope);
    SetLastError(entry);result=scope.nested?static_paint_dispatch(a,w,m,p,l,NULL,NULL,NULL):DefSubclassProc(w,m,p,l);
    error=GetLastError();scope.completed=TRUE;NTW16S_UNWIND_END(unwind);SetLastError(error);return result;
}
typedef struct status_print_unwind {attachment *owner;ntw16s_vb_paint *paint;status_draw_scope *prior;spin_draw_scope *spin_prior;BOOL completed,nested;} status_print_unwind;
static void status_print_end(void *data)
{status_print_unwind *s=data;if(!s->completed){ntw16s_vb_paint_end(s->paint);status_current=s->prior;spin_current=s->spin_prior;if(!s->nested)release(s->owner);}}
static NTW16S_UNWIND_FUNCTION LRESULT status_print_dispatch(attachment *a,HWND w,UINT m,WPARAM p,LPARAM l,ntw16s_vb_paint *paint,status_draw_scope *prior,spin_draw_scope *spin_prior)
{status_print_unwind state={a,paint,prior,spin_prior,FALSE,static_paint!=NULL};ntw16s_unwind_scope unwind;DWORD entry=GetLastError(),error;LRESULT result;
 NTW16S_UNWIND_BEGIN(unwind,status_print_end,&state);SetLastError(entry);result=state.nested?static_paint_dispatch(a,w,m,p,l,NULL,NULL,NULL):DefSubclassProc(w,m,p,l);error=GetLastError();state.completed=TRUE;NTW16S_UNWIND_END(unwind);SetLastError(error);return result;}
/* The application owns owner-drawn swatches and text. Only the documented
 * native button rectangle is eligible for post-paint chrome. */
static BOOL color_identity(HWND w,attachment *a,COMBOBOXINFO *info)
{
    DWORD style=(DWORD)GetWindowLongPtrW(w,GWL_STYLE),ex=(DWORD)GetWindowLongPtrW(w,GWL_EXSTYLE);
    WNDCLASSEXW native={.cbSize=sizeof(native)};RECT client,overlap;
    if(a->detached || !IsWindow(w) || !a->color_witness_owner ||
       (HMODULE)GetClassLongPtrW(w,GCLP_HMODULE)!=a->color_witness_owner ||
       (style&3)!=CBS_DROPDOWNLIST || !(style&CBS_OWNERDRAWFIXED) || !(style&CBS_HASSTRINGS) ||
       (style&CBS_OWNERDRAWVARIABLE) || (ex&(WS_EX_LAYOUTRTL|WS_EX_RTLREADING)) ||
       !GetParent(w) || (a->color_parent && GetParent(w)!=a->color_parent) ||
       GetWindowThreadProcessId(GetParent(w),NULL)!=GetCurrentThreadId() ||
       !GetClassInfoExW(NULL,L"COMBOBOX",&native) ||
       GetClassLongPtrW(w,GCL_CBWNDEXTRA)<(ULONG_PTR)native.cbWndExtra ||
       !GetComboBoxInfo(w,info) || a->detached || info->hwndCombo!=w || info->hwndItem!=w ||
       !info->hwndList || GetWindowThreadProcessId(info->hwndList,NULL)!=GetCurrentThreadId() ||
       !GetClientRect(w,&client) || info->rcButton.left<client.left || info->rcButton.top<client.top ||
       info->rcButton.right>client.right || info->rcButton.bottom>client.bottom || IsRectEmpty(&info->rcButton) ||
       IntersectRect(&overlap,&info->rcButton,&info->rcItem))return FALSE;
    LRESULT count=SendMessageW(w,CB_GETCOUNT,0,0);if(a->detached || count<0)return FALSE;
    LRESULT selected=SendMessageW(w,CB_GETCURSEL,0,0);
    return !a->detached && IsWindow(w) && (selected==CB_ERR || (selected>=0 && selected<count));
}
static void native_button(HWND w,attachment *a,HDC dc,const RECT *dirty,RECT r,BOOL pressed)
{
    RECT q;POINT origin;win16_style_palette palette;
    if(!dc || GetMapMode(dc)!=MM_TEXT || (GetLayout(dc)&LAYOUT_RTL) ||
       !GetWindowOrgEx(dc,&origin) || origin.x || origin.y ||
       !GetViewportOrgEx(dc,&origin) || origin.x || origin.y)return;
    int dpi=(int)GetDpiForWindow(w);if(dpi<=0 || dpi>768)return;
    int u=max(1,MulDiv(1,dpi,96)),gw=MulDiv(7,dpi,96),gh=MulDiv(9,dpi,96);
    if(r.right-r.left<gw+2*u || r.bottom-r.top<gh+2*u || a->detached || !IsWindow(w))return;
    int saved=SaveDC(dc);if(!saved)return;
    if(IntersectClipRect(dc,r.left,r.top,r.right,r.bottom)==ERROR ||
       (dirty && IntersectClipRect(dc,dirty->left,dirty->top,dirty->right,dirty->bottom)==ERROR)){
        RestoreDC(dc,saved);return;
    }
    win16_style_default_palette(&palette);HBRUSH brush=(HBRUSH)GetStockObject(DC_BRUSH);
    SetDCBrushColor(dc,palette.face);FillRect(dc,&r,brush);SetDCBrushColor(dc,palette.edge);
    for(int i=0;i<u;++i){FrameRect(dc,&r,brush);InflateRect(&r,-1,-1);}
    q=r;SetDCBrushColor(dc,pressed?palette.shadow:palette.light);q.bottom=q.top+u;FillRect(dc,&q,brush);
    q=r;q.right=q.left+u;FillRect(dc,&q,brush);
    q=r;SetDCBrushColor(dc,pressed?palette.light:palette.shadow);q.top=q.bottom-u;FillRect(dc,&q,brush);
    q=r;q.left=q.right-u;FillRect(dc,&q,brush);
    int left=(r.left+r.right-gw)/2+(pressed?u:0),top=(r.top+r.bottom-gh)/2+(pressed?u:0);
    SetDCBrushColor(dc,IsWindowEnabled(w)?palette.edge:palette.shadow);
    for(int y=0;y<9;++y)for(int x=0;x<7;++x)if(combo_arrow[y][x]=='0'){
        SetRect(&q,left+MulDiv(x,dpi,96),top+MulDiv(y,dpi,96),
            left+MulDiv(x+1,dpi,96),top+MulDiv(y+1,dpi,96));FillRect(dc,&q,brush);
    }
    RestoreDC(dc,saved);
}
static void color_button(HWND w,attachment *a,HDC dc,const RECT *dirty)
{
    COMBOBOXINFO info={.cbSize=sizeof(info)};
    if(!color_identity(w,a,&info))return;
    BOOL pressed=SendMessageW(w,CB_GETDROPPEDSTATE,0,0)!=0;
    if(!a->detached && IsWindow(w) && GetParent(w)==a->color_parent)
        native_button(w,a,dc,dirty,info.rcButton,pressed);
}
static BOOL date_identity(HWND w,attachment *a,DATETIMEPICKERINFO *info)
{
    DWORD style=(DWORD)GetWindowLongPtrW(w,GWL_STYLE),ex=(DWORD)GetWindowLongPtrW(w,GWL_EXSTYLE);
    WNDCLASSEXW native={.cbSize=sizeof(native)};RECT client;SYSTEMTIME value={0};
    if(a->detached || !IsWindow(w) || !a->date_witness_owner ||
       (HMODULE)GetClassLongPtrW(w,GCLP_HMODULE)!=a->date_witness_owner ||
       (style&(DTS_UPDOWN|DTS_SHOWNONE|DTS_APPCANPARSE)) || (ex&(WS_EX_LAYOUTRTL|WS_EX_RTLREADING)) ||
       !GetParent(w) || (a->date_parent && GetParent(w)!=a->date_parent) ||
       GetWindowThreadProcessId(GetParent(w),NULL)!=GetCurrentThreadId() ||
       !GetClassInfoExW(NULL,DATETIMEPICK_CLASSW,&native) ||
       GetClassLongPtrW(w,GCL_CBWNDEXTRA)<(ULONG_PTR)native.cbWndExtra ||
       !SendMessageW(w,DTM_GETDATETIMEPICKERINFO,0,(LPARAM)info) || a->detached ||
       info->hwndEdit || info->hwndUD || !IsRectEmpty(&info->rcCheck) ||
       !GetClientRect(w,&client) || info->rcButton.left<client.left || info->rcButton.top<client.top ||
       info->rcButton.right>client.right || info->rcButton.bottom>client.bottom || IsRectEmpty(&info->rcButton))return FALSE;
    LRESULT status=SendMessageW(w,DTM_GETSYSTEMTIME,0,(LPARAM)&value);
    return !a->detached && IsWindow(w) && status==GDT_VALID;
}
static void date_button(HWND w,attachment *a,HDC dc,const RECT *dirty)
{
    DATETIMEPICKERINFO info={.cbSize=sizeof(info)};
    if(date_identity(w,a,&info) && !a->detached && GetParent(w)==a->date_parent)
        native_button(w,a,dc,dirty,info.rcButton,(info.stateButton&STATE_SYSTEM_PRESSED)!=0);
}
static LRESULT CALLBACK attachment_proc(HWND w,UINT m,WPARAM p,LPARAM l,UINT_PTR id,DWORD_PTR data)
{
    attachment *a=(attachment *)data;LRESULT result;DWORD status_error=GetLastError();BOOL status=a->status||a->spin,color=a->color_candidate||a->date_candidate;(void)id;
    if(a->controlbar_candidate && a->controlbar_parent && GetParent(w)!=a->controlbar_parent){a->controlbar_retired=TRUE;a->controlbar_verified=FALSE;}
    if(a->panel_frame_candidate && a->panel_frame_parent && GetParent(w)!=a->panel_frame_parent){a->panel_frame_retired=TRUE;a->panel_frame_verified=FALSE;}
    if(a->group_frame_candidate && a->group_frame_parent && GetParent(w)!=a->group_frame_parent){a->group_frame_retired=TRUE;a->group_frame_verified=FALSE;}
    RECT color_dirty;BOOL color_update=color && m==WM_PAINT && GetUpdateRect(w,&color_dirty,FALSE);
    HRGN print_clip=NULL;int print_clip_kind=-1;
    if((a->color_installed || a->date_installed) && !a->detached && m==WM_PRINTCLIENT && p){
        print_clip=CreateRectRgn(0,0,0,0);
        if(print_clip)print_clip_kind=GetClipRgn((HDC)p,print_clip);
    }
    ++a->calls;
    if(a->status && (m==WM_SETFONT || m==WM_FONTCHANGE || m==WM_SETTINGCHANGE || m==WM_THEMECHANGED || m==WM_DPICHANGED))a->status_font_invalid=TRUE;
    if(a->status && (m==WM_SETTEXT || m==SB_SETPARTS || m==SB_SETTEXTA || m==SB_SETTEXTW || m==SB_SIMPLE ||
        m==SB_SETICON || m==SB_SETMINHEIGHT || m==WM_SETFONT || m==WM_STYLECHANGED ||
        m==WM_SIZE || m==WM_ENABLE || m==WM_SETTINGCHANGE || m==WM_DESTROY || m==WM_NCDESTROY))++a->status_epoch;
    status_draw_scope scope={0},*prior=status_current;
    spin_draw_scope spin_scope={0},*spin_prior=spin_current;
    BOOL spin_message=a->spin && !a->detached && (m==WM_PAINT || m==WM_PRINT || m==WM_PRINTCLIENT ||
        m==WM_LBUTTONDOWN || m==WM_LBUTTONUP || m==WM_MOUSEMOVE || m==WM_ENABLE || m==WM_CANCELMODE ||
        m==WM_SETFOCUS || m==WM_KILLFOCUS);
    if(spin_prior)spin_current=NULL;
    if(spin_message){spin_scope.owner=a;spin_scope.window=w;spin_scope.parent=GetParent(w);
        spin_scope.dc=(m==WM_PRINT || m==WM_PRINTCLIENT)?(HDC)p:NULL;spin_current=&spin_scope;
        spin_scope.previous_stack=spin_stack;spin_stack=&spin_scope;}
    BOOL painting=a->status && !a->detached && (m==WM_PAINT || m==WM_PRINT || m==WM_PRINTCLIENT);
    if(prior && prior->window!=w)status_current=NULL;
    if(painting){
        scope.previous=status_current;scope.owner=a;scope.window=w;scope.dc=m==WM_PAINT?NULL:(HDC)p;scope.epoch=a->status_epoch;
        scope.simple=status_identity(w,a,&scope.native) && SendMessageW(w,SB_ISSIMPLE,0,0) && !a->detached && scope.epoch==a->status_epoch;
        if(scope.simple){
            LRESULT length=SendMessageW(w,SB_GETTEXTLENGTHW,0,0);
            if(a->detached || scope.epoch!=a->status_epoch || HIWORD(length)!=0 || LOWORD(length)==0 || LOWORD(length)>512)scope.simple=FALSE;
            else {
                int copied=GetWindowTextW(w,scope.simple_text,(int)LOWORD(length)+1);
                if(a->detached || scope.epoch!=a->status_epoch || copied!=(int)LOWORD(length) || !status_identity(w,a,NULL))scope.simple=FALSE;
                else scope.simple_length=copied;
            }
        }
        if(!scope.simple && status_identity(w,a,&scope.native) && !SendMessageW(w,SB_ISSIMPLE,0,0) && !a->detached && scope.epoch==a->status_epoch){
            LRESULT count=SendMessageW(w,SB_GETPARTS,0,0);
            if(count>0 && count<=64 && !a->detached && scope.epoch==a->status_epoch){
                for(unsigned i=0;i<(unsigned)count;++i){RECT r={0};LRESULT type=SendMessageW(w,SB_GETTEXTLENGTHW,i,0);
                    if(a->detached || scope.epoch!=a->status_epoch)break;
                    if((HIWORD(type)&~(SBT_OWNERDRAW|SBT_NOBORDERS)) || !SendMessageW(w,SB_GETRECT,i,(LPARAM)&r))continue;
                    if(a->detached || scope.epoch!=a->status_epoch)break;
                    if(!(HIWORD(type)&SBT_OWNERDRAW))scope.text_panes[scope.text_count++]=r;
                    if(!(HIWORD(type)&SBT_NOBORDERS))scope.panes[scope.count++]=r;
                }
            }
        }
        status_current=&scope;
    }
    if(a->picture && !a->detached && m!=WM_DESTROY && m!=WM_NCDESTROY && !ntw16s_vb_picture(w))
        (void)Ntw16sWindowRemove(w);
    if(a->button_owner && !a->detached && m!=WM_DESTROY && m!=WM_NCDESTROY && !ntw16s_vb_graphical_button(w))
        (void)Ntw16sWindowRemove(w);
    if(a->file_owner && !a->detached && m!=WM_DESTROY && m!=WM_NCDESTROY &&
        ntw16s_vb_checklist_class(w) && !ntw16s_vb_checklist_owner(w))
        (void)Ntw16sWindowRemove(w);
    if((a->file_owner || a->button_owner || a->checklist) && !a->detached && a->report_parent && m!=WM_DESTROY && m!=WM_NCDESTROY && GetParent(w)!=a->report_parent)
        (void)Ntw16sWindowRemove(w);
    if(a->inplace_candidate && !a->detached && m!=WM_DESTROY && m!=WM_NCDESTROY && !inplace_or_lookup(w,a->inplace_owner,a->inplace_parent))(void)Ntw16sWindowRemove(w);
    if(a->scroll_candidate && !a->detached && GetParent(w)!=a->scroll_parent)(void)Ntw16sWindowRemove(w);
    if(m==WM_NCDESTROY)(void)Ntw16sWindowTerminal(w);
    ntw16s_vb_paint paint;ntw16s_vb_paint_begin(&paint,w,m,&a->fonts);
    if(a->file_owner || a->picture || a->button_owner || a->vb_user_candidate)paint.cancelled=&a->detached;
    if(paint.native_active)paint.native_dc=(HDC)p;
    if(paint.active && !a->detached && !a->protocol)report_try_bind(w,a);
    if(paint.active && a->report_module && !a->file_owner && !a->detached){
        DWORD style=(DWORD)GetWindowLongPtrW(w,GWL_STYLE);
        if((a->tree || ((style&LVS_TYPEMASK)==LVS_REPORT && !(style&(LVS_OWNERDRAWFIXED|LVS_OWNERDATA)) &&
            !SendMessageW(w,LVM_ISGROUPVIEWENABLED,0,0))) && !a->detached)
            ntw16s_report_paint_enable(&paint,w,a->report_module,&a->detached);
    }
    BOOL box_witnessed=FALSE;
    if(m==WM_NCPAINT && a->controlbar_candidate)a->controlbar_verified=FALSE;
    if(m==WM_PAINT && a->rich_candidate && (!a->rich_installed || a->rich_month))a->rich_witness=FALSE;
    if(status || color)SetLastError(status_error);
    if(m==WM_PRINTCLIENT && status_current==&scope)result=status_print_dispatch(a,w,m,p,l,&paint,prior,spin_prior);
    else if(print_clip)result=color_print_dispatch(a,w,m,p,l,print_clip,&paint,prior,spin_prior);
    else if(a->modern_spin && spin_message)result=modern_spin_dispatch(a,w,m,p,l,&spin_scope,spin_prior,prior,&paint);
    else if(((a->box_candidate && !a->box_installed) || a->controlbar_candidate) && !a->detached && m==WM_NCPAINT)result=box_dispatch(a,w,m,p,l,&paint,prior,spin_prior,&box_witnessed);
    else if(static_paint || ((a->static_candidate || a->scroll_candidate || a->edit_candidate || a->group_button_candidate || a->grid_candidate || a->inplace_candidate || a->vcl_toolbar_candidate || a->track_candidate || a->tab_candidate || a->color_candidate || a->date_candidate || a->status_candidate || a->rich_candidate || a->group_frame_candidate || a->panel_frame_candidate) && !a->detached && m==WM_PAINT))
        result=static_paint_dispatch(a,w,m,p,l,&paint,prior,spin_prior);
    else result=DefSubclassProc(w,m,p,l);
    if(status || color)status_error=GetLastError();
    ntw16s_vb_paint_end(&paint);
    status_current=prior;
    if(spin_message){spin_stack=spin_scope.previous_stack;spin_buffers_clear(&spin_scope);}
    spin_current=spin_prior;
    if(box_witnessed && !a->box_installed && !a->detached && box_identity(w,a->box_owner,a->box_parent)){
        DWORD saved=GetLastError();a->box_installed=TRUE;
        if(win16_style_grid_install_created(w,NULL,a,&a->detached) && !a->detached && box_identity(w,a->box_owner,a->box_parent))RedrawWindow(w,NULL,NULL,RDW_FRAME|RDW_INVALIDATE);
        else if(!a->detached){a->box_installed=FALSE;win16_style_control_remove_owned(w,a);}
        SetLastError(saved);
    }
    if(m==WM_PAINT && a->vcl_toolbar_candidate && !a->vcl_toolbar_attempted && !a->detached && a->vcl_toolbar_witness){
        DWORD saved=GetLastError();a->vcl_toolbar_attempted=TRUE;
        ntw16s_control_protocol *protocol=vcl_toolbar_identity(w,a->vcl_toolbar_owner,a->vcl_toolbar_parent)?ntw16s_control_protocol_bind_verified_toolbar(w,a->vcl_toolbar_witness):NULL;
        if(protocol && !a->detached){
            a->protocol=protocol;BOOL installed=win16_style_toolbar_install_created(w,NULL,a,&a->detached);
            if(installed && !a->detached){a->vcl_toolbar_installed=TRUE;InvalidateRect(w,NULL,TRUE);}
            else if(!a->detached){win16_style_control_remove_owned(w,a);ntw16s_control_protocol_release(a->protocol);a->protocol=NULL;}
        }else if(protocol)ntw16s_control_protocol_release(protocol);
        SetLastError(saved);
    }
    /* Runtime controls may initially have a temporary parent. Bind the
     * verified border to the current parent only at the first real paint. */
    if(m==WM_PAINT && a->vb_data_candidate && !a->vb_data_attempted && !a->detached){
        DWORD saved=GetLastError();HWND parent=GetParent(w);
        if(vb_data_role(w,parent)){
            a->vb_data_attempted=TRUE;a->vb_data_parent=parent;a->vb_data_edge=TRUE;
            BOOL installed=win16_style_grid_install_created(w,NULL,a,&a->detached);
            if(installed && !a->detached && vb_data_role(w,parent))RedrawWindow(w,NULL,NULL,RDW_INVALIDATE|RDW_FRAME);
            else if(!a->detached){a->vb_data_edge=FALSE;win16_style_control_remove_owned(w,a);}
        }
        SetLastError(saved);
    }
    if(m==WM_PAINT && a->vb_user_candidate && !a->vb_user_attempted && !a->detached){
        DWORD saved=GetLastError();HWND parent=GetParent(w);
        if(vb_user_edge_identity(w,parent)){
            a->vb_user_attempted=TRUE;
            a->vb_user_parent=parent;a->vb_user_edge=TRUE;
            BOOL installed=win16_style_grid_install_created(w,NULL,a,&a->detached);
            if(installed && !a->detached)RedrawWindow(w,NULL,NULL,RDW_INVALIDATE|RDW_FRAME);
            else if(!a->detached){a->vb_user_edge=FALSE;win16_style_control_remove_owned(w,a);}
        }
        SetLastError(saved);
    }
    if(m==WM_PAINT && a->rich_candidate && !a->rich_installed && a->rich_witness && rich_identity(w,a)){
        DWORD saved=GetLastError();a->rich_installed=TRUE;
        if((a->rich_month || win16_style_grid_install_created(w,NULL,a,&a->detached)) && !a->detached && rich_identity(w,a)){if(!a->rich_month)RedrawWindow(w,NULL,NULL,RDW_FRAME|RDW_INVALIDATE);}
        else if(!a->detached){a->rich_installed=FALSE;win16_style_control_remove_owned(w,a);}
        SetLastError(saved);
    }
    if(m==WM_PAINT && a->grid_candidate && !a->grid_attempted && !a->detached){
        DWORD saved=GetLastError();SCROLLINFO info={.cbSize=sizeof(info),.fMask=SIF_ALL};DWORD style=(DWORD)GetWindowLongPtrW(w,GWL_STYLE);
        a->grid_attempted=TRUE;
        if(grid_identity(w,a->grid_owner,a->grid_parent) &&
            ((class_is(w,L"TValueListEditor") && !(style&(WS_HSCROLL|WS_VSCROLL))) ||
             (ntw16s_native_get_scroll_info(w,(style&WS_HSCROLL)?SB_HORZ:SB_VERT,&info) &&
              info.nMin<=info.nMax && info.nPos>=info.nMin && info.nPos<=info.nMax)) && !a->detached){
            BOOL installed=win16_style_grid_install_created(w,NULL,a,&a->detached);
            if(installed && !a->detached){a->grid_installed=TRUE;RedrawWindow(w,NULL,NULL,RDW_INVALIDATE|RDW_FRAME);}
            else if(!a->detached)win16_style_control_remove_owned(w,a);
        }
        SetLastError(saved);
    }
    if(m==WM_PAINT && a->group_button_candidate && !a->group_button_attempted && !a->detached && a->group_button_witness_owner){
        DWORD saved=GetLastError();a->group_button_attempted=TRUE;
        ntw16s_control_protocol *protocol=group_button_identity(w)?ntw16s_control_protocol_bind_verified_group_button(w,a->group_button_witness_owner):NULL;
        if(protocol && !a->detached){
            a->protocol=protocol;
            BOOL installed=win16_style_control_install_created(w,FALSE,FALSE,FALSE,NULL,a,&a->detached);
            if(installed && !a->detached){a->group_button_parent=GetParent(w);a->group_button_installed=TRUE;InvalidateRect(w,NULL,TRUE);}
            else if(!a->detached){win16_style_control_remove_owned(w,a);ntw16s_control_protocol_release(a->protocol);a->protocol=NULL;}
        }else if(protocol)ntw16s_control_protocol_release(protocol);
        SetLastError(saved);
    }
    if(m==WM_PAINT && a->scroll_candidate && !a->scroll_attempted && !a->detached && a->scroll_witness){
        DWORD saved=GetLastError();a->scroll_attempted=TRUE;
        ntw16s_control_protocol *protocol=GetParent(w)==a->scroll_parent?ntw16s_control_protocol_bind_verified_scroll(w,a->scroll_witness):NULL;
        if(protocol && !a->detached && GetParent(w)==a->scroll_parent){a->protocol=protocol;
            BOOL installed=win16_style_control_install_created(w,FALSE,FALSE,FALSE,NULL,a,&a->detached);
            if(installed && !a->detached && GetParent(w)==a->scroll_parent){a->scroll_installed=TRUE;InvalidateRect(w,NULL,TRUE);}
            else if(!a->detached){win16_style_control_remove_owned(w,a);ntw16s_control_protocol_release(a->protocol);a->protocol=NULL;}
        }else if(protocol)ntw16s_control_protocol_release(protocol);
        SetLastError(saved);
    }
    if(m==WM_PAINT && a->edit_candidate && !a->edit_attempted && !a->detached && a->edit_witness_owner){
        DWORD saved=GetLastError();a->edit_attempted=TRUE;
        ntw16s_control_protocol *protocol=a->combo_candidate && !combo_same_role(w,a)?NULL:a->link_candidate?ntw16s_control_protocol_bind_verified_link(w,a->edit_witness_owner):a->combo_candidate?ntw16s_control_protocol_bind_verified_combo(w,a->edit_witness_owner):ntw16s_control_protocol_bind_verified_edit(w,a->edit_witness_owner);
        if(protocol && !a->detached && (!a->combo_candidate || combo_same_role(w,a))){
            a->protocol=protocol;
            BOOL installed=win16_style_control_install_created(w,!a->link_candidate && !a->combo_candidate,(a->edit_original_style&WS_BORDER)!=0,FALSE,NULL,a,&a->detached);
            if(installed && !a->detached && !a->link_candidate && !a->combo_candidate && !a->spinedit_candidate && (a->edit_original_style&ES_MULTILINE)){
                a->themed=w;
                installed=ntw16s_theme_enable(w,a) && win16_style_edit_scrollbars_enable(w);
            }
            if(installed && !a->detached && (!a->combo_candidate || combo_same_role(w,a))){a->edit_installed=TRUE;InvalidateRect(w,NULL,TRUE);}
            else if(!a->detached){win16_style_control_remove_owned(w,a);if(a->themed){ntw16s_theme_remove(w,a);a->themed=NULL;}
                ntw16s_control_protocol_release(a->protocol);a->protocol=NULL;}
        }else if(protocol)ntw16s_control_protocol_release(protocol);
        /* An empty link has no ideal height. Retry on a later paint only
         * after a fresh same-owner native SysLink forwarding witness. */
        if(a->link_candidate && !a->detached && !a->edit_installed){
            a->edit_attempted=FALSE;a->edit_witness_owner=NULL;
        }
        SetLastError(saved);
    }
    if(m==WM_PAINT && a->tab_candidate && !a->tab_attempted && !a->detached && a->tab_witness_owner){
        DWORD saved=GetLastError();a->tab_attempted=TRUE;
        ntw16s_control_protocol *protocol=GetParent(w)==a->tab_parent?ntw16s_control_protocol_bind_verified_tab(w,a->tab_witness_owner):NULL;
        if(protocol && !a->detached){a->protocol=protocol;
            BOOL installed=win16_style_control_install_created(w,FALSE,FALSE,FALSE,NULL,a,&a->detached);
            if(installed && !a->detached){a->tab_installed=TRUE;InvalidateRect(w,NULL,TRUE);}
            else if(!a->detached){win16_style_control_remove_owned(w,a);ntw16s_control_protocol_release(a->protocol);a->protocol=NULL;}
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
    if(m==WM_PAINT && a->color_candidate && !a->color_attempted && !a->detached && a->color_witness_owner){
        DWORD saved=GetLastError();COMBOBOXINFO info={.cbSize=sizeof(info)};a->color_attempted=TRUE;
        if(color_identity(w,a,&info)){a->color_parent=GetParent(w);a->color_installed=TRUE;}
        SetLastError(saved);
    }
    if(m==WM_PAINT && a->status_candidate && !a->status_attempted && !a->detached && a->status_witness_owner){
        DWORD saved=GetLastError();a->status_attempted=TRUE;
        if(status_identity(w,a,NULL)){a->status_parent=GetParent(w);a->status_installed=TRUE;a->status=TRUE;InvalidateRect(w,NULL,TRUE);}
        SetLastError(saved);
    }
    if(m==WM_PAINT && a->date_candidate && !a->date_attempted && !a->detached && a->date_witness_owner){
        DWORD saved=GetLastError();DATETIMEPICKERINFO info={.cbSize=sizeof(info)};a->date_attempted=TRUE;
        if(date_identity(w,a,&info)){a->date_parent=GetParent(w);a->date_installed=TRUE;}
        SetLastError(saved);
    }
    if((a->color_installed || a->date_installed) && !a->detached && ((m==WM_PRINTCLIENT && print_clip_kind>=0) || (m==WM_PAINT && color_update))){
        DWORD saved=GetLastError();HDC dc=m==WM_PRINTCLIENT?(HDC)p:GetDC(w);
        if(dc){
            if(m==WM_PRINTCLIENT){int state=SaveDC(dc);if(state){
                if(SelectClipRgn(dc,print_clip_kind?print_clip:NULL)!=ERROR){
                    if(a->color_installed)color_button(w,a,dc,NULL);else date_button(w,a,dc,NULL);
                }
                RestoreDC(dc,state);
            }}else {if(a->color_installed)color_button(w,a,dc,&color_dirty);else date_button(w,a,dc,&color_dirty);ReleaseDC(w,dc);}
        }
        SetLastError(saved);
    }
    if(print_clip){DWORD saved=GetLastError();DeleteObject(print_clip);SetLastError(saved);}
    release(a);if(status || color)SetLastError(status_error);return result;
}
static void bit_draw_end(void *data)
{
    bit_draw_scope *scope=data;bit_current=scope->previous;
    if(!scope->completed)release(scope->owner);
}
static NTW16S_UNWIND_FUNCTION LRESULT bit_draw_dispatch(attachment *a,HWND w,UINT m,WPARAM p,LPARAM l,const DRAWITEMSTRUCT *item)
{
    bit_draw_scope scope={bit_current,a,item->hwndItem,item->hDC,item->rcItem,item->itemState,FALSE};ntw16s_unwind_scope unwind;
    DWORD entry=GetLastError(),error;LRESULT result;bit_current=&scope;
    NTW16S_UNWIND_BEGIN(unwind,bit_draw_end,&scope);
    SetLastError(entry);result=DefSubclassProc(w,m,p,l);error=GetLastError();scope.completed=TRUE;
    NTW16S_UNWIND_END(unwind);SetLastError(error);return result;
}
/* Observe this verified child's native custom/owner-draw DC. Never request extra stages,
 * rewrite the application's result, select its font, or inspect item lParam. */
static LRESULT CALLBACK report_notify(HWND w,UINT m,WPARAM p,LPARAM l,UINT_PTR id,DWORD_PTR data)
{
    DWORD entry=GetLastError();attachment *a=(attachment *)data;BOOL draw=FALSE;HWND list=a->report_window;HDC dc=NULL;DWORD stage=0;
    ntw16s_vb_paint file_paint;BOOL file_active=FALSE;
    checklist_draw_scope checklist_scope;BOOL checklist_active=FALSE;
    ++a->calls;
    if(a->bit_candidate && !a->detached && m==WM_DRAWITEM && l && GetParent(list)==w && bit_identity(list,a->report_module)){
        const DRAWITEMSTRUCT *item=(const DRAWITEMSTRUCT *)l;
        if(item->CtlType==ODT_BUTTON && item->itemAction==ODA_DRAWENTIRE && item->hwndItem==list && item->hDC){
            HWND target=WindowFromDC(item->hDC);
            if(!target || target==list){SetLastError(entry);LRESULT result=bit_draw_dispatch(a,w,m,p,l,item);DWORD error=GetLastError();release(a);SetLastError(error);return result;}
        }
    }
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
    if(!a->file_owner && !a->button_owner && !a->checklist && !a->bit_candidate && m==WM_NOTIFY && l){
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
    attachment *a=lookup(w);return a && (!a->controlbar_candidate || (a->controlbar_verified && controlbar_identity(w,a))) && (!a->box_candidate || (a->box_installed && box_identity(w,a->box_owner,a->box_parent))) && (!a->modern_spin || a->modern_spin_verified) && (!a->vb_data_candidate || a->vb_data_edge) && (!a->vb_user_candidate || ntw16s_vb_user_label_target(w)) && (!a->static_candidate || a->static_verified) && (!a->scroll_candidate || (a->scroll_installed && GetParent(w)==a->scroll_parent)) && (!a->edit_candidate || a->edit_installed) && (!a->group_button_candidate || a->group_button_installed) && (!a->grid_candidate || a->grid_installed) && (!a->vcl_toolbar_candidate || a->vcl_toolbar_installed) && (!a->track_candidate || a->track_installed) && (!a->tab_candidate || a->tab_installed) && (!a->color_candidate || a->color_installed) && (!a->date_candidate || a->date_installed) && (!a->status_candidate || a->status_installed) && (!a->rich_candidate || (a->rich_installed && rich_identity(w,a))) && (!a->bit_candidate || a->bit_verified) && (!a->inplace_candidate || (a->inplace_verified && inplace_or_lookup(w,a->inplace_owner,a->inplace_parent))) && (!a->group_frame_candidate || (a->group_frame_verified && !a->group_frame_retired && GetParent(w)==a->group_frame_parent)) && (!a->panel_frame_candidate || (a->panel_frame_verified && !a->panel_frame_retired && GetParent(w)==a->panel_frame_parent));
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
    BOOL control,edit,installed,report=FALSE,tree=FALSE,legacy_progress=FALSE,picture=ntw16s_vb_picture(w),checklist=checklist_identity(w),status=ready && status_identity(w,NULL,NULL),spin=ready && (spin_identity(w)||modern_spin_shape(w)),status_candidate=FALSE,bit_candidate=FALSE,inplace_candidate=FALSE,group_frame_candidate=FALSE,panel_frame_candidate=FALSE,static_candidate=FALSE,scroll_candidate=FALSE,edit_candidate=FALSE,group_button_candidate=FALSE,grid_candidate=FALSE,vcl_toolbar_candidate=FALSE,track_candidate=FALSE,tab_candidate=FALSE,color_candidate=FALSE,date_candidate=FALSE;HWND parent;HMODULE owner;attachment *a;DWORD error;
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
        scroll_candidate=ready && GetClassNameW(w,name,ARRAYSIZE(name)) && !lstrcmpW(name,L"TScrollBar") &&
            RealGetWindowClassW(w,real,ARRAYSIZE(real)) && !lstrcmpW(real,L"TScrollBar");
        edit_candidate=ready && GetClassNameW(w,name,ARRAYSIZE(name)) && (!lstrcmpW(name,L"TEdit") || !lstrcmpW(name,L"TMemo") || !lstrcmpW(name,L"TMaskEdit") || !lstrcmpW(name,L"TDBEdit") || !lstrcmpW(name,L"TLabeledEdit") || !lstrcmpW(name,L"TNumberBox") || !lstrcmpW(name,L"TLinkLabel") || !lstrcmpW(name,L"TSpinEdit") || !lstrcmpW(name,L"TComboBox") || !lstrcmpW(name,L"TDBComboBox")) &&
            RealGetWindowClassW(w,real,ARRAYSIZE(real)) && !lstrcmpW(name,real) && ((lstrcmpW(name,L"TComboBox") && lstrcmpW(name,L"TDBComboBox")) || combo_identity(w,GetParent(w)));
        tab_candidate=ready && GetClassNameW(w,name,ARRAYSIZE(name)) && !lstrcmpW(name,L"TTabControl") &&
            RealGetWindowClassW(w,real,ARRAYSIZE(real)) && !lstrcmpW(real,L"TTabControl") &&
            !(GetWindowLongPtrW(w,GWL_EXSTYLE)&(WS_EX_LAYOUTRTL|WS_EX_RTLREADING));
        track_candidate=ready && GetClassNameW(w,name,ARRAYSIZE(name)) && !lstrcmpW(name,L"TTrackBar") &&
            RealGetWindowClassW(w,real,ARRAYSIZE(real)) && !lstrcmpW(real,L"TTrackBar");
        color_candidate=ready && GetClassNameW(w,name,ARRAYSIZE(name)) && !lstrcmpW(name,L"TColorBox") &&
            RealGetWindowClassW(w,real,ARRAYSIZE(real)) && !lstrcmpW(real,L"TColorBox");
        bit_candidate=ready && bit_identity(w,owner);
        inplace_candidate=ready && inplace_or_lookup(w,owner,GetParent(w));
        group_frame_candidate=ready && GetClassNameW(w,name,ARRAYSIZE(name)) && (!lstrcmpW(name,L"TGroupBox") || !lstrcmpW(name,L"TRadioGroup")) && RealGetWindowClassW(w,real,ARRAYSIZE(real)) && !lstrcmpW(real,name);
        panel_frame_candidate=ready && GetClassNameW(w,name,ARRAYSIZE(name)) && !lstrcmpW(name,L"TPanel") && RealGetWindowClassW(w,real,ARRAYSIZE(real)) && !lstrcmpW(real,name);
        group_button_candidate=ready && group_button_identity(w);
        grid_candidate=ready && grid_identity(w,owner,GetParent(w));
        vcl_toolbar_candidate=ready && vcl_toolbar_identity(w,owner,GetParent(w));
        status_candidate=ready && GetClassNameW(w,name,ARRAYSIZE(name)) && !lstrcmpW(name,L"TStatusBar") &&
            RealGetWindowClassW(w,real,ARRAYSIZE(real)) && !lstrcmpW(real,L"TStatusBar");
        date_candidate=ready && GetClassNameW(w,name,ARRAYSIZE(name)) && !lstrcmpW(name,L"TDateTimePicker") &&
            RealGetWindowClassW(w,real,ARRAYSIZE(real)) && !lstrcmpW(real,L"TDateTimePicker");
    }
    BOOL box_candidate=ready && box_identity(w,owner,GetParent(w));
    BOOL controlbar_candidate=ready && class_is(w,L"TControlBar") && owner && GetParent(w) && (HMODULE)GetClassLongPtrW(GetParent(w),GCLP_HMODULE)==owner;
    BOOL rich_month=class_is(w,L"TMonthCalendar");
    BOOL rich_hotkey=class_is(w,L"THotKey");
    BOOL rich_popup=class_is(w,L"TPopupListbox");
    BOOL rich_unicode=rich_popup || rich_month || rich_hotkey || class_is(w,L"TRichEdit");
    BOOL rich_candidate=ready && (rich_unicode || class_is(w,L"TRichEditURL")) && owner && GetParent(w) &&
        (rich_popup?(owner==GetModuleHandleW(NULL) && inplace_identity(GetParent(w),owner,GetParent(GetParent(w))) && (original_style&WS_BORDER) && (original_style&WS_VSCROLL) && !(original_style&(LBS_OWNERDRAWFIXED|LBS_OWNERDRAWVARIABLE|LBS_NODATA))):((HMODULE)GetClassLongPtrW(GetParent(w),GCLP_HMODULE)==owner && (rich_month || (GetWindowLongPtrW(w,GWL_EXSTYLE)&WS_EX_CLIENTEDGE))));
    BOOL vb_user_edge=vb_user_edge_role(w,GetParent(w));
    BOOL vb_data_edge=vb_data_role(w,GetParent(w));
    /* An external OCX can create itself with frame bits during construction
     * and then become a custom client surface.  Without an explicitly
     * verified public protocol it must stay native; only an application-owned
     * generic child frame may use the ordinary frame path. */
    BOOL vb_mdi_child=(original_style&WS_CAPTION)==WS_CAPTION &&
        (GetWindowLongPtrW(w,GWL_EXSTYLE)&WS_EX_MDICHILD) &&
        (ntw16s_vb5_class(w,L"ThunderRT5Form") || ntw16s_vb_class(w,L"ThunderRT6FormDC")) &&
        class_is(GetParent(w),L"MDIClient") &&
        (HMODULE)GetClassLongPtrW(GetParent(w),GCLP_HMODULE)==GetModuleHandleW(L"user32.dll");
    if(!control && !report && !tree && !picture && !checklist && !legacy_progress && !status && !spin && !static_candidate && !scroll_candidate && !edit_candidate && !group_button_candidate && !grid_candidate && !inplace_candidate && !vcl_toolbar_candidate && !track_candidate && !tab_candidate && !color_candidate && !date_candidate && !status_candidate && !bit_candidate && !group_frame_candidate && !panel_frame_candidate && (original_style&WS_CHILD) &&
            owner && owner!=GetModuleHandleW(NULL) && !vb_mdi_child && !vb_user_edge && !vb_data_edge && !rich_candidate && !box_candidate && !controlbar_candidate){
        ntw16s_control_protocol_release(protocol);SetLastError(ERROR_NOT_SUPPORTED);return FALSE;
    }
    /* A custom client-only child and MDICLIENT keep application painting.
     * MDI child frames have WS_CAPTION and take the ordinary frame route. */
    if(!control && !report && !picture && !checklist && !legacy_progress && !status && !spin && !static_candidate && !scroll_candidate && !edit_candidate && !group_button_candidate && !grid_candidate && !inplace_candidate && !vcl_toolbar_candidate && !track_candidate && !tab_candidate && !color_candidate && !date_candidate && !status_candidate && !bit_candidate && !group_frame_candidate && !panel_frame_candidate && (original_style&WS_CHILD) && !(original_style&(WS_CAPTION|WS_THICKFRAME)) && !vb_user_edge && !vb_data_edge && !rich_candidate && !box_candidate && !controlbar_candidate){
        SetLastError(ERROR_NOT_SUPPORTED);return FALSE;
    }
    a=calloc(1,sizeof(*a));
    if(!a){ntw16s_control_protocol_release(protocol);SetLastError(ERROR_NOT_ENOUGH_MEMORY);return FALSE;}
    a->calls=1;a->controlbar_candidate=controlbar_candidate;a->controlbar_parent=GetParent(w);a->controlbar_owner=owner;a->rich_popup=rich_popup;a->rich_month=rich_month;a->rich_hotkey=rich_hotkey;a->box_candidate=box_candidate;a->box_owner=owner;a->box_parent=GetParent(w);a->rich_unicode=rich_unicode;a->rich_candidate=rich_candidate;a->rich_parent=rich_popup?GetDesktopWindow():GetParent(w);a->rich_owner=owner;a->group_frame_candidate=group_frame_candidate;a->panel_frame_candidate=panel_frame_candidate;a->vb_user_candidate=vb_user_edge;a->vb_data_candidate=vb_data_edge;a->protocol=protocol;a->file_owner=ntw16s_vb_owner_file(w);a->picture=picture;a->button_owner=ntw16s_vb_graphical_button(w);a->checklist=checklist;a->status=status;a->spin=spin;a->modern_spin=spin&&modern_spin_shape(w);a->static_candidate=static_candidate;
    a->scroll_candidate=scroll_candidate;a->scroll_parent=GetParent(w);a->edit_candidate=edit_candidate;a->link_candidate=edit_candidate && class_is(w,L"TLinkLabel");a->combo_candidate=edit_candidate && (class_is(w,L"TComboBox") || class_is(w,L"TDBComboBox"));a->combo_parent=GetParent(w);a->combo_original_type=original_style&3;a->spinedit_candidate=edit_candidate && class_is(w,L"TSpinEdit");a->group_button_candidate=group_button_candidate;a->grid_candidate=grid_candidate;a->inplace_candidate=inplace_candidate;a->inplace_owner=owner;a->inplace_parent=GetParent(w);a->vcl_toolbar_candidate=vcl_toolbar_candidate;a->vcl_toolbar_owner=owner;a->vcl_toolbar_parent=GetParent(w);a->grid_owner=owner;a->grid_parent=GetParent(w);a->edit_original_style=original_style;a->track_candidate=track_candidate;a->tab_candidate=tab_candidate;a->tab_parent=GetParent(w);a->color_candidate=color_candidate;a->date_candidate=date_candidate;a->status_candidate=status_candidate;a->bit_candidate=bit_candidate;
    if(report || tree || a->file_owner || a->button_owner || a->checklist || a->bit_candidate){a->report_window=w;a->tree=tree;
        a->report_module=(a->file_owner || a->button_owner || a->bit_candidate)?(HMODULE)GetClassLongPtrW(w,GCLP_HMODULE):tree?ntw16s_control_protocol_tree_module(w):ntw16s_control_protocol_list_module(w);a->report_parent=GetParent(w);}
    if(!SetWindowSubclass(w,attachment_proc,(UINT_PTR)attachment_proc,(DWORD_PTR)a)){
        ntw16s_control_protocol_release(protocol);free(a);return FALSE;
    }
    if((report || tree || a->file_owner || a->button_owner || a->checklist || a->bit_candidate) && (!a->report_parent || !SetWindowSubclass(a->report_parent,report_notify,(UINT_PTR)w,(DWORD_PTR)a))){
        DWORD error=GetLastError();Ntw16sWindowRemove(w);release(a);SetLastError(error);return FALSE;
    }
    SetLastError(ERROR_SUCCESS);
    if(protocol && class_is(w,L"COMBOBOX")){
        COMBOBOXINFO info={.cbSize=sizeof(info)};
        if(ntw16s_native_get_combo_info(w,&info) && info.hwndItem!=w && lookup(info.hwndItem))
            Ntw16sWindowRemove(info.hwndItem);
    }
    if(a->detached){release(a);SetLastError(ERROR_CANCELLED);return FALSE;}
    if(a->box_candidate || a->controlbar_candidate || a->rich_candidate || a->vb_data_candidate || a->vb_user_candidate || a->checklist || a->status || a->spin || a->static_candidate || a->scroll_candidate || a->edit_candidate || a->group_button_candidate || a->grid_candidate || a->inplace_candidate || a->vcl_toolbar_candidate || a->track_candidate || a->tab_candidate || a->color_candidate || a->date_candidate || a->status_candidate || a->bit_candidate || a->group_frame_candidate || a->panel_frame_candidate)installed=TRUE;
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
        class_is(w,L"LISTBOX") && (original_style&LBS_USETABSTOPS) && !a->file_owner,NULL,a,&a->detached);
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
    if(installed && !a->detached && !a->vb_data_candidate && !a->scroll_candidate && !a->edit_candidate && !a->group_button_candidate && !a->grid_candidate && !a->vcl_toolbar_candidate && !a->track_candidate && !a->tab_candidate && !a->color_candidate && !a->date_candidate && !a->status_candidate && !a->bit_candidate && a->themed!=w)ntw16s_theme_remove(w,NULL);
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
