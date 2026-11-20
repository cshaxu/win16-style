#include "lib-win32-style/application/control/rebar.h"
#include "lib-win32-style/application/graphics/dc_geometry.h"
#include "special_paint.h"
#include "basic_paint.h"
#include "../../../application/graphics/paint_frame.h"
#include "../resources/scroll_assets.h"
#include "../../../application/control/control_protocol.h"
#include "../../../application/graphics/bitmap.h"
#include "control_class.h"
#include "control_state.h"
#include "choice_paint.h"
#include "vb_text.h"
#include <commctrl.h>
#include <vssym32.h>

BOOL win16_control_color_frame(HDC dc,const RECT *bounds,const RECT *clip,COLORREF color)
{
    int saved=SaveDC(dc);if(!saved)return FALSE;
    BOOL drew=FALSE;
    if((!clip || IntersectClipRect(dc,clip->left,clip->top,clip->right,clip->bottom)!=ERROR) &&
       SetDCBrushColor(dc,color)!=CLR_INVALID)
        drew=FrameRect(dc,bounds,(HBRUSH)GetStockObject(DC_BRUSH));
    RestoreDC(dc,saved);return drew;
}
BOOL win16_control_color_swatch_frame(HDC dc,RECT *bounds,const RECT *original,COLORREF color)
{
    if(!win16_control_color_frame(dc,original,NULL,color))return FALSE;
    *bounds=*original;InflateRect(bounds,-1,-1);return TRUE;
}

void win16_control_group_frame(HDC dc,const RECT *bounds,const RECT *clip,const RECT *exclude,
    COLORREF color,void (*drawn)(void *),void *context)
{
    int saved=SaveDC(dc);if(!saved)return;
    if(clip)IntersectClipRect(dc,clip->left,clip->top,clip->right,clip->bottom);
    if(exclude)ExcludeClipRect(dc,exclude->left,exclude->top,exclude->right,exclude->bottom);
    if(SetDCBrushColor(dc,color)!=CLR_INVALID && FrameRect(dc,bounds,(HBRUSH)GetStockObject(DC_BRUSH)) && drawn)drawn(context);
    RestoreDC(dc,saved);
}

void win16_control_panel_shadow(HDC dc,const POINT *points,BOOL (WINAPI *draw)(HDC,const POINT *,int),
    COLORREF color,void (*drawn)(void *),void *context)
{
    int saved=SaveDC(dc);if(!saved)return;
    if(SelectObject(dc,GetStockObject(DC_PEN)) && SetDCPenColor(dc,color)!=CLR_INVALID &&
        draw(dc,points,3) && drawn)drawn(context);
    RestoreDC(dc,saved);
}

void win16_control_month_fill(HDC dc,const RECT *bounds,const RECT *clip,COLORREF color)
{
    int saved=SaveDC(dc);if(!saved)return;
    if(clip && IntersectClipRect(dc,clip->left,clip->top,clip->right,clip->bottom)==ERROR){RestoreDC(dc,saved);return;}
    HBRUSH brush=(HBRUSH)GetStockObject(DC_BRUSH);SetDCBrushColor(dc,color);
    FillRect(dc,bounds,brush);RestoreDC(dc,saved);
}

BOOL win16_control_month_text(HTHEME theme,HDC dc,int part,int state,HFONT font,LPCWSTR text,int count,DWORD flags,const RECT *bounds,
    int (WINAPI *draw)(HDC,LPCWSTR,int,LPRECT,UINT),BOOL (*room)(void *,int),void *context)
{
    RECT output=*bounds,measure=*bounds;COLORREF color;HGDIOBJ previous=SelectObject(dc,font);
    if(SUCCEEDED(GetThemeColor(theme,part,state,TMT_TEXTCOLOR,&color)) && previous && previous!=HGDI_ERROR){
        SetTextColor(dc,color);SetBkMode(dc,TRANSPARENT);
        int height=draw(dc,text,count,&measure,flags|DT_CALCRECT);
        if(height>0 && measure.right-measure.left<=bounds->right-bounds->left &&
           measure.bottom-measure.top<=bounds->bottom-bounds->top && room(context,measure.right-measure.left) &&
           IntersectClipRect(dc,bounds->left,bounds->top,bounds->right,bounds->bottom)!=ERROR)
            return draw(dc,text,count,&output,flags)!=0;
    }
    return FALSE;
}

