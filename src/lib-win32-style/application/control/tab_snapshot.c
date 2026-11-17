#include "tab_snapshot.h"
#include <stdlib.h>
#include <wchar.h>

static WCHAR *item_text(HWND w,int index,TCITEMW *item,const style_instance *instance,const style_control_native_state *state,unsigned epoch)
{
    for(int capacity=128;capacity<=32768;capacity*=2){
        WCHAR *text=calloc((size_t)capacity,sizeof(*text));if(!text)return NULL;
        *item=(TCITEMW){.mask=TCIF_TEXT|TCIF_IMAGE|TCIF_STATE,.dwStateMask=TCIS_BUTTONPRESSED|TCIS_HIGHLIGHTED,
            .pszText=text,.cchTextMax=capacity};
        BOOL ok=(BOOL)SendMessageW(w,TCM_GETITEMW,index,(LPARAM)item);
        text[capacity-1]=0;
        if(!ok || !style_control_native_current(w,instance,state,epoch)){free(text);return NULL;}
        if(wcslen(text)<(size_t)capacity-1)return text;
        free(text);
    }
    return NULL;
}
BOOL style_control_tab_paint(HWND w,HDC dc,const style_instance *instance,const style_control_native_state *state,
    const style_control_tab_painter *painter)
{
    if(!dc)return FALSE;
    style_control_tab_snapshot snapshot={.epoch=state->epoch};
    BOOL painted=FALSE;int saved=SaveDC(dc);if(!saved)return FALSE;
    LRESULT count=SendMessageW(w,TCM_GETITEMCOUNT,0,0);
    if(!style_control_native_current(w,instance,state,snapshot.epoch) || count<0 || count>4096)goto done;
    snapshot.count=(int)count;
    if(!GetClientRect(w,&snapshot.client))goto done;
    if(IsRectEmpty(&snapshot.client)){painted=TRUE;goto done;}
    IntersectClipRect(dc,snapshot.client.left,snapshot.client.top,snapshot.client.right,snapshot.client.bottom);
    snapshot.page=snapshot.client;SendMessageW(w,TCM_ADJUSTRECT,FALSE,(LPARAM)&snapshot.page);
    if(!style_control_native_current(w,instance,state,snapshot.epoch) || IsRectEmpty(&snapshot.page))goto done;
    ExcludeClipRect(dc,snapshot.page.left,snapshot.page.top,snapshot.page.right,snapshot.page.bottom);
    unsigned children=0;
    for(HWND child=GetWindow(w,GW_CHILD);child && children++<256;child=GetWindow(child,GW_HWNDNEXT)){
        RECT r;if(IsWindowVisible(child) && GetWindowRect(child,&r)){
            MapWindowPoints(NULL,w,(POINT*)&r,2);ExcludeClipRect(dc,r.left,r.top,r.right,r.bottom);}
    }
    painter->surface(w,painter->context,dc,&snapshot);
    snapshot.selected=(int)SendMessageW(w,TCM_GETCURSEL,0,0);
    snapshot.focus=(int)SendMessageW(w,TCM_GETCURFOCUS,0,0);
    snapshot.cues=(UINT)SendMessageW(w,WM_QUERYUISTATE,0,0);
    if(!style_control_native_current(w,instance,state,snapshot.epoch))goto done;
    painter->prepare(w,painter->context,dc,&snapshot);
    for(unsigned pass=0;pass<2;++pass)for(int index=0;index<snapshot.count;++index){
        BOOL active=index==snapshot.selected;if(active!=(BOOL)pass)continue;
        style_control_tab_item item={.index=index,.active=active};RECT visible;
        BOOL rect_ok=(BOOL)SendMessageW(w,TCM_GETITEMRECT,index,(LPARAM)&item.rect);
        if(!style_control_native_current(w,instance,state,snapshot.epoch) || !rect_ok)goto done;
        if(!IntersectRect(&visible,&item.rect,&snapshot.client))continue;
        WCHAR *text=item_text(w,index,&item.native,instance,state,snapshot.epoch);if(!text)goto done;
        item.text=text;item.images=(HIMAGELIST)SendMessageW(w,TCM_GETIMAGELIST,0,0);
        if(!style_control_native_current(w,instance,state,snapshot.epoch)){free(text);goto done;}
        /* No application messages between borrowing the list and consuming it. */
        int cx=0,cy=0;
        item.image=item.images && item.native.iImage>=0 && item.native.iImage<ImageList_GetImageCount(item.images) &&
            ImageList_GetIconSize(item.images,&cx,&cy);
        item.image_size=(SIZE){cx,cy};
        painter->item(w,painter->context,dc,&snapshot,&item);
        free(text);
    }
    painted=style_control_native_current(w,instance,state,snapshot.epoch);
done:
    RestoreDC(dc,saved);return painted;
}
