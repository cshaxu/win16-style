#include "control_class.h"
#include "attachment.h"
/* Internal guard for a witnessed VCL group button. */
BOOL ntw16s_group_button_valid(HWND);
BOOL ntw16s_grid_valid(HWND);
BOOL ntw16s_vcl_toolbar_valid(HWND);
BOOL ntw16s_vcl_tab_valid(HWND);
#include "controls.h"
#include "control_glyphs.h"
#include "combo_glyph.h"
#include "scroll_glyphs.h"
#include "scroll_track.h"
#include "font.h"
#include "native_api.h"
#include "tab_paint.h"
#include "header_paint.h"
#include "progress_paint.h"
#include "toolbar_paint.h"
#include "trackbar_paint.h"
#include "updown_paint.h"
#include "vb_text.h"
#include "injection/unwind.h"
#include <commctrl.h>
#include <oleacc.h>
#include <stdlib.h>
#include <limits.h>

typedef struct control_style {
    win16_style_palette palette;
    unsigned calls;
    BOOL detached,removing,file_owner;
    HFONT font,original_font;
    BOOL edit_border;
    HWND combo_list,combo_edit,combo_parent;
    HFONT combo_edit_font,combo_edit_source,combo_fonts[32];
    unsigned combo_font_count,combo_paints;
    BOOL combo_bound;
    struct control_style *owner;
    const void *creation_owner;
    ntw16s_control_protocol *protocol;
    win16_scroll_track scroll;
    UINT_PTR scroll_timer;
    unsigned scroll_epoch;
    unsigned tab_epoch;
    unsigned header_epoch,progress_epoch;
    HWND track_parent,track_window;
    BOOL track_internal;
    HWND toolbar_parent,toolbar_window;
    BOOL toolbar_internal;
    ntw16s_updown_paint updown;
    int header_pressed;
    BOOL scroll_reporting,scroll_key;
    LONG scroll_report;
    BOOL list_scrollbar,edit_scrollbars,frame_scrollbars,native_border,list_redraw_off,legacy_progress_border;
    unsigned list_suppression;
    HWND list_capture_before;
    LONG embedded_object;
    BOOL scroll_caret,scroll_caret_shown;
    SIZE scroll_caret_size;
    int tab_count,*tab_positions;
} control_style;
typedef struct track_paint_scope {
    struct track_paint_scope *previous;
    control_style *owner;
    const NMCUSTOMDRAW *draw;
    HDC dc;
    UINT_PTR item;
    RECT rect;
    BOOL active,pending;
} track_paint_scope;
static _Thread_local track_paint_scope *track_paint;
static void track_paint_end(void *data)
{track_paint_scope *scope=data;track_paint=scope->previous;}
static NTW16S_UNWIND_FUNCTION LRESULT track_paint_dispatch(control_style *s,HWND w,UINT m,WPARAM a,LPARAM b)
{
    track_paint_scope scope={.previous=track_paint,.owner=s};ntw16s_unwind_scope unwind;
    DWORD entry=GetLastError(),error;LRESULT result;
    track_paint=&scope;NTW16S_UNWIND_BEGIN(unwind,track_paint_end,&scope);
    SetLastError(entry);result=DefSubclassProc(w,m,a,b);error=GetLastError();
    NTW16S_UNWIND_END(unwind);SetLastError(error);return result;
}
static void dispose_control(control_style *s)
{for(unsigned i=0;i<s->combo_font_count;++i)win16_style_system_release(s->combo_fonts[i]);win16_style_system_release(s->font);ntw16s_control_protocol_release(s->protocol);free(s->tab_positions);free(s);}
static LRESULT CALLBACK control_proc(HWND,UINT,WPARAM,LPARAM,UINT_PTR,DWORD_PTR);
static LRESULT CALLBACK trackbar_notify(HWND,UINT,WPARAM,LPARAM,UINT_PTR,DWORD_PTR);
static LRESULT CALLBACK toolbar_notify(HWND,UINT,WPARAM,LPARAM,UINT_PTR,DWORD_PTR);
static BOOL updown_acquire(void *owner)
{
    control_style *s=owner;DWORD_PTR value;
    if(s->detached || !GetWindowSubclass(s->updown.window,control_proc,(UINT_PTR)control_proc,&value) || value!=(DWORD_PTR)s)return FALSE;
    ++s->calls;return TRUE;
}
static void updown_release(void *owner)
{control_style *s=owner;if(!--s->calls && s->detached)dispose_control(s);}
static ntw16s_updown_glyph *const updown_glyphs[]={
    &scroll_uparrow,&scroll_dnarrow,&scroll_lfarrow,&scroll_rgarrow,
    &scroll_uparrowd,&scroll_dnarrowd,&scroll_lfarrowd,&scroll_rgarrowd,
    &scroll_uparrowi,&scroll_dnarrowi,&scroll_lfarrowi,&scroll_rgarrowi};

/* Keep native-painter borrowed font returns alive through reentrant setters.
 * Finite cache follows the existing 32-lease VB text scope limit. */
static HFONT combo_edit_font(control_style *s,HFONT source)
{
    HFONT font=win16_style_vb_font_acquire(source);if(!font)return source;
    for(unsigned i=0;i<s->combo_font_count;++i)if(s->combo_fonts[i]==font){win16_style_system_release(font);return font;}
    if(s->combo_font_count==ARRAYSIZE(s->combo_fonts)){win16_style_system_release(font);return source;}
    s->combo_fonts[s->combo_font_count++]=font;return font;
}
static void combo_text_unbind(control_style *,BOOL);
static LRESULT CALLBACK combo_text_proc(HWND w,UINT m,WPARAM a,LPARAM b,UINT_PTR id,DWORD_PTR data)
{
    control_style *s=(control_style *)data;++s->calls;LRESULT result;
    HFONT pinned=s->combo_edit_font,outbound=NULL;win16_style_system_retain(pinned);
    BOOL related=!s->detached && s->combo_edit==w && GetParent(w)==s->combo_parent;
    BOOL painting=related && (m==WM_PAINT || m==WM_PRINT || m==WM_PRINTCLIENT);if(painting)++s->combo_paints;
    if(m==WM_NCDESTROY){RemoveWindowSubclass(w,combo_text_proc,id);if(s->combo_edit==w){s->combo_edit=NULL;if(s->combo_bound){s->combo_bound=FALSE;--s->calls;}}}
    if(related && m==WM_GETFONT)result=(LRESULT)(s->combo_paints && s->combo_edit_font?s->combo_edit_font:s->combo_edit_source);
    else {
        if(related && m==WM_SETFONT){
            HFONT source=win16_style_system_source((HFONT)a),font=combo_edit_font(s,source);
            s->combo_edit_source=source;s->combo_edit_font=font;
            outbound=s->combo_edit_font;win16_style_system_retain(outbound);a=(WPARAM)outbound;
        }
        result=DefSubclassProc(w,m,a,b);
    }
    /* A native setter can reenter removal; restore only after it returns. */
    if(!related && m!=WM_NCDESTROY && !s->detached && s->combo_edit==w)combo_text_unbind(s,FALSE);
    if(m==WM_SETFONT && s->detached && IsWindow(w)){
        DWORD_PTR successor;HFONT source=s->combo_edit_source;
        if(GetWindowSubclass(w,combo_text_proc,(UINT_PTR)combo_text_proc,&successor))source=((control_style *)successor)->combo_edit_source;
        else if(GetWindowSubclass(w,control_proc,(UINT_PTR)control_proc,&successor))source=((control_style *)successor)->original_font;
        SendMessageW(w,WM_SETFONT,(WPARAM)source,b);
    }
    if(painting)--s->combo_paints;
    win16_style_system_release(outbound);win16_style_system_release(pinned);
    if(!--s->calls && s->detached)dispose_control(s);
    return result;
}
static void combo_text_unbind(control_style *s,BOOL terminal)
{
    HWND edit=s->combo_edit;DWORD_PTR data;
    if(!s->combo_bound)return;
    BOOL owned=edit && GetWindowSubclass(edit,combo_text_proc,(UINT_PTR)combo_text_proc,&data) && data==(DWORD_PTR)s;
    if(owned && !RemoveWindowSubclass(edit,combo_text_proc,(UINT_PTR)combo_text_proc))return;
    s->combo_edit=NULL;s->combo_bound=FALSE;--s->calls;
    if(owned && !terminal && IsWindow(edit)){
        SendMessageW(edit,WM_SETFONT,(WPARAM)s->combo_edit_source,FALSE);
        if(IsWindow(edit))InvalidateRect(edit,NULL,TRUE);
    }
}
static void trackbar_unbind(control_style *s)
{
    DWORD_PTR value;
    if(s->track_parent && GetWindowSubclass(s->track_parent,trackbar_notify,(UINT_PTR)s->track_window,&value) &&
        value==(DWORD_PTR)s)RemoveWindowSubclass(s->track_parent,trackbar_notify,(UINT_PTR)s->track_window);
    s->track_parent=NULL;s->track_internal=FALSE;
}
static void toolbar_unbind(control_style *s)
{
    DWORD_PTR value;
    if(s->toolbar_parent && GetWindowSubclass(s->toolbar_parent,toolbar_notify,(UINT_PTR)s->toolbar_window,&value) &&
        value==(DWORD_PTR)s)RemoveWindowSubclass(s->toolbar_parent,toolbar_notify,(UINT_PTR)s->toolbar_window);
    s->toolbar_parent=NULL;s->toolbar_internal=FALSE;
}
/* Extra part notifications are private to this renderer, and are requested
 * only after the application returned exactly CDRF_DODEFAULT. Its own requested
 * stages and all non-drawing notifications keep their original route/result. */
static LRESULT CALLBACK trackbar_notify(HWND parent,UINT m,WPARAM p,LPARAM l,UINT_PTR id,DWORD_PTR data)
{
    control_style *s=(control_style *)data;DWORD entry=GetLastError();LRESULT result;BOOL internal=FALSE;
    ++s->calls;NMCUSTOMDRAW *draw=NULL;
    if(m==WM_NOTIFY && l && !s->detached && GetParent(s->track_window)==parent){
        NMHDR *header=(NMHDR *)l;
        if(header->hwndFrom==s->track_window && header->code==NM_CUSTOMDRAW)draw=(NMCUSTOMDRAW *)l;
    }
    if(draw && ntw16s_control_protocol_runtime_trackbar(s->track_window)){
        track_paint_scope *scope=track_paint;
        BOOL match=scope && scope->owner==s && scope->active && scope->draw==draw && scope->dc==draw->hdc;
        if(match && draw->dwDrawStage==CDDS_ITEMPOSTPAINT && scope->pending && scope->item==draw->dwItemSpec){
            scope->pending=FALSE;
            if(EqualRect(&scope->rect,&draw->rc))win16_style_trackbar_runtime_border(s->track_window,draw,&s->palette);
            result=CDRF_DODEFAULT;
        }else if(match && draw->dwDrawStage==CDDS_POSTPAINT){scope->active=FALSE;result=CDRF_DODEFAULT;}
        else {
            SetLastError(entry);result=DefSubclassProc(parent,m,p,l);DWORD error=GetLastError();
            if(!s->detached && scope && track_paint==scope && scope->owner==s){
                if(draw->dwDrawStage==CDDS_PREPAINT && result==CDRF_NOTIFYITEMDRAW &&
                   win16_style_trackbar_supported(s->track_window)){
                    scope->draw=draw;scope->dc=draw->hdc;scope->active=TRUE;
                    result=CDRF_NOTIFYITEMDRAW|CDRF_NOTIFYPOSTPAINT;
                }else if(match && draw->dwDrawStage==CDDS_ITEMPREPAINT && result==CDRF_DODEFAULT && !scope->pending &&
                    (draw->dwItemSpec==TBCD_CHANNEL || draw->dwItemSpec==TBCD_THUMB)){
                    scope->item=draw->dwItemSpec;scope->rect=draw->rc;scope->pending=TRUE;result=CDRF_NOTIFYPOSTPAINT;
                }
            }
            if(!--s->calls && s->detached)dispose_control(s);
            SetLastError(error);return result;
        }
        if(!--s->calls && s->detached)dispose_control(s);
        SetLastError(entry);return result;
    }
    if(draw && draw->dwDrawStage!=CDDS_PREPAINT && s->track_internal)internal=TRUE;
    if(m==WM_NCDESTROY){RemoveWindowSubclass(parent,trackbar_notify,id);s->track_parent=NULL;s->track_internal=FALSE;}
    SetLastError(entry);
    result=internal?CDRF_DODEFAULT:DefSubclassProc(parent,m,p,l);DWORD error=GetLastError();
    if(draw && !s->detached){
        if(draw->dwDrawStage==CDDS_PREPAINT){
            s->track_internal=result==CDRF_DODEFAULT && win16_style_trackbar_supported(s->track_window);
            if(s->track_internal)result=CDRF_NOTIFYITEMDRAW|CDRF_NOTIFYPOSTPAINT;
        }else if(internal && draw->dwDrawStage==CDDS_ITEMPREPAINT &&
            win16_style_trackbar_part(s->track_window,draw,&s->palette))result=CDRF_SKIPDEFAULT;
        else if(internal && draw->dwDrawStage==CDDS_POSTPAINT)s->track_internal=FALSE;
    }
    if(!--s->calls && s->detached)dispose_control(s);
    SetLastError(error);return result;
}
/* The OCX wrapper retains command routing and its return values.  Item drawing
 * is requested only after it selects the default custom-draw route. */
static LRESULT CALLBACK toolbar_notify(HWND parent,UINT m,WPARAM p,LPARAM l,UINT_PTR id,DWORD_PTR data)
{
    control_style *s=(control_style *)data;DWORD entry=GetLastError();LRESULT result;BOOL internal=FALSE;
    ++s->calls;NMCUSTOMDRAW *draw=NULL;
    if(m==WM_NOTIFY && l && !s->detached && GetParent(s->toolbar_window)==parent){
        NMHDR *header=(NMHDR *)l;
        if(header->hwndFrom==s->toolbar_window && header->code==NM_CUSTOMDRAW)draw=(NMCUSTOMDRAW *)l;
    }
    if(draw && draw->dwDrawStage!=CDDS_PREPAINT && s->toolbar_internal)internal=TRUE;
    if(m==WM_NCDESTROY){RemoveWindowSubclass(parent,toolbar_notify,id);s->toolbar_parent=NULL;s->toolbar_internal=FALSE;}
    SetLastError(entry);result=internal?CDRF_DODEFAULT:DefSubclassProc(parent,m,p,l);DWORD error=GetLastError();
    if(draw && !s->detached){
        if(draw->dwDrawStage==CDDS_PREPAINT){
            s->toolbar_internal=result==CDRF_DODEFAULT && win16_style_toolbar_supported(s->toolbar_window);
            if(s->toolbar_internal)result=CDRF_NOTIFYITEMDRAW|CDRF_NOTIFYPOSTPAINT;
        }else if(internal && draw->dwDrawStage==CDDS_ITEMPREPAINT &&
            win16_style_toolbar_part(s->toolbar_window,draw,&s->palette))result=CDRF_SKIPDEFAULT;
        else if(internal && draw->dwDrawStage==CDDS_POSTPAINT)s->toolbar_internal=FALSE;
    }
    if(!--s->calls && s->detached)dispose_control(s);
    SetLastError(error);return result;
}

/* DefWindowProc implements WM_SETREDRAW(TRUE) by setting WS_VISIBLE.
 * Never start an internal bracket on an application-hidden control. Nested
 * native leaves share the already-open bracket, whose FALSE cleared that bit. */
static BOOL can_suppress_redraw(HWND w,const control_style *s)
{return !s->list_redraw_off && (s->list_suppression || (GetWindowLongPtrW(w,GWL_STYLE)&WS_VISIBLE));}
static BOOL is_static(HWND w)
{return ntw16s_class_is(w,L"STATIC");}
static BOOL is_edit(HWND w)
{return ntw16s_class_is(w,L"EDIT");}
static BOOL is_list(HWND w)
{WCHAR name[16];return ntw16s_class_is(w,L"LISTBOX") || (GetClassNameW(w,name,16) && !lstrcmpiW(name,L"ComboLBox"));}
static BOOL is_combo(HWND w)
{return ntw16s_class_is(w,L"COMBOBOX");}
static BOOL is_scroll(HWND w)
{return ntw16s_class_is(w,L"SCROLLBAR");}
static BOOL is_link(HWND w)
{return ntw16s_class_is(w,L"SysLink");}
static BOOL is_tab(HWND w)
{return ntw16s_class_is(w,WC_TABCONTROLW) || ntw16s_control_protocol_is(w,WC_TABCONTROLW);}
static BOOL is_header(HWND w)
{return ntw16s_class_is(w,WC_HEADERW);}
static BOOL is_separator(HWND w)
{DWORD type=(DWORD)GetWindowLongPtrW(w,GWL_STYLE)&SS_TYPEMASK;
 return is_static(w) && type>=SS_ETCHEDHORZ && type<=SS_ETCHEDFRAME;}

