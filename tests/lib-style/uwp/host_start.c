#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <stdint.h>
#include "lib-style/uwp/host.h"
#define CHECK(x) do{if(!(x)){fprintf(stderr,"UWP HOST START FAIL line=%d error=%lu\n",__LINE__,(unsigned long)GetLastError());return 1;}}while(0)
static HWND owner=(HWND)(uintptr_t)1,title=(HWND)(uintptr_t)2,surface=(HWND)(uintptr_t)3;
static style_uwp_host *current;
static int mode,installed,removed,destroyed,refreshed,before,after,disposed;
static BOOL WINAPI fake_install(HWND w,SUBCLASSPROC p,UINT_PTR id,DWORD_PTR d)
{
    (void)p;(void)id;(void)d;++installed;
    if((mode==1 && w==title) || (mode==3 && w==surface) || (mode==7 && w==owner)){SetLastError(1234);return FALSE;}
    return TRUE;
}
static BOOL WINAPI fake_remove(HWND w,SUBCLASSPROC p,UINT_PTR id)
{(void)w;(void)p;(void)id;++removed;return TRUE;}
static HWND WINAPI fake_create(DWORD ex,LPCWSTR cls,LPCWSTR text,DWORD style,int x,int y,int width,int height,HWND parent,HMENU menu,HINSTANCE instance,LPVOID parameter)
{
    (void)ex;(void)cls;(void)text;(void)style;(void)x;(void)y;(void)width;(void)height;(void)parent;(void)menu;(void)instance;(void)parameter;
    if(mode==2){SetLastError(2345);return NULL;}
    if(mode==5){style_uwp_host_detach(current);SetLastError(3456);}
    return surface;
}
static BOOL WINAPI fake_destroy(HWND w){if(w==surface)++destroyed;return TRUE;}
static HWND WINAPI fake_capture(void){return NULL;}
static BOOL WINAPI fake_release(void){return TRUE;}
static LRESULT WINAPI fake_default(HWND w,UINT m,WPARAM a,LPARAM b)
{(void)w;(void)m;(void)a;(void)b;return 0;}
#define SetWindowSubclass fake_install
#define RemoveWindowSubclass fake_remove
#define CreateWindowExW fake_create
#define DestroyWindow fake_destroy
#define GetCapture fake_capture
#define ReleaseCapture fake_release
#define DefSubclassProc fake_default
#include "lib-style/uwp/host.c"
#undef SetWindowSubclass
#undef RemoveWindowSubclass
#undef CreateWindowExW
#undef DestroyWindow
#undef GetCapture
#undef ReleaseCapture
#undef DefSubclassProc
static void prior(void *p){(void)p;++before;}
static void following(void *p){(void)p;++after;}
static void dispose(void *p){(void)p;++disposed;}
static BOOL refresh(void *p)
{
    (void)p;++refreshed;
    if(mode==4){SetLastError(4567);return FALSE;}
    if(mode==6){style_uwp_host_detach(current);SetLastError(5678);return FALSE;}
    return TRUE;
}
int main(void)
{
    for(mode=0;mode<=7;++mode){
        style_uwp_host host={.owner=owner,.title=title,
            .before_title_remove=prior,.after_title_remove=following,.dispose=dispose,.calls=1};
        current=&host;installed=removed=destroyed=refreshed=before=after=disposed=0;SetLastError(0);
        BOOL result=style_uwp_host_start(&host,refresh);DWORD error=GetLastError();
        if(mode==0){CHECK(result && !error && host.surface==surface && refreshed==1 && !host.detached);
            CHECK(style_uwp_host_detach(&host));}
        else if(mode==7){
            CHECK(!result && error==1234 && host.detached && installed==1 && !removed && !destroyed && !before && !after && !refreshed);
            style_uwp_host_release(&host);CHECK(disposed==1);
            continue;
        }else {
            CHECK(!result && host.detached && before==1 && after==1);
            if(mode==1)CHECK(error==1234 && installed==2 && destroyed==0 && !refreshed);
            if(mode==2)CHECK(error==2345 && installed==2 && destroyed==0 && !refreshed);
            if(mode==3)CHECK(error==1234 && installed==3 && destroyed==1 && !refreshed);
            if(mode==4)CHECK(error==4567 && installed==3 && destroyed==1 && refreshed==1);
            if(mode==5)CHECK(error==3456 && installed==2 && destroyed==1 && !refreshed && !host.surface);
            if(mode==6)CHECK(error==5678 && installed==3 && destroyed==1 && refreshed==1 && !host.surface);
        }
        CHECK(before==1 && after==1 && host.detached && !host.surface);
    }
    puts("UWP HOST START PASS success=1 owner=1 title=1 create=1 subclass=1 refresh=1 reentrant_create=1 reentrant_refresh=1");return 0;
}
