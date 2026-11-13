#include "lib-style/common/control_custom_draw.h"
static BOOL supported(HWND window){return window==NULL;}
static BOOL paint(HWND window,const NMCUSTOMDRAW *draw,const void *context)
{(void)window;(void)draw;return *(const BOOL *)context;}
static HWND post_child;
static BOOL post_active,post_painted,post_valid=TRUE;
static unsigned post_calls;
static LRESULT app_result;
static BOOL valid_post(HWND window,void *owner)
{return window==post_child && owner==&post_active && post_valid;}
static BOOL paint_post(HWND window,void *owner,const NMCUSTOMDRAW *draw)
{(void)window;(void)owner;(void)draw;++post_calls;return TRUE;}
static LRESULT CALLBACK app_notify(HWND window,UINT message,WPARAM wp,LPARAM lp,UINT_PTR id,DWORD_PTR data)
{(void)id;(void)data;if(message==WM_NOTIFY)return app_result;return DefSubclassProc(window,message,wp,lp);}
static LRESULT CALLBACK post_notify(HWND window,UINT message,WPARAM wp,LPARAM lp,UINT_PTR id,DWORD_PTR data)
{(void)id;(void)data;return style_control_custom_draw_post(window,message,wp,lp,&post_active,post_child,
    &post_active,&post_painted,valid_post,paint_post);}
int main(void)
{
    NMCUSTOMDRAW draw={0};BOOL active=FALSE,painted=TRUE;
    HWND parent=CreateWindowW(L"STATIC",L"",WS_POPUP,0,0,20,20,NULL,NULL,GetModuleHandleW(NULL),NULL);
    HWND child=parent?CreateWindowW(L"STATIC",L"",WS_CHILD,0,0,10,10,parent,NULL,GetModuleHandleW(NULL),NULL):NULL;
    if(!parent || !child)return 7;
    draw.hdr.hwndFrom=child;draw.hdr.code=NM_CUSTOMDRAW;
    if(style_control_custom_draw_message(parent,child,WM_NOTIFY,(LPARAM)&draw,FALSE)!=&draw ||
       style_control_custom_draw_message(parent,child,WM_PAINT,(LPARAM)&draw,FALSE) ||
       style_control_custom_draw_message(parent,child,WM_NOTIFY,(LPARAM)&draw,TRUE) ||
       style_control_custom_draw_message(child,child,WM_NOTIFY,(LPARAM)&draw,FALSE))return 8;
    draw.hdr.hwndFrom=parent;
    if(style_control_custom_draw_message(parent,child,WM_NOTIFY,(LPARAM)&draw,FALSE))return 9;
    post_child=child;draw.hdr.hwndFrom=child;
    if(!SetWindowSubclass(parent,app_notify,1,0) || !SetWindowSubclass(parent,post_notify,2,0))return 11;
    draw.dwDrawStage=CDDS_PREPAINT;app_result=CDRF_SKIPDEFAULT;
    if(SendMessageW(parent,WM_NOTIFY,0,(LPARAM)&draw)!=CDRF_SKIPDEFAULT || post_active)return 12;
    app_result=CDRF_DODEFAULT;
    if(SendMessageW(parent,WM_NOTIFY,0,(LPARAM)&draw)!=CDRF_NOTIFYITEMDRAW || !post_active)return 13;
    draw.dwDrawStage=CDDS_ITEMPREPAINT;
    if(SendMessageW(parent,WM_NOTIFY,0,(LPARAM)&draw)!=CDRF_NOTIFYPOSTPAINT)return 14;
    draw.dwDrawStage=CDDS_ITEMPOSTPAINT;post_valid=FALSE;app_result=CDRF_SKIPDEFAULT;
    if(SendMessageW(parent,WM_NOTIFY,0,(LPARAM)&draw)!=CDRF_SKIPDEFAULT || post_calls)return 15;
    post_valid=TRUE;app_result=CDRF_DODEFAULT;
    if(SendMessageW(parent,WM_NOTIFY,0,(LPARAM)&draw)!=CDRF_DODEFAULT || !post_painted || post_calls!=1)return 16;
    if(!DestroyWindow(parent))return 10;
    draw.dwDrawStage=CDDS_PREPAINT;
    if(style_control_custom_draw_after(NULL,&draw,&active,FALSE,CDRF_SKIPDEFAULT,&painted,supported,paint)!=CDRF_SKIPDEFAULT || active)return 1;
    if(style_control_custom_draw_after(NULL,&draw,&active,FALSE,CDRF_DODEFAULT,&painted,supported,paint)!=(CDRF_NOTIFYITEMDRAW|CDRF_NOTIFYPOSTPAINT) || !active)return 2;
    draw.dwDrawStage=CDDS_ITEMPREPAINT;
    if(style_control_custom_draw_after(NULL,&draw,&active,FALSE,CDRF_DODEFAULT,&painted,supported,paint)!=CDRF_DODEFAULT)return 3;
    if(style_control_custom_draw_after(NULL,&draw,&active,TRUE,CDRF_DODEFAULT,&painted,supported,paint)!=CDRF_SKIPDEFAULT)return 4;
    painted=FALSE;
    if(style_control_custom_draw_after(NULL,&draw,&active,TRUE,CDRF_DODEFAULT,&painted,supported,paint)!=CDRF_DODEFAULT)return 5;
    draw.dwDrawStage=CDDS_POSTPAINT;
    if(style_control_custom_draw_after(NULL,&draw,&active,TRUE,CDRF_DODEFAULT,&painted,supported,paint)!=CDRF_DODEFAULT || active)return 6;
    return 0;
}
