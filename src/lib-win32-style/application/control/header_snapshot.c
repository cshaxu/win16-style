#include "header_snapshot.h"
#include <commctrl.h>
#include <stdlib.h>
#include <wchar.h>
BOOL style_header_supported(HWND window,DWORD extra_exclusions)
{
    DWORD style=(DWORD)GetWindowLongPtrW(window,GWL_STYLE);
    return !(style&(HDS_FILTERBAR|HDS_CHECKBOXES|HDS_OVERFLOW|HDS_HIDDEN|extra_exclusions));
}
void style_header_draw_text(HDC dc,RECT rect,PCWSTR text,int format,
    const style_header_colors *colors,BOOL pressed,COLORREF fallback)
{
    unsigned state=pressed!=FALSE;
    SetBkMode(dc,TRANSPARENT);
    SetTextColor(dc,colors && colors->has_text[state]?colors->text[state]:fallback);
    int alignment=format&HDF_JUSTIFYMASK;
    UINT flags=DT_VCENTER|DT_SINGLELINE|DT_NOPREFIX|DT_END_ELLIPSIS;
    flags|=alignment==HDF_RIGHT?DT_RIGHT:alignment==HDF_CENTER?DT_CENTER:DT_LEFT;
    if(format&HDF_RTLREADING)flags|=DT_RTLREADING;
    DrawTextW(dc,text,-1,&rect,flags);
}

void style_header_snapshot_clear(style_header_snapshot *snapshot)
{
    for(int i=0;i<snapshot->count;++i)free(snapshot->items[i].text);
    free(snapshot->items);*snapshot=(style_header_snapshot){0};
}
static WCHAR *header_text(HWND w,int index,int *format,const style_instance *instance,const style_control_native_state *state,unsigned epoch,UINT allowed)
{
    for(int capacity=128;capacity<=32768;capacity*=2){
        WCHAR *text=calloc((size_t)capacity,sizeof(*text));if(!text)return NULL;
        HDITEMW item={.mask=HDI_TEXT|HDI_FORMAT,.pszText=text,.cchTextMax=capacity};
        BOOL ok=(BOOL)SendMessageW(w,HDM_GETITEMW,index,(LPARAM)&item);text[capacity-1]=0;
        if(!ok || !style_control_native_current(w,instance,state,epoch) || item.pszText!=text ||
            (item.fmt&~allowed)){free(text);return NULL;}
        if(wcslen(text)<(size_t)capacity-1){*format=item.fmt;return text;}
        free(text);
    }
    return NULL;
}
BOOL style_header_snapshot_read(HWND w,const style_instance *instance,const style_control_native_state *state,UINT allowed,style_header_snapshot *snapshot)
{
    *snapshot=(style_header_snapshot){.epoch=state->epoch};
    LRESULT count=SendMessageW(w,HDM_GETITEMCOUNT,0,0);
    if(!style_control_native_current(w,instance,state,snapshot->epoch) || count<0 || count>1024 || !GetClientRect(w,&snapshot->client))return FALSE;
    snapshot->items=count?calloc((size_t)count,sizeof(*snapshot->items)):NULL;
    if(count && !snapshot->items)return FALSE;
    snapshot->count=(int)count;
    for(int i=0;i<count;++i){
        BOOL ok=(BOOL)SendMessageW(w,HDM_GETITEMRECT,i,(LPARAM)&snapshot->items[i].rect);
        if(!ok || !style_control_native_current(w,instance,state,snapshot->epoch))goto failed;
        snapshot->items[i].text=header_text(w,i,&snapshot->items[i].format,instance,state,snapshot->epoch,allowed);
        if(!snapshot->items[i].text)goto failed;
    }
    return TRUE;
failed:
    style_header_snapshot_clear(snapshot);return FALSE;
}

static const UINT header_formats=HDF_STRING|HDF_JUSTIFYMASK|HDF_RTLREADING;

static BOOL paint_item(HDC dc,RECT frame,RECT label,PCWSTR text,int format,
    BOOL button,BOOL bordered,BOOL pressed,int unit,style_line_profile profile,
    const style_relief_palette *palette,const style_header_colors *colors,COLORREF ink)
{
    if(!style_paint_header_edge(dc,frame,button,bordered,pressed,unit,profile,palette))return FALSE;
    if(*text)style_header_draw_text(dc,label,text,format,colors,pressed,ink);
    return TRUE;
}

BOOL style_header_client_supported(HWND w)
{return style_header_supported(w,0);}