static BOOL supported(HWND w)
{
    DWORD style=(DWORD)GetWindowLongPtrW(w,GWL_STYLE),type=ntw16s_button_type(w,style);
    if(ntw16s_vb_owner_file(w))return TRUE;
    if((ntw16s_vb_checkbox(w) || ntw16s_vb_option(w)) &&
        !ntw16s_vb_plain_choice(w) && !ntw16s_vb_graphical_choice(w))return FALSE;
    if(is_scroll(w))return !(style&SBS_SIZEGRIP);
    if(is_link(w))return TRUE;
    if(is_tab(w))return win16_style_tab_supported(w);
    if(is_header(w))return win16_style_header_supported(w);
    if(ntw16s_class_is(w,PROGRESS_CLASSW) || ntw16s_control_protocol_is(w,PROGRESS_CLASSW))return win16_style_progress_supported(w);
    if(is_combo(w))return ((style&3)==CBS_DROPDOWNLIST || (style&3)==CBS_DROPDOWN || (style&3)==CBS_SIMPLE) &&
        !(style&(CBS_OWNERDRAWFIXED|CBS_OWNERDRAWVARIABLE));
    if(is_list(w))return !(style&(LBS_OWNERDRAWFIXED|LBS_OWNERDRAWVARIABLE|LBS_NODATA|LBS_USETABSTOPS));
    if(is_edit(w))return TRUE;
    if(is_static(w)){type=style&SS_TYPEMASK;return type==SS_LEFT || type==SS_CENTER || type==SS_RIGHT ||
        type==SS_ICON || type==SS_BITMAP || type==SS_SIMPLE || type==SS_LEFTNOWORDWRAP ||
        (type>=SS_BLACKRECT && type<=SS_WHITEFRAME) || (type>=SS_ETCHEDHORZ && type<=SS_ETCHEDFRAME);}
    if(!ntw16s_class_is(w,L"BUTTON"))return FALSE;
    if((style&(BS_ICON|BS_BITMAP)) && type!=BS_PUSHBUTTON && type!=BS_DEFPUSHBUTTON && !(style&BS_PUSHLIKE))return FALSE;
    if(type==BS_PUSHBUTTON || type==BS_DEFPUSHBUTTON || type==BS_GROUPBOX)return TRUE;
    return type==BS_CHECKBOX || type==BS_AUTOCHECKBOX || type==BS_RADIOBUTTON ||
        type==BS_AUTORADIOBUTTON || type==BS_3STATE || type==BS_AUTO3STATE;
}

static void frame(HDC dc,RECT r,COLORREF color,int width)
{
    HBRUSH brush=CreateSolidBrush(color);int i;
    if(!brush)return;
    for(i=0;i<width && r.right>r.left && r.bottom>r.top;++i){FrameRect(dc,&r,brush);
    InflateRect(&r,-1,-1);
    }
    DeleteObject(brush);
}

static RECT button_border(HDC dc,RECT r,const win16_style_palette *palette,int u,int border,BOOL pushed)
{
    RECT bounds=r,corner;
    frame(dc,bounds,palette->edge,border);
    SetDCBrushColor(dc,palette->light);
    SetRect(&corner,r.left,r.top,r.left+u,r.top+u);FillRect(dc,&corner,(HBRUSH)GetStockObject(DC_BRUSH));
    OffsetRect(&corner,r.right-r.left-u,0);FillRect(dc,&corner,(HBRUSH)GetStockObject(DC_BRUSH));
    OffsetRect(&corner,0,r.bottom-r.top-u);FillRect(dc,&corner,(HBRUSH)GetStockObject(DC_BRUSH));
    OffsetRect(&corner,-(r.right-r.left-u),0);FillRect(dc,&corner,(HBRUSH)GetStockObject(DC_BRUSH));
    InflateRect(&bounds,-border,-border);
    RECT strip=bounds;int band=pushed?u:2*u;
    SetDCBrushColor(dc,pushed?palette->shadow:palette->light);
    strip.bottom=strip.top+band;FillRect(dc,&strip,(HBRUSH)GetStockObject(DC_BRUSH));
    strip=bounds;strip.right=strip.left+band;FillRect(dc,&strip,(HBRUSH)GetStockObject(DC_BRUSH));
    if(!pushed){SetDCBrushColor(dc,palette->shadow);
        for(int i=0;i<2;++i){strip=bounds;InflateRect(&strip,-i*u,-i*u);
            strip.top=strip.bottom-u;FillRect(dc,&strip,(HBRUSH)GetStockObject(DC_BRUSH));
            strip=bounds;InflateRect(&strip,-i*u,-i*u);
            strip.left=strip.right-u;FillRect(dc,&strip,(HBRUSH)GetStockObject(DC_BRUSH));}}
    return bounds;
}
BOOL win16_style_checklist_frame(HDC dc,const RECT *rect,UINT type,UINT flags,const void *address)
{
    const UINT allowed=DFCS_CHECKED|DFCS_PUSHED;
    if(type!=DFC_BUTTON || (flags&15u)!=DFCS_BUTTONCHECK || (flags&~allowed) ||
        !ntw16s_checklist_draw_target(dc,rect,address))return FALSE;
    int width=rect->right-rect->left,height=rect->bottom-rect->top;
    if(width>256 || height>256)return FALSE;
    DWORD *pixels=calloc((size_t)width*(size_t)height,sizeof(*pixels));if(!pixels)return FALSE;
    int column=((flags&DFCS_CHECKED)?1:0)+((flags&DFCS_PUSHED)?2:0);
    for(int y=0;y<13;y++)for(int x=0;x<14;x++){
        int left=MulDiv(x,width,14),right=MulDiv(x+1,width,14);
        int top=MulDiv(y,height,13),bottom=MulDiv(y+1,height,13);
        DWORD color=control_checks[y][column*14+x]=='0'?0:0x00ffffff;
        for(int py=top;py<bottom;py++)for(int px=left;px<right;px++)pixels[py*width+px]=color;
    }
    BITMAPINFO info={0};info.bmiHeader.biSize=sizeof(info.bmiHeader);
    info.bmiHeader.biWidth=width;info.bmiHeader.biHeight=-height;info.bmiHeader.biPlanes=1;info.bmiHeader.biBitCount=32;
    int drawn=SetDIBitsToDevice(dc,rect->left,rect->top,(DWORD)width,(DWORD)height,0,0,0,(UINT)height,pixels,&info,DIB_RGB_COLORS);
    free(pixels);return drawn!=0;
}
static control_style *graphical_target(HDC dc,UINT type,UINT flags,const void *address,HWND *w)
{
    DWORD_PTR value;HMODULE caller=NULL;
    const UINT allowed=DFCS_BUTTONPUSH|DFCS_PUSHED|DFCS_INACTIVE|DFCS_ADJUSTRECT|DFCS_CHECKED;
    if(type!=DFC_BUTTON || !(flags&DFCS_BUTTONPUSH) || (flags&~allowed) ||
            !ntw16s_report_paint_target(dc,w) || !ntw16s_vb_graphical_button(*w) ||
            ((flags&DFCS_CHECKED) && !ntw16s_vb_class(*w,L"ThunderRT6OptionButton") &&
             !ntw16s_vb_class(*w,L"ThunderRT6CheckBox")) ||
            !GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,(const WCHAR *)address,&caller) ||
            caller!=(HMODULE)GetClassLongPtrW(*w,GCLP_HMODULE) ||
            !GetWindowSubclass(*w,control_proc,(UINT_PTR)control_proc,&value))return NULL;
    control_style *s=(void *)value;return !s->detached && s->frame_scrollbars?s:NULL;
}
BOOL win16_style_bitbtn_face(HWND w,HDC dc,const RECT *rect,const RECT *clip,UINT state)
{
    if(!w || !dc || !rect || rect->right-rect->left<4 || rect->bottom-rect->top<4)return FALSE;
    int saved=SaveDC(dc);if(!saved)return FALSE;
    if(clip && IntersectClipRect(dc,clip->left,clip->top,clip->right,clip->bottom)==ERROR){RestoreDC(dc,saved);return FALSE;}
    win16_style_palette palette;win16_style_default_palette(&palette);int u=max(1,MulDiv(1,(int)GetDpiForWindow(w),96));
    SetDCBrushColor(dc,palette.face);FillRect(dc,rect,(HBRUSH)GetStockObject(DC_BRUSH));
    (void)button_border(dc,*rect,&palette,u,u,(state&ODS_SELECTED)!=0);RestoreDC(dc,saved);return TRUE;
}
BOOL win16_style_graphical_frame_prepare(HDC dc,const RECT *rect,UINT type,UINT flags,const void *address,RECT *copy)
{
    HWND w;
    /* Ordinary API calls remain untouched, including parameter validation.
     * Copy only a trusted owned-runtime primitive inside its exact scope. */
    if(!rect || !copy || !graphical_target(dc,type,flags,address,&w))return FALSE;
    *copy=*rect;return TRUE;
}
BOOL win16_style_graphical_frame(HDC dc,const RECT *rect,UINT type,UINT flags,const void *address)
{
    HWND w;RECT client;POINT origin;control_style *s=graphical_target(dc,type,flags,address,&w);
    if(!s || !rect || !GetClientRect(w,&client) || !EqualRect(rect,&client) ||
            GetMapMode(dc)!=MM_TEXT || (GetLayout(dc)&LAYOUT_RTL) || !GetViewportOrgEx(dc,&origin) || origin.x || origin.y)return FALSE;
    int saved=SaveDC(dc);if(!saved)return FALSE;
    int u=max(1,MulDiv(1,(int)GetDpiForWindow(w),96));
    /* This native primitive owns its face before VB draws the caption/image.
     * The native call has already preserved its return/rect/error semantics. */
    SetDCBrushColor(dc,s->palette.face);FillRect(dc,rect,(HBRUSH)GetStockObject(DC_BRUSH));
    (void)button_border(dc,*rect,&s->palette,u,u,(flags&(DFCS_PUSHED|DFCS_CHECKED))!=0);
    RestoreDC(dc,saved);return TRUE;
}
static void other_control(HWND w,control_style *s,HDC dc,RECT r,HFONT font,const WCHAR *text,BOOL label)
{
    DWORD style=(DWORD)GetWindowLongPtrW(w,GWL_STYLE);UINT type=(label?(style&SS_TYPEMASK):ntw16s_button_type(w,style));
    int u=max(1,MulDiv(1,(int)GetDpiForWindow(w),96));RECT bounds=r;TEXTMETRICW tm;
    LRESULT button_state=label?0:SendMessageW(w,BM_GETSTATE,0,0);
    UINT flags=DT_LEFT;BOOL pushed=(button_state&BST_PUSHED)!=0;
    if(!label && (style&BS_PUSHLIKE))pushed=pushed || SendMessageW(w,BM_GETCHECK,0,0)!=BST_UNCHECKED;
    if(s->detached)return;
    SelectObject(dc,font);GetTextMetricsW(dc,&tm);SetBkMode(dc,TRANSPARENT);SetTextColor(dc,s->palette.edge);
    if(label && type>=SS_ETCHEDHORZ && type<=SS_ETCHEDFRAME){
        /* A later STATIC separator has no Win3.x etched relief. Keep its
         * geometry and express it using the same flat border as SS_*FRAME. */
        if(type==SS_ETCHEDHORZ)r.bottom=min(r.bottom,r.top+u);
        if(type==SS_ETCHEDVERT)r.right=min(r.right,r.left+u);
        frame(dc,r,s->palette.edge,u);return;
    }
    if(label && type>=SS_BLACKRECT && type<=SS_WHITEFRAME){
        COLORREF color=(type==SS_BLACKRECT || type==SS_BLACKFRAME)?s->palette.edge:
            (type==SS_GRAYRECT || type==SS_GRAYFRAME)?s->palette.shadow:s->palette.light;
        if(type>=SS_BLACKFRAME)frame(dc,r,color,u);
        else {SetDCBrushColor(dc,color);FillRect(dc,&r,(HBRUSH)GetStockObject(DC_BRUSH));}return;
    }
    if(!label && type==BS_GROUPBOX){
        RECT title=r;title.left+=8*u;DrawTextW(dc,text,-1,&title,DT_CALCRECT|DT_SINGLELINE);
        bounds.top+=tm.tmHeight/2;frame(dc,bounds,s->palette.shadow,u);
        InflateRect(&title,u,0);SetDCBrushColor(dc,s->palette.light);FillRect(dc,&title,(HBRUSH)GetStockObject(DC_BRUSH));
        title.left+=u;DrawTextW(dc,text,-1,&title,DT_SINGLELINE);return;
    }
    if(!label){
        /* Retail USER 1:8E0C draws one continuous frame: two border units
         * for an enabled default, one otherwise. It does not add a spaced
         * second rectangle. USER 25:18BC suppresses default emphasis when
         * disabled. Native BUTTON/dialog code still owns all these states. */
        int border=(type==BS_DEFPUSHBUTTON && IsWindowEnabled(w)?2:1)*u;
        SetDCBrushColor(dc,s->palette.face);FillRect(dc,&bounds,(HBRUSH)GetStockObject(DC_BRUSH));
        bounds=button_border(dc,r,&s->palette,u,border,pushed);
        InflateRect(&bounds,-3*u,-3*u);if(pushed)OffsetRect(&bounds,u,u);
        flags=DT_CENTER|DT_VCENTER|DT_SINGLELINE;
    }else {flags=type==SS_CENTER?DT_CENTER:type==SS_RIGHT?DT_RIGHT:DT_LEFT;
        /* StaticCallback expands tabs and preserves explicit line breaks.
         * LEFTNOWORDWRAP removes word wrapping, not multiline text. SIMPLE
         * is the distinct single-line, always-enabled original fast path. */
        flags|=type==SS_SIMPLE?DT_SINGLELINE:DT_EXPANDTABS|(type==SS_LEFTNOWORDWRAP?0:DT_WORDBREAK);
        if(style&SS_NOPREFIX)flags|=DT_NOPREFIX;}
    if(!IsWindowEnabled(w) && !(label && type==SS_SIMPLE))SetTextColor(dc,s->palette.shadow);
    if(!label && (style&(BS_ICON|BS_BITMAP))){
        UINT image_type=(style&BS_ICON)?IMAGE_ICON:IMAGE_BITMAP;
        HANDLE image=(HANDLE)SendMessageW(w,BM_GETIMAGE,image_type,0);
        BUTTON_IMAGELIST images={0};BOOL listed=FALSE;
        int cx=0,cy=0;BITMAP bits;
        if(s->detached || !IsWindow(w))return;
        if(!image){
            listed=(BOOL)SendMessageW(w,BCM_GETIMAGELIST,0,(LPARAM)&images);
            if(s->detached || !IsWindow(w))return;
            listed=listed && images.himl && images.himl!=BCCL_NOGLYPH &&
                ImageList_GetIconSize(images.himl,&cx,&cy);
        }
        if(image && image_type==IMAGE_ICON){
            ICONINFO info;
            if(GetIconInfo((HICON)image,&info)){
                if(GetObjectW(info.hbmColor?info.hbmColor:info.hbmMask,sizeof(bits),&bits)){
                    cx=bits.bmWidth;cy=info.hbmColor?bits.bmHeight:bits.bmHeight/2;}
                if(info.hbmColor)DeleteObject(info.hbmColor);
                if(info.hbmMask)DeleteObject(info.hbmMask);
            }
        }else if(image && GetObjectW(image,sizeof(bits),&bits)){cx=bits.bmWidth;cy=bits.bmHeight;}
        if((image || listed) && cx>0 && cy>0){
            int x=bounds.left+(bounds.right-bounds.left-cx)/2,y=bounds.top+(bounds.bottom-bounds.top-cy)/2;
            if(listed){
                bounds.left+=images.margin.left;bounds.top+=images.margin.top;
                bounds.right-=images.margin.right;bounds.bottom-=images.margin.bottom;
                x=bounds.left+(bounds.right-bounds.left-cx)/2;y=bounds.top+(bounds.bottom-bounds.top-cy)/2;
                if(images.uAlign==BUTTON_IMAGELIST_ALIGN_LEFT)x=bounds.left;
                else if(images.uAlign==BUTTON_IMAGELIST_ALIGN_RIGHT)x=bounds.right-cx;
                else if(images.uAlign==BUTTON_IMAGELIST_ALIGN_TOP)y=bounds.top;
                else if(images.uAlign==BUTTON_IMAGELIST_ALIGN_BOTTOM)y=bounds.bottom-cy;
            }else {
                if((style&BS_CENTER)==BS_LEFT)x=bounds.left;
                else if((style&BS_CENTER)==BS_RIGHT)x=bounds.right-cx;
                if((style&BS_VCENTER)==BS_TOP)y=bounds.top;
                else if((style&BS_VCENTER)==BS_BOTTOM)y=bounds.bottom-cy;
            }
            int saved=SaveDC(dc);
            if(saved){IntersectClipRect(dc,bounds.left,bounds.top,bounds.right,bounds.bottom);
                if(listed){
                    int count=ImageList_GetImageCount(images.himl),index=0;
                    if(count>1)index=!IsWindowEnabled(w)?3:pushed?2:(button_state&BST_HOT)?1:type==BS_DEFPUSHBUTTON?4:0;
                    if(!s->detached && index<count)ImageList_DrawEx(images.himl,index,dc,x,y,cx,cy,CLR_NONE,
                        s->palette.shadow,count==1 && !IsWindowEnabled(w)?ILD_BLEND50:ILD_NORMAL);
                }else DrawStateW(dc,NULL,NULL,(LPARAM)image,0,x,y,cx,cy,
                        (image_type==IMAGE_ICON?DST_ICON:DST_BITMAP)|(IsWindowEnabled(w)?DSS_NORMAL:DSS_DISABLED));
                RestoreDC(dc,saved);}
        }
        if(GetFocus()==w){RECT focus=bounds;InflateRect(&focus,-u,-u);DrawFocusRect(dc,&focus);}
        return;
    }
    DrawTextW(dc,text,-1,&bounds,flags);
    if(!label && GetFocus()==w){
        /* USER 25:15DE..1694 bounds focus around the measured label, not
         * around the whole button face. Default status is independent. */
        RECT focus=bounds,measure={0,0,0,0};int width,height;
        DrawTextW(dc,text,-1,&measure,DT_CALCRECT|DT_SINGLELINE);
        width=min(bounds.right-bounds.left,measure.right+4*u);
        height=min(bounds.bottom-bounds.top,measure.bottom+3*u);
        focus.left+=(bounds.right-bounds.left-width)/2;focus.right=focus.left+width;
        focus.top=max(bounds.top,bounds.top+(bounds.bottom-bounds.top-measure.bottom)/2-u);
        focus.bottom=min(bounds.bottom,focus.top+height);
        DrawFocusRect(dc,&focus);
    }
}

