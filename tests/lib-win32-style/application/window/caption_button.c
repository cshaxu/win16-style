#include "lib-win32-style/application/window/caption_button.h"
#include <stdio.h>

static style_caption_button state;
static BOOL releasing,stale_capture;
static LRESULT CALLBACK procedure(HWND window,UINT message,WPARAM wp,LPARAM lp)
{
    if(message==WM_CAPTURECHANGED && releasing && state.pressed)stale_capture=TRUE;
    return DefWindowProcW(window,message,wp,lp);
}
int main(void)
{
    HINSTANCE module=GetModuleHandleW(NULL);
    WNDCLASSW cls={.hInstance=module,.lpfnWndProc=procedure,.lpszClassName=L"CaptionButtonStateTest"};
    if(!RegisterClassW(&cls))return 1;
    HWND window=CreateWindowW(cls.lpszClassName,L"Caption button",WS_OVERLAPPEDWINDOW,
        20,20,200,100,NULL,NULL,module,NULL);
    HWND other=CreateWindowW(cls.lpszClassName,L"Other owned window",WS_POPUP,
        20,20,100,60,NULL,NULL,module,NULL);
    int result=0;
    if(!window || !other){result=2;goto done;}
    if(!style_caption_button_begin(window,&state,41,TRUE) || state.pressed!=41 || !state.hot || GetCapture()!=window){result=3;goto done;}
    if(style_caption_button_move(window,&state,FALSE,FALSE,FALSE) || state.hot || state.pressed!=41){result=4;goto done;}
    releasing=TRUE;
    if(!style_caption_button_move(window,&state,TRUE,TRUE,FALSE) || state.pressed || GetCapture()==window || stale_capture){result=5;goto done;}
    releasing=FALSE;
    if(!style_caption_button_begin(window,&state,99,TRUE)){result=6;goto done;}
    SetCapture(other);
    if(style_caption_button_cancel(window,&state,WM_CAPTURECHANGED)!=99 || state.pressed || state.hot || GetCapture()!=other){result=7;goto done;}
    state=(style_caption_button){.pressed=7,.hot=TRUE};
    if(style_caption_button_cancel(window,&state,WM_CANCELMODE)!=7 || GetCapture()!=other){result=8;goto done;}
    ReleaseCapture();
    if(!style_caption_button_begin(window,&state,12,FALSE)){result=9;goto done;}
    releasing=TRUE;
    if(style_caption_button_cancel(window,&state,WM_CANCELMODE)!=12 || state.pressed || state.hot || GetCapture()==window || stale_capture){result=10;goto done;}
    releasing=FALSE;
    if(!style_caption_button_begin(window,&state,27,FALSE)){result=11;goto done;}
    releasing=TRUE;
    if(style_caption_button_move(window,&state,FALSE,TRUE,TRUE) || state.pressed || GetCapture()==window || stale_capture){result=12;goto done;}
    releasing=FALSE;
    HWND destroyed=other;DestroyWindow(other);other=NULL;
    if(style_caption_button_begin(destroyed,&state,33,TRUE) || state.pressed || state.hot){result=13;goto done;}
done:
    if(GetCapture()==window || GetCapture()==other)ReleaseCapture();
    if(other)DestroyWindow(other);
    if(window)DestroyWindow(window);
    UnregisterClassW(cls.lpszClassName,module);
    printf("CAPTION BUTTON STATE bits=%u result=%d stale_capture=%d\n",(unsigned)(sizeof(void *)*8),result,stale_capture);
    return result;
}
