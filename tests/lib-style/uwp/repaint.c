#include "lib-style/uwp/repaint.h"
#include <stdio.h>
static style_uwp_repaint state;static BOOL detached;static int refs,paints,activations;
static void retain(void *context){if(context==&state)++refs;}
static void release(void *context){if(context==&state)--refs;}
static void activate(HWND owner,void *context){if(owner && context==&state)++activations;}
static LRESULT CALLBACK proc(HWND w,UINT m,WPARAM a,LPARAM b)
{if(m==WM_NCPAINT){++paints;return 0;}return DefWindowProcW(w,m,a,b);}
static BOOL pump_until(const int *value,int wanted,DWORD timeout)
{
    ULONGLONG until=GetTickCount64()+timeout;MSG message;
    while(*value<wanted && GetTickCount64()<until){
        while(PeekMessageW(&message,NULL,0,0,PM_REMOVE)){TranslateMessage(&message);DispatchMessageW(&message);}
        if(*value<wanted)MsgWaitForMultipleObjects(0,NULL,FALSE,20,QS_ALLINPUT);
    }
    return *value==wanted;
}
int main(void)
{
    HINSTANCE instance=GetModuleHandleW(NULL);WNDCLASSW wc={.lpfnWndProc=proc,.hInstance=instance,.lpszClassName=L"StyleUwpRepaintTest"};
    if(!RegisterClassW(&wc))return 1;
    HWND w=CreateWindowW(wc.lpszClassName,L"",WS_POPUP,0,0,30,30,NULL,NULL,instance,NULL);if(!w)return 2;
    paints=0;
    const style_uwp_repaint_services services={retain,release,activate};
    if(!style_uwp_repaint_queue(w,&state,&detached,&state,&services) ||
       !style_uwp_repaint_queue(w,&state,&detached,&state,&services) || !state.pending)return 3;
    if(!pump_until(&paints,1,2000) || refs || state.pending || state.repainting)return 4;
    if(!style_uwp_repaint_queue(w,&state,&detached,&state,&services))return 5;
    style_uwp_repaint_cancel(&state);if(state.timer || state.pending || state.waiting)return 6;
    ULONGLONG start=GetTickCount64();state.waiting=TRUE;state.deadline=start+70;
    if(!style_uwp_repaint_queue(w,&state,&detached,&state,&services) || !pump_until(&activations,1,2000) ||
       GetTickCount64()-start<70 || refs || state.waiting)return 7;
    style_uwp_repaint_cancel(&state);detached=TRUE;
    if(style_uwp_repaint_queue(w,&state,&detached,&state,&services))return 8;
    DestroyWindow(w);UnregisterClassW(wc.lpszClassName,instance);puts("UWP_REPAINT_PASS");return 0;
}