BOOL win16_control_cool_grip(HDC dc,const RECT *bounds,const classic_bar_colors *colors)
{
    RECT r=*bounds;
    const style_relief_palette palette={
        .face=classic_bar_color(colors,CLASSIC_BAR_FACE,GetSysColor(COLOR_BTNFACE)),
        .highlight=classic_bar_color(colors,CLASSIC_BAR_HIGHLIGHT,GetSysColor(COLOR_BTNHIGHLIGHT)),
        .shadow=classic_bar_color(colors,CLASSIC_BAR_SHADOW,GetSysColor(COLOR_BTNSHADOW))
    };
    return style_paint_relief(dc,&r,BDR_RAISEDINNER,BF_RECT|BF_MIDDLE,&palette);
}

void win16_style_month_arrow(HDC dc,const RECT *r,const RECT *clip,int part,int state,const win16_style_palette *palette,
    const style_scroll_arrow_art *art)
{
    if(!dc || !r || IsRectEmpty(r) || r->right-r->left>128 || r->right-r->left!=r->bottom-r->top)return;
    int saved=SaveDC(dc);
    if(!saved)return;
    BOOL previous=part==MC_NAVPREV;
    UINT flags=previous?DFCS_SCROLLLEFT:DFCS_SCROLLRIGHT;
    if(state==(previous?MCNP_DISABLED:MCNN_DISABLED))flags|=DFCS_INACTIVE;
    else if(state==(previous?MCNP_PRESSED:MCNN_PRESSED))flags|=DFCS_PUSHED;
    if(!clip || IntersectClipRect(dc,clip->left,clip->top,clip->right,clip->bottom)!=ERROR)
        win16_control_scroll_arrow_art(dc,*r,flags,palette,art);
    RestoreDC(dc,saved);
}

/* USER18:0208..027F: scrollbar-class size box shares the inner border
 * and fills with the system scrollbar brush, not a later sizing-grip image.
 * Native creation/alignment and parent size/maximize commands stay native. */
void win16_control_draw_sizebox(HWND w,HDC dc,const COLORREF *face)
{
    RECT outer,client,r;UINT dpi=GetDpiForWindow(w);int saved,cx,cy,bx,by;
    if(!dc || !GetWindowRect(w,&outer) || !GetClientRect(w,&client))return;
    saved=SaveDC(dc);if(!saved)return;
    cx=GetSystemMetricsForDpi(SM_CXVSCROLL,dpi);cy=GetSystemMetricsForDpi(SM_CYHSCROLL,dpi);
    bx=GetSystemMetricsForDpi(SM_CXBORDER,dpi);by=GetSystemMetricsForDpi(SM_CYBORDER,dpi);
    SetRect(&r,outer.right-outer.left-cx+bx,outer.bottom-outer.top-cy+by,
        outer.right-outer.left,outer.bottom-outer.top);
    IntersectClipRect(dc,client.left,client.top,client.right,client.bottom);
    HBRUSH brush=GetSysColorBrush(COLOR_SCROLLBAR);
    if(face){
        SetDCBrushColor(dc,*face);
        brush=GetStockObject(DC_BRUSH);
    }
    FillRect(dc,&r,brush);RestoreDC(dc,saved);
}
void win16_control_legacy_progress_border(HWND w,const win16_style_palette *palette,HDC target)
{
    RECT outer,client;POINT origin={0,0};DWORD saved_error=GetLastError();
    DWORD style=(DWORD)GetWindowLongPtrW(w,GWL_STYLE),ex=(DWORD)GetWindowLongPtrW(w,GWL_EXSTYLE);
    if(!style_control_protocol_legacy_progress(w) ||
        !(ex&WS_EX_STATICEDGE) || (ex&(WS_EX_LAYOUTRTL|WS_EX_RTLREADING|WS_EX_CLIENTEDGE)) ||
        (style&(PBS_MARQUEE|WS_HSCROLL|WS_VSCROLL)) ||
        !GetWindowRect(w,&outer) || !GetClientRect(w,&client) || !ClientToScreen(w,&origin))goto done;
    int width=outer.right-outer.left,height=outer.bottom-outer.top;
    int left=origin.x-outer.left,top=origin.y-outer.top;
    if(width<3 || height<3 || left<1 || top<1 || left+client.right>=width || top+client.bottom>=height)goto done;
    RECT bounds={0,0,width,height};HDC dc=target?target:GetWindowDC(w);if(!dc)goto done;
    int saved=SaveDC(dc);if(saved){style_paint_solid_frame(dc,bounds,palette->edge,1);RestoreDC(dc,saved);}
    if(!target)ReleaseDC(w,dc);
done: SetLastError(saved_error);
}

