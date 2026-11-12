#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "lib-style/uwp/surface.h"
static int mode,outer_deleted,inner_deleted,updated,positioned;
static int hidden,painted,reentered;
static HWND owner=(HWND)(uintptr_t)1,surface=(HWND)(uintptr_t)2,above=(HWND)(uintptr_t)3;
static HRGN WINAPI fake_outer(int a,int b,int c,int d){(void)a;(void)b;(void)c;(void)d;return (HRGN)(uintptr_t)4;}
static HRGN WINAPI fake_inner(const RECT *r){(void)r;return (HRGN)(uintptr_t)5;}
static int WINAPI fake_combine(HRGN a,HRGN b,HRGN c,int op){(void)a;(void)b;(void)c;(void)op;return mode==1?ERROR:SIMPLEREGION;}
static BOOL WINAPI fake_delete(HGDIOBJ object){if(object==(HGDIOBJ)(uintptr_t)4)++outer_deleted;else if(object==(HGDIOBJ)(uintptr_t)5)++inner_deleted;return TRUE;}
static int WINAPI fake_region(HWND w,HRGN r,BOOL redraw){(void)w;(void)r;(void)redraw;return mode==2?0:1;}
static BOOL WINAPI fake_update(HWND w,HDC dest,const POINT *at,const SIZE *size,HDC src,const POINT *origin,COLORREF key,const BLENDFUNCTION *blend,DWORD flags)
{(void)w;(void)dest;(void)at;(void)size;(void)src;(void)origin;(void)key;(void)blend;(void)flags;++updated;return mode!=3;}
static HWND WINAPI fake_getwindow(HWND w,UINT command){(void)command;return w==owner?surface:above;}
static BOOL WINAPI fake_position(HWND w,HWND insert,int x,int y,int cx,int cy,UINT flags)
{(void)x;(void)y;(void)cx;(void)cy;(void)flags;++positioned;return w==surface && insert==above && mode!=4;}
static BOOL WINAPI fake_flush(void){return TRUE;}
static BOOL WINAPI fake_iconic(HWND w){(void)w;return FALSE;}
static BOOL WINAPI fake_visible(HWND w){(void)w;return TRUE;}
static BOOL WINAPI fake_show(HWND w,int command){(void)w;if(command==SW_HIDE)++hidden;return TRUE;}
static BOOL fake_bitmap(style_bitmap *bitmap,HDC dc,SIZE size)
{(void)dc;return bitmap->pixels && bitmap->size.cx==size.cx && bitmap->size.cy==size.cy;}
#define CreateRectRgn fake_outer
#define CreateRectRgnIndirect fake_inner
#define CombineRgn fake_combine
#define DeleteObject fake_delete
#define SetWindowRgn fake_region
#define UpdateLayeredWindow fake_update
#define GetWindow fake_getwindow
#define SetWindowPos fake_position
#define GdiFlush fake_flush
#define IsIconic fake_iconic
#define IsWindowVisible fake_visible
#define ShowWindow fake_show
#define style_bitmap_ensure fake_bitmap
#include "lib-style/uwp/surface.c"
#undef CreateRectRgn
#undef CreateRectRgnIndirect
#undef CombineRgn
#undef DeleteObject
#undef SetWindowRgn
#undef UpdateLayeredWindow
#undef GetWindow
#undef SetWindowPos
#undef GdiFlush
#undef IsIconic
#undef IsWindowVisible
#undef ShowWindow
#undef style_bitmap_ensure
static BOOL geometry(void *context,RECT *outer,RECT *hole)
{(void)context;*outer=(RECT){10,20,16,24};*hole=(RECT){2,1,4,3};return TRUE;}
static void paint(void *context,HDC dc)
{(void)dc;++painted;if(style_uwp_surface_refresh(context))++reentered;}
int main(void)
{
    BYTE pixels[6*4*4];RECT outer={10,20,16,24},hole={2,1,4,3};BOOL detached=FALSE;
    style_bitmap bitmap={0};bitmap.dc=(HDC)(uintptr_t)6;bitmap.pixels=pixels;bitmap.size=(SIZE){6,4};
    for(mode=0;mode<=4;++mode){
        memset(pixels,0,sizeof(pixels));outer_deleted=inner_deleted=updated=positioned=0;
        BOOL ok=style_uwp_surface_present(owner,surface,&outer,&hole,&bitmap,&detached);
        if(ok!=(mode==0) || inner_deleted!=1 || outer_deleted!=(mode==1 || mode==2) ||
           updated!=(mode==0 || mode>=3) || positioned!=(mode==0 || mode==4))return 1;
        for(int y=0;y<4;++y)for(int x=0;x<6;++x){BYTE expected=(x>=2 && x<4 && y>=1 && y<3)?0:255;
            if(pixels[(y*6+x)*4+3]!=expected)return 2;}
    }
    detached=TRUE;outer_deleted=inner_deleted=updated=positioned=0;
    if(style_uwp_surface_present(owner,surface,&outer,&hole,&bitmap,&detached) ||
       outer_deleted || inner_deleted || updated || positioned)return 3;
    BOOL updating=FALSE;detached=FALSE;style_uwp_surface_view view={owner,&surface,&detached,&updating,&bitmap,NULL,geometry,paint};
    view.context=&view;mode=0;painted=reentered=hidden=updated=positioned=0;
    if(!style_uwp_surface_refresh(&view) || updating || painted!=1 || reentered!=1 || updated!=1 || positioned!=1 || hidden)return 4;
    mode=3;painted=reentered=hidden=updated=positioned=0;
    if(style_uwp_surface_refresh(&view) || updating || painted!=1 || reentered!=1 || updated!=1 || positioned || hidden!=1)return 5;
    puts("UWP PRESENT PASS alpha=1 region=1 ownership=1 update=1 position=1 detached=1 refresh=1 reentry=1 failure=1");return 0;
}