static void fill(HDC dc,RECT r,COLORREF color)
{SetDCBrushColor(dc,color);FillRect(dc,&r,GetStockObject(DC_BRUSH));}
BOOL style_header_client_paint(HWND w,HDC dc,const style_relief_palette *p,HFONT font,
    const style_instance *instance,const style_control_native_state *state,const style_header_colors *colors,
    style_line_profile profile)
{
    BOOL painted=FALSE;int saved=0;style_header_snapshot snapshot;
    if(!dc || !profile || !style_header_client_supported(w) || !style_header_snapshot_read(w,instance,state,header_formats,&snapshot))return FALSE;
    RECT client=snapshot.client;int count=snapshot.count;style_header_item *items=snapshot.items;
    saved=SaveDC(dc);if(!saved)goto done;
    IntersectClipRect(dc,client.left,client.top,client.right,client.bottom);
    COLORREF normal=colors && colors->present?colors->normal:p->face;
    fill(dc,client,normal);SelectObject(dc,font?font:GetStockObject(SYSTEM_FONT));
    BOOL enabled=IsWindowEnabled(w);
    int u=max(1,MulDiv(1,(int)GetDpiForWindow(w),96));
    RECT bottom=client;bottom.top=max(bottom.top,bottom.bottom-u);fill(dc,bottom,p->dark);
    DWORD style=(DWORD)GetWindowLongPtrW(w,GWL_STYLE);
    BOOL button=(style&HDS_BUTTONS)!=0,captured=button && GetCapture()==w;
    const style_relief_palette edge={.face=normal,.light=p->light,.highlight=p->highlight,.shadow=p->shadow,.dark=p->dark};
    for(int i=0;i<count;++i){
        RECT r=items[i].rect,visible;if(!IntersectRect(&visible,&client,&r))continue;
        int inner=SaveDC(dc);if(!inner)goto done;
        IntersectClipRect(dc,r.left,r.top,r.right,r.bottom);
        BOOL pressed=captured && i==state->pressed;
        RECT frame=r;
        for(int n=0;n<u && !IsRectEmpty(&r);++n)InflateRect(&r,-1,-1);
        if(pressed){
            if(colors && colors->present && colors->pressed!=normal){
                RECT interior=r;interior.bottom=min(interior.bottom,bottom.top);
                fill(dc,interior,colors->pressed);
            }
            OffsetRect(&r,u,u);
        }
        InflateRect(&r,-3*u,0);
        int format=items[i].format;
        if(GetWindowLongPtrW(w,GWL_EXSTYLE)&WS_EX_RTLREADING)format|=HDF_RTLREADING;
        if(!paint_item(dc,frame,r,items[i].text,format,button,(style&WS_BORDER)!=0,
            pressed,u,profile,&edge,enabled?colors:NULL,enabled?p->text:p->gray)){
            RestoreDC(dc,inner);
            goto done;
        }
        RestoreDC(dc,inner);
    }
    painted=style_control_native_current(w,instance,state,snapshot.epoch);
done:
    if(saved)RestoreDC(dc,saved);
    style_header_snapshot_clear(&snapshot);return painted;
}

BOOL style_header_native_supported(HWND window)
{return style_header_supported(window,HDS_HOTTRACK);}

BOOL style_header_native_part(HWND window,const NMCUSTOMDRAW *draw,const void *context)
{
    const style_header_notification *parameters=context;
    if(!parameters->profile)return FALSE;
    const style_instance *instance=parameters->instance;
    DWORD style=(DWORD)GetWindowLongPtrW(window,GWL_STYLE);
    const style_relief_palette *p=&parameters->palette;
    WCHAR text[256]={0};
    HDITEMW item={.mask=HDI_FORMAT,.pszText=text,.cchTextMax=ARRAYSIZE(text)};
    LRESULT index=(LRESULT)draw->dwItemSpec;
    if(index<0 || !SendMessageW(window,HDM_GETITEMW,index,(LPARAM)&item) || instance->detached ||
       (item.fmt&~header_formats))return FALSE;
    item.mask=HDI_TEXT;
    if((item.fmt&HDF_STRING) && (!SendMessageW(window,HDM_GETITEMW,index,(LPARAM)&item) || instance->detached ||
       item.pszText!=text || text[ARRAYSIZE(text)-2]))return FALSE;
    HDC dc=draw->hdc;
    RECT r=draw->rc;
    if(IsRectEmpty(&r))return TRUE;
    int saved=SaveDC(dc);
    if(!saved)return FALSE;
    BOOL pressed=(style&HDS_BUTTONS) && (draw->uItemState&CDIS_SELECTED);
    const style_header_colors *colors=parameters->colors;
    COLORREF face=colors->present?(pressed?colors->pressed:colors->normal):p->face;
    int unit=max(1,MulDiv(1,(int)GetDpiForWindow(window),96));
    RECT label=r;
    InflateRect(&label,-6,0);
    if(pressed)OffsetRect(&label,1,1);
    BOOL painted=IntersectClipRect(dc,r.left,r.top,r.right,r.bottom)!=ERROR &&
        style_paint_color(dc,&r,face) &&
        paint_item(dc,r,label,text,item.fmt,(style&HDS_BUTTONS)!=0,(style&WS_BORDER)!=0,
            pressed,unit,parameters->profile,p,IsWindowEnabled(window)?colors:NULL,p->text);
    return RestoreDC(dc,saved) && painted;
}
