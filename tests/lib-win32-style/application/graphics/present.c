#include <windows.h>
#include <stdio.h>
#include "lib-win32-style/application/graphics/present.h"

typedef struct paint_state {unsigned calls;style_instance *instance;BOOL retire;} paint_state;
static void paint(HWND window,void *context,HDC dc)
{
    RECT r;paint_state *state=context;
    ++state->calls;GetClientRect(window,&r);
    SetDCBrushColor(dc,RGB(11,37,79));FillRect(dc,&r,(HBRUSH)GetStockObject(DC_BRUSH));
    if(state->retire)state->instance->detached=TRUE;
}
int main(void)
{
    HWND w=CreateWindowW(L"STATIC",L"",WS_POPUP,0,0,80,50,NULL,NULL,GetModuleHandleW(NULL),NULL);
    HDC screen=GetDC(NULL),dc=CreateCompatibleDC(screen);
    HBITMAP bitmap=screen?CreateCompatibleBitmap(screen,80,50):NULL;
    HGDIOBJ old=bitmap&&dc?SelectObject(dc,bitmap):NULL;
    if(screen)ReleaseDC(NULL,screen);
    if(!w || !dc || !bitmap || !old || old==HGDI_ERROR)return 1;
    style_instance instance={0};paint_state state={.instance=&instance};
    style_present_ops ops={paint,NULL};
    style_present(w,&instance,&state,RGB(0,0,0),&ops,dc);
    BOOL rendered=state.calls==1 && GetPixel(dc,20,20)==RGB(11,37,79);
    instance.detached=TRUE;style_present(w,&instance,&state,RGB(0,0,0),&ops,dc);
    BOOL unchanged=state.calls==1 && GetPixel(dc,20,20)==RGB(11,37,79);
    instance.detached=FALSE;state.retire=TRUE;
    SetPixel(dc,20,20,RGB(41,51,61));
    style_present(w,&instance,&state,RGB(0,0,0),&ops,dc);
    BOOL retired=instance.detached && state.calls==2 && GetPixel(dc,20,20)==RGB(41,51,61);
    RECT bounds={10,7,40,30};style_present_buffer buffer;
    instance.detached=FALSE;state.retire=FALSE;SetPixel(dc,5,5,RGB(81,91,101));
    style_present_buffered(w,&instance,&state,paint,dc,&bounds);
    BOOL bounded=state.calls==3 && GetPixel(dc,20,20)==RGB(11,37,79) && GetPixel(dc,5,5)==RGB(81,91,101);
    SetPixel(dc,20,20,RGB(41,51,61));state.retire=TRUE;
    style_present_buffered(w,&instance,&state,paint,dc,&bounds);
    bounded=bounded && instance.detached && state.calls==4 && GetPixel(dc,20,20)==RGB(41,51,61);
    SetPixel(dc,5,5,RGB(11,37,79));
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
    if(!rendered || !unchanged || !retired || !bounded || !blank || !preserved)return 2;
    puts("STYLE PRESENT PASS buffered=1 detached=1 retire_before_commit=1 bounded=1");return 0;
}
