#include "lib-style/common/control_custom_draw.h"
static BOOL supported(HWND window){return window==NULL;}
static BOOL paint(HWND window,const NMCUSTOMDRAW *draw,const void *context)
{(void)window;(void)draw;return *(const BOOL *)context;}
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
