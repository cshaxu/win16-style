#include <windows.h>
#include <stdio.h>
#include "lib-style/common/present.h"

static void paint(HWND window,void *context,HDC dc)
{
    RECT r;unsigned *calls=context;
    ++*calls;GetClientRect(window,&r);
    SetDCBrushColor(dc,RGB(11,37,79));FillRect(dc,&r,(HBRUSH)GetStockObject(DC_BRUSH));
}
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
    style_present(w,&view,dc);
    BOOL rendered=calls==1 && GetPixel(dc,20,20)==RGB(11,37,79);
    detached=TRUE;style_present(w,&view,dc);
    BOOL unchanged=calls==1 && GetPixel(dc,20,20)==RGB(11,37,79);
    SelectObject(dc,old);DeleteObject(bitmap);DeleteDC(dc);DestroyWindow(w);
    if(!rendered || !unchanged)return 2;
    puts("STYLE PRESENT PASS buffered=1 detached=1");return 0;
}