static void draw(HWND w,control_style *s,HDC dc)
{
    RECT r,glyph,label,focus,pixel;int saved,x,y,row,column,height,length;
    UINT dpi=GetDpiForWindow(w);DWORD style=(DWORD)GetWindowLongPtrW(w,GWL_STYLE);
    UINT type=ntw16s_button_type(w,style);LRESULT state,check;WCHAR *text;HFONT font;TEXTMETRICW tm;
    BOOL radio=type==BS_RADIOBUTTON || type==BS_AUTORADIOBUTTON;
    BOOL label_control=is_static(w);
    if(label_control && (style&SS_TYPEMASK)==SS_ICON){
        /* statctl.c retains image ownership/sizing in native STATIC. The
         * painter borrows its current handle, never reads image data as text.
         * CENTERIMAGE is a host extension; its natural-size image is clipped. */
        HICON icon=(HICON)SendMessageW(w,STM_GETICON,0,0);int cx,cy;
        if(s->detached || !IsWindow(w))return;
        saved=SaveDC(dc);if(!saved)return;
        GetClientRect(w,&r);cx=r.right;cy=r.bottom;x=y=0;
        IntersectClipRect(dc,r.left,r.top,r.right,r.bottom);
        SetDCBrushColor(dc,s->palette.light);FillRect(dc,&r,(HBRUSH)GetStockObject(DC_BRUSH));
        if(icon && (style&SS_CENTERIMAGE)){
            ICONINFO info;BITMAP bits;
            if(GetIconInfo(icon,&info)){
                if(GetObjectW(info.hbmColor?info.hbmColor:info.hbmMask,sizeof(bits),&bits)){
                    cx=bits.bmWidth;cy=info.hbmColor?bits.bmHeight:bits.bmHeight/2;
                    x=(r.right-cx)/2;y=(r.bottom-cy)/2;
                }
                if(info.hbmColor)DeleteObject(info.hbmColor);
                if(info.hbmMask)DeleteObject(info.hbmMask);
            }
        }
        if(icon)DrawIconEx(dc,x,y,icon,cx,cy,0,NULL,DI_NORMAL);
        RestoreDC(dc,saved);return;
    }
    state=label_control?0:SendMessageW(w,BM_GETSTATE,0,0);check=label_control?0:SendMessageW(w,BM_GETCHECK,0,0);
    length=GetWindowTextLengthW(w);text=calloc((size_t)length+1,sizeof(WCHAR));
    if(!text)return;
    GetWindowTextW(w,text,length+1);
    font=s->font && (ntw16s_vb_frame(w) || ntw16s_vb_button(w) ||
        ntw16s_control_protocol_is(w,L"BUTTON"))?
        s->font:(HFONT)SendMessageW(w,WM_GETFONT,0,0);
    if(s->detached || !IsWindow(w)){free(text);return;}
    if(!font)font=(HFONT)GetStockObject(SYSTEM_FONT);
    saved=SaveDC(dc);if(!saved){free(text);return;}
    GetClientRect(w,&r);SetDCBrushColor(dc,s->palette.light);
    /* A group box surrounds sibling controls, not an opaque child surface.
     * Repainting its interior erases those siblings during ordinary motion. */
    if(label_control || type!=BS_GROUPBOX)FillRect(dc,&r,(HBRUSH)GetStockObject(DC_BRUSH));
    if(label_control || type==BS_PUSHBUTTON || type==BS_DEFPUSHBUTTON || type==BS_GROUPBOX || (style&BS_PUSHLIKE)){
        other_control(w,s,dc,r,font,text,label_control);RestoreDC(dc,saved);free(text);return;}
    height=MulDiv(13,(int)dpi,96);glyph=r;glyph.top+=(r.bottom-height)/2;
    glyph.bottom=glyph.top+height;glyph.right=glyph.left+MulDiv(14,(int)dpi,96);
    if(style&BS_LEFTTEXT)OffsetRect(&glyph,r.right-glyph.right,0);
    row=radio?1:check==BST_INDETERMINATE?2:0;
    column=(check!=BST_UNCHECKED?1:0)+((state&BST_PUSHED)?2:0);
    for(y=0;y<13;++y)for(x=0;x<14;++x){
        SetDCBrushColor(dc,control_checks[row*13+y][column*14+x]=='0'?s->palette.edge:s->palette.light);
        SetRect(&pixel,glyph.left+MulDiv(x,(int)dpi,96),glyph.top+MulDiv(y,(int)dpi,96),
            glyph.left+MulDiv(x+1,(int)dpi,96),glyph.top+MulDiv(y+1,(int)dpi,96));
        FillRect(dc,&pixel,(HBRUSH)GetStockObject(DC_BRUSH));
    }
    label=r;if(style&BS_LEFTTEXT)label.right=glyph.left-MulDiv(4,(int)dpi,96);
    else label.left=glyph.right+MulDiv(4,(int)dpi,96);
    SelectObject(dc,font);GetTextMetricsW(dc,&tm);SetBkMode(dc,TRANSPARENT);SetTextColor(dc,s->palette.edge);
    label.top+=(r.bottom-tm.tmHeight)/2;label.bottom=label.top+tm.tmHeight;
    if(IsWindowEnabled(w))DrawTextW(dc,text,-1,&label,DT_LEFT|DT_SINGLELINE);
    else {HBRUSH brush=CreateSolidBrush(s->palette.edge);
        if(brush){DrawStateW(dc,brush,NULL,(LPARAM)text,0,label.left,label.top,
            max(0,label.right-label.left),tm.tmHeight,DST_PREFIXTEXT|DSS_UNION|DSS_MONO);DeleteObject(brush);}}
    if(state&BST_FOCUS){focus=label;DrawTextW(dc,text,-1,&focus,DT_CALCRECT|DT_SINGLELINE);
        focus.right=min(focus.right,label.right);InflateRect(&focus,1,1);DrawFocusRect(dc,&focus);}
    RestoreDC(dc,saved);free(text);
}

/* The native list owns strings, sorting, selection and scrolling. Query only
 * visible rows; never replace item data or convert application owner-draw. */
static void draw_list(HWND w,control_style *s,HDC dc)
{
    RECT client,row;int saved,index,count,caret;DWORD style;HFONT font;
    if(!dc)return;
    style=(DWORD)GetWindowLongPtrW(w,GWL_STYLE);
    index=(int)SendMessageW(w,LB_GETTOPINDEX,0,0);
    count=(int)SendMessageW(w,LB_GETCOUNT,0,0);
    caret=(int)SendMessageW(w,LB_GETCARETINDEX,0,0);
    font=s->font && ntw16s_vb_list(w)?s->font:(HFONT)SendMessageW(w,WM_GETFONT,0,0);
    if(s->detached)return;
    saved=SaveDC(dc);if(!saved)return;
    GetClientRect(w,&client);IntersectClipRect(dc,0,0,client.right,client.bottom);
    SetDCBrushColor(dc,s->palette.light);FillRect(dc,&client,(HBRUSH)GetStockObject(DC_BRUSH));
    SelectObject(dc,font?font:GetStockObject(SYSTEM_FONT));SetBkMode(dc,TRANSPARENT);
    for(;index>=0 && index<count;++index){
        LRESULT length,selected;WCHAR *text;RECT label,visible;int row_saved;
        if(SendMessageW(w,LB_GETITEMRECT,index,(LPARAM)&row)==LB_ERR)break;
        /* Multicolumn indices advance down a column, then to its right.
         * Keep native item geometry/scroll origin; a below-client item does
         * not imply that the following column is invisible. */
        if(style&LBS_MULTICOLUMN){if(row.left>=client.right)break;}
        else if(row.top>=client.bottom)break;
        /* USER's single-column item rect extends its right edge by xOrigin;
         * native drawing separately offsets the DC by that origin. Preserve
         * the same content coordinates, not the public scrollbar's position
         * (SetScrollInfo alone does not change the list's content origin). */
        if(!(style&LBS_MULTICOLUMN))OffsetRect(&row,client.right-row.right,0);
        if(!IntersectRect(&visible,&row,&client))continue;
        length=SendMessageW(w,LB_GETTEXTLEN,index,0);
        if(s->detached || length==LB_ERR)break;
        text=calloc((size_t)length+1,sizeof(WCHAR));if(!text)break;
        if(SendMessageW(w,LB_GETTEXT,index,(LPARAM)text)==LB_ERR){free(text);break;}
        selected=(style&(LBS_MULTIPLESEL|LBS_EXTENDEDSEL))?SendMessageW(w,LB_GETSEL,index,0):
            SendMessageW(w,LB_GETCURSEL,0,0)==index;
        if(s->detached){free(text);break;}
        row_saved=SaveDC(dc);if(!row_saved){free(text);break;}
        IntersectClipRect(dc,visible.left,visible.top,visible.right,visible.bottom);
        SetDCBrushColor(dc,selected>0?s->palette.active:s->palette.light);
        FillRect(dc,&row,(HBRUSH)GetStockObject(DC_BRUSH));
        SetTextColor(dc,!IsWindowEnabled(w)?s->palette.shadow:selected>0?s->palette.active_text:s->palette.edge);
        /* Original LBPrintCallback starts multi-column text at0, ordinary
         * single-column text at2, relative to the native item rectangle. */
        label=row;if(!(style&LBS_MULTICOLUMN))label.left+=2;
        if(style&LBS_USETABSTOPS)
            TabbedTextOutW(dc,label.left,label.top,text,(int)length,s->tab_count,s->tab_positions,row.left);
        else DrawTextW(dc,text,-1,&label,DT_LEFT|DT_SINGLELINE|DT_NOPREFIX);
        if(GetFocus()==w && caret==index)DrawFocusRect(dc,&row);
        RestoreDC(dc,row_saved);
        free(text);
    }
    RestoreDC(dc,saved);
}

static void edit_border(HWND w,control_style *s,HDC target)
{
    HDC dc;RECT bounds;int saved;
    if(!s->edit_border)return;
    dc=target?target:GetDC(w);if(!dc)return;
    saved=SaveDC(dc);if(saved){
        GetClientRect(w,&bounds);frame(dc,bounds,s->palette.edge,max(1,MulDiv(1,(int)GetDpiForWindow(w),96)));
        RestoreDC(dc,saved);
    }
    if(!target)ReleaseDC(w,dc);
}

static void list_border(HWND w,control_style *s,HDC target)
{
    RECT bounds,inside;HDC dc;int saved;BOOL edge=(GetWindowLongPtrW(w,GWL_EXSTYLE)&WS_EX_CLIENTEDGE)!=0;
    if(!(GetWindowLongPtrW(w,GWL_STYLE)&WS_BORDER) && !edge)return;
    if(!GetWindowRect(w,&bounds))return;
    OffsetRect(&bounds,-bounds.left,-bounds.top);
    dc=target?target:GetWindowDC(w);if(!dc)return;
    saved=SaveDC(dc);
    if(saved){
        if(edge){inside=bounds;InflateRect(&inside,-1,-1);frame(dc,inside,s->palette.light,1);}
        frame(dc,bounds,s->palette.edge,1);RestoreDC(dc,saved);}
    if(!target)ReleaseDC(w,dc);
}

static void draw_combo(HWND w,control_style *s,HDC dc)
{
    COMBOBOXINFO info={.cbSize=sizeof(info)};RECT client,button,text,pixel;HFONT font;
    WCHAR *label;int length,saved,x,y,left,top,u;BOOL selected,pressed,editable,simple;
    if(!dc || !ntw16s_native_get_combo_info(w,&info))return;
    length=GetWindowTextLengthW(w);label=calloc((size_t)length+1,sizeof(WCHAR));if(!label)return;
    GetWindowTextW(w,label,length+1);font=s->font && (ntw16s_vb_combo(w) || ntw16s_control_protocol_is(w,L"COMBOBOX"))?
        s->font:(HFONT)SendMessageW(w,WM_GETFONT,0,0);
    pressed=SendMessageW(w,CB_GETDROPPEDSTATE,0,0)!=0;
    simple=(GetWindowLongPtrW(w,GWL_STYLE)&3)==CBS_SIMPLE;
    editable=(GetWindowLongPtrW(w,GWL_STYLE)&3)!=CBS_DROPDOWNLIST;
    if(s->detached){free(label);return;}
    saved=SaveDC(dc);if(!saved){free(label);return;}
    u=max(1,MulDiv(1,(int)GetDpiForWindow(w),96));GetClientRect(w,&client);
    /* Native child EDIT owns its caret, text, selection and IME. Do not paint
     * over that child while drawing the combo's surrounding chrome. */
    if(editable || ntw16s_vb_drive(w))ExcludeClipRect(dc,info.rcItem.left,info.rcItem.top,info.rcItem.right,info.rcItem.bottom);
    if(simple){RECT list;
        if(GetWindowRect(info.hwndList,&list)){
            MapWindowPoints(NULL,w,(POINT *)&list,2);ExcludeClipRect(dc,list.left,list.top,list.right,list.bottom);}
        SetDCBrushColor(dc,s->palette.light);FillRect(dc,&client,(HBRUSH)GetStockObject(DC_BRUSH));
        text=info.rcItem;InflateRect(&text,u,u);frame(dc,text,s->palette.edge,u);
        RestoreDC(dc,saved);free(label);return;
    }
    SetDCBrushColor(dc,s->palette.light);FillRect(dc,&client,(HBRUSH)GetStockObject(DC_BRUSH));
    frame(dc,client,s->palette.edge,u);
    button=info.rcButton;SetDCBrushColor(dc,s->palette.face);FillRect(dc,&button,(HBRUSH)GetStockObject(DC_BRUSH));
    frame(dc,button,s->palette.edge,u);InflateRect(&button,-u,-u);
    pixel=button;pixel.bottom=pixel.top+u;SetDCBrushColor(dc,pressed?s->palette.shadow:s->palette.light);
    FillRect(dc,&pixel,(HBRUSH)GetStockObject(DC_BRUSH));pixel=button;pixel.right=pixel.left+u;
    FillRect(dc,&pixel,(HBRUSH)GetStockObject(DC_BRUSH));
    SetDCBrushColor(dc,pressed?s->palette.light:s->palette.shadow);pixel=button;pixel.top=pixel.bottom-u;
    FillRect(dc,&pixel,(HBRUSH)GetStockObject(DC_BRUSH));pixel=button;pixel.left=pixel.right-u;
    FillRect(dc,&pixel,(HBRUSH)GetStockObject(DC_BRUSH));
    left=(button.left+button.right-7*u)/2;top=(button.top+button.bottom-9*u)/2;
    if(pressed){left+=u;top+=u;}
    SetDCBrushColor(dc,IsWindowEnabled(w)?s->palette.edge:s->palette.shadow);
    for(y=0;y<9;++y)for(x=0;x<7;++x)if(combo_arrow[y][x]=='0'){
        SetRect(&pixel,left+x*u,top+y*u,left+(x+1)*u,top+(y+1)*u);FillRect(dc,&pixel,(HBRUSH)GetStockObject(DC_BRUSH));}
    if(editable || ntw16s_vb_drive(w)){RestoreDC(dc,saved);free(label);return;}
    text=info.rcItem;selected=GetFocus()==w && !pressed;
    SetDCBrushColor(dc,selected?s->palette.active:s->palette.light);FillRect(dc,&text,(HBRUSH)GetStockObject(DC_BRUSH));
    SelectObject(dc,font?font:GetStockObject(SYSTEM_FONT));SetBkMode(dc,TRANSPARENT);
    SetTextColor(dc,!IsWindowEnabled(w)?s->palette.shadow:selected?s->palette.active_text:s->palette.edge);
    text.left+=2;DrawTextW(dc,label,-1,&text,DT_SINGLELINE|DT_VCENTER|DT_NOPREFIX);
    if(selected)DrawFocusRect(dc,&info.rcItem);
    RestoreDC(dc,saved);free(label);
}

static void detach_combo_list(control_style *s)
{
    DWORD_PTR data;HWND list=s->combo_list;s->combo_list=NULL;
    /* An independently removed/reinstalled child is no longer ours. */
    if(list && GetWindowSubclass(list,control_proc,(UINT_PTR)control_proc,&data) &&
        ((control_style *)data)->owner==s)win16_style_control_remove(list);
}
static BOOL terminal_combo_list(control_style *s)
{
    DWORD_PTR data;HWND list=s->combo_list;s->combo_list=NULL;
    /* An independently removed/reinstalled child is no longer ours. */
    if(list && GetWindowSubclass(list,control_proc,(UINT_PTR)control_proc,&data) &&
        ((control_style *)data)->owner==s)return win16_style_control_terminal(list);
    return TRUE;
}

