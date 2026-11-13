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
    RECT bounds={10,7,40,30};style_present_buffer buffer;
    BOOL blank=style_present_buffer_begin(&buffer,dc,&bounds,FALSE);
    if(blank){SetDCBrushColor(buffer.memory,RGB(20,60,100));
        BOOL filled=FillRect(buffer.memory,&bounds,(HBRUSH)GetStockObject(DC_BRUSH))!=0;
        BOOL committed=style_present_buffer_end(&buffer,filled);blank=filled && committed;}
    else (void)style_present_buffer_end(&buffer,FALSE);
    blank=blank && GetPixel(dc,15,10)==RGB(20,60,100) && GetPixel(dc,5,5)==RGB(11,37,79);
    BOOL preserved=style_present_buffer_begin(&buffer,dc,&bounds,TRUE);
    if(preserved){BOOL pixel=GetPixel(buffer.memory,15,10)==RGB(20,60,100);
        BOOL committed=style_present_buffer_end(&buffer,TRUE);preserved=pixel && committed;}
    else (void)style_present_buffer_end(&buffer,FALSE);
    SelectObject(dc,old);DeleteObject(bitmap);DeleteDC(dc);DestroyWindow(w);
    if(!rendered || !unchanged || !blank || !preserved)return 2;
    puts("STYLE PRESENT PASS buffered=1 detached=1");return 0;
}
