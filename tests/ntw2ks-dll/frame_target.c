#include <windows.h>
#include <stdio.h>

static LRESULT CALLBACK procedure(HWND w,UINT message,WPARAM wp,LPARAM lp)
{return DefWindowProcW(w,message,wp,lp);}

int wmain(void)
{
    HINSTANCE instance=GetModuleHandleW(NULL);
    WNDCLASSW wc={.lpfnWndProc=procedure,.hInstance=instance,.lpszClassName=L"Win2kFrameFixture"};
    if(!RegisterClassW(&wc))return 1;
    HWND w=CreateWindowExW(0,wc.lpszClassName,L"Classic frame",WS_OVERLAPPEDWINDOW,
                            80,80,420,260,NULL,NULL,instance,NULL);
    if(!w)return 2;
    BOOL attached=GetPropW(w,L"Win2kStyle.Frame")!=NULL;
    ShowWindow(w,SW_SHOW);UpdateWindow(w);
    RedrawWindow(w,NULL,NULL,RDW_FRAME|RDW_INVALIDATE|RDW_UPDATENOW);
    TITLEBARINFOEX info={.cbSize=sizeof(info)};
    SendMessageW(w,WM_GETTITLEBARINFOEX,0,(LPARAM)&info);
    RECT window;GetWindowRect(w,&window);
    HDC dc=GetWindowDC(w);
    int x=info.rcTitleBar.left-window.left+40;
    int y=info.rcTitleBar.top-window.top+2;
    COLORREF pixel=dc?GetPixel(dc,x,y):CLR_INVALID;
    COLORREF pixel2=dc?GetPixel(dc,x+80,y):CLR_INVALID;
    if(dc)ReleaseDC(w,dc);
    LRESULT hit=SendMessageW(w,WM_NCHITTEST,0,MAKELPARAM(info.rcTitleBar.left+50,(info.rcTitleBar.top+info.rcTitleBar.bottom)/2));
    SendMessageW(w,WM_SYSCOMMAND,SC_MAXIMIZE,0);
    BOOL maximized=IsZoomed(w);
    SendMessageW(w,WM_SYSCOMMAND,SC_RESTORE,0);
    BOOL restored=!IsZoomed(w) && GetPropW(w,L"Win2kStyle.Frame")!=NULL;
    printf("WIN2K FRAME bits=%u attached=%d title=%ldx%ld pixels=%06lx/%06lx hit=%lld\n",
        (unsigned)(sizeof(void*)*8),attached,info.rcTitleBar.right-info.rcTitleBar.left,
        info.rcTitleBar.bottom-info.rcTitleBar.top,(unsigned long)pixel,(unsigned long)pixel2,(long long)hit);
    SendMessageW(w,WM_SYSCOMMAND,SC_CLOSE,0);
    BOOL closed=!IsWindow(w);if(!closed)DestroyWindow(w);
    return attached && pixel!=CLR_INVALID && pixel2!=CLR_INVALID && pixel!=pixel2 &&
           hit==HTCAPTION && maximized && restored && closed?0:3;
}
