#include <windows.h>
#include <stdio.h>
#include "lib-style/common/present.h"

static BOOL fail_memory,fail_save;
static HDC WINAPI test_create_dc(HDC dc){return fail_memory?NULL:CreateCompatibleDC(dc);}
static int WINAPI test_save_dc(HDC dc){return fail_save?0:SaveDC(dc);}
#define CreateCompatibleDC test_create_dc
#define SaveDC test_save_dc
#include "lib-style/common/present.c"
#undef CreateCompatibleDC
#undef SaveDC
static void paint(HWND window,void *context,HDC dc)
{
    RECT r;unsigned *calls=context;++*calls;GetClientRect(window,&r);
    SetDCBrushColor(dc,RGB(71,29,5));FillRect(dc,&r,(HBRUSH)GetStockObject(DC_BRUSH));
}
static BOOL group(HWND window,void *context){(void)window;(void)context;return TRUE;}
int main(void)
{
    HWND w=CreateWindowW(L"STATIC",L"",WS_POPUP,0,0,80,50,NULL,NULL,GetModuleHandleW(NULL),NULL);
    HDC screen=GetDC(NULL),dc=CreateCompatibleDC(screen);
    HBITMAP bitmap=screen?CreateCompatibleBitmap(screen,80,50):NULL;
    HGDIOBJ old=bitmap&&dc?SelectObject(dc,bitmap):NULL;
    if(screen)ReleaseDC(NULL,screen);
    if(!w || !dc || !bitmap || !old || old==HGDI_ERROR)return 1;
    unsigned calls=0;BOOL detached=FALSE;
    style_present_view view={&detached,RGB(0,0,0),&calls,paint,NULL,NULL};
    fail_memory=TRUE;style_present(w,&view,dc);
    BOOL fallback=calls==1 && GetPixel(dc,20,20)==RGB(71,29,5);
    fail_memory=FALSE;fail_save=TRUE;view.is_group=group;style_present(w,&view,dc);
    BOOL group_safe=calls==1 && GetPixel(dc,20,20)==RGB(71,29,5);
    SelectObject(dc,old);DeleteObject(bitmap);DeleteDC(dc);DestroyWindow(w);
    if(!fallback || !group_safe)return 2;
    puts("STYLE PRESENT FAULT PASS fallback=1 group_save=1");return 0;
}
