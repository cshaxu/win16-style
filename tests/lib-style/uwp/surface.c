#include <windows.h>
#include <stdio.h>
#include "lib-style/uwp/surface.h"
#define CHECK(x) do{if(!(x)){fprintf(stderr,"UWP SURFACE FAIL line=%d error=%lu\n",__LINE__,GetLastError());return 1;}}while(0)
typedef struct refresh_fixture {unsigned geometry,paint;BOOL fail,detach;BOOL *detached;} refresh_fixture;
static BOOL refresh_geometry(void *context,RECT *outer,RECT *hole)
{
    refresh_fixture *f=context;++f->geometry;
    *outer=(RECT){20,20,100,70};*hole=(RECT){5,5,75,45};return !f->fail;
}
static void refresh_paint(void *context,HDC dc)
{
    refresh_fixture *f=context;(void)dc;++f->paint;
    if(f->detach)*f->detached=TRUE;
}
int main(void)
{
    RECT bounds={0,0,100,80},client_bounds={8,20,92,72};SIZE corner={16,20};
    CHECK(style_uwp_resize_margin_hit(HTBORDER,(POINT){7,30},&bounds,&client_bounds,corner,TRUE)==HTLEFT);
    CHECK(style_uwp_resize_margin_hit(HTBORDER,(POINT){92,79},&bounds,&client_bounds,corner,TRUE)==HTBOTTOMRIGHT);
    CHECK(style_uwp_resize_margin_hit(HTBORDER,(POINT){50,72},&bounds,&client_bounds,corner,TRUE)==HTBOTTOM);
    CHECK(style_uwp_resize_margin_hit(HTCLIENT,(POINT){7,30},&bounds,&client_bounds,corner,TRUE)==HTCLIENT);
    CHECK(style_uwp_resize_margin_hit(HTBORDER,(POINT){7,30},&bounds,&client_bounds,corner,FALSE)==HTBORDER);
    CHECK(style_uwp_resize_margin_hit(HTBORDER,(POINT){7,30},NULL,&client_bounds,corner,TRUE)==HTBORDER);
    style_bitmap bitmap={0};SIZE first={80,50},second={120,40},bad={0,10};
    CHECK(style_bitmap_ensure(&bitmap,NULL,first));
    HDC original=bitmap.dc;HBITMAP image=bitmap.bitmap;CHECK(bitmap.pixels && original && image);
    bitmap.pixels[0]=77;CHECK(style_bitmap_ensure(&bitmap,NULL,first) && bitmap.dc==original && bitmap.bitmap==image && bitmap.pixels[0]==77);
    CHECK(!style_bitmap_ensure(&bitmap,NULL,bad) && GetLastError()==ERROR_INVALID_PARAMETER && bitmap.dc==original);
    CHECK(style_bitmap_ensure(&bitmap,NULL,second) && bitmap.pixels && bitmap.size.cx==120 && bitmap.size.cy==40);
    style_bitmap_dispose(&bitmap);style_bitmap_dispose(&bitmap);CHECK(!bitmap.dc && !bitmap.bitmap && !bitmap.pixels);
    HWND root=CreateWindowW(L"STATIC",L"Surface fixture",WS_OVERLAPPEDWINDOW|WS_VISIBLE,20,20,420,280,NULL,NULL,GetModuleHandleW(NULL),NULL);
    CHECK(root);ShowWindow(root,SW_SHOWNOACTIVATE);RECT client;CHECK(GetClientRect(root,&client));
    HWND surface=CreateWindowExW(WS_EX_LAYERED,L"STATIC",L"",WS_POPUP,0,0,80,50,root,NULL,GetModuleHandleW(NULL),NULL);
    CHECK(surface);BOOL detached=FALSE,updating=FALSE;style_bitmap refresh_bitmap={0};
    refresh_fixture fixture={.detached=&detached};
    style_uwp_surface_view view={root,&surface,&detached,&updating,&refresh_bitmap,&fixture,refresh_geometry,refresh_paint};
    updating=TRUE;CHECK(style_uwp_surface_refresh(&view) && !fixture.geometry);updating=FALSE;
    detached=TRUE;CHECK(!style_uwp_surface_refresh(&view) && !fixture.geometry);detached=FALSE;
    fixture.fail=TRUE;CHECK(!style_uwp_surface_refresh(&view) && fixture.geometry==1 && !fixture.paint && !updating);
    fixture.fail=FALSE;fixture.detach=TRUE;
    CHECK(!style_uwp_surface_refresh(&view) && fixture.geometry==2 && fixture.paint==1 && detached && !updating);
    fixture.detach=FALSE;detached=FALSE;SetLastError(ERROR_SUCCESS);
    BOOL native_present=style_uwp_surface_refresh(&view);DWORD present_error=GetLastError();
    CHECK(fixture.geometry==3 && fixture.paint==2 && !detached && !updating);
    if(native_present)CHECK(IsWindowVisible(surface));
    else printf("UWP SURFACE NATIVE PRESENT SKIP error=%lu\n",(unsigned long)present_error);
    style_bitmap_dispose(&refresh_bitmap);CHECK(DestroyWindow(surface));
    HWND title=CreateWindowW(L"STATIC",L"title",WS_CHILD|WS_VISIBLE,0,0,client.right,32,root,NULL,GetModuleHandleW(NULL),NULL);
    CHECK(title);RECT outer,content,screen_title;BOOL zoomed;
    CHECK(style_uwp_frame_bounds(root,title,&outer,&content,&zoomed) && !zoomed && GetWindowRect(title,&screen_title));
    CHECK(content.top==screen_title.bottom-outer.top && outer.right>outer.left && content.right>content.left);
    ShowWindow(title,SW_HIDE);CHECK(!style_uwp_frame_bounds(root,title,&outer,&content,&zoomed));
    CHECK(style_uwp_frame_bounds(root,NULL,&outer,&content,&zoomed));
    ShowWindow(title,SW_SHOW);ShowWindow(root,SW_MAXIMIZE);CHECK(IsZoomed(root) && GetClientRect(root,&client));
    CHECK(MoveWindow(title,0,0,client.right,32,TRUE));
    MONITORINFO monitor={.cbSize=sizeof(monitor)};
    CHECK(GetMonitorInfoW(MonitorFromWindow(root,MONITOR_DEFAULTTONEAREST),&monitor));
    CHECK(style_uwp_frame_bounds(root,title,&outer,&content,&zoomed) && zoomed);
    CHECK(outer.left>=monitor.rcWork.left && outer.top>=monitor.rcWork.top &&
          outer.right<=monitor.rcWork.right && outer.bottom<=monitor.rcWork.bottom &&
          content.left>=0 && content.top>=0 && content.right<=outer.right-outer.left &&
          content.bottom<=outer.bottom-outer.top);
    CHECK(DestroyWindow(root));
    printf("UWP SURFACE PASS bitmap=1 reuse=1 resize=1 invalid=1 bounds=1 hidden=1 maximize=1 margin=1 refresh_failure=1 reentry=1 detach=1 native_present=%d\n",native_present);return 0;
}
