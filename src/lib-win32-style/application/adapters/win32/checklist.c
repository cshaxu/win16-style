#include "checklist.h"
#include "../../control/control_class.h"
#include "../../graphics/dc_geometry.h"

BOOL style_runtime_checklist_target(HDC dc,HDC expected,HWND window,HWND parent,const RECT *item,const RECT *r,const void *address,BOOL (*identity)(HWND),int maximum_side)
{
    HMODULE module=NULL;
    if(!r || dc!=expected || !identity(window) || GetParent(window)!=parent ||
        WindowFromDC(dc)!=window || !style_dc_pixel_viewport(dc) ||
        !GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            (const WCHAR *)address,&module) || module!=(HMODULE)GetClassLongPtrW(window,GCLP_HMODULE))return FALSE;
    return maximum_side>0 && r->left>=item->left && r->top>=item->top &&
        r->right<=item->right && r->bottom<=item->bottom &&
        r->left-item->left<=maximum_side && r->right-r->left>0 && r->bottom-r->top>0 &&
        r->right-r->left<=maximum_side && r->bottom-r->top<=maximum_side;
}

BOOL style_runtime_checklist_begin(style_runtime_checklists *manager,style_runtime_checklist_scope *scope,
    const style_runtime_observation *observation,HWND parent,UINT message,LPARAM data,BOOL (*identity)(HWND))
{
    HWND window=observation->runtime->report.window;
    if(observation->instance->detached || message!=WM_DRAWITEM || !data ||
        GetParent(window)!=parent || !identity(window))return FALSE;
    const DRAWITEMSTRUCT *item=(const DRAWITEMSTRUCT *)data;
    if(item->CtlType!=ODT_LISTBOX || item->hwndItem!=window || !item->hDC ||
        item->itemID==(UINT)-1 || WindowFromDC(item->hDC)!=window)return FALSE;
    *scope=(style_runtime_checklist_scope){manager->current,*observation,item->hDC,item->rcItem};
    manager->current=scope;return TRUE;
}
void style_runtime_checklist_end(style_runtime_checklists *manager,const style_runtime_checklist_scope *scope)
{manager->current=scope->previous;}
BOOL style_runtime_checklist_draw_target(const style_runtime_checklists *manager,HDC dc,const RECT *bounds,
    const void *caller,BOOL (*identity)(HWND),int maximum_side96)
{
    const style_runtime_checklist_scope *scope=manager->current;
    if(!scope || !scope->observation.instance || scope->observation.instance->detached)return FALSE;
    HWND window=scope->observation.runtime->report.window;
    return style_runtime_checklist_target(dc,scope->dc,window,scope->observation.runtime->report.parent,
        &scope->item,bounds,caller,identity,MulDiv(maximum_side96,(int)GetDpiForWindow(window),96));
}

/* VB5 and VB6 draw the checkbox as an 11x11 runtime-owned bitmap, directly onto
 * the list's DC. Restrict this to the witnessed identity, geometry and
 * public list state; other bitmaps and unsupported layouts remain native. */
static BOOL vb_checklist_unchanged(HWND w,HDC dc,HWND parent,DWORD style,DWORD ex,BOOL enabled)
{
    return IsWindow(w) && WindowFromDC(dc)==w && GetParent(w)==parent &&
        (DWORD)GetWindowLongPtrW(w,GWL_STYLE)==style && (DWORD)GetWindowLongPtrW(w,GWL_EXSTYLE)==ex &&
        IsWindowEnabled(w)==enabled && style_control_vb_checklist_class(w);
}
BOOL style_runtime_vb_checklist(HDC dc,int x,int y,int width,int height,HDC source,int sx,int sy,DWORD rop,const void *address,style_runtime_checklist_painter paint)
{
    static _Thread_local BOOL active;
    HMODULE caller=NULL;HWND w,parent;RECT item;LRESULT hit,index;BITMAP bitmap;
    DWORD style,ex;BOOL result=FALSE,enabled;UINT dpi;
    if(active || !dc || !source || width!=11 || height!=11 || rop!=SRCCOPY ||
        (sx!=0 && sx!=11 && sx!=22 && sx!=33) || sy!=0 || GetObjectType(source)!=OBJ_MEMDC ||
        GetObjectW(GetCurrentObject(source,OBJ_BITMAP),sizeof(bitmap),&bitmap)!=sizeof(bitmap) ||
        bitmap.bmWidth!=44 || bitmap.bmHeight!=11 ||
        !(w=WindowFromDC(dc)) || GetWindowThreadProcessId(w,NULL)!=GetCurrentThreadId() ||
        !style_control_vb_checklist_class(w) ||
        ((dpi=GetDpiForWindow(w))!=96 && dpi!=144) ||
        !style_dc_pixel_viewport(dc) ||
        !GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            (const WCHAR *)address,&caller) || caller!=(HMODULE)GetClassLongPtrW(w,GCLP_HMODULE))return FALSE;
    style=(DWORD)GetWindowLongPtrW(w,GWL_STYLE);ex=(DWORD)GetWindowLongPtrW(w,GWL_EXSTYLE);
    enabled=IsWindowEnabled(w);parent=GetParent(w);
    if((enabled?sx>=22:sx<22) || !parent || (ex&(WS_EX_LAYOUTRTL|WS_EX_RTLREADING)) ||
        (style&(LBS_OWNERDRAWFIXED|LBS_OWNERDRAWVARIABLE|LBS_HASSTRINGS|LBS_MULTIPLESEL|LBS_EXTENDEDSEL|LBS_NODATA))!=
            (LBS_OWNERDRAWFIXED|LBS_HASSTRINGS|LBS_MULTIPLESEL))return FALSE;
    active=TRUE;
    hit=SendMessageW(w,LB_ITEMFROMPOINT,0,MAKELPARAM(x,y));index=LOWORD(hit);
    if(HIWORD(hit)==0 && vb_checklist_unchanged(w,dc,parent,style,ex,enabled) &&
        SendMessageW(w,LB_GETITEMRECT,(WPARAM)index,(LPARAM)&item)!=LB_ERR &&
        vb_checklist_unchanged(w,dc,parent,style,ex,enabled) &&
        x==item.left+(style_control_vb5_class(w,L"ThunderRT5ListBox")?0:2) &&
        (dpi==144?(style_control_vb5_class(w,L"ThunderRT5ListBox")?
            (item.bottom-item.top==23 && y==item.top+6):
            (item.bottom-item.top==22 && y==item.top+5)):y==item.top+2) &&
        item.right-x>=width && item.bottom-y>=height){
        RECT glyph={x,y,x+width,y+height};
        result=paint(dc,&glyph,(sx%22)==11?DFCS_CHECKED:0);
    }
    active=FALSE;return result;
}