static void scroll_arrow(HDC dc,RECT r,const char bits[17][18],const win16_style_palette *p)
{
    int x,y;COLORREF colors[4]={p->edge,p->light,p->shadow,p->face};RECT pixel;
    for(y=0;y<17;++y)for(x=0;x<17;++x){
        SetDCBrushColor(dc,colors[bits[y][x]-'0']);
        SetRect(&pixel,r.left+MulDiv(x,r.right-r.left,17),r.top+MulDiv(y,r.bottom-r.top,17),
            r.left+MulDiv(x+1,r.right-r.left,17),r.top+MulDiv(y+1,r.bottom-r.top,17));
        FillRect(dc,&pixel,(HBRUSH)GetStockObject(DC_BRUSH));
    }
}

static BOOL scroll_layout(HWND w,win16_scroll_layout *layout)
{
    RECT r;SCROLLINFO info={.cbSize=sizeof(info),.fMask=SIF_RANGE|SIF_PAGE|SIF_POS};
    int dpi=(int)GetDpiForWindow(w);
    win16_scroll_metrics metrics={MulDiv(17,dpi,96),MulDiv(17,dpi,96),max(1,MulDiv(1,dpi,96))};LONGLONG maximum;
    if(!GetClientRect(w,&r) || !ntw16s_native_get_scroll_info(w,SB_CTL,&info))return FALSE;
    /* An application can choose a nonstandard control thickness. Preserve
     * the original square glyph aspect in both paint and hit-test geometry. */
    metrics.arrow=metrics.thumb=(GetWindowLongPtrW(w,GWL_STYLE)&SBS_VERT)?r.right-r.left:r.bottom-r.top;
    maximum=(LONGLONG)info.nMax-(info.nPage?info.nPage-1:0);
    if(maximum<info.nMin)maximum=info.nMin;
    return win16_scroll_measure(&r,(GetWindowLongPtrW(w,GWL_STYLE)&SBS_VERT)!=0,
        &metrics,info.nMin,(LONG)maximum,info.nPos,layout);
}
static BOOL embedded_scroll_layout(HWND w,LONG object,win16_scroll_layout *layout,POINT *screen_origin)
{
    BOOL vertical=object==OBJID_VSCROLL;
    SCROLLBARINFO bar={.cbSize=sizeof(bar)};SCROLLINFO info={.cbSize=sizeof(info),.fMask=SIF_RANGE|SIF_PAGE|SIF_POS};
    RECT r;int dpi=(int)GetDpiForWindow(w);LONGLONG maximum;
    win16_scroll_metrics metrics={MulDiv(17,dpi,96),MulDiv(17,dpi,96),max(1,MulDiv(1,dpi,96))};
    if(!GetScrollBarInfo(w,vertical?OBJID_VSCROLL:OBJID_HSCROLL,&bar) || (bar.rgstate[0]&STATE_SYSTEM_INVISIBLE) ||
        !ntw16s_native_get_scroll_info(w,vertical?SB_VERT:SB_HORZ,&info))return FALSE;
    SetRect(&r,0,0,bar.rcScrollBar.right-bar.rcScrollBar.left,bar.rcScrollBar.bottom-bar.rcScrollBar.top);
    metrics.arrow=metrics.thumb=vertical?r.right:r.bottom;
    screen_origin->x=bar.rcScrollBar.left;screen_origin->y=bar.rcScrollBar.top;
    maximum=(LONGLONG)info.nMax-(info.nPage?info.nPage-1:0);if(maximum<info.nMin)maximum=info.nMin;
    return win16_scroll_measure(&r,vertical,&metrics,info.nMin,(LONG)maximum,info.nPos,layout);
}

static BOOL horizontal_thumb_messages(HWND w)
{
    SCROLLINFO info={.cbSize=sizeof(info),.fMask=SIF_RANGE|SIF_PAGE};
    return ntw16s_native_get_scroll_info(w,SB_HORZ,&info) && info.nMin>=0 &&
        (LONGLONG)info.nMax-(info.nPage?info.nPage-1:0)<=0xffff;
}
static BOOL list_axis_owned(HWND w,LONG object)
{
    if(GetWindowLongPtrW(w,GWL_STYLE)&LBS_MULTICOLUMN)return object==OBJID_HSCROLL;
    return object==OBJID_VSCROLL || (object==OBJID_HSCROLL && horizontal_thumb_messages(w));
}
static BOOL edit_thumb_messages(HWND w,LONG object)
{
    /* Above this native EDIT boundary, thumb messages ignore HIWORD and read
     * USER-owned nTrackPos, which this public-API adapter cannot assign. */
    LRESULT lines=SendMessageW(w,EM_GETLINECOUNT,0,0);
    if(lines<=0 || lines>=0xffff)return FALSE;
    if(object==OBJID_HSCROLL && !horizontal_thumb_messages(w))return FALSE;
    return TRUE;
}

static BOOL draw_scroll_pixels(HWND w,control_style *s,HDC dc,LONG object,BOOL vertical)
{
    SCROLLBARINFO info={.cbSize=sizeof(info)};RECT r,first,last,thumb,pixel;int saved;
    if(!dc || !GetScrollBarInfo(w,object,&info) || (info.rgstate[0]&STATE_SYSTEM_INVISIBLE))return FALSE;
    saved=SaveDC(dc);if(!saved)return FALSE;
    if(object==OBJID_CLIENT)GetClientRect(w,&r);
    else {RECT outer;GetWindowRect(w,&outer);
        OffsetViewportOrgEx(dc,info.rcScrollBar.left-outer.left,info.rcScrollBar.top-outer.top,NULL);
        SetRect(&r,0,0,info.rcScrollBar.right-info.rcScrollBar.left,info.rcScrollBar.bottom-info.rcScrollBar.top);}
    IntersectClipRect(dc,r.left,r.top,r.right,r.bottom);
    /* Native state remains authoritative; opted-in bars replace geometry
     * below with the same fixed-thumb layout used for hit testing. */
    {static const WORD checks[8]={0x55,0xaa,0x55,0xaa,0x55,0xaa,0x55,0xaa};
        HBITMAP bitmap=CreateBitmap(8,8,1,1,checks);HBRUSH brush=bitmap?CreatePatternBrush(bitmap):NULL;
        SetTextColor(dc,s->palette.face);SetBkColor(dc,s->palette.light);
        if(brush){FillRect(dc,&r,brush);DeleteObject(brush);}
        if(bitmap)DeleteObject(bitmap);
        if(!brush){RestoreDC(dc,saved);return FALSE;}}
    first=last=thumb=r;
    if(vertical){first.bottom=first.top+info.dxyLineButton;last.top=last.bottom-info.dxyLineButton;
        thumb.top=info.xyThumbTop;thumb.bottom=info.xyThumbBottom;}
    else {first.right=first.left+info.dxyLineButton;last.left=last.right-info.dxyLineButton;
        thumb.left=info.xyThumbTop;thumb.right=info.xyThumbBottom;}
    if(object==OBJID_CLIENT || s->frame_scrollbars || (s->edit_scrollbars && edit_thumb_messages(w,object)) || (s->list_scrollbar &&
        list_axis_owned(w,object))){win16_scroll_layout layout;POINT origin;
        if(object==OBJID_CLIENT?scroll_layout(w,&layout):embedded_scroll_layout(w,object,&layout,&origin)){
            first=layout.first;last=layout.last;thumb=layout.thumb;
            if(s->scroll.active && (object==OBJID_CLIENT || object==s->embedded_object) && s->scroll.part==WIN16_SCROLL_THUMB){
                if(vertical)OffsetRect(&thumb,0,s->scroll.pixel-thumb.top);
                else OffsetRect(&thumb,s->scroll.pixel-thumb.left,0);}
            info.xyThumbBottom=layout.thumb_fits?1:0;info.xyThumbTop=0;
            info.rgstate[1]&=~STATE_SYSTEM_PRESSED;info.rgstate[5]&=~STATE_SYSTEM_PRESSED;
            if(s->scroll.active && (object==OBJID_CLIENT || object==s->embedded_object) && s->scroll.inside){
                if(s->scroll.part==WIN16_SCROLL_LINE_BEFORE)info.rgstate[1]|=STATE_SYSTEM_PRESSED;
                if(s->scroll.part==WIN16_SCROLL_LINE_AFTER)info.rgstate[5]|=STATE_SYSTEM_PRESSED;}
        }
    }
    frame(dc,r,s->palette.edge,1);
    scroll_arrow(dc,first,(info.rgstate[1]&STATE_SYSTEM_UNAVAILABLE)?(vertical?scroll_uparrowi:scroll_lfarrowi):
        vertical?(info.rgstate[1]&STATE_SYSTEM_PRESSED?scroll_uparrowd:scroll_uparrow):
        (info.rgstate[1]&STATE_SYSTEM_PRESSED?scroll_lfarrowd:scroll_lfarrow),&s->palette);
    scroll_arrow(dc,last,(info.rgstate[5]&STATE_SYSTEM_UNAVAILABLE)?(vertical?scroll_dnarrowi:scroll_rgarrowi):
        vertical?(info.rgstate[5]&STATE_SYSTEM_PRESSED?scroll_dnarrowd:scroll_dnarrow):
        (info.rgstate[5]&STATE_SYSTEM_PRESSED?scroll_rgarrowd:scroll_rgarrow),&s->palette);
    if(info.xyThumbBottom>info.xyThumbTop){
        SetDCBrushColor(dc,s->palette.face);FillRect(dc,&thumb,(HBRUSH)GetStockObject(DC_BRUSH));
        frame(dc,thumb,s->palette.edge,1);InflateRect(&thumb,-1,-1);
        SetDCBrushColor(dc,s->palette.light);pixel=thumb;pixel.bottom=pixel.top+1;FillRect(dc,&pixel,(HBRUSH)GetStockObject(DC_BRUSH));
        pixel=thumb;pixel.right=pixel.left+1;FillRect(dc,&pixel,(HBRUSH)GetStockObject(DC_BRUSH));
        SetDCBrushColor(dc,s->palette.shadow);pixel=thumb;pixel.top=pixel.bottom-1;FillRect(dc,&pixel,(HBRUSH)GetStockObject(DC_BRUSH));
        pixel=thumb;pixel.left=pixel.right-1;FillRect(dc,&pixel,(HBRUSH)GetStockObject(DC_BRUSH));
    }
    RestoreDC(dc,saved);return TRUE;
}

static void draw_scroll(HWND w,control_style *s,HDC dc,LONG object,BOOL vertical)
{
    RECT r;SCROLLBARINFO bar={.cbSize=sizeof(bar)};HDC memory;HBITMAP bitmap,old;POINT brush,device;BOOL painted;
    if(!dc || !GetScrollBarInfo(w,object,&bar) || (bar.rgstate[0]&STATE_SYSTEM_INVISIBLE))return;
    if(object==OBJID_CLIENT)GetClientRect(w,&r);
    else {RECT window;GetWindowRect(w,&window);r=bar.rcScrollBar;OffsetRect(&r,-window.left,-window.top);}
    if(IsRectEmpty(&r))return;
    memory=CreateCompatibleDC(dc);bitmap=CreateCompatibleBitmap(dc,r.right-r.left,r.bottom-r.top);
    if(!memory || !bitmap){if(memory)DeleteDC(memory);if(bitmap)DeleteObject(bitmap);
        draw_scroll_pixels(w,s,dc,object,vertical);return;}
    old=(HBITMAP)SelectObject(memory,bitmap);
    if(!old || (HGDIOBJ)old==HGDI_ERROR){DeleteObject(bitmap);DeleteDC(memory);
        draw_scroll_pixels(w,s,dc,object,vertical);return;}
    /* Preserve the checker phase while composing only this bar. One BitBlt
     * prevents the background erase and subsequent thumb draw being visible
     * as separate states. No persistent buffer or redraw timer is needed. */
    device.x=r.left;device.y=r.top;LPtoDP(dc,&device,1);GetBrushOrgEx(dc,&brush);
    SetBrushOrgEx(memory,brush.x-device.x,brush.y-device.y,NULL);
    SetViewportOrgEx(memory,-r.left,-r.top,NULL);
    painted=draw_scroll_pixels(w,s,memory,object,vertical);
    SetViewportOrgEx(memory,0,0,NULL);
    /* A failed painter has no complete image to publish. Keep the previous
     * target pixels rather than copying an uninitialized temporary bitmap. */
    if(painted)BitBlt(dc,r.left,r.top,r.right-r.left,r.bottom-r.top,memory,0,0,SRCCOPY);
    SelectObject(memory,old);DeleteObject(bitmap);DeleteDC(memory);
}

static void nonclient_scrollbars(HWND w,control_style *s,HDC target)
{
    DWORD style=(DWORD)GetWindowLongPtrW(w,GWL_STYLE);HDC dc;
    if(!(style&(WS_HSCROLL|WS_VSCROLL)))return;
    dc=target?target:GetWindowDC(w);if(!dc)return;
    if(style&WS_HSCROLL)draw_scroll(w,s,dc,OBJID_HSCROLL,FALSE);
    if(style&WS_VSCROLL)draw_scroll(w,s,dc,OBJID_VSCROLL,TRUE);
    if(!target)ReleaseDC(w,dc);
}

typedef struct list_view_state {
    LRESULT top,selection,caret;
} list_view_state;
static list_view_state list_view(HWND w)
{
    list_view_state v;
    v.top=SendMessageW(w,LB_GETTOPINDEX,0,0);
    v.selection=SendMessageW(w,LB_GETCURSEL,0,0);
    v.caret=SendMessageW(w,LB_GETCARETINDEX,0,0);return v;
}

void win16_style_scrollbars_paint(HWND w,HDC target,const win16_style_palette *palette)
{
    control_style s={0};DWORD_PTR attached;
    if(palette)s.palette=*palette;else win16_style_default_palette(&s.palette);
    if(GetWindowSubclass(w,control_proc,(UINT_PTR)control_proc,&attached)){
        control_style *owner=(control_style *)attached;
        if(!palette)s.palette=owner->palette;
        s.list_scrollbar=owner->list_scrollbar;s.edit_scrollbars=owner->edit_scrollbars;s.scroll=owner->scroll;
        s.frame_scrollbars=owner->frame_scrollbars;
        s.embedded_object=owner->embedded_object;
    }
    nonclient_scrollbars(w,&s,target);
}