static void vb_data_arrow(HWND w,HDC dc,int x,int y,int width,HDC source,const RECT *client_bounds,const win16_style_palette *palette);

void win16_control_vb_data_arrow_candidate(HDC dc,int x,int y,int width,int height,HDC source,int sx,int sy,int sw,int sh,DWORD rop,const void *caller,BOOL (*attached_palette)(HWND,win16_style_palette *))
{
    HWND w=WindowFromDC(dc);RECT client;POINT origin,viewport;BITMAP bitmap={0};HMODULE owner=NULL;
    if(!w || GetWindowThreadProcessId(w,NULL)!=GetCurrentThreadId() ||
       !style_control_vb_class(w,L"ThunderRT6Data") || !GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,(const WCHAR *)caller,&owner) ||
       owner!=GetModuleHandleW(L"MSVBVM60.DLL") ||
       !GetClientRect(w,&client) || client.right<70 || !((GetDpiForWindow(w)==96 && client.bottom==22) || (GetDpiForWindow(w)==144 && client.bottom==29)) || rop!=SRCCOPY || sx || sy || height!=13 || sh!=13 || width!=sw || (y!=(client.bottom-13)/2 && y!=(client.bottom-13)/2+1) ||
       GetMapMode(dc)!=MM_TEXT || GetLayout(dc)!=0 || !GetWindowOrgEx(dc,&origin) || origin.x || origin.y ||
       !GetViewportOrgEx(dc,&viewport) || viewport.x || viewport.y ||
       !GetObjectW(GetCurrentObject(source,OBJ_BITMAP),sizeof(bitmap),&bitmap) || bitmap.bmWidth!=sw || bitmap.bmHeight!=sh)return;
    win16_style_palette palette;
    if(attached_palette(w,&palette))vb_data_arrow(w,dc,x,y,width,source,&client,&palette);
}

