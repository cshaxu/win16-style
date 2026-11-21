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