static UINT scroll_disabled(HWND w,LONG object)
{
    SCROLLBARINFO info={.cbSize=sizeof(info)};UINT disabled=0;
    if(!IsWindowEnabled(w) || !GetScrollBarInfo(w,object,&info))return ESB_DISABLE_BOTH;
    if(info.rgstate[1]&STATE_SYSTEM_UNAVAILABLE)disabled|=ESB_DISABLE_LTUP;
    if(info.rgstate[5]&STATE_SYSTEM_UNAVAILABLE)disabled|=ESB_DISABLE_RTDN;
    return disabled;
}
static void scroll_caret_destroy(HWND w,control_style *s)
{
    GUITHREADINFO info={.cbSize=sizeof(info)};
    if(s->scroll_caret && GetGUIThreadInfo(GetCurrentThreadId(),&info) && info.hwndCaret==w)DestroyCaret();
    s->scroll_caret=s->scroll_caret_shown=FALSE;
}
static void scroll_caret_sync(HWND w,control_style *s)
{
    win16_scroll_layout layout;RECT r;int inset=max(1,MulDiv(2,(int)GetDpiForWindow(w),96));
    if(s->detached || !is_scroll(w))return;
    if(GetFocus()!=w || !IsWindowEnabled(w) || !scroll_layout(w,&layout) || !layout.thumb_fits){
        scroll_caret_destroy(w,s);return;}
    r=layout.thumb;
    if(s->scroll.active && s->scroll.part==WIN16_SCROLL_THUMB){
        if(layout.vertical)OffsetRect(&r,0,s->scroll.pixel-r.top);
        else OffsetRect(&r,s->scroll.pixel-r.left,0);}
    InflateRect(&r,-inset,-inset);
    if(IsRectEmpty(&r)){scroll_caret_destroy(w,s);return;}
    if(s->scroll_caret && (s->scroll_caret_size.cx!=r.right-r.left || s->scroll_caret_size.cy!=r.bottom-r.top))
        scroll_caret_destroy(w,s);
    if(!s->scroll_caret){
        if(!CreateCaret(w,(HBITMAP)1,r.right-r.left,r.bottom-r.top))return;
        s->scroll_caret=TRUE;s->scroll_caret_size.cx=r.right-r.left;s->scroll_caret_size.cy=r.bottom-r.top;}
    SetCaretPos(r.left,r.top);
    if(s->scroll.active && s->scroll_caret_shown){HideCaret(w);s->scroll_caret_shown=FALSE;}
    else if(!s->scroll.active && !s->scroll_caret_shown){ShowCaret(w);s->scroll_caret_shown=TRUE;}
}
static void scroll_stop_timer(HWND w,control_style *s)
{if(s->scroll_timer){KillTimer(w,s->scroll_timer);s->scroll_timer=0;}}
static void scroll_actions(HWND w,control_style *s,const win16_scroll_actions *actions,unsigned epoch)
{
    UINT i;BOOL reporting=s->scroll_reporting;LONG previous=s->scroll_report;
    if(s->detached || s->scroll_epoch!=epoch)return;
    if(actions->timer_ms!=WIN16_SCROLL_KEEP_TIMER){
        scroll_stop_timer(w,s);
        if(actions->timer_ms && s->scroll.active)
            s->scroll_timer=SetTimer(w,(UINT_PTR)&s->scroll,actions->timer_ms,NULL);
    }
    for(i=0;i<actions->count;++i){
        HWND parent=GetParent(w);UINT message=(GetWindowLongPtrW(w,GWL_STYLE)&SBS_VERT)?WM_VSCROLL:WM_HSCROLL;
        s->scroll_reporting=TRUE;s->scroll_report=actions->notices[i].position;
        if(s->frame_scrollbars){
            SendMessageW(w,s->embedded_object==OBJID_HSCROLL?WM_HSCROLL:WM_VSCROLL,
                MAKEWPARAM(actions->notices[i].command,(WORD)s->scroll_report),0);
        }else if(s->edit_scrollbars){UINT command=actions->notices[i].command;
            SendMessageW(w,s->embedded_object==OBJID_HSCROLL?WM_HSCROLL:WM_VSCROLL,MAKEWPARAM(command,(WORD)s->scroll_report),0);
        }else if(s->list_scrollbar){UINT command=actions->notices[i].command;
            BOOL multi=(GetWindowLongPtrW(w,GWL_STYLE)&LBS_MULTICOLUMN)!=0;
            if(!multi && s->embedded_object==OBJID_HSCROLL)
                SendMessageW(w,WM_HSCROLL,MAKEWPARAM(command,(WORD)s->scroll_report),0);
            else if(command==SB_THUMBTRACK || command==SB_THUMBPOSITION){
                LONGLONG index=s->scroll_report;
                if(multi){RECT client;LRESULT height=SendMessageW(w,LB_GETITEMHEIGHT,0,0);
                    if(s->detached || s->scroll_epoch!=epoch)return;
                    if(height<=0 || !GetClientRect(w,&client))return;
                    index*=max(1,client.bottom/(LONG)height);
                }
                if(index>=0 && index<=MAXLONG)SendMessageW(w,LB_SETTOPINDEX,(WPARAM)index,0);
            }else SendMessageW(w,multi?WM_HSCROLL:WM_VSCROLL,MAKEWPARAM(command,0),0);
        }else if(parent)SendMessageW(parent,message,MAKEWPARAM(actions->notices[i].command,(WORD)s->scroll_report),(LPARAM)w);
        s->scroll_reporting=reporting;s->scroll_report=previous;
        /* Calls may destroy/remove the attachment, or start another gesture. */
        if(s->detached || s->scroll_epoch!=epoch)return;
    }
    if(actions->repaint && s->frame_scrollbars){HDC dc=GetWindowDC(w);
        if(dc){draw_scroll(w,s,dc,s->embedded_object,s->embedded_object==OBJID_VSCROLL);ReleaseDC(w,dc);}}
    else if(actions->repaint){RedrawWindow(w,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW);
        if(!s->detached && (s->list_scrollbar || s->edit_scrollbars))nonclient_scrollbars(w,s,NULL);}
    scroll_caret_sync(w,s);
}
static void scroll_release_capture(HWND w,control_style *s)
{
    HWND previous=s->list_capture_before;s->list_capture_before=NULL;
    /* Do not release capture borrowed from the combo's own popup tracking,
     * and never take capture back from a window that already acquired it. */
    if(GetCapture()==w){
        if((s->list_scrollbar || s->edit_scrollbars || s->frame_scrollbars) && previous && IsWindow(previous) &&
            GetWindowThreadProcessId(previous,NULL)==GetCurrentThreadId()){
            if(previous!=w)SetCapture(previous);
        }else ReleaseCapture();
    }
}
static void scroll_finish(HWND w,control_style *s,BOOL cancel)
{
    win16_scroll_actions actions;unsigned epoch=s->scroll_epoch;
    if(!s->scroll.active)return;
    win16_scroll_end(&s->scroll,cancel,&actions);scroll_stop_timer(w,s);
    scroll_release_capture(w,s);
    scroll_actions(w,s,&actions,epoch);
}
static BOOL scroll_input(HWND w,control_style *s,UINT m,WPARAM a,LPARAM b)
{
    win16_scroll_layout layout;win16_scroll_actions actions;POINT point;
    point.x=(short)LOWORD(b);point.y=(short)HIWORD(b);
    if(m==WM_LBUTTONDOWN || m==WM_LBUTTONDBLCLK){
        RECT limit;unsigned epoch;
        if(!IsWindowEnabled(w) || s->scroll.active || !scroll_layout(w,&layout))return TRUE;
        /* USER18:17AE inflates the original bar by 4*CXBORDER, CYBORDER.
         * Do not substitute the much wider later-NT drag tolerance. */
        limit=layout.bar;{int unit=max(1,MulDiv(1,(int)GetDpiForWindow(w),96));InflateRect(&limit,4*unit,unit);}
        if(!win16_scroll_begin(&s->scroll,&layout,scroll_disabled(w,OBJID_CLIENT),point,&limit,&actions))return TRUE;
        epoch=++s->scroll_epoch;SetFocus(w);
        if(s->detached || epoch!=s->scroll_epoch || !s->scroll.active)return TRUE;
        SetCapture(w);
        if(s->detached || epoch!=s->scroll_epoch)return TRUE;
        if(GetCapture()!=w){scroll_finish(w,s,TRUE);return TRUE;}
        scroll_actions(w,s,&actions,epoch);return TRUE;
    }
    if(m==WM_MOUSEMOVE || (m==WM_TIMER && s->scroll_timer && a==s->scroll_timer)){
        if(s->scroll.active && scroll_layout(w,&layout)){
            if(m==WM_TIMER){if(!GetCursorPos(&point) || !ScreenToClient(w,&point))return TRUE;}
            win16_scroll_motion(&s->scroll,&layout,scroll_disabled(w,OBJID_CLIENT),point,m==WM_TIMER,&actions);
            scroll_actions(w,s,&actions,s->scroll_epoch);
        }return TRUE;
    }
    if(m==WM_LBUTTONUP || m==WM_CANCELMODE || m==WM_CAPTURECHANGED){
        scroll_finish(w,s,m!=WM_LBUTTONUP);return TRUE;
    }
    if(m==WM_MOUSELEAVE)return TRUE;
    if(m==WM_KEYDOWN){UINT command;
        if(!IsWindowEnabled(w))return TRUE;
        switch(a){case VK_LEFT:case VK_UP:command=SB_LINEUP;break;
        case VK_RIGHT:case VK_DOWN:command=SB_LINEDOWN;break;
        case VK_PRIOR:command=SB_PAGEUP;break;case VK_NEXT:command=SB_PAGEDOWN;break;
        case VK_HOME:command=SB_TOP;break;case VK_END:command=SB_BOTTOM;break;
        default:return FALSE;}
        ZeroMemory(&actions,sizeof(actions));actions.timer_ms=WIN16_SCROLL_KEEP_TIMER;
        actions.count=1;actions.notices[0].command=command;actions.repaint=TRUE;s->scroll_key=TRUE;
        scroll_actions(w,s,&actions,s->scroll_epoch);return TRUE;
    }
    if(m==WM_KEYUP && s->scroll_key){
        s->scroll_key=FALSE;ZeroMemory(&actions,sizeof(actions));actions.timer_ms=WIN16_SCROLL_KEEP_TIMER;
        actions.count=1;actions.notices[0].command=SB_ENDSCROLL;
        scroll_actions(w,s,&actions,s->scroll_epoch);return TRUE;
    }
    return FALSE;
}

static BOOL embedded_scroll_input(HWND w,control_style *s,UINT m,WPARAM a,LPARAM b)
{
    win16_scroll_layout layout;win16_scroll_actions actions;POINT point,origin;
    BOOL multi=s->list_scrollbar && (GetWindowLongPtrW(w,GWL_STYLE)&LBS_MULTICOLUMN)!=0;
    LONG object=multi?OBJID_HSCROLL:OBJID_VSCROLL;
    BOOL down=m==WM_NCLBUTTONDOWN || m==WM_NCLBUTTONDBLCLK;
    if(!s->list_scrollbar && !s->edit_scrollbars && !s->frame_scrollbars)return FALSE;
    /* A dropped combo already owns capture: its bar receives CLIENT clicks.
     * Native LISTBOX normally releases capture, calls the synchronous NC
     * tracker, then takes capture back. Our tracker returns between messages;
     * entering it through that native wrapper loses the borrowed ownership.
     * Normalize bar hits here, leaving item/outside clicks with native USER. */
    if(s->owner && s->list_scrollbar && !s->scroll.active && GetCapture()==w &&
        (m==WM_LBUTTONDOWN || m==WM_LBUTTONDBLCLK)){
        LRESULT hit;
        point.x=(short)LOWORD(b);point.y=(short)HIWORD(b);ClientToScreen(w,&point);
        hit=SendMessageW(w,WM_NCHITTEST,0,MAKELPARAM(point.x,point.y));
        if(s->detached)return TRUE;
        if(hit==HTVSCROLL || hit==HTHSCROLL){
            down=TRUE;a=(WPARAM)hit;b=MAKELPARAM(point.x,point.y);
        }
    }
    if(s->scroll.active)object=s->embedded_object;
    else if(down)object=a==HTHSCROLL?OBJID_HSCROLL:OBJID_VSCROLL;
    /* Querying EDIT's line count dispatches back through this subclass.
     * Only input/tracking messages may enter the eligibility query. */
    if(!down && m!=WM_MOUSEMOVE && m!=WM_LBUTTONUP && m!=WM_CANCELMODE &&
        m!=WM_CAPTURECHANGED && !(m==WM_TIMER && s->scroll_timer && a==s->scroll_timer))return FALSE;
    if((s->edit_scrollbars && !edit_thumb_messages(w,object)) ||
        (s->list_scrollbar && !list_axis_owned(w,object))){
        if(s->scroll.active)scroll_finish(w,s,TRUE);
        return FALSE; /* Keep native wide-text input, not truncated positioning. */
    }
    if(down && a==(WPARAM)(object==OBJID_HSCROLL?HTHSCROLL:HTVSCROLL)){RECT limit;unsigned epoch;
        if(s->scroll.active || !IsWindowEnabled(w) || !embedded_scroll_layout(w,object,&layout,&origin))return TRUE;
        point.x=(short)LOWORD(b)-origin.x;point.y=(short)HIWORD(b)-origin.y;
        limit=layout.bar;{int u=max(1,MulDiv(1,(int)GetDpiForWindow(w),96));InflateRect(&limit,4*u,u);}
        if(!win16_scroll_begin(&s->scroll,&layout,scroll_disabled(w,object),point,&limit,&actions))return TRUE;
        epoch=++s->scroll_epoch;s->embedded_object=object;s->list_capture_before=GetCapture();
        if(s->list_capture_before!=w)SetCapture(w);
        if(s->detached || s->scroll_epoch!=epoch)return TRUE;
        if(GetCapture()!=w){scroll_finish(w,s,TRUE);return TRUE;}
        scroll_actions(w,s,&actions,epoch);return TRUE;
    }
    if(!s->scroll.active)return FALSE;
    if(m==WM_MOUSEMOVE || (m==WM_TIMER && a==s->scroll_timer)){
        if(!embedded_scroll_layout(w,object,&layout,&origin)){scroll_finish(w,s,TRUE);return TRUE;}
        if(m==WM_TIMER){if(!GetCursorPos(&point))return TRUE;}
        else{point.x=(short)LOWORD(b);point.y=(short)HIWORD(b);ClientToScreen(w,&point);}
        point.x-=origin.x;point.y-=origin.y;
        win16_scroll_motion(&s->scroll,&layout,scroll_disabled(w,object),point,m==WM_TIMER,&actions);
        scroll_actions(w,s,&actions,s->scroll_epoch);return TRUE;
    }
    if(m==WM_LBUTTONUP || m==WM_CANCELMODE || m==WM_CAPTURECHANGED){
        scroll_finish(w,s,m!=WM_LBUTTONUP);return TRUE;}
    return FALSE;
}

/* Only screen WM_PAINT uses this transient surface. Keep WM_PRINTCLIENT on
 * its borrowed DC. Seed the dirty rectangle so group interiors and native
 * combo EDIT/list children excluded by the painter remain unchanged. */
static void present_control(HWND w,control_style *s,HDC target,
    void (*painter)(HWND,control_style *,HDC))
{
    RECT dirty,client;HDC memory=NULL;HBITMAP bitmap=NULL,old=NULL;BOOL ready=FALSE;
    BOOL group;int target_saved=0;
    if(!target || GetClipBox(target,&dirty)==ERROR || !GetClientRect(w,&client) ||
        !IntersectRect(&dirty,&dirty,&client))return;
    group=ntw16s_class_is(w,L"BUTTON") &&
        (GetWindowLongPtrW(w,GWL_STYLE)&BS_TYPEMASK)==BS_GROUPBOX;
    if(group){HWND sibling;RECT r;
        target_saved=SaveDC(target);if(!target_saved)return;
        /* Group interiors belong to the parent background. Never overwrite
         * the sibling controls visually enclosed by this transparent group. */
        for(sibling=GetWindow(GetParent(w),GW_CHILD);sibling;sibling=GetWindow(sibling,GW_HWNDNEXT))
            if(sibling!=w && IsWindowVisible(sibling) && GetWindowRect(sibling,&r)){
                MapWindowPoints(NULL,w,(POINT *)&r,2);
                ExcludeClipRect(target,r.left,r.top,r.right,r.bottom);}
    }
    memory=CreateCompatibleDC(target);
    if(memory)bitmap=CreateCompatibleBitmap(target,dirty.right-dirty.left,dirty.bottom-dirty.top);
    if(bitmap)old=(HBITMAP)SelectObject(memory,bitmap);
    if(old && old!=(HBITMAP)HGDI_ERROR &&
        BitBlt(memory,0,0,dirty.right-dirty.left,dirty.bottom-dirty.top,target,dirty.left,dirty.top,SRCCOPY) &&
        SetWindowOrgEx(memory,dirty.left,dirty.top,NULL)){
        if(group){HWND parent=GetParent(w);POINT origin={0,0};int saved=SaveDC(memory);
            SetDCBrushColor(memory,s->palette.light);FillRect(memory,&client,(HBRUSH)GetStockObject(DC_BRUSH));
            if(saved && parent){MapWindowPoints(w,parent,&origin,1);
                OffsetViewportOrgEx(memory,-origin.x,-origin.y,NULL);
                SendMessageW(parent,WM_PRINTCLIENT,(WPARAM)memory,PRF_CLIENT);}
            if(saved)RestoreDC(memory,saved);
        }
        if(!s->detached)painter(w,s,memory);
        if(!s->detached)ready=BitBlt(target,dirty.left,dirty.top,dirty.right-dirty.left,
            dirty.bottom-dirty.top,memory,dirty.left,dirty.top,SRCCOPY);
    }
    if(old && old!=(HBITMAP)HGDI_ERROR)SelectObject(memory,old);
    if(bitmap)DeleteObject(bitmap);
    if(memory)DeleteDC(memory);
    if(!ready && !s->detached)painter(w,s,target);
    if(target_saved)RestoreDC(target,target_saved);
}

/* Original LBSetTabStops stores set-time pixels. GdiGetCharDimensions uses
 * the alphabet average for proportional fonts, not tmAveCharWidth. Keep
 * that public-API arithmetic here; private PLBIV/CFONT are not imported. */
static LRESULT set_list_tabs(HWND w,control_style *s,WPARAM count,LPARAM values)
{
    int *pixels=NULL,average,i;LRESULT result;
    if(count>INT_MAX || (count && !values) || count>((size_t)-1)/sizeof(int))return FALSE;
    if(count){HDC dc;HFONT old;TEXTMETRICW tm;SIZE size;
        pixels=malloc((size_t)count*sizeof(int));if(!pixels)return FALSE;
        dc=GetDC(w);if(!dc){free(pixels);return FALSE;}
        old=SelectObject(dc,s->font);
        if(!GetTextMetricsW(dc,&tm)){SelectObject(dc,old);ReleaseDC(w,dc);free(pixels);return FALSE;}
        average=tm.tmAveCharWidth;
        if(tm.tmPitchAndFamily&TMPF_FIXED_PITCH){
            if(!GetTextExtentPoint32W(dc,L"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ",52,&size)){
                SelectObject(dc,old);ReleaseDC(w,dc);free(pixels);return FALSE;}
            average=((size.cx/26)+1)/2;
        }
        SelectObject(dc,old);ReleaseDC(w,dc);
        for(i=0;i<(int)count;++i){LONGLONG value=((LONGLONG)((const int *)values)[i]*average+2)/4;
            if(value<INT_MIN || value>INT_MAX){free(pixels);return FALSE;}pixels[i]=(int)value;}
    }
    result=DefSubclassProc(w,LB_SETTABSTOPS,count,values);
    if(result && !s->detached){free(s->tab_positions);s->tab_positions=pixels;s->tab_count=(int)count;}
    else free(pixels);
    return result;
}
/* USER18:0208..027F: scrollbar-class size box shares the inner border
 * and fills with the system scrollbar brush, not a later sizing-grip image.
 * Native creation/alignment and parent size/maximize commands stay native. */