static void vb_data_arrow(HWND w,HDC dc,int x,int y,int width,HDC source,const RECT *client_bounds,const win16_style_palette *palette)
{
    RECT client=*client_bounds;
    int n=-1;const int xs[4]={4,24,client.right-33,client.right-15};
    const int starts[4]={0,18,client.right-35,client.right-18},widths[4]={18,17,17,18};
    int base_y=(client.bottom-13)/2;
    for(int i=0;i<4;++i)if(x==xs[i]+(y-base_y) && width==(i==0 || i==3?11:8)){n=i;break;}
    if(n<0)return;
    BOOL disabled=!IsWindowEnabled(w),pressed=(y==base_y+1);const char (*bits)[18]=n<2?
        (disabled?win16_control_scroll_glyph(WIN16_SCROLL_LEFT_DISABLED):pressed?win16_control_scroll_glyph(WIN16_SCROLL_LEFT_PRESSED):win16_control_scroll_glyph(WIN16_SCROLL_LEFT)):
        (disabled?win16_control_scroll_glyph(WIN16_SCROLL_RIGHT_DISABLED):pressed?win16_control_scroll_glyph(WIN16_SCROLL_RIGHT_PRESSED):win16_control_scroll_glyph(WIN16_SCROLL_RIGHT));
    COLORREF colors[4]={palette->edge,palette->light,palette->shadow,palette->face};DWORD pixels[18*29];
    int extra=client.bottom-17,mid=extra/2;
    for(int j=0;j<client.bottom;++j){int by=j<2?j:j>=client.bottom-2?j-extra:j<2+mid?2:j<15+mid?j-mid:14;
        for(int i=0;i<widths[n];++i){int bx=i<2?i:i>=widths[n]-2?i-(widths[n]-17):i;if(bx>14)bx=14;
            COLORREF c=colors[bits[by][bx]-'0'];pixels[j*widths[n]+i]=((DWORD)GetRValue(c)<<16)|((DWORD)GetGValue(c)<<8)|GetBValue(c);}}
    if(n==0 || n==3){int column=n==0?1:8;unsigned mark=0;
        for(int j=0;j<13;++j)for(int i=0;i<2;++i)if(GetPixel(source,column+i,j)==RGB(0,0,0)){
            int px=x+column+i-starts[n],py=y+j;if(px>=0 && px<widths[n] && py>=0 && py<client.bottom){COLORREF c=disabled?palette->shadow:palette->edge;
                pixels[py*widths[n]+px]=((DWORD)GetRValue(c)<<16)|((DWORD)GetGValue(c)<<8)|GetBValue(c);++mark;}}
        if(mark!=22)return;
    }
    BITMAPINFO info={0};info.bmiHeader=(BITMAPINFOHEADER){.biSize=sizeof(BITMAPINFOHEADER),.biWidth=widths[n],.biHeight=-client.bottom,.biPlanes=1,.biBitCount=32,.biCompression=BI_RGB};
    (void)SetDIBitsToDevice(dc,starts[n],0,widths[n],client.bottom,0,0,0,client.bottom,pixels,&info,DIB_RGB_COLORS);
}

static BOOL runtime_push_frame(HWND w,HDC dc,RECT bounds,UINT state,const win16_style_palette *palette,
    const win16_runtime_push *theme)
{
    int unit=max(1,MulDiv(1,(int)GetDpiForWindow(w),96));
    style_relief_palette colors=win16_control_push_palette(palette);
    style_push_profile profile=STYLE_PUSH_STEPPED;
    if(theme->profile){
        const classic_surface_colors part=classic_push_color(&theme->colors,(state&DFCS_INACTIVE)!=0,
            (state&(DFCS_PUSHED|DFCS_CHECKED))!=0,FALSE);
        colors.face=colors.light=part.face;
        colors.dark=part.edge;
        profile=theme->profile;
    }
    return style_paint_push_frame(dc,&bounds,state,FALSE,unit,profile,&colors);
}

BOOL win16_style_bitbtn_face(HWND w,HDC dc,const RECT *rect,const RECT *clip,UINT state,
    const win16_style_palette *palette,const win16_runtime_push *theme)
{
    if(!w || !dc || !rect || rect->right-rect->left<4 || rect->bottom-rect->top<4)return FALSE;
    int saved=SaveDC(dc);if(!saved)return FALSE;
    if(clip && IntersectClipRect(dc,clip->left,clip->top,clip->right,clip->bottom)==ERROR){RestoreDC(dc,saved);return FALSE;}
    UINT flags=((state&ODS_SELECTED)?DFCS_PUSHED:0)|((state&ODS_DISABLED)?DFCS_INACTIVE:0);
    BOOL painted=runtime_push_frame(w,dc,*rect,flags,palette,theme);
    RestoreDC(dc,saved);return painted;
}

