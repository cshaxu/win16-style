#include "lib-style/common/dc_geometry.h"
#include "lib-style/common/pixel_grid.h"
#include "lib-style/common/pixel_dib.h"
#include "choice_paint.h"
#include "control_class.h"
#include "control_glyphs.h"

BOOL win16_control_checklist_target(HDC dc,HDC expected,HWND window,HWND parent,const RECT *item,const RECT *r,const void *address,BOOL (*identity)(HWND))
{
    HMODULE module=NULL;
    if(!r || dc!=expected || !identity(window) || GetParent(window)!=parent ||
        WindowFromDC(dc)!=window || !style_dc_pixel_viewport(dc) ||
        !GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            (const WCHAR *)address,&module) || module!=(HMODULE)GetClassLongPtrW(window,GCLP_HMODULE))return FALSE;
    int dpi=(int)GetDpiForWindow(window);
    int max_side=MulDiv(16,dpi,96);
    return dpi>0 && r->left>=item->left && r->top>=item->top &&
        r->right<=item->right && r->bottom<=item->bottom &&
        r->left-item->left<=max_side && r->right-r->left>0 && r->bottom-r->top>0 &&
        r->right-r->left<=max_side && r->bottom-r->top<=max_side;
}

BOOL win16_control_checklist_glyph(HDC dc,const RECT *rect,UINT flags)
{
    int width=rect->right-rect->left,height=rect->bottom-rect->top;
    if(width<=0 || height<=0 || width>256 || height>256)return FALSE;
    int column=((flags&DFCS_CHECKED)?1:0)+((flags&DFCS_PUSHED)?2:0);
    const DWORD colors[2]={0,0x00ffffff};
    return style_paint_indexed_dib(dc,rect,&control_checks[0][column*14],57,(SIZE){14,13},colors);
}
void win16_control_choice_glyph(HDC dc,RECT glyph,UINT dpi,int row,int column,const win16_style_palette *palette)
{
    const COLORREF colors[2]={palette->edge,palette->light};
    const style_pixel_grid grid={&control_checks[row*13][column*14],14,13,57,-1,colors,{glyph.left,glyph.top},{(int)dpi,(int)dpi},{96,96}};
    style_paint_pixel_grid(dc,&grid);
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
BOOL win16_control_vb_checklist(HDC dc,int x,int y,int width,int height,HDC source,int sx,int sy,DWORD rop,const void *address)
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
        result=win16_control_checklist_glyph(dc,&glyph,(sx%22)==11?DFCS_CHECKED:0);
    }
    active=FALSE;return result;
}