static void draw_sizebox(HWND w,control_style *s,HDC dc)
{
    RECT outer,client,r;UINT dpi=GetDpiForWindow(w);int saved,cx,cy,bx,by;
    (void)s;
    if(!dc || !GetWindowRect(w,&outer) || !GetClientRect(w,&client))return;
    saved=SaveDC(dc);if(!saved)return;
    cx=GetSystemMetricsForDpi(SM_CXVSCROLL,dpi);cy=GetSystemMetricsForDpi(SM_CYHSCROLL,dpi);
    bx=GetSystemMetricsForDpi(SM_CXBORDER,dpi);by=GetSystemMetricsForDpi(SM_CYBORDER,dpi);
    SetRect(&r,outer.right-outer.left-cx+bx,outer.bottom-outer.top-cy+by,
        outer.right-outer.left,outer.bottom-outer.top);
    IntersectClipRect(dc,client.left,client.top,client.right,client.bottom);
    FillRect(dc,&r,GetSysColorBrush(COLOR_SCROLLBAR));RestoreDC(dc,saved);
}
static void draw_tab(HWND w,control_style *s,HDC dc)
{
    HFONT font=s->font;win16_style_system_retain(font);
    if(!win16_style_tab_paint(w,dc,&s->palette,font,&s->detached,&s->tab_epoch) && !s->detached)
        DefSubclassProc(w,WM_PRINTCLIENT,(WPARAM)dc,PRF_CLIENT);
    win16_style_system_release(font);
}
static void draw_header(HWND w,control_style *s,HDC dc)
{
    HFONT font=s->font;win16_style_system_retain(font);
    if(!win16_style_header_paint(w,dc,&s->palette,font,&s->detached,&s->header_epoch,s->header_pressed) && !s->detached)
        DefSubclassProc(w,WM_PRINTCLIENT,(WPARAM)dc,PRF_CLIENT);
    win16_style_system_release(font);
}
static void draw_progress(HWND w,control_style *s,HDC dc)
{
    if(!win16_style_progress_paint(w,dc,&s->palette,&s->detached,&s->progress_epoch) && !s->detached)
        DefSubclassProc(w,WM_PRINTCLIENT,(WPARAM)dc,PRF_CLIENT);
}
/* The v5 progress color and state getters are unavailable. Only paint the
 * verified static edge, after the native procedure; leave the client intact. */
static void legacy_progress_border(HWND w,control_style *s,HDC target)
{
    RECT outer,client;POINT origin={0,0};DWORD saved_error=GetLastError();
    DWORD style=(DWORD)GetWindowLongPtrW(w,GWL_STYLE),ex=(DWORD)GetWindowLongPtrW(w,GWL_EXSTYLE);
    if(!ntw16s_control_protocol_legacy_progress(w) ||
        !(ex&WS_EX_STATICEDGE) || (ex&(WS_EX_LAYOUTRTL|WS_EX_RTLREADING|WS_EX_CLIENTEDGE)) ||
        (style&(PBS_MARQUEE|WS_HSCROLL|WS_VSCROLL)) ||
        !GetWindowRect(w,&outer) || !GetClientRect(w,&client) || !ClientToScreen(w,&origin))goto done;
    int width=outer.right-outer.left,height=outer.bottom-outer.top;
    int left=origin.x-outer.left,top=origin.y-outer.top;
    if(width<3 || height<3 || left<1 || top<1 || left+client.right>=width || top+client.bottom>=height)goto done;
    RECT bounds={0,0,width,height};HDC dc=target?target:GetWindowDC(w);if(!dc)goto done;
    int saved=SaveDC(dc);if(saved){frame(dc,bounds,s->palette.edge,1);RestoreDC(dc,saved);}
    if(!target)ReleaseDC(w,dc);
done: SetLastError(saved_error);
}
static LRESULT CALLBACK control_proc(HWND w,UINT m,WPARAM a,LPARAM b,UINT_PTR id,DWORD_PTR data)
{
    control_style *s=(control_style *)data;LRESULT result=0;HFONT previous=NULL,pinned=s->font,outbound=NULL;(void)id;++s->calls;
    BOOL tab=is_tab(w),header=is_header(w),progress=ntw16s_class_is(w,PROGRESS_CLASSW) || ntw16s_control_protocol_is(w,PROGRESS_CLASSW),native_font=tab || header;
    BOOL combo_paint=ntw16s_vb_combo(w) && (m==WM_PAINT || m==WM_PRINT || m==WM_PRINTCLIENT);if(combo_paint)++s->combo_paints;
    if(progress && win16_style_progress_changes(m))++s->progress_epoch;
    if(tab && win16_style_tab_changes(m))++s->tab_epoch;
    if(header && win16_style_header_changes(m))++s->header_epoch;
    win16_style_system_retain(pinned);
    /* A verified VCL control belongs only to its original parent and role.
     * Let the native transition finish, then retire this exact attachment. */
    if(m!=WM_NCDESTROY && (!ntw16s_group_button_valid(w) || !ntw16s_grid_valid(w) || !ntw16s_vcl_toolbar_valid(w) || !ntw16s_vcl_tab_valid(w))){
        if(m==WM_SETFONT)s->original_font=win16_style_system_source((HFONT)a);
        result=DefSubclassProc(w,m,a,b);
        if(!s->detached && IsWindow(w) && Ntw16sWindowRemove(w) && IsWindow(w))InvalidateRect(w,NULL,TRUE);
        goto finished;
    }
    /* Unverified graphical/owner-drawn VB variants stay native. A live
     * transition retires only this painter; do not keep drawing a plain box. */
    if(m!=WM_NCDESTROY && (ntw16s_vb_checkbox(w) || ntw16s_vb_option(w)) &&
            !ntw16s_vb_plain_choice(w) && !ntw16s_vb_graphical_choice(w) && !ntw16s_vb_graphical_button(w)){
        if(m==WM_SETFONT)s->original_font=win16_style_system_source((HFONT)a);
        result=DefSubclassProc(w,m,a,b);
        if(!s->detached)win16_style_control_remove(w);
        goto finished;
    }
    /* VB owns an IFont and can change it without WM_SETFONT. Query at the
     * paint boundary; never overwrite that application's font property. */
    if(!s->frame_scrollbars && (m==WM_PAINT || m==WM_PRINT || m==WM_PRINTCLIENT) &&
            (native_font || ntw16s_vb_frame(w) || ntw16s_vb_edit(w) || ntw16s_vb_button(w) || ntw16s_vb_combo(w) || ntw16s_vb_list(w))){
        HFONT source=(HFONT)SendMessageW(w,WM_GETFONT,0,0);
        if(!s->detached && source!=s->original_font){
            HFONT replacement=native_font?win16_style_system_acquire(source):win16_style_vb_font_acquire(source);
            previous=s->font;s->original_font=source;s->font=replacement?replacement:source;
        }
    }
    if(s->detached)goto finished;
    if(m==WM_SETFONT && !s->frame_scrollbars){
        HFONT original=win16_style_system_source((HFONT)a);
        BOOL vb=ntw16s_vb_frame(w) || ntw16s_vb_edit(w) || ntw16s_vb_button(w) || ntw16s_vb_combo(w) || ntw16s_vb_list(w);
        HFONT replacement=vb?win16_style_vb_font_acquire(original):win16_style_system_acquire(original);
        previous=s->font;s->original_font=original;
        s->font=replacement?replacement:original;
        outbound=s->font;win16_style_system_retain(outbound);
        /* Native tab geometry/hit testing belongs to the application's font.
         * Only our painter borrows the existing System glyph lease. */
        a=(WPARAM)(native_font?original:s->font);
    }
    if(m==WM_NCDESTROY){(void)win16_style_control_terminal(w);
        result=DefSubclassProc(w,m,a,b);}
    else if(tab){
        if(m==WM_PAINT && win16_style_tab_supported(w)){
            PAINTSTRUCT ps;HDC dc=BeginPaint(w,&ps);present_control(w,s,dc,draw_tab);EndPaint(w,&ps);
        }else if(m==WM_PRINTCLIENT && win16_style_tab_supported(w))draw_tab(w,s,(HDC)a);
        else result=DefSubclassProc(w,m,a,b);
        if(!s->detached && win16_style_tab_changes(m))
            RedrawWindow(w,NULL,NULL,RDW_INVALIDATE|(s->calls==1?RDW_UPDATENOW:0));
    }
    else if(header){
        if(m==WM_LBUTTONDOWN){
            HDHITTESTINFO hit={.pt={(short)LOWORD(b),(short)HIWORD(b)}};
            LRESULT item=SendMessageW(w,HDM_HITTEST,0,(LPARAM)&hit);
            if(s->detached)goto finished;
            s->header_pressed=(item>=0 && (hit.flags&HHT_ONHEADER))?(int)item:-1;
        }
        if(m==WM_LBUTTONUP || m==WM_CANCELMODE || m==WM_CAPTURECHANGED)s->header_pressed=-1;
        if(m==WM_PAINT && win16_style_header_supported(w)){
            PAINTSTRUCT ps;HDC dc=BeginPaint(w,&ps);present_control(w,s,dc,draw_header);EndPaint(w,&ps);
        }else if(m==WM_PRINTCLIENT && win16_style_header_supported(w))draw_header(w,s,(HDC)a);
        else result=DefSubclassProc(w,m,a,b);
        if(!s->detached && win16_style_header_changes(m))
            RedrawWindow(w,NULL,NULL,RDW_INVALIDATE|(s->calls==1?RDW_UPDATENOW:0));
    }
    else if(progress){
        BOOL classic=win16_style_progress_supported(w);
        if(m==WM_PAINT && classic){PAINTSTRUCT ps;HDC dc=BeginPaint(w,&ps);present_control(w,s,dc,draw_progress);EndPaint(w,&ps);}
        else if(m==WM_PRINTCLIENT && classic)draw_progress(w,s,(HDC)a);
        else if(m==WM_PRINT && classic && (b&PRF_CLIENT)){
            int saved=SaveDC((HDC)a);if(saved){result=DefSubclassProc(w,m,a,b&~PRF_CLIENT);RestoreDC((HDC)a,saved);}
            if(!s->detached)draw_progress(w,s,(HDC)a);
        }else result=DefSubclassProc(w,m,a,b);
        if(!s->detached && win16_style_progress_changes(m))
            RedrawWindow(w,NULL,NULL,RDW_INVALIDATE|(s->calls==1?RDW_UPDATENOW:0));
    }
    else if(ntw16s_class_is(w,UPDOWN_CLASSW)){result=DefSubclassProc(w,m,a,b);}
    else if(ntw16s_class_is(w,TRACKBAR_CLASSW) || ntw16s_control_protocol_is(w,TRACKBAR_CLASSW)){
        if(ntw16s_control_protocol_runtime_trackbar(w) && (m==WM_PAINT || m==WM_PRINT || m==WM_PRINTCLIENT))
            result=track_paint_dispatch(s,w,m,a,b);
        else result=DefSubclassProc(w,m,a,b);
        if(!s->detached && m==WM_STYLECHANGED)SendMessageW(w,WM_SYSCOLORCHANGE,0,0);
        if(!s->detached && s->track_parent && GetParent(w)!=s->track_parent){
            trackbar_unbind(s);SendMessageW(w,WM_SYSCOLORCHANGE,0,0);
        }
    }
    else if(is_link(w)){
        /* Preserve native markup, hit testing, focus and NM_CLICK/NM_RETURN.
         * Only the shared WM_SETFONT path above changes link typography. */
        result=DefSubclassProc(w,m,a,b);
    }
    else if(s->frame_scrollbars){
        if(embedded_scroll_input(w,s,m,a,b))result=0;
        else if(m==WM_NCMOUSEMOVE && (a==HTHSCROLL || a==HTVSCROLL))result=0;
        else {if(m==WM_ENABLE && !a)scroll_finish(w,s,TRUE);
            int saved=(s->native_border && m==WM_PRINT)?SaveDC((HDC)a):0;
            if(!s->detached)result=DefSubclassProc(w,m,a,b);
            if(saved)RestoreDC((HDC)a,saved);}
        if(!s->detached && s->native_border && (m==WM_NCPAINT || (m==WM_PRINT && (b&PRF_NONCLIENT)))){
            list_border(w,s,m==WM_PRINT?(HDC)a:NULL);nonclient_scrollbars(w,s,m==WM_PRINT?(HDC)a:NULL);
        }
        if(!s->detached && s->legacy_progress_border && (m==WM_NCPAINT || (m==WM_PRINT && (b&PRF_NONCLIENT))))
            legacy_progress_border(w,s,m==WM_PRINT?(HDC)a:NULL);
    }
    else if(is_scroll(w) && (GetWindowLongPtrW(w,GWL_STYLE)&SBS_SIZEBOX)){
        if(m==WM_PAINT){PAINTSTRUCT ps;HDC dc=BeginPaint(w,&ps);present_control(w,s,dc,draw_sizebox);EndPaint(w,&ps);}
        else if(m==WM_PRINTCLIENT)draw_sizebox(w,s,(HDC)a);
        else if(m==WM_ERASEBKGND)result=1;
        else result=DefSubclassProc(w,m,a,b);
    }
    else if(is_scroll(w)){
        if(scroll_input(w,s,m,a,b))result=0;
        else if(m==WM_SETFOCUS || m==WM_SIZE)scroll_caret_sync(w,s);
        else if(m==WM_KILLFOCUS)scroll_caret_destroy(w,s);
        else if(m==SBM_GETSCROLLINFO){SCROLLINFO *info=(SCROLLINFO *)b;
            result=DefSubclassProc(w,m,a,b);
            if(result && info && info->cbSize==sizeof(*info) && (info->fMask&SIF_TRACKPOS)){
                if(s->scroll_reporting)info->nTrackPos=s->scroll_report;
                else if(s->scroll.active)info->nTrackPos=s->scroll.position;}}
        else if(m==WM_PAINT){PAINTSTRUCT ps;HDC dc=BeginPaint(w,&ps);draw_scroll(w,s,dc,OBJID_CLIENT,(GetWindowLongPtrW(w,GWL_STYLE)&SBS_VERT)!=0);EndPaint(w,&ps);scroll_caret_sync(w,s);}
        else if(m==WM_PRINTCLIENT)draw_scroll(w,s,(HDC)a,OBJID_CLIENT,(GetWindowLongPtrW(w,GWL_STYLE)&SBS_VERT)!=0);
        else if(m==WM_ERASEBKGND)result=1;
        else {BOOL redraw=m==SBM_SETPOS?b!=0:m==SBM_SETSCROLLINFO?a!=0:m!=SBM_SETRANGE;
            if(m==WM_ENABLE && !a)scroll_finish(w,s,TRUE);
            if(s->detached)goto finished;
            if(m==SBM_SETPOS)b=FALSE;
            if(m==SBM_SETSCROLLINFO)a=FALSE;
            if(m==SBM_SETRANGEREDRAW)m=SBM_SETRANGE;
            result=DefSubclassProc(w,m,a,b);
            if(redraw && !s->detached && (m==SBM_SETPOS || m==SBM_SETSCROLLINFO || m==SBM_SETRANGE || m==SBM_SETRANGEREDRAW ||
                m==SBM_ENABLE_ARROWS || m==WM_ENABLE || m==WM_SETFOCUS || m==WM_KILLFOCUS ||
                m==WM_LBUTTONDOWN || m==WM_LBUTTONUP || m==WM_KEYDOWN))RedrawWindow(w,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW);
            if(redraw)scroll_caret_sync(w,s);}
    }
    else if(s->file_owner){
        /* Runtime owner-draw remains authoritative. Overlay chrome only after
         * its native paint; never clear or recreate the image/text item. */
        if(ntw16s_vb_owner_file(w) && ntw16s_vb_directory(w) && embedded_scroll_input(w,s,m,a,b))result=0;
        else result=DefSubclassProc(w,m,a,b);
        if(!s->detached && ntw16s_vb_owner_file(w)){
            if(m==WM_PAINT){
                if(ntw16s_vb_drive(w)){HDC dc=GetDC(w);if(dc){draw_combo(w,s,dc);ReleaseDC(w,dc);}}
                else {list_border(w,s,NULL);nonclient_scrollbars(w,s,NULL);}
            }else if(m==WM_PRINTCLIENT && ntw16s_vb_drive(w))draw_combo(w,s,(HDC)a);
            else if(m==WM_NCPAINT || (m==WM_PRINT && (b&PRF_NONCLIENT))){
                if(ntw16s_vb_directory(w)){list_border(w,s,m==WM_PRINT?(HDC)a:NULL);nonclient_scrollbars(w,s,m==WM_PRINT?(HDC)a:NULL);}
            }
        }
    }
    else if(is_combo(w)){
        if(m==WM_SETREDRAW){s->list_redraw_off=!a;result=DefSubclassProc(w,m,a,b);}
        else if(m==WM_PAINT){PAINTSTRUCT ps;HDC dc=BeginPaint(w,&ps);present_control(w,s,dc,draw_combo);EndPaint(w,&ps);}
        else if(m==WM_PRINTCLIENT)draw_combo(w,s,(HDC)a);
        else if(m==WM_ERASEBKGND)result=1;
        else {BOOL suppress=can_suppress_redraw(w,s) &&
                (m==CB_SETCURSEL || m==CB_SHOWDROPDOWN || m==WM_COMMAND || m==WM_ENABLE ||
                 m==WM_SETFONT || m==WM_SETFOCUS || m==WM_KILLFOCUS || m==WM_KEYDOWN ||
                 m==WM_LBUTTONDOWN || m==WM_LBUTTONUP);
            if(suppress && s->list_suppression++==0)DefSubclassProc(w,WM_SETREDRAW,FALSE,0);
            result=DefSubclassProc(w,m,a,b);
            if(suppress && --s->list_suppression==0 && !s->detached && !s->list_redraw_off)
                DefSubclassProc(w,WM_SETREDRAW,TRUE,0);
            if(!s->detached && !s->list_suppression && !s->list_redraw_off && (m==CB_SETCURSEL || m==CB_SHOWDROPDOWN || m==WM_COMMAND || m==WM_ENABLE ||
                m==WM_SETFONT || m==WM_SETFOCUS || m==WM_KILLFOCUS || m==WM_KEYDOWN ||
                m==WM_LBUTTONDOWN || m==WM_LBUTTONUP))RedrawWindow(w,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW);}
    }
    else if(is_list(w)){
        if(embedded_scroll_input(w,s,m,a,b))result=0;
        else if(m==LB_SETTABSTOPS && (GetWindowLongPtrW(w,GWL_STYLE)&LBS_USETABSTOPS))result=set_list_tabs(w,s,a,b);
        else if(m==WM_SETREDRAW){s->list_redraw_off=!a;result=DefSubclassProc(w,m,a,b);}
        else if(m==WM_PAINT){PAINTSTRUCT ps;HDC dc=BeginPaint(w,&ps);present_control(w,s,dc,draw_list);EndPaint(w,&ps);}
        else if(m==WM_PRINTCLIENT)draw_list(w,s,(HDC)a);
        else if(m==WM_NCPAINT || m==WM_PRINT){
            /* This subclass owns the complete list nonclient surface. Native
             * painting first exposes a second scrollbar skin before ours. */
            if(m==WM_PRINT && (b&PRF_CLIENT)){
                HDC dc=(HDC)a;int saved=SaveDC(dc);RECT wr;POINT origin={0,0};
                if(saved){GetWindowRect(w,&wr);ClientToScreen(w,&origin);
                    OffsetViewportOrgEx(dc,origin.x-wr.left,origin.y-wr.top,NULL);
                    draw_list(w,s,dc);RestoreDC(dc,saved);}
            }
            if(!s->detached && (m==WM_NCPAINT || (b&PRF_NONCLIENT))){
                list_border(w,s,m==WM_PRINT?(HDC)a:NULL);
                nonclient_scrollbars(w,s,m==WM_PRINT?(HDC)a:NULL);}
        }
        else if(m==WM_NCMOUSEMOVE || m==WM_NCMOUSELEAVE){
            /* Win3.x has no scrollbar hover skin. Do not start the host's
             * themed hover/leave repaint; clicks still use native tracking. */
            result=0;
        }
        else if(m==WM_ERASEBKGND)result=1;
        else {
            BOOL moving=m==WM_MOUSEMOVE,changed=FALSE;list_view_state before={0},after;
            BOOL suppress=can_suppress_redraw(w,s) &&
                (m==WM_MOUSEWHEEL || m==WM_VSCROLL || m==WM_HSCROLL || m==WM_KEYDOWN || m==WM_KEYUP ||
                m==WM_SETFOCUS || m==WM_KILLFOCUS || m==WM_ENABLE || m==WM_SETFONT ||
                m==WM_MOUSEMOVE || m==WM_LBUTTONDOWN || m==WM_LBUTTONUP ||
                m==LB_ADDSTRING || m==LB_INSERTSTRING || m==LB_DELETESTRING || m==LB_RESETCONTENT ||
                m==LB_SETCURSEL || m==LB_SETSEL || m==LB_SELITEMRANGE || m==LB_SELITEMRANGEEX ||
                m==LB_SETCARETINDEX || m==LB_SETTOPINDEX);
            if(moving)before=list_view(w);
            if(suppress && s->list_suppression++==0)DefSubclassProc(w,WM_SETREDRAW,FALSE,0);
            result=DefSubclassProc(w,m,a,b);
            if(suppress && --s->list_suppression==0 && !s->detached && !s->list_redraw_off)
                DefSubclassProc(w,WM_SETREDRAW,TRUE,0);
            if(!s->detached && moving){after=list_view(w);
                changed=before.top!=after.top || before.selection!=after.selection || before.caret!=after.caret;}
            if(!s->detached && !s->list_suppression && !s->list_redraw_off && (m==LB_SETCURSEL || m==LB_SETSEL || m==LB_SELITEMRANGE || m==LB_SELITEMRANGEEX ||
                m==LB_SETCARETINDEX || m==LB_SETTOPINDEX || m==LB_ADDSTRING || m==LB_INSERTSTRING ||
                m==LB_DELETESTRING || m==LB_RESETCONTENT || m==LB_SETCOLUMNWIDTH || m==LB_SETHORIZONTALEXTENT || m==WM_SETFONT || m==WM_ENABLE ||
                m==WM_SETFOCUS || m==WM_KILLFOCUS || m==WM_LBUTTONDOWN || m==WM_LBUTTONUP ||
                changed || m==WM_KEYDOWN || m==WM_KEYUP || m==WM_VSCROLL || m==WM_HSCROLL || m==WM_MOUSEWHEEL)){
                RedrawWindow(w,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW);
                if(!s->detached)nonclient_scrollbars(w,s,NULL);}
        }
    }
    else if(is_edit(w)){
        /* EDIT retains its native buffer, selection, caret, undo and IME.
         * Original flat EDIT borders are painted inside the client area. */
        if(embedded_scroll_input(w,s,m,a,b))goto finished;
        if(s->edit_scrollbars && (m==WM_NCMOUSEMOVE || m==WM_NCMOUSELEAVE))goto finished;
        if(s->edit_scrollbars && m==WM_NCPAINT){nonclient_scrollbars(w,s,NULL);goto finished;}
        if(s->edit_scrollbars && m==WM_SETREDRAW)s->list_redraw_off=!a;
        {BOOL suppress=s->edit_scrollbars && can_suppress_redraw(w,s) &&
            (m==WM_VSCROLL || m==WM_HSCROLL || m==WM_MOUSEWHEEL || m==EM_LINESCROLL || m==WM_SETTEXT ||
            m==EM_REPLACESEL || m==EM_UNDO || m==WM_UNDO || m==WM_KEYDOWN || m==WM_CHAR ||
            (m==WM_MOUSEMOVE && GetCapture()==w && (a&MK_LBUTTON)) || m==WM_LBUTTONDOWN || m==WM_LBUTTONUP);
        if(suppress && s->list_suppression++==0)DefSubclassProc(w,WM_SETREDRAW,FALSE,0);
        int native_dc=(m==WM_PRINT || m==WM_PRINTCLIENT)?SaveDC((HDC)a):0;
        result=DefSubclassProc(w,m,a,b);
        if(native_dc)RestoreDC((HDC)a,native_dc);
        if(suppress && --s->list_suppression==0 && !s->detached && !s->list_redraw_off)
            DefSubclassProc(w,WM_SETREDRAW,TRUE,0);
        if(suppress && !s->detached)RedrawWindow(w,NULL,NULL,RDW_INVALIDATE|RDW_FRAME|RDW_UPDATENOW);
        }
        if(!s->detached && (m==WM_PRINTCLIENT || (m==WM_PRINT && (b&PRF_CLIENT)) || m==WM_PAINT || m==WM_ENABLE))
            edit_border(w,s,(m==WM_PRINT || m==WM_PRINTCLIENT)?(HDC)a:NULL);
        if(!s->detached && (m==WM_NCPAINT || (m==WM_PRINT && (b&PRF_NONCLIENT))))
            {nonclient_scrollbars(w,s,m==WM_PRINT?(HDC)a:NULL);
             if(ntw16s_vb_edit(w) || ntw16s_native_superclass(w,L"EDIT") ||
                (GetWindowLongPtrW(w,GWL_EXSTYLE)&WS_EX_CLIENTEDGE))list_border(w,s,m==WM_PRINT?(HDC)a:NULL);}
        else if(!s->detached && (m==WM_VSCROLL || m==WM_HSCROLL || m==WM_MOUSEWHEEL ||
            m==EM_LINESCROLL || m==WM_SETTEXT || m==WM_KEYDOWN || m==WM_CHAR))
            nonclient_scrollbars(w,s,NULL);
    }
    /* Bitmap statics have no themed chrome or text. Keep the native image
     * painter and image ownership, rather than inventing a second blitter. */
    else if(is_static(w) && (GetWindowLongPtrW(w,GWL_STYLE)&SS_TYPEMASK)==SS_BITMAP){
        if(m==WM_PRINTCLIENT){
            HDC dc=(HDC)a;int saved=SaveDC(dc);RECT r;
            /* The centered native painter excludes its image rectangle while
             * filling the border. Do not leak that clip into the caller DC. */
            if(saved){GetClientRect(w,&r);IntersectClipRect(dc,r.left,r.top,r.right,r.bottom);
                result=DefSubclassProc(w,m,a,b);RestoreDC(dc,saved);}
        }else result=DefSubclassProc(w,m,a,b);
    }
    /* Native etched STATIC adds WS_EX_STATICEDGE and can leave zero client
     * height. Own its complete line rectangle without changing the stored
     * style or its outer geometry; detach reinstates native NC calculation. */
    else if(is_separator(w) && (m==WM_NCCALCSIZE || m==WM_NCPAINT))result=0;
    else if(ntw16s_vb_frame(w)){
        /* Unlike native GROUPBOX, a VB Frame owns children and paints its
         * windowless labels. Always run the original painter first. */
        result=DefSubclassProc(w,m,a,b);
        if(!s->detached && (m==WM_PAINT || m==WM_PRINTCLIENT ||
                (m==WM_PRINT && (b&PRF_CLIENT)))){
            HDC dc=(m==WM_PAINT)?GetDC(w):(HDC)a;
            if(dc){draw(w,s,dc);if(m==WM_PAINT)ReleaseDC(w,dc);}
        }
    }
    else if(m==WM_SETREDRAW){s->list_redraw_off=!a;result=DefSubclassProc(w,m,a,b);}
    else if(m==WM_PAINT){PAINTSTRUCT ps;HDC dc=BeginPaint(w,&ps);present_control(w,s,dc,draw);EndPaint(w,&ps);}
    else if(m==WM_PRINT && is_separator(w)){
        /* STATIC prints etched shapes internally rather than forwarding
         * WM_PRINTCLIENT. Keep printed and on-screen separators identical. */
        if((b&PRF_CLIENT) && (!(b&PRF_CHECKVISIBLE) || IsWindowVisible(w)))draw(w,s,(HDC)a);
    }
    else if(m==WM_PRINTCLIENT)draw(w,s,(HDC)a);
    else if(m==WM_ERASEBKGND)result=1;
    else {
        BOOL moving=m==WM_MOUSEMOVE && !is_static(w),changed=FALSE;
        LRESULT previous=moving?SendMessageW(w,BM_GETSTATE,0,0):0;
        /* Only bracket native drawing leaves, not BM_CLICK or mouse/key
         * dispatch: automatic radio-group traversal must see the original
         * visibility throughout the outer input operation. */
        BOOL suppress=!is_static(w) && can_suppress_redraw(w,s) &&
            (m==BM_SETCHECK || m==BM_SETSTATE || m==BM_SETSTYLE || m==WM_ENABLE ||
             m==WM_SETFOCUS || m==WM_KILLFOCUS || m==WM_SETTEXT || m==WM_SETFONT ||
             m==WM_MOUSEMOVE);
        if(suppress && s->list_suppression++==0)DefSubclassProc(w,WM_SETREDRAW,FALSE,0);
        result=DefSubclassProc(w,m,a,b);
        if(suppress && --s->list_suppression==0 && !s->detached && !s->list_redraw_off)
            DefSubclassProc(w,WM_SETREDRAW,TRUE,0);
        if(!s->detached && moving)changed=previous!=SendMessageW(w,BM_GETSTATE,0,0);
        if(!s->detached && s->calls==1 && !s->list_suppression && !s->list_redraw_off && (m==BM_SETCHECK || m==BM_SETSTATE || m==BM_SETSTYLE || m==BM_CLICK || m==WM_ENABLE ||
            m==WM_SETFOCUS || m==WM_KILLFOCUS || m==WM_SETTEXT || m==WM_SETFONT ||
            m==BM_SETIMAGE || m==BCM_SETIMAGELIST || (is_static(w) && (m==STM_SETICON || m==STM_SETIMAGE)) ||
            m==WM_LBUTTONDOWN || m==WM_LBUTTONUP || changed || m==WM_KEYDOWN || m==WM_KEYUP))
            RedrawWindow(w,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW);
    }
finished:
    if(m==WM_SETFONT && s->detached && IsWindow(w)){
        /* A native setter can finish AFTER its callback removed this leaf,
         * overwriting the font restored during removal with our retired
         * variant. Settle only after that native call has returned. A new
         * independent owner, if installed reentrantly, keeps its own source. */
        DWORD_PTR successor;HFONT source=s->original_font;
        if(GetWindowSubclass(w,control_proc,(UINT_PTR)control_proc,&successor))
            source=((control_style *)successor)->original_font;
        SendMessageW(w,WM_SETFONT,(WPARAM)source,b);
    }
    if(combo_paint)--s->combo_paints;
    /* Native processing (including nested paint/remove/setfont) has finished
     * with the previous lease. Never delete the application's borrowed font. */
    win16_style_system_release(previous);
    win16_style_system_release(outbound);win16_style_system_release(pinned);
    if(!--s->calls && s->detached)dispose_control(s);
    return result;
}

