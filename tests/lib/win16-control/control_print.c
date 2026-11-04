#include <windows.h>
#include <stdio.h>
#include "lib-win32/win16-common/present.h"

static HWND root,child,owned;
static BOOL detached,detach_on_erase;
static char events[16];
static unsigned count;
static void record(char event){if(count<sizeof(events))events[count]=event;++count;}

static LRESULT CALLBACK window_proc(HWND w,UINT message,WPARAM wp,LPARAM lp)
{
    if(w==root && message==WM_ERASEBKGND){record('E');if(detach_on_erase)detached=TRUE;return 1;}
    if(w==root && message==WM_PRINTCLIENT){record('C');return 0;}
    if(message==WM_PRINT && w==child){record('H');return 0;}
    if(message==WM_PRINT && w==owned){record('O');return 0;}
    return DefWindowProcW(w,message,wp,lp);
}

int main(void)
{
    HINSTANCE instance=GetModuleHandleW(NULL);
    WNDCLASSW cls={.hInstance=instance,.lpfnWndProc=window_proc,.lpszClassName=L"Win16ControlPrintFixture"};
    if(!RegisterClassW(&cls))return 1;
    root=CreateWindowW(cls.lpszClassName,L"",WS_OVERLAPPEDWINDOW,40,40,240,180,NULL,NULL,instance,NULL);
    child=CreateWindowW(cls.lpszClassName,L"",WS_CHILD|WS_VISIBLE,10,10,60,30,root,NULL,instance,NULL);
    owned=CreateWindowW(cls.lpszClassName,L"",WS_POPUP,90,90,40,30,root,NULL,instance,NULL);
    if(!root || !child || !owned)return 2;
    ShowWindow(root,SW_SHOWNOACTIVATE);ShowWindow(owned,SW_SHOWNOACTIVATE);
    HDC screen=GetDC(NULL),dc=CreateCompatibleDC(NULL);
    HBITMAP bitmap=dc && screen?CreateCompatibleBitmap(screen,240,180):NULL;
    if(screen)ReleaseDC(NULL,screen);
    HGDIOBJ old=bitmap?SelectObject(dc,bitmap):NULL;if(!dc || !bitmap || !old || old==HGDI_ERROR)return 3;
    count=0;
    win16_control_print_client_and_children(root,&detached,dc,PRF_NONCLIENT|PRF_CLIENT|PRF_CHILDREN|PRF_OWNED|PRF_ERASEBKGND);
    if(count!=4 || events[0]!='E' || events[1]!='C' || events[2]!='H' || events[3]!='O'){
        fprintf(stderr,"PRINT_ORDER_FAIL count=%u events=%.*s\n",count,(int)(count<sizeof(events)?count:sizeof(events)),events);return 4;}
    count=0;detach_on_erase=TRUE;
    win16_control_print_client_and_children(root,&detached,dc,PRF_CLIENT|PRF_CHILDREN|PRF_OWNED|PRF_ERASEBKGND);
    if(count!=1 || events[0]!='E' || !detached)return 5;
    SelectObject(dc,old);DeleteObject(bitmap);DeleteDC(dc);
    DestroyWindow(owned);DestroyWindow(root);UnregisterClassW(cls.lpszClassName,instance);
    puts("WIN16_CONTROL_PRINT_OK order=1 detach=1 child=1 owned=1");return 0;
}