BOOL win16_control_checklist_frame(HDC dc,const RECT *rect,UINT type,UINT flags,const void *address,BOOL (*target)(HDC,const RECT *,const void *))
{
    const UINT allowed=DFCS_CHECKED|DFCS_PUSHED;
    if(type!=DFC_BUTTON || (flags&15u)!=DFCS_BUTTONCHECK || (flags&~allowed) ||
        !target(dc,rect,address))return FALSE;
    return win16_control_checklist_glyph(dc,rect,flags);
}
static control_style *graphical_target(HDC dc,UINT type,UINT flags,const void *address,HWND *w,SUBCLASSPROC proc)
{
    DWORD_PTR value;HMODULE caller=NULL;
    const UINT allowed=DFCS_BUTTONPUSH|DFCS_PUSHED|DFCS_INACTIVE|DFCS_ADJUSTRECT|DFCS_CHECKED;
    if(type!=DFC_BUTTON || !(flags&DFCS_BUTTONPUSH) || (flags&~allowed) ||
            !style_runtime_text_report_target(dc,w) || !ntw16s_vb_graphical_button(*w) ||
            ((flags&DFCS_CHECKED) && !style_control_vb_class(*w,L"ThunderRT6OptionButton") &&
             !style_control_vb_class(*w,L"ThunderRT6CheckBox") && !style_control_vb5_class(*w,L"ThunderRT5CheckBox") &&
             !style_control_vb5_class(*w,L"ThunderRT5OptionButton")) ||
            !GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,(const WCHAR *)address,&caller) ||
            caller!=(HMODULE)GetClassLongPtrW(*w,GCLP_HMODULE) ||
            !GetWindowSubclass(*w,proc,(UINT_PTR)proc,&value))return NULL;
    control_style *s=(void *)value;return !s->instance.detached && s->bindings.scroll.resources.frame?s:NULL;
}
BOOL win16_control_graphical_frame_prepare(HDC dc,const RECT *rect,UINT type,UINT flags,const void *address,RECT *copy,SUBCLASSPROC proc)
{
    HWND w;
    /* Ordinary API calls remain untouched, including parameter validation.
     * Copy only a trusted owned-runtime primitive inside its exact scope. */
    if(!rect || !copy || !graphical_target(dc,type,flags,address,&w,proc))return FALSE;
    *copy=*rect;return TRUE;
}
BOOL win16_control_graphical_frame(HDC dc,const RECT *rect,UINT type,UINT flags,const void *address,SUBCLASSPROC proc)
{
    HWND w;RECT client;control_style *s=graphical_target(dc,type,flags,address,&w,proc);
    if(!s || !rect || !GetClientRect(w,&client) || !EqualRect(rect,&client) ||
            !style_dc_pixel_viewport(dc))return FALSE;
    win16_style_palette palette=s->palette;
    if(s->frame_colors.present){
        palette.light=s->frame_colors.light;
        palette.shadow=s->frame_colors.shadow;
    }
    const win16_runtime_push theme={s->push_colors,s->themed?
        (s->controls.push?s->controls.push:STYLE_PUSH_STEPPED):STYLE_PUSH_DEFAULT};
    int saved=SaveDC(dc);
    if(!saved)return FALSE;
    BOOL painted=runtime_push_frame(w,dc,*rect,flags,&palette,&theme);
    RestoreDC(dc,saved);return painted;
}

BOOL win16_control_cool_grip_identity(HWND w,void *owner,const NMCUSTOMDRAW *n,WNDPROC native,BOOL (*valid)(HWND,void *))
{
    const style_control_rebar_scope scope={w,owner,native,valid};style_control_rebar_probe p;RECT overlap,r=n->rc;
    if(!style_control_rebar_read(&scope,n,CCS_VERT,&p) ||
       (p.bar_back!=CLR_NONE && p.bar_back!=GetSysColor(COLOR_BTNFACE)) || (p.bar_fore!=CLR_NONE && p.bar_fore!=GetSysColor(COLOR_BTNTEXT)) ||
       p.back!=GetSysColor(COLOR_BTNFACE) || p.fore!=GetSysColor(COLOR_BTNTEXT) || p.bitmap || p.text[0] || p.text_capacity>ARRAYSIZE(p.text) || p.image!=-1 ||
       !p.child || GetParent(p.child)!=w || (p.style&~(RBBS_BREAK|RBBS_FIXEDBMP|RBBS_GRIPPERALWAYS)) || !style_control_rebar_geometry(&scope,&p))return FALSE;
    return !IsRectEmpty(&r) && r.left==p.band.left && r.top==p.band.top && r.bottom==p.band.bottom && p.border.left>0 &&
        r.right<=p.band.left+p.border.left && r.right-r.left<=MulDiv(12,p.dpi,96) && r.left>=p.client.left && r.top>=p.client.top &&
        r.right<=p.client.right && r.bottom<=p.client.bottom && !IntersectRect(&overlap,&r,&p.child_rect);
}