static BOOL install(HWND w,const win16_style_palette *palette,BOOL edit,BOOL original_border,BOOL frame_bars,
    int tab_count,const int *tabs,const void *creation_owner,const BOOL *cancelled)
{
    DWORD_PTR existing;control_style *s;BOOL result=TRUE,tabbed=tab_count>=0;LONG_PTR style=GetWindowLongPtrW(w,GWL_STYLE);
    if(GetWindowThreadProcessId(w,NULL)!=GetCurrentThreadId()){SetLastError(ERROR_INVALID_PARAMETER);return FALSE;}
    if(GetWindowSubclass(w,control_proc,(UINT_PTR)control_proc,&existing)){SetLastError(ERROR_ALREADY_EXISTS);return FALSE;}
    if(tabbed){
        if(!is_list(w) || !(style&LBS_USETABSTOPS) || (style&(LBS_OWNERDRAWFIXED|LBS_OWNERDRAWVARIABLE|LBS_NODATA))){
            SetLastError(ERROR_NOT_SUPPORTED);return FALSE;}
        if((tab_count && !tabs) || (size_t)tab_count>((size_t)-1)/sizeof(int)){
            SetLastError(ERROR_INVALID_PARAMETER);return FALSE;}
    }else if(!frame_bars && (!supported(w) || is_edit(w)!=edit)){SetLastError(ERROR_NOT_SUPPORTED);return FALSE;}
    s=calloc(1,sizeof(*s));if(!s)return FALSE;
    s->creation_owner=creation_owner;
    s->header_pressed=-1;
    SetLastError(ERROR_SUCCESS);
    s->protocol=ntw16s_control_protocol_acquire(w);
    if(!s->protocol && GetLastError()==ERROR_ARITHMETIC_OVERFLOW){dispose_control(s);SetLastError(ERROR_ARITHMETIC_OVERFLOW);return FALSE;}
    if(tabbed && tab_count){s->tab_positions=malloc((size_t)tab_count*sizeof(int));
        if(!s->tab_positions){dispose_control(s);return FALSE;}
        CopyMemory(s->tab_positions,tabs,(size_t)tab_count*sizeof(int));s->tab_count=tab_count;}
    s->edit_border=edit && original_border;
    s->frame_scrollbars=frame_bars;
    s->file_owner=!frame_bars && ntw16s_vb_owner_file(w);
    if(!frame_bars){s->original_font=(HFONT)SendMessageW(w,WM_GETFONT,0,0);
        if(cancelled && *cancelled){dispose_control(s);SetLastError(ERROR_CANCELLED);return FALSE;}
        /* The font query may reenter installation. Do not replace that
         * successful attachment's subclass data with this outer attempt. */
        if(GetWindowSubclass(w,control_proc,(UINT_PTR)control_proc,&existing)){
            dispose_control(s);SetLastError(ERROR_ALREADY_EXISTS);return FALSE;}
        s->original_font=win16_style_system_source(s->original_font);}
    if(palette)s->palette=*palette;else win16_style_default_palette(&s->palette);
    if(!SetWindowSubclass(w,control_proc,(UINT_PTR)control_proc,(DWORD_PTR)s)){dispose_control(s);return FALSE;}
    /* Child attachment and WM_SETFONT can synchronously remove/destroy us.
     * Retain this attempt until its result is known, just like a callback. */
    ++s->calls;
    if(!frame_bars && is_combo(w) && !s->file_owner){
        COMBOBOXINFO info={.cbSize=sizeof(info)};
        if(!ntw16s_native_get_combo_info(w,&info) || !win16_style_control_install(info.hwndList,palette)){
            if(!s->detached)win16_style_control_remove(w);
            result=FALSE;
            goto installed;
            }
        if(s->detached){win16_style_control_remove(info.hwndList);goto installed;}
        s->combo_list=info.hwndList;
        if(GetWindowSubclass(info.hwndList,control_proc,(UINT_PTR)control_proc,&existing))
            ((control_style *)existing)->owner=s;
        BOOL native_edit=ntw16s_vb_combo(w) && info.hwndItem!=w && ntw16s_vb_combo_edit(info.hwndItem);
        if(s->detached)goto installed;
        if(native_edit){
            HFONT source=(HFONT)SendMessageW(info.hwndItem,WM_GETFONT,0,0);
            if(s->detached)goto installed;
            s->combo_parent=w;s->combo_edit_source=win16_style_system_source(source);
            DWORD_PTR child_owner;
            if(GetWindowSubclass(info.hwndItem,combo_text_proc,(UINT_PTR)combo_text_proc,&child_owner)){
                win16_style_control_remove(w);SetLastError(ERROR_ALREADY_EXISTS);result=FALSE;goto installed;
            }
            if(!SetWindowSubclass(info.hwndItem,combo_text_proc,(UINT_PTR)combo_text_proc,(DWORD_PTR)s)){
                DWORD error=GetLastError();win16_style_control_remove(w);SetLastError(error);result=FALSE;goto installed;
            }
            s->combo_edit=info.hwndItem;s->combo_bound=TRUE;++s->calls;
            SendMessageW(info.hwndItem,WM_SETFONT,(WPARAM)s->combo_edit_source,FALSE);
            if(s->detached)goto installed;
        }
    }
    if(!frame_bars){SendMessageW(w,WM_SETFONT,(WPARAM)s->original_font,FALSE);
        if(!s->detached && is_separator(w))SetWindowPos(w,NULL,0,0,0,0,
            SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_FRAMECHANGED);
        if(!s->detached)RedrawWindow(w,NULL,NULL,RDW_INVALIDATE|RDW_FRAME);}
installed:
    if(s->detached){result=FALSE;SetLastError(ERROR_CANCELLED);}
    if(!--s->calls && s->detached)dispose_control(s);
    return result;
}
BOOL win16_style_control_install(HWND w,const win16_style_palette *palette)
{return install(w,palette,FALSE,FALSE,FALSE,-1,NULL,NULL,NULL);}
BOOL win16_style_control_install_created(HWND w,BOOL edit,BOOL original_border,BOOL tabbed,
    const win16_style_palette *palette,const void *owner,const BOOL *cancelled)
{return install(w,palette,edit,original_border,FALSE,tabbed?0:-1,NULL,owner,cancelled);}
BOOL win16_style_tabbed_list_install(HWND w,const win16_style_palette *palette,int count,const int *pixels)
{if(count<0){SetLastError(ERROR_INVALID_PARAMETER);return FALSE;}return install(w,palette,FALSE,FALSE,FALSE,count,pixels,NULL,NULL);}
BOOL win16_style_frame_scrollbars_install(HWND w,const win16_style_palette *palette)
{return install(w,palette,FALSE,FALSE,TRUE,-1,NULL,NULL,NULL);}
BOOL win16_style_frame_scrollbars_install_owned(HWND w,const win16_style_palette *palette,const void *owner,const BOOL *cancelled)
{
    if(cancelled && *cancelled){SetLastError(ERROR_CANCELLED);return FALSE;}
    return install(w,palette,FALSE,FALSE,TRUE,-1,NULL,owner,cancelled);
}
void win16_style_frame_scrollbars_palette_owned(HWND w,const win16_style_palette *palette,const void *owner)
{
    DWORD_PTR data;
    if(GetWindowSubclass(w,control_proc,(UINT_PTR)control_proc,&data) &&
        ((control_style *)data)->frame_scrollbars && ((control_style *)data)->creation_owner==owner)
        ((control_style *)data)->palette=*palette;
}
BOOL win16_style_updown_install_created(HWND w,const win16_style_palette *palette,const void *owner,const BOOL *cancelled)
{
    DWORD_PTR value;HMODULE native=ntw16s_control_protocol_updown_module(w);
    if(!native || !ntw16s_updown_api_enabled()){SetLastError(ERROR_NOT_SUPPORTED);return FALSE;}
    if(!win16_style_frame_scrollbars_install_owned(w,palette,owner,cancelled))return FALSE;
    if(!GetWindowSubclass(w,control_proc,(UINT_PTR)control_proc,&value) ||
        ((control_style *)value)->creation_owner!=owner){SetLastError(ERROR_CANCELLED);return FALSE;}
    control_style *s=(control_style *)value;
    ntw16s_updown_register(&s->updown,w,native,&s->palette,updown_glyphs,s,updown_acquire,updown_release);
    return TRUE;
}
BOOL win16_style_trackbar_install_created(HWND w,const win16_style_palette *palette,const void *owner,const BOOL *cancelled)
{
    DWORD_PTR value;
    if(!win16_style_trackbar_supported(w)){SetLastError(ERROR_NOT_SUPPORTED);return FALSE;}
    if(!win16_style_frame_scrollbars_install_owned(w,palette,owner,cancelled))return FALSE;
    if(!GetWindowSubclass(w,control_proc,(UINT_PTR)control_proc,&value) ||
        ((control_style *)value)->creation_owner!=owner){SetLastError(ERROR_CANCELLED);return FALSE;}
    control_style *s=(control_style *)value;++s->calls;s->track_window=w;s->track_parent=GetParent(w);
    BOOL okay=s->track_parent && SetWindowSubclass(s->track_parent,trackbar_notify,(UINT_PTR)w,(DWORD_PTR)s);
    /* Native trackbars cache part drawing. Invalidate only this new control's
     * native cache after binding, without changing theme/global properties. */
    if(okay)SendMessageW(w,WM_SYSCOLORCHANGE,0,0);
    if(s->detached)okay=FALSE;
    if(!okay && !s->detached)win16_style_control_remove_owned(w,owner);
    if(!--s->calls && s->detached)dispose_control(s);
    return okay;
}
BOOL win16_style_toolbar_install_created(HWND w,const win16_style_palette *palette,const void *owner,const BOOL *cancelled)
{
    DWORD_PTR value;
    if(!ntw16s_control_protocol_toolbar(w)){SetLastError(ERROR_NOT_SUPPORTED);return FALSE;}
    if(!win16_style_frame_scrollbars_install_owned(w,palette,owner,cancelled))return FALSE;
    if(!GetWindowSubclass(w,control_proc,(UINT_PTR)control_proc,&value) ||
        ((control_style *)value)->creation_owner!=owner){SetLastError(ERROR_CANCELLED);return FALSE;}
    control_style *s=(control_style *)value;++s->calls;s->toolbar_window=w;s->toolbar_parent=GetParent(w);
    BOOL okay=s->toolbar_parent && SetWindowSubclass(s->toolbar_parent,toolbar_notify,(UINT_PTR)w,(DWORD_PTR)s);
    if(s->detached)okay=FALSE;
    if(!okay && !s->detached)win16_style_control_remove_owned(w,owner);
    if(!--s->calls && s->detached)dispose_control(s);
    return okay;
}
BOOL win16_style_progress_install_created(HWND w,const win16_style_palette *palette,const void *owner,const BOOL *cancelled)
{
    if(!win16_style_progress_supported(w)){SetLastError(ERROR_NOT_SUPPORTED);return FALSE;}
    /* No text or font property belongs to this painter. Reuse the owned
     * passive-font leaf; the progress branch owns its paint, not bar input. */
    return win16_style_frame_scrollbars_install_owned(w,palette,owner,cancelled);
}
BOOL win16_style_legacy_progress_border_install_created(HWND w,const win16_style_palette *palette,const void *owner,const BOOL *cancelled)
{
    DWORD_PTR data;
    if(!ntw16s_control_protocol_legacy_progress(w)){SetLastError(ERROR_NOT_SUPPORTED);return FALSE;}
    if(!win16_style_frame_scrollbars_install_owned(w,palette,owner,cancelled))return FALSE;
    if(!GetWindowSubclass(w,control_proc,(UINT_PTR)control_proc,&data) ||
        ((control_style *)data)->creation_owner!=owner){SetLastError(ERROR_CANCELLED);return FALSE;}
    ((control_style *)data)->legacy_progress_border=TRUE;
    RedrawWindow(w,NULL,NULL,RDW_INVALIDATE|RDW_FRAME);
    return TRUE;
}
BOOL win16_style_tree_install_created(HWND w,const win16_style_palette *palette,const void *owner,const BOOL *cancelled)
{
    DWORD_PTR data;
    if(!ntw16s_control_protocol_is(w,WC_TREEVIEWW)){SetLastError(ERROR_NOT_SUPPORTED);return FALSE;}
    if(!win16_style_frame_scrollbars_install_owned(w,palette,owner,cancelled))return FALSE;
    if(!GetWindowSubclass(w,control_proc,(UINT_PTR)control_proc,&data) ||
        ((control_style *)data)->creation_owner!=owner){SetLastError(ERROR_CANCELLED);return FALSE;}
    ((control_style *)data)->native_border=TRUE;
    return TRUE;
}
BOOL win16_style_grid_install_created(HWND w,const win16_style_palette *palette,const void *owner,const BOOL *cancelled)
{
    DWORD_PTR data;
    if(!win16_style_frame_scrollbars_install_owned(w,palette,owner,cancelled))return FALSE;
    if(!GetWindowSubclass(w,control_proc,(UINT_PTR)control_proc,&data) ||
        ((control_style *)data)->creation_owner!=owner){SetLastError(ERROR_CANCELLED);return FALSE;}
    ((control_style *)data)->native_border=TRUE;
    return TRUE;
}
BOOL win16_style_picture_install_created(HWND w,const win16_style_palette *palette,const void *owner,const BOOL *cancelled)
{
    DWORD_PTR data;
    if(!ntw16s_vb_picture(w)){SetLastError(ERROR_NOT_SUPPORTED);return FALSE;}
    if(!win16_style_frame_scrollbars_install_owned(w,palette,owner,cancelled))return FALSE;
    if(!GetWindowSubclass(w,control_proc,(UINT_PTR)control_proc,&data) ||
        ((control_style *)data)->creation_owner!=owner){SetLastError(ERROR_CANCELLED);return FALSE;}
    ((control_style *)data)->native_border=TRUE;
    return TRUE;
}
void win16_style_frame_scrollbars_remove(HWND w)
{
    DWORD_PTR data;
    if(GetWindowSubclass(w,control_proc,(UINT_PTR)control_proc,&data) && ((control_style *)data)->frame_scrollbars)
        win16_style_control_remove(w);
}
void win16_style_frame_scrollbars_palette(HWND w,const win16_style_palette *palette)
{
    DWORD_PTR data;
    if(GetWindowSubclass(w,control_proc,(UINT_PTR)control_proc,&data) && ((control_style *)data)->frame_scrollbars)
        ((control_style *)data)->palette=*palette;
}
BOOL win16_style_get_scroll_info(HWND w,int bar,LPSCROLLINFO info)
{
    DWORD_PTR data;BOOL result=ntw16s_native_get_scroll_info(w,bar,info);DWORD error=GetLastError();
    if(result && (bar==SB_HORZ || bar==SB_VERT) && info && info->cbSize==sizeof(*info) &&
        (info->fMask&SIF_TRACKPOS) && GetWindowSubclass(w,control_proc,(UINT_PTR)control_proc,&data)){
        control_style *s=(control_style *)data;
        if(s->frame_scrollbars && s->embedded_object==(bar==SB_HORZ?OBJID_HSCROLL:OBJID_VSCROLL)){
            if(s->scroll_reporting)info->nTrackPos=s->scroll_report;
            else if(s->scroll.active)info->nTrackPos=s->scroll.position;
        }
    }
    SetLastError(error);return result;
}
int win16_style_set_scroll_info(HWND w,int bar,LPCSCROLLINFO info,BOOL redraw)
{
    DWORD_PTR data;int result;DWORD error;
    BOOL styled=(bar==SB_HORZ || bar==SB_VERT) &&
        GetWindowSubclass(w,control_proc,(UINT_PTR)control_proc,&data) && ((control_style *)data)->frame_scrollbars;
    result=ntw16s_native_set_scroll_info(w,bar,info,styled?FALSE:redraw);
    error=GetLastError();
    if(styled && redraw){
        if(GetWindowSubclass(w,control_proc,(UINT_PTR)control_proc,&data)){
            HDC dc=GetWindowDC(w);
            if(dc){draw_scroll(w,(control_style *)data,dc,bar==SB_HORZ?OBJID_HSCROLL:OBJID_VSCROLL,bar==SB_VERT);ReleaseDC(w,dc);}
        }else RedrawWindow(w,NULL,NULL,RDW_FRAME|RDW_INVALIDATE);
    }
    SetLastError(error);return result;
}
BOOL win16_style_list_scrollbar_enable(HWND w)
{
    DWORD_PTR data;
    if(GetWindowThreadProcessId(w,NULL)!=GetCurrentThreadId() || !is_list(w) ||
        !GetWindowSubclass(w,control_proc,(UINT_PTR)control_proc,&data)){
        SetLastError(ERROR_NOT_SUPPORTED);return FALSE;}
    ((control_style *)data)->list_scrollbar=TRUE;
    RedrawWindow(w,NULL,NULL,RDW_FRAME|RDW_INVALIDATE);return TRUE;
}
BOOL win16_style_edit_install(HWND w,BOOL original_border,const win16_style_palette *palette)
{return install(w,palette,TRUE,original_border,FALSE,-1,NULL,NULL,NULL);}
BOOL win16_style_edit_scrollbars_enable(HWND w)
{
    DWORD_PTR data;LONG_PTR style=GetWindowLongPtrW(w,GWL_STYLE);
    if(GetWindowThreadProcessId(w,NULL)!=GetCurrentThreadId() || !is_edit(w) ||
        !(style&ES_MULTILINE) ||
        !GetWindowSubclass(w,control_proc,(UINT_PTR)control_proc,&data)){
        SetLastError(ERROR_NOT_SUPPORTED);return FALSE;}
    ((control_style *)data)->edit_scrollbars=TRUE;
    RedrawWindow(w,NULL,NULL,RDW_FRAME|RDW_INVALIDATE);return TRUE;
}
BOOL win16_style_control_remove(HWND w)
{
    DWORD_PTR data;control_style *s;
    if(GetWindowThreadProcessId(w,NULL)!=GetCurrentThreadId() ||
        !GetWindowSubclass(w,control_proc,(UINT_PTR)control_proc,&data))return FALSE;
    s=(control_style *)data;
    if(s->removing)return TRUE;
    ++s->calls;s->removing=TRUE;
    /* End the live gesture while queries still see its full-width report.
     * The application may remove again or destroy the HWND in this notice. */
    scroll_finish(w,s,TRUE);
    if(s->detached){if(!--s->calls)dispose_control(s);return TRUE;}
    if(!RemoveWindowSubclass(w,control_proc,(UINT_PTR)control_proc)){
        s->removing=FALSE;--s->calls;return FALSE;}
    s->detached=TRUE;ntw16s_updown_retire(&s->updown);BOOL track=s->track_window!=NULL;trackbar_unbind(s);toolbar_unbind(s);combo_text_unbind(s,FALSE);detach_combo_list(s);
    if(s->font && IsWindow(w))SendMessageW(w,WM_SETFONT,(WPARAM)s->original_font,FALSE);
    if(IsWindow(w) && is_separator(w))SetWindowPos(w,NULL,0,0,0,0,
        SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_FRAMECHANGED);
    scroll_caret_destroy(w,s);
    if(s->list_suppression && !s->list_redraw_off)SendMessageW(w,WM_SETREDRAW,TRUE,0);
    scroll_stop_timer(w,s);
    if(s->scroll.active){s->scroll.active=FALSE;scroll_release_capture(w,s);}
    if(!--s->calls)dispose_control(s);
    if(track && IsWindow(w))SendMessageW(w,WM_SYSCOLORCHANGE,0,0);
    RedrawWindow(w,NULL,NULL,RDW_INVALIDATE|RDW_FRAME);
    if(is_scroll(w) && GetFocus()==w)SendMessageW(w,WM_SETFOCUS,0,0);
    return TRUE;
}
BOOL win16_style_control_remove_owned(HWND w,const void *owner)
{
    DWORD_PTR data;
    if(!owner || GetWindowThreadProcessId(w,NULL)!=GetCurrentThreadId() ||
        !GetWindowSubclass(w,control_proc,(UINT_PTR)control_proc,&data) ||
        ((control_style *)data)->creation_owner!=owner)return FALSE;
    return win16_style_control_remove(w);
}
BOOL win16_style_control_terminal(HWND w)
{
    DWORD_PTR data;control_style *s;
    BOOL result;
    if(!GetWindowSubclass(w,control_proc,(UINT_PTR)control_proc,&data))return TRUE;
    s=(control_style *)data;
    if(!RemoveWindowSubclass(w,control_proc,(UINT_PTR)control_proc)){
        SetLastError(ERROR_INVALID_STATE);return FALSE;
    }
    ++s->calls;s->detached=TRUE;ntw16s_updown_retire(&s->updown);trackbar_unbind(s);toolbar_unbind(s);
    scroll_caret_destroy(w,s);
    scroll_stop_timer(w,s);
    combo_text_unbind(s,TRUE);result=terminal_combo_list(s);
    if(!--s->calls)dispose_control(s);
    return result;
}
