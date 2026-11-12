#include <windows.h>
#include <stdio.h>
#include "lib-style/uwp/surface.h"
#define CHECK(x) do{if(!(x)){fprintf(stderr,"UWP SURFACE FAIL line=%d error=%lu\n",__LINE__,GetLastError());return 1;}}while(0)
int main(void)
{
    style_uwp_bitmap bitmap={0};SIZE first={80,50},second={120,40},bad={0,10};
    CHECK(style_uwp_bitmap_ensure(&bitmap,first));
    HDC original=bitmap.dc;HBITMAP image=bitmap.bitmap;CHECK(bitmap.pixels && original && image);
    bitmap.pixels[0]=77;CHECK(style_uwp_bitmap_ensure(&bitmap,first) && bitmap.dc==original && bitmap.bitmap==image && bitmap.pixels[0]==77);
    CHECK(!style_uwp_bitmap_ensure(&bitmap,bad) && GetLastError()==ERROR_INVALID_PARAMETER && bitmap.dc==original);
    CHECK(style_uwp_bitmap_ensure(&bitmap,second) && bitmap.pixels && bitmap.size.cx==120 && bitmap.size.cy==40);
    style_uwp_bitmap_dispose(&bitmap);style_uwp_bitmap_dispose(&bitmap);CHECK(!bitmap.dc && !bitmap.bitmap && !bitmap.pixels);
    HWND root=CreateWindowW(L"STATIC",L"Surface fixture",WS_OVERLAPPEDWINDOW|WS_VISIBLE,20,20,420,280,NULL,NULL,GetModuleHandleW(NULL),NULL);
    CHECK(root);ShowWindow(root,SW_SHOWNOACTIVATE);RECT client;CHECK(GetClientRect(root,&client));
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
    puts("UWP SURFACE PASS bitmap=1 reuse=1 resize=1 invalid=1 bounds=1 hidden=1 maximize=1");return 0;
}
